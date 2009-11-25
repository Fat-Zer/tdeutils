/*
 * battery.cpp
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (c) 2002 Marc Mutz <mutz@kde.org>
 * Copyright (c) 2006 Flavio Castelli <flavio.castelli@gmail.com>
 *
 * Requires the Qt widget libraries, available at no cost at
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

// other Qt headers:
#include <qlayout.h>
#include <qlabel.h>
#include <qcheckbox.h>
#include <qhbox.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgrid.h>
#include <qpushbutton.h>
#include <qtooltip.h>
extern void wake_laptop_daemon();


BatteryConfig::BatteryConfig (QWidget * parent, const char *name)
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

    QVBoxLayout *top_layout = new QVBoxLayout( this, KDialog::marginHint(),
					       KDialog::spacingHint() );

    // do we show the monitor
    runMonitor = new QCheckBox( i18n("&Show battery monitor"), this );
    top_layout->addWidget( runMonitor );
    QToolTip::add( runMonitor, i18n( "This box enables the battery state icon in the panel" ) );
    connect( runMonitor, SIGNAL(clicked()), this, SLOT(configChanged()) );
    connect( runMonitor, SIGNAL(clicked()), this, SLOT(runMonitorChanged()) );

    // show also the battery level percentage
    showLevel = new QCheckBox( i18n("Show battery level percentage"), this );
    top_layout->addWidget( showLevel );
    QToolTip::add( showLevel, i18n( "This box enables a text message near the battery state icon containing battery level percentage" ) );
    connect( showLevel, SIGNAL(clicked()), this, SLOT(configChanged()) );
    
    notifyMe = new QCheckBox( i18n("&Notify me whenever my battery becomes fully charged"), this );
    top_layout->addWidget( notifyMe );
    QToolTip::add( notifyMe, i18n( "This box enables a dialog box that pops up when your battery becomes fully charged" ) );
    connect( notifyMe, SIGNAL(clicked()), this, SLOT(configChanged()) );

    blankSaver = new QCheckBox( i18n("&Use a blank screen saver when running on battery"), this );
    top_layout->addWidget( blankSaver );
    connect( blankSaver, SIGNAL(clicked()), this, SLOT(configChanged()) );

    if (!apm) {
      top_layout->addWidget( laptop_portable::no_power_management_explanation(this) );
    } else {
      iconloader = new KIconLoader("klaptopdaemon");

      // the poll time (in seconds)
      QHBox *hb = new QHBox( this );
      hb->setSpacing( KDialog::spacingHint() );
      top_layout->addWidget( hb );

      QLabel* poll_label = new QLabel( i18n("&Check status every:"), hb );
      editPoll = new QSpinBox( 1, 3600, 1, hb ); // min,max,step
      QToolTip::add( editPoll, i18n( "Choose how responsive the laptop software will be when it checks the battery status" ) );
      editPoll->setSuffix( i18n("keep short, unit in spinbox", "sec") );
      poll_label->setBuddy( editPoll );
      connect( editPoll, SIGNAL(valueChanged(int)),
	       this, SLOT(configChanged()) );
      QWidget* spacer = new QWidget( hb );
      hb->setStretchFactor( spacer, 1 );

      // group box to hold the icons together
      QVGroupBox* icons_groupbox = new QVGroupBox( i18n("Select Battery Icons"), this );
      icons_groupbox->layout()->setSpacing( KDialog::spacingHint() );
      top_layout->addWidget( icons_groupbox, 0, Qt::AlignLeft );

      // layout to hold the icons inside the groupbox
      QGrid *icon_grid = new QGrid( 3 /*cols*/, icons_groupbox );
      icon_grid->setSpacing( KDialog::spacingHint() );

      buttonNoBattery = new KIconButton( iconloader, icon_grid );
      buttonNoCharge  = new KIconButton( iconloader, icon_grid );
      buttonCharge    = new KIconButton( iconloader, icon_grid );
      (void)new QLabel( buttonNoBattery, i18n("No &battery"), icon_grid);
      (void)new QLabel( buttonNoCharge, i18n("&Not charging"), icon_grid);
      (void)new QLabel( buttonCharge, i18n("Char&ging"), icon_grid);
      buttonNoBattery->setIconType( KIcon::NoGroup, KIcon::Any, 1);
      buttonNoCharge->setIconType( KIcon::NoGroup, KIcon::Any, 1);
      buttonCharge->setIconType( KIcon::NoGroup, KIcon::Any, 1);
      connect(buttonNoBattery, SIGNAL(iconChanged(QString)), this, SLOT(iconChanged()));
      connect(buttonNoCharge, SIGNAL(iconChanged(QString)), this, SLOT(iconChanged()));
      connect(buttonCharge, SIGNAL(iconChanged(QString)), this, SLOT(configChanged()));


      int num_batteries;
      QStringList battery_names, battery_states, battery_values;
      laptop_portable::get_battery_status(num_batteries, battery_names, battery_states, battery_values);
      if (num_batteries > 0) {
	    QHBoxLayout *hl = new QHBoxLayout();
	    top_layout->addLayout(hl);

	    QHGroupBox *hb = new QHGroupBox(i18n("Current Battery Status"), this);
	    for (int i = 0; i < num_batteries; i++) {

		QWidget *wp;
		if (num_batteries == 1) {
			wp = new QWidget(hb);
		} else {
			wp = new QVGroupBox(battery_names[i], hb);
		}
	    	QVBoxLayout *vb = new QVBoxLayout(wp);

		QLabel *l;

		l = new QLabel(wp);					// icon indicating state
		vb->addWidget(l);
		batt_label_1.append(l);

		l = new QLabel(QString(""), wp);
		vb->addWidget(l);
		batt_label_2.append(l);

		l = new QLabel(QString(""), wp);
		vb->addWidget(l);
		batt_label_3.append(l);
	    }	
	    hl->addWidget(hb);
	    hl->addStretch(1);
            (void)startTimer(30*1000);	// update 2x every minute
      }

      // TODO: remove linefeed from string, can't do it right now coz we have a string freeze
      QLabel* explain = new KRichTextLabel( i18n("This panel controls whether the battery status monitor\nappears in the system tray and what it looks like.").replace("\n"," "), this);
      top_layout->addWidget(explain, 0);
      laptop_portable::extra_config(this, config, top_layout);
    }

    top_layout->addStretch(1);
    startMonitor = new QPushButton( i18n("&Start Battery Monitor"), this);
    connect(startMonitor, SIGNAL(clicked()), this, SLOT(slotStartMonitor()));
    top_layout->addWidget( startMonitor, 0, Qt::AlignRight );

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


QString BatteryConfig::quickHelp() const
{
  return i18n("<h1>Laptop Battery</h1>This module allows you to monitor "
	"your batteries. To make use of this module, you must have power management system software "
	"installed. (And, of course, you should have batteries in your machine.)");
}


void BatteryConfig::slotStartMonitor()
{
	wake_laptop_daemon();
	if (!enablemonitor) {
		KMessageBox::information(0, i18n("<qt>The battery monitor has been started, but the tray icon is currently disabled.  You can make it appear by selecting the <b>Show battery monitor</b> entry on this page and applying your changes.</qt>"), QString::null, "howToEnableMonitor");
	}
}

void
BatteryConfig::ConvertIcon(int percent, QPixmap &pm, QPixmap &result)
{
	QImage image = pm.convertToImage();

	int w = image.width();
	int h = image.height();
	int count = 0;
	QRgb rgb;
	int x, y;
	for (x = 0; x < w; x++)
	for (y = 0; y < h; y++) {
		rgb = image.pixel(x, y);
		if (qRed(rgb) == 0xff &&
		    qGreen(rgb) == 0xff &&
		    qBlue(rgb) == 0xff)
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
		QRgb blue = qRgb(0x00,0x00,0xff);

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
			if (qRed(rgb) == 0xff &&
		    	    qGreen(rgb) == 0xff &&
		    	    qBlue(rgb) == 0xff) {
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
    QStringList battery_names, battery_states, battery_values;
    laptop_portable::get_battery_status(num_batteries, battery_names, battery_states, battery_values);
    if (num_batteries > 0) {
	    for (int i = 0; i < num_batteries; i++) {
		if (battery_states[i] == "yes") {
			QPixmap result;
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

void BatteryConfig::timerEvent(QTimerEvent *)
{
    BatteryStateUpdate();
}

#include "battery.moc"


