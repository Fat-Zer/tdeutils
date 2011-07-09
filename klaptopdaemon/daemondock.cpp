/*
 * daemondock.cpp
 * Copyright (C) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (C) 2006 Flavio Castelli <flavio.castelli@gmail.com>
 *
 * This file contains the docked widget for the laptop battery monitor
 *
 * $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <tqcursor.h>

#include <tqslider.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <dcopclient.h>
#include <dcopref.h>
#include "kpcmciainfo.h"
#include "daemondock.h"
#include "portable.h"
#include <kiconloader.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kconfig.h>
#include <kglobalsettings.h>
#include <kmessagebox.h>
#include <tqvbox.h>
#include <tqstringlist.h>
#include <tqbitmap.h>
#include <tqpainter.h>
#include <kiconeffect.h>
extern void wake_laptop_daemon();

laptop_dock::laptop_dock( laptop_daemon* tqparent )
  : KSystemTray()
{
    setCaption(i18n("KLaptop Daemon"));
    KGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages
    _pcmcia = NULL;
    pdaemon = tqparent;
    current_code = -1;
    brightness_widget = 0;
    instance = new KInstance("klaptopdaemon");
      // popup menu for right mouse button
    rightPopup = contextMenu();
    SetupPopup();
}

void
laptop_dock::SetupPopup()
{
    rightPopup->clear();
    rightPopup->insertTitle(SmallIcon("laptop_battery"), "KLaptop", 999);	// otherwise we look like KDED

    int has_brightness = laptop_portable::has_brightness();
    int can_standby = laptop_portable::has_standby();
    int can_suspend = laptop_portable::has_suspend();
    int can_hibernate = laptop_portable::has_hibernation();
    TQStringList profile_list;
    int current_profile;
    bool *active_list;
    bool has_performance = laptop_portable::get_system_performance(0, current_profile, profile_list, active_list);
    TQStringList throttle_list;
    int current_throttle;
    bool has_throttle = laptop_portable::get_system_throttling(0, current_throttle, throttle_list, active_list);
    rightPopup->insertItem(SmallIcon("configure"), i18n("&Configure KLaptop..."), this, TQT_SLOT(invokeSetup()));

    if (has_brightness)
	rightPopup->insertItem(i18n("Screen Brightness..."), this, TQT_SLOT(invokeBrightness()));
    if (has_performance) {
	performance_popup = new TQPopupMenu(0, "performance");
	performance_popup->setCheckable(1);
	rightPopup->insertItem(i18n("Performance Profile..."), performance_popup);
	connect( performance_popup, TQT_SIGNAL( activated( int ) ), this, TQT_SLOT( activate_performance( int ) ) );
	connect( performance_popup, TQT_SIGNAL( aboutToShow() ),    this, TQT_SLOT( fill_performance() ) );
    } else {
	performance_popup = 0;
    }
    if (has_throttle) {
	throttle_popup = new TQPopupMenu(0, "throttle");
	throttle_popup->setCheckable(1);
	rightPopup->insertItem(i18n("CPU Throttling..."), throttle_popup);
	connect( throttle_popup, TQT_SIGNAL( activated( int ) ), this, TQT_SLOT( activate_throttle( int ) ) );
	connect( throttle_popup, TQT_SIGNAL( aboutToShow() ),    this, TQT_SLOT( fill_throttle() ) );
    } else {
	throttle_popup = 0;
    }

    if (can_standby || can_suspend || can_hibernate) {
    	rightPopup->insertSeparator();
    	if (can_standby) rightPopup->insertItem(i18n("Standby..."), this, TQT_SLOT(invokeStandby()));
	//	if (can_suspend) rightPopup->insertItem(i18n("&Lock && Suspend..."), this, TQT_SLOT(invokeLockSuspend()));
    	if (can_suspend) rightPopup->insertItem(i18n("&Suspend..."), this, TQT_SLOT(invokeLockSuspend()));
	//        if (can_hibernate) rightPopup->insertItem(i18n("&Lock && Hibernate..."), this, TQT_SLOT(invokeLockHibernation()));
	if (can_hibernate) rightPopup->insertItem(i18n("&Hibernate..."), this, TQT_SLOT(invokeLockHibernation()));
    }

    rightPopup->insertSeparator();
    rightPopup->insertItem(i18n("&Hide Monitor"), this, TQT_SLOT(slotHide()));
    rightPopup->insertItem(SmallIcon("exit"), KStdGuiItem::quit().text(), this, TQT_SLOT(slotQuit()));
}

laptop_dock::~laptop_dock()
{
	delete instance;
	if (brightness_widget)
		delete brightness_widget;
	if (throttle_popup)
		delete throttle_popup;
	if (performance_popup)
		delete performance_popup;
}

void
laptop_dock::activate_throttle(int ind)
{
	pdaemon->SetThrottle(throttle_popup->text(ind));
}

void
laptop_dock::fill_throttle()
{
    throttle_popup->clear();
    int current;
    TQStringList list;
    bool *active_list;
    bool has_throttle = laptop_portable::get_system_throttling(1, current, list, active_list);
    if (!has_throttle && !list.empty())
	    return;
    int n=0;
    for (TQValueListIterator<TQString> i = list.begin();i != list.end();i++) {
	throttle_popup->insertItem(*i, n);
    	throttle_popup->setItemEnabled(n, active_list[n]);
    	n++;
    }
    throttle_popup->setItemChecked(current, 1);
}

void
laptop_dock::activate_performance(int ind)
{
	pdaemon->SetPerformance(performance_popup->text(ind));
}

void
laptop_dock::fill_performance()
{
    performance_popup->clear();
    int current;
    TQStringList list;
    bool *active_list;
    bool has_performance = laptop_portable::get_system_performance(1, current, list, active_list);
    if (!has_performance && !list.empty())
	    return;
    int n=0;
    for (TQValueListIterator<TQString> i = list.begin();i != list.end();i++) {
	performance_popup->insertItem(*i, n);
    	performance_popup->setItemEnabled(n, active_list[n]);
	n++;
    }
    performance_popup->setItemChecked(current, 1);
}

void
laptop_dock::invokeBrightnessSlider(int v)
{
	v=255-v;
	pdaemon->SetBrightness(0, v);
	brightness = v;
}

void
laptop_dock::invokeBrightness()
{
	brightness = pdaemon->GetBrightness();

	if (brightness < 0)
		brightness = 255;

	if (brightness_widget == 0) {
		brightness_widget = new TQVBox(0L, "Brightness", WStyle_Customize | WType_Popup);
		brightness_widget->setFrameStyle(TQFrame::PopupPanel);
		brightness_widget->setMargin(KDialog::marginHint());
		brightness_slider = new TQSlider(0, 255, 16, 255-brightness, Qt::Vertical, brightness_widget, 0);
		brightness_slider->setMinimumHeight(40);
		brightness_slider->setMinimumWidth(15);
		connect(brightness_slider, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(invokeBrightnessSlider(int)));
		brightness_widget->resize(brightness_widget->tqsizeHint());
	} else {
		brightness_slider->setValue(255-brightness);
	}
	if (!brightness_widget->isVisible()) {
		TQRect desktop = KGlobalSettings::desktopGeometry(this);
		int sw = desktop.width();
		int sh = desktop.height();
		int sx = desktop.x();
		int sy = desktop.y();
		TQPoint pos = TQCursor::pos();
		int x = pos.x();
		int y = pos.y();
		y -= brightness_widget->tqgeometry().height();
		int w = brightness_widget->width();
		int h = brightness_widget->height();
		if (x+w > sw)
			x = pos.x()-w;
		if (y+h > sh)
			y = pos.y()-h;
		if (x < sx)
			x = pos.x();
		if (y < sy)
			y = pos.y();
		brightness_widget->move(x, y);
		brightness_widget->show();
	}
}

void laptop_dock::slotGoRoot(int /*id*/) {
#ifdef NOTDEF
	TQString kdesu = KStandardDirs::findExe("kdesu");
	if (!kdesu.isEmpty()) {
		int rc = KMessageBox::warningContinueCancel(0,
				i18n("You will need to supply a root password "
					"to allow KLaptopDaemon to restart "
					"itself as the superuser. It may take "
					"up to a minute for the new daemon to "
					"start up and the old one to close."),
				i18n("KLaptopDaemon"), KStdGuiItem::cont(),
				"switchToPCMCIAPrompt");
		if (rc == KMessageBox::Continue) {
			KProcess *_rootProcess;
			_rootProcess = new KProcess;
			*_rootProcess << kdesu;
			*_rootProcess << "-u";
			*_rootProcess << "root";
			//*_rootProcess << "--nonewdcop";
			*_rootProcess << KStandardDirs::findExe("klaptopdaemon");
			connect(_rootProcess, TQT_SIGNAL(processExited(KProcess*)),
				this, TQT_SLOT(rootExited(KProcess*)));
			_rootProcess->start(KProcess::NotifyOnExit);
			// We should disable this menu item here now.
		}
	} else {
		KMessageBox::sorry(0, i18n("PCMCIA cannot be enabled since kdesu cannot be found. Please make sure that it is installed correctly."),
				i18n("KLaptopDaemon"));
	}
#else
	KMessageBox::sorry(0, i18n("PCMCIA cannot be enabled just now."),
			i18n("KLaptopDaemon"));
#endif
}


void laptop_dock::slotHide() {
	int confirm = KMessageBox::questionYesNo(0, i18n("Are you sure you want to hide the battery monitor? Your battery will still be monitored in the background."), TQString(), i18n("Hide Monitor"), i18n("Do Not Hide"), "hideConfirm");

	if (confirm != KMessageBox::Yes)
		return;

	// just tell ourselves to hide the battery
	KConfig *config = new KConfig("kcmlaptoprc");
	if (config) {
		config->setGroup("BatteryDefault");
		config->writeEntry("Enable", false);
		config->sync();
		delete config;
	}
  	wake_laptop_daemon();
}


void laptop_dock::slotQuit() {
	int confirm = KMessageBox::questionYesNo(0, i18n("Are you sure you want to quit the battery monitor?"), TQString(), KStdGuiItem::quit(), KStdGuiItem::cancel(), "quitConfirm");

	if (confirm != KMessageBox::Yes)
		return;

	confirm = KMessageBox::questionYesNo(0, i18n("Do you wish to disable the battery monitor from starting in the future?"), TQString(), i18n("Disable"), i18n("Keep Enabled"), "restartMonitor");

	if (confirm == KMessageBox::Yes) {
		// just tell ourselves to hide the battery
		KConfig *config = new KConfig("kcmlaptoprc");
		if (config) {
			config->setGroup("BatteryDefault");
			config->writeEntry("Enable", false);
			config->sync();
			delete config;
		}
	}
	pdaemon->quit();
}


void laptop_dock::rootExited(KProcess *p) {
	if (p && p->isRunning()) {
		p->detach();
	}
	exit(0);
}


void laptop_dock::slotEjectAction(int id) {
KPCMCIACard *f = NULL;
  f = _ejectActions[id];

  if (f) f->eject();
}


void laptop_dock::slotSuspendAction(int id) {
KPCMCIACard *f = NULL;
  f = _suspendActions[id];

  if (f) f->suspend();
}


void laptop_dock::slotResumeAction(int id) {
KPCMCIACard *f = NULL;
  f = _resumeActions[id];

  if (f) f->resume();
}


void laptop_dock::slotResetAction(int id) {
KPCMCIACard *f = NULL;
  f = _resetActions[id];

  if (f) f->reset();
}


void laptop_dock::slotInsertAction(int id) {
KPCMCIACard *f = NULL;
  f = _insertActions[id];

  if (f) f->insert();
}


void laptop_dock::slotDisplayAction(int id) {
KPCMCIAInfo *f =  new KPCMCIAInfo(_pcmcia);
   f->showTab(_displayActions[id]->num());
}

void laptop_dock::mousePressEvent( TQMouseEvent *event )
{
 	if(event->button() == Qt::LeftButton) {
		TQPopupMenu *popup = new TQPopupMenu(0, "popup");

		if (!pdaemon->exists()) {
			popup->insertItem(i18n("Power Manager Not Found"));
		} else {
			TQString tmp;

			if (pdaemon->val >= 0) {
				if (pdaemon->left >= 0) {
					TQString num3;
					num3.setNum(pdaemon->left%60);
					num3 = num3.rightJustify(2, '0');
					tmp = i18n("%1:%2 hours left").tqarg(pdaemon->left/60).tqarg(num3);
				} else {
					// no remaining time available
					tmp = i18n("%1% charged").tqarg(pdaemon->val);
				}
			} else {
				tmp = i18n("No Battery");
			}
			popup->insertItem(tmp);
			popup->setItemEnabled(0, 0);
			popup->insertSeparator();
			if (pdaemon->powered && pdaemon->val >= 0) {
				popup->insertItem(i18n("Charging"));
			} else {
				popup->insertItem(i18n("Not Charging"));
			}
			popup->setItemEnabled(1, 0);
		}

		/**
		 *        CPU Frequency scaling support
		 *        - show the cpu profile and current frequency
		 */
		if (laptop_portable::has_cpufreq()) {
			TQString speed = laptop_portable::cpu_frequency();
			if (!speed.isEmpty()) {
				popup->insertSeparator();
				popup->insertItem(i18n("CPU: %1").tqarg(speed));
			}
		}

                /**
                 *        ADD the PCMCIA entries here
                 */
                if (_pcmcia && _pcmcia->haveCardServices()) {
                   TQString slotname = i18n("Slot %1"); // up here so we only construct it once
                   int id;
                   popup->insertSeparator();
                   _ejectActions.clear();
                   _resetActions.clear();
                   _insertActions.clear();
                   _suspendActions.clear();
                   _resumeActions.clear();
                   _displayActions.clear();
                   id = popup->insertItem(i18n("Card Slots..."), this, TQT_SLOT(slotDisplayAction(int)));
                   _displayActions.insert(id, _pcmcia->getCard(0));
                   for (int i = 0; i < _pcmcia->getCardCount(); i++) {
                      KPCMCIACard *thiscard;
                      thiscard = _pcmcia->getCard(i);
                      if (thiscard && (thiscard->present())) {
                         TQPopupMenu *thisSub = new TQPopupMenu(popup, thiscard->name().latin1());
                         id = thisSub->insertItem(i18n("Details..."), this, TQT_SLOT(slotDisplayAction(int)));
                         _displayActions.insert(id, thiscard);

                         // add the actions
                         TQPopupMenu *actionsSub = new TQPopupMenu(thisSub, "actions");
                         id = actionsSub->insertItem(i18n("Eject"), this, TQT_SLOT(slotEjectAction(int)));
                         actionsSub->setItemEnabled(id, !(thiscard->status() & CARD_STATUS_BUSY));
                         _ejectActions.insert(id, thiscard);
                         id = actionsSub->insertItem(i18n("Suspend"), this, TQT_SLOT(slotSuspendAction(int)));
                         actionsSub->setItemEnabled(id, !(thiscard->status() & (CARD_STATUS_SUSPEND|CARD_STATUS_BUSY)));
                         _suspendActions.insert(id, thiscard);
                         id = actionsSub->insertItem(i18n("Resume"), this, TQT_SLOT(slotResumeAction(int)));
                         actionsSub->setItemEnabled(id, (thiscard->status() & CARD_STATUS_SUSPEND));
                         _resumeActions.insert(id, thiscard);
                         id = actionsSub->insertItem(i18n("Reset"), this, TQT_SLOT(slotResetAction(int)));
                         _resetActions.insert(id, thiscard);
                         id = actionsSub->insertItem(i18n("Insert"), this, TQT_SLOT(slotInsertAction(int)));
                         _insertActions.insert(id, thiscard);
                         actionsSub->setItemEnabled(id, !(thiscard->status() & (CARD_STATUS_READY|CARD_STATUS_SUSPEND)));
                         thisSub->insertItem(i18n("Actions"), actionsSub);

                         // add a few bits of information
                         thisSub->insertSeparator();
                         thisSub->insertItem(slotname.tqarg(thiscard->num()+1));
                         if (thiscard->status() & CARD_STATUS_READY)
                            thisSub->insertItem(i18n("Ready"));
                         if (thiscard->status() & CARD_STATUS_BUSY)
                            thisSub->insertItem(i18n("Busy"));
                         if (thiscard->status() & CARD_STATUS_SUSPEND)
                            thisSub->insertItem(i18n("Suspended"));
                         popup->insertItem(thiscard->name(), thisSub);
                      }
                   }
                } else if (_pcmcia && geteuid() != 0) {
			popup->insertItem(i18n("Enable PCMCIA"));
		}

		popup->popup(TQCursor::pos());
	}

}
void laptop_dock::mouseReleaseEvent( TQMouseEvent *e )
{
    if ( !TQT_TQRECT_OBJECT(rect()).tqcontains( e->pos() ) )
        return;

    switch ( e->button() ) {
    case Qt::LeftButton:
        break;
    case Qt::MidButton:
        // fall through
    case Qt::RightButton:
	{
		KPopupMenu *menu = contextMenu();
        	contextMenuAboutToShow( menu );
        	menu->popup( e->globalPos() );
	}
        break;
    default:
        // nothing
        break;
    }
}
void laptop_dock::showEvent( TQShowEvent * )
{

}
void laptop_dock::invokeHibernation()
{
        laptop_portable::invoke_hibernation();
}

void laptop_dock::invokeLockHibernation()
{
  DCOPRef dr("kdesktop", "KScreensaverIface");
  DCOPReply reply=dr.call("lock");

  laptop_portable::invoke_hibernation();
}
void laptop_dock::invokeStandby()
{
	laptop_portable::invoke_standby();
}

void laptop_dock::invokeSuspend()
{
	laptop_portable::invoke_suspend();
}

void laptop_dock::invokeLockSuspend()
{
  DCOPClient* client = kapp->dcopClient();
  if (client)
      client->send("kdesktop", "KScreensaverIface", "lock()", TQString(""));
  laptop_portable::invoke_suspend();
}

void laptop_dock::invokeSetup()
{
	KProcess proc;
	proc << KStandardDirs::findExe("kcmshell");
	proc << "laptop";
	proc.start(KProcess::DontCare);
	proc.detach();
}

void
laptop_dock::reload_icon()
{
	// we will try to deduce the pixmap (or gif) name now.  it will
	// vary depending on the dock and power
	TQString pixmap_name;

	if (!pdaemon->exists())
		pixmap_name = pdaemon->noBatteryIcon();
	else if (!pdaemon->powered)
		pixmap_name = pdaemon->noChargeIcon();
	else
		pixmap_name = pdaemon->chargeIcon();

	pm = loadIcon( pixmap_name, instance );
}

void laptop_dock::displayPixmap()
{
	int new_code;

	if (!pdaemon->exists())
		new_code = 1;
	else if (!pdaemon->powered)
		new_code = 2;
	else
		new_code = 3;

	if (current_code != new_code) {
		current_code = new_code;
		reload_icon();
	}

	// at this point, we have the file to display.  so display it

	TQImage image = pm.convertToImage();
	const TQBitmap *bmtqmask = pm.tqmask();
	TQImage tqmask;
	if (bmtqmask)
		tqmask = bmtqmask->convertToImage();

	int w = image.width();
	int h = image.height();
	int count = 0;
	TQRgb rgb;
	int x, y;
	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++)
	if (!bmtqmask || tqmask.pixelIndex(x, y) != 0){
		rgb = image.pixel(x, y);
		if (tqRed(rgb) == 0xff &&
		    tqGreen(rgb) == 0xff &&
		    tqBlue(rgb) == 0xff)
			count++;
	}
	int c = (count*pdaemon->val)/100;
	if (pdaemon->val == 100) {
		c = count;
	} else
	if (pdaemon->val != 100 && c == count)
		c = count-1;


	if (c) {
		uint ui;
		TQRgb blue = tqRgb(0x00,0x00,0xff);

		if (image.depth() <= 8) {
			ui = image.numColors();		// this fix thanks to Sven Krumpke
			image.setNumColors(ui+1);
			image.setColor(ui, blue);
		} else {
			ui = 0xff000000|blue;
		}

		for (y = h-1; y >= 0; y--)
		for (x = 0; x < w; x++)
		if (!bmtqmask || tqmask.pixelIndex(x, y) != 0){
			rgb = image.pixel(x, y);
			if (tqRed(rgb) == 0xff &&
		    	    tqGreen(rgb) == 0xff &&
		    	    tqBlue(rgb) == 0xff) {
				image.setPixel(x, y, ui);
				c--;
				if (c <= 0)
					goto quit;
			}
		}
	}
quit:
        
        TQString tmp;
        TQString levelString;
        
	if (!pdaemon->exists()) {
		tmp = i18n("Laptop power management not available");
                levelString =  i18n("N/A");
	} else
	if (pdaemon->powered) {
		if (pdaemon->val == 100) {
			tmp = i18n("Plugged in - fully charged");;
                        levelString = "100%";
		} else if (pdaemon->val >= 0) {
                        levelString.sprintf ("%i%%", pdaemon->val);
			if (pdaemon->left >= 0) {
				TQString num3;
				num3.setNum(pdaemon->left%60);
				num3 = num3.rightJustify(2, '0');
				tmp = i18n("Plugged in - %1% charged (%2:%3 hours left)")
					.tqarg(pdaemon->val).tqarg(pdaemon->left/60).tqarg(num3);
			} else {
				// no remaining time available
				tmp = i18n("Plugged in - %1% charged").tqarg(pdaemon->val);
			}
		} else {
			tmp = i18n("Plugged in - no battery");
                        levelString = i18n("N/A");
		}
	} else {
		if (pdaemon->val >= 0) {
                        levelString.sprintf ("%i%%", pdaemon->val);
			if (pdaemon->left >= 0) {
				TQString num3;
				num3.setNum(pdaemon->left%60);
				num3 = num3.rightJustify(2, '0');
				tmp = i18n("Running on batteries - %1% charged (%2:%3 hours left)")
						.tqarg(pdaemon->val).tqarg(pdaemon->left/60).tqarg(num3);
			} else {
				tmp = i18n("Running on batteries - %1% charged").tqarg(pdaemon->val);
			}
		} else {
			// running without any power source...
			// happens eg. due to ACPI not being able to handle battery hot-plugin
			tmp = i18n("No power source found");
                        levelString = i18n("N/A");
		}
	}

    
        // prepare icon for systemtray
        
        KConfig* config =  new KConfig("kcmlaptoprc");
        bool showlevel = false;
        
        if (config)
        {
            config->setGroup("BatteryDefault");
            showlevel= config->readBoolEntry("ShowLevel", false);
            delete config;
        }
        
        if (showlevel)
        {
            // lot of code taken from kmail kmsystemtray class
            
            int oldPixmapWidth = image.size().width();
            int oldPixmapHeight = image.size().height();
            
            TQFont percentageFont = KGlobalSettings::generalFont();
            percentageFont.setBold(true);
        
            // decrease the size of the font for the number of unread messages if the
            // number doesn't fit into the available space
            float percentageFontSize = percentageFont.pointSizeFloat();
            TQFontMetrics qfm( percentageFont );
            int width = qfm.width( levelString );
            if( width > oldPixmapWidth )
            {
            percentageFontSize *= float( oldPixmapWidth ) / float( width );
            percentageFont.setPointSizeFloat( percentageFontSize );
            }
        
            // Create an image which represents the number of unread messages
            // and which has a transparent background.
            // Unfortunately this required the following twisted code because for some
            // reason text that is drawn on a transparent pixmap is invisible
            // (apparently the alpha channel isn't changed when the text is drawn).
            // Therefore I have to draw the text on a solid background and then remove
            // the background by making it transparent with TQPixmap::setMask. This
            // involves the slow createHeuristicMask() function (from the API docs:
            // "This function is slow because it involves transformation to a TQImage,
            // non-trivial computations and a transformation back to a TQBitmap."). Then
            // I have to convert the resulting TQPixmap to a TQImage in order to overlay
            // the light KMail icon with the number (because KIconEffect::overlay only
            // works with TQImage). Finally the resulting TQImage has to be converted
            // back to a TQPixmap.
            // That's a lot of work for overlaying the KMail icon with the number of
            // unread messages, but every other approach I tried failed miserably.
            //                                                           IK, 2003-09-22
            TQPixmap percentagePixmap( oldPixmapWidth, oldPixmapHeight );
            percentagePixmap.fill( TQt::white );
            TQPainter p( &percentagePixmap );
            p.setFont( percentageFont );
            p.setPen( TQt::black );
            p.drawText( percentagePixmap.rect(), TQt::AlignCenter, levelString );
            percentagePixmap.setMask( percentagePixmap.createHeuristicMask() );
            TQImage percentageImage = percentagePixmap.convertToImage();
        
            // Overlay the light KMail icon with the number image
            TQImage iconWithPercentageImage = image.copy();
            KIconEffect::overlay( iconWithPercentageImage, percentageImage );
        
            TQPixmap iconWithPercentage;
            iconWithPercentage.convertFromImage( iconWithPercentageImage );
            
            setPixmap( iconWithPercentage );
        }
        else
        {
            TQPixmap q;
            q.convertFromImage(image);
            if (bmtqmask)
                    q.setMask(*bmtqmask);
            setPixmap(q);
        }
        adjustSize();
            
        TQToolTip::add(this, tmp);
}

#include "daemondock.moc"

