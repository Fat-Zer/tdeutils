/*
 * battery.cpp
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (c) 2002 Marc Mutz <mutz@kde.org>
 * Copyright (c) 2006 Flavio Castelli <flavio.castelli@gmail.com>
 *
 * Requires the TQt widget libraries, available at no cost at
 * http://www.troll.no/
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

// my headers:
#include "battery.h"
#include "version.h"
#include "portable.h"

// other KDE headers:
#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <krichtextlabel.h>

// other TQt headers:
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqvgroupbox.h>
#include <tqhgroupbox.h>
#include <tqgrid.h>
#include <tqpushbutton.h>
#include <tqtooltip.h>
extern void wake_laptop_daemon();


BatteryConfig::BatteryConfig (TQWidget * parent, const char *name)
  : KCModule(parent, name),
    editPoll(0),
    iconloader(0),
    buttonNoBattery(0),
    buttonNoCharge(0),
    buttonCharge(0)
{
    KGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

    apm = laptop_portable::has_power_management();
    config =  new KConfig("kcmlaptoprc");
    instance = new KInstance("klaptopdaemon");

    TQVBoxLayout *top_layout = new TQVBoxLayout( this, KDialog::marginHint(),
					       KDialog::spacingHint() );

    // do we show the monitor
    runMonitor = new TQCheckBox( i18n("&Show battery monitor"), this );
    top_layout->addWidget( runMonitor );
    TQToolTip::add( runMonitor, i18n( "This box enables the battery state icon in the panel" ) );
    connect( runMonitor, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );
    connect( runMonitor, TQT_SIGNAL(clicked()), this, TQT_SLOT(runMonitorChanged()) );

    // show also the battery level percentage
    showLevel = new TQCheckBox( i18n("Show battery level percentage"), this );
    top_layout->addWidget( showLevel );
    TQToolTip::add( showLevel, i18n( "This box enables a text message near the battery state icon containing battery level percentage" ) );
    connect( showLevel, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );
    
    notifyMe = new TQCheckBox( i18n("&Notify me whenever my battery becomes fully charged"), this );
    top_layout->addWidget( notifyMe );
    TQToolTip::add( notifyMe, i18n( "This box enables a dialog box that pops up when your battery becomes fully charged" ) );
    connect( notifyMe, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );

    blankSaver = new TQCheckBox( i18n("&Use a blank screen saver when running on battery"), this );
    top_layout->addWidget( blankSaver );
    connect( blankSaver, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()) );

    if (!apm) {
      top_layout->addWidget( laptop_portable::no_power_management_explanation(this) );
    } else {
      iconloader = new KIconLoader("klaptopdaemon");

      // the poll time (in seconds)
      TQHBox *hb = new TQHBox( this );
      hb->setSpacing( KDialog::spacingHint() );
      top_layout->addWidget( hb );

      TQLabel* poll_label = new TQLabel( i18n("&Check status every:"), hb );
      editPoll = new TQSpinBox( 1, 3600, 1, hb ); // min,max,step
      TQToolTip::add( editPoll, i18n( "Choose how responsive the laptop software will be when it checks the battery status" ) );
      editPoll->setSuffix( i18n("keep short, unit in spinbox", "sec") );
      poll_label->setBuddy( editPoll );
      connect( editPoll, TQT_SIGNAL(valueChanged(int)),
	       this, TQT_SLOT(configChanged()) );
      TQWidget* spacer = new TQWidget( hb );
      hb->setStretchFactor( spacer, 1 );

      // group box to hold the icons together
      TQVGroupBox* icons_groupbox = new TQVGroupBox( i18n("Select Battery Icons"), this );
      icons_groupbox->tqlayout()->setSpacing( KDialog::spacingHint() );
      top_layout->addWidget( icons_groupbox, 0, TQt::AlignLeft );

      // tqlayout to hold the icons inside the groupbox
      TQGrid *icon_grid = new TQGrid( 3 /*cols*/, icons_groupbox );
      icon_grid->setSpacing( KDialog::spacingHint() );

      buttonNoBattery = new KIconButton( iconloader, icon_grid );
      buttonNoCharge  = new KIconButton( iconloader, icon_grid );
      buttonCharge    = new KIconButton( iconloader, icon_grid );
      (void)new TQLabel( buttonNoBattery, i18n("No &battery"), icon_grid);
      (void)new TQLabel( buttonNoCharge, i18n("&Not charging"), icon_grid);
      (void)new TQLabel( buttonCharge, i18n("Char&ging"), icon_grid);
      buttonNoBattery->setIconType( KIcon::NoGroup, KIcon::Any, 1);
      buttonNoCharge->setIconType( KIcon::NoGroup, KIcon::Any, 1);
      buttonCharge->setIconType( KIcon::NoGroup, KIcon::Any, 1);
      connect(buttonNoBattery, TQT_SIGNAL(iconChanged(TQString)), this, TQT_SLOT(iconChanged()));
      connect(buttonNoCharge, TQT_SIGNAL(iconChanged(TQString)), this, TQT_SLOT(iconChanged()));
      connect(buttonCharge, TQT_SIGNAL(iconChanged(TQString)), this, TQT_SLOT(configChanged()));


      int num_batteries;
      TQStringList battery_names, battery_states, battery_values;
      laptop_portable::get_battery_status(num_batteries, battery_names, battery_states, battery_values);
      if (num_batteries > 0) {
	    TQHBoxLayout *hl = new TQHBoxLayout();
	    top_layout->addLayout(hl);

	    TQHGroupBox *hb = new TQHGroupBox(i18n("Current Battery tqStatus"), this);
	    for (int i = 0; i < num_batteries; i++) {

		TQWidget *wp;
		if (num_batteries == 1) {
			wp = new TQWidget(hb);
		} else {
			wp = new TQVGroupBox(battery_names[i], hb);
		}
	    	TQVBoxLayout *vb = new TQVBoxLayout(wp);

		TQLabel *l;

		l = new TQLabel(wp);					// icon indicating state
		vb->addWidget(l);
		batt_label_1.append(l);

		l = new TQLabel(TQString(""), wp);
		vb->addWidget(l);
		batt_label_2.append(l);

		l = new TQLabel(TQString(""), wp);
		vb->addWidget(l);
		batt_label_3.append(l);
	    }	
	    hl->addWidget(hb);
	    hl->addStretch(1);
            (void)startTimer(30*1000);	// update 2x every minute
      }

      // TODO: remove linefeed from string, can't do it right now coz we have a string freeze
      TQLabel* explain = new KRichTextLabel( i18n("This panel controls whether the battery status monitor\nappears in the system tray and what it looks like.").replace("\n"," "), this);
      top_layout->addWidget(explain, 0);
      laptop_portable::extra_config(this, config, top_layout);
    }

    top_layout->addStretch(1);
    startMonitor = new TQPushButton( i18n("&Start Battery Monitor"), this);
    connect(startMonitor, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotStartMonitor()));
    top_layout->addWidget( startMonitor, 0, TQt::AlignRight );

    load();
}

BatteryConfig::~BatteryConfig()
{
  delete instance;
  delete config;
}


void BatteryConfig::save()
{
        enablemonitor = runMonitor->isChecked();
        showlevel = showLevel->isChecked();
        useblanksaver = blankSaver->isChecked();
        notifyme = notifyMe->isChecked();

        if (apm) {
                poll_time = editPoll->value();
                nobattery =  buttonNoBattery->icon();
                chargebattery =  buttonCharge->icon();
                nochargebattery =  buttonNoCharge->icon();
        }
        config->setGroup("BatteryDefault");

        config->writeEntry("Enable", enablemonitor);
        config->writeEntry("ShowLevel", showlevel);
        config->writeEntry("NotifyMe", notifyme);
        config->writeEntry("BlankSaver", useblanksaver);
        config->writeEntry("Poll", poll_time);
        config->writeEntry("NoBatteryPixmap", nobattery);
        config->writeEntry("ChargePixmap", chargebattery);
        config->writeEntry("NoChargePixmap", nochargebattery);
	config->sync();
        changed(false);
	wake_laptop_daemon();
}

void BatteryConfig::load()
{
	load( false );
}

void BatteryConfig::load(bool useDefaults)
{
        config->setReadDefaults( useDefaults );
        config->setGroup("BatteryDefault");

        poll_time = config->readNumEntry("Poll", 20);
        enablemonitor = config->readBoolEntry("Enable", true);
        showlevel = config->readBoolEntry("ShowLevel", false);
        notifyme = config->readBoolEntry("NotifyMe", false);
        useblanksaver = config->readBoolEntry("BlankSaver", false);

        nobattery = config->readEntry("NoBatteryPixmap", "laptop_nobattery");
        nochargebattery = config->readEntry("NoChargePixmap", "laptop_nocharge");
        chargebattery = config->readEntry("ChargePixmap", "laptop_charge");

        runMonitor->setChecked(enablemonitor);
        showLevel->setChecked(showlevel);
	blankSaver->setChecked(useblanksaver);
	notifyMe->setChecked(notifyme);
        if (apm) {
                editPoll->setValue(poll_time);
                buttonNoCharge->setIcon(nochargebattery);
                buttonCharge->setIcon(chargebattery);
	        buttonNoBattery->setIcon(nobattery);
        }
        battery_pm = SmallIcon(nochargebattery, 20, KIcon::DefaultState, instance);
        battery_nopm = SmallIcon(nobattery, 20, KIcon::DefaultState, instance);
        emit changed(useDefaults);
	BatteryStateUpdate();
}

void BatteryConfig::defaults()
{
	load( true );
}

void BatteryConfig::runMonitorChanged()
{
    showLevel->setEnabled (runMonitor->isChecked());
}

void BatteryConfig::configChanged()
{
  emit changed(true);
}


TQString BatteryConfig::quickHelp() const
{
  return i18n("<h1>Laptop Battery</h1>This module allows you to monitor "
	"your batteries. To make use of this module, you must have power management system software "
	"installed. (And, of course, you should have batteries in your machine.)");
}


void BatteryConfig::slotStartMonitor()
{
	wake_laptop_daemon();
	if (!enablemonitor) {
		KMessageBox::information(0, i18n("<qt>The battery monitor has been started, but the tray icon is currently disabled.  You can make it appear by selecting the <b>Show battery monitor</b> entry on this page and applying your changes.</qt>"), TQString(), "howToEnableMonitor");
	}
}

void
BatteryConfig::ConvertIcon(int percent, TQPixmap &pm, TQPixmap &result)
{
	TQImage image = pm.convertToImage();

	int w = image.width();
	int h = image.height();
	int count = 0;
	TQRgb rgb;
	int x, y;
	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++) {
		rgb = image.pixel(x, y);
		if (tqRed(rgb) == 0xff &&
		    tqGreen(rgb) == 0xff &&
		    tqBlue(rgb) == 0xff)
			count++;
	}
	int c = (count*percent)/100;
	if (percent == 100) {
		c = count;
	} else
	if (percent != 100 && c == count)
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
		for (x = 0; x < w; x++) {
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

	result.convertFromImage(image);
}

void
BatteryConfig::BatteryStateUpdate()
{
    int num_batteries;
    TQStringList battery_names, battery_states, battery_values;
    laptop_portable::get_battery_status(num_batteries, battery_names, battery_states, battery_values);
    if (num_batteries > 0) {
	    for (int i = 0; i < num_batteries; i++) {
		if (battery_states[i] == "yes") {
			TQPixmap result;
			ConvertIcon(battery_values[i].toInt(), battery_pm, result);
			batt_label_1.at(i)->setPixmap(result);

			batt_label_2.at(i)->setText(battery_values[i]+"%");

			batt_label_3.at(i)->setText(i18n("Present"));
		} else {
			batt_label_1.at(i)->setPixmap(battery_nopm);

			batt_label_2.at(i)->setText("");

			batt_label_3.at(i)->setText(i18n("Not present"));
		}
	    }	
    }
}

void BatteryConfig::iconChanged()
{
    nobattery =  buttonNoBattery->icon();
    nochargebattery =  buttonNoCharge->icon();
    battery_pm = SmallIcon(nochargebattery, 20, KIcon::DefaultState, instance);
    battery_nopm = SmallIcon(nobattery, 20, KIcon::DefaultState, instance);
    emit changed(true);
    BatteryStateUpdate();
}

void BatteryConfig::timerEvent(TQTimerEvent *)
{
    BatteryStateUpdate();
}

#include "battery.moc"


