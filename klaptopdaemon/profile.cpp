/*
 * brightness.cpp
 *
 * Copyright (c) 2003 Paul Campbell <paul@taniwha.com>
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
#include "profile.h"
#include "version.h"
#include "portable.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

// other KDE headers:
#include <klocale.h>
#include <kconfig.h>
#include <knuminput.h>
#include <kiconloader.h>
#include <kicondialog.h>
#include <kapplication.h>
#include <kmessagebox.h>
#include <kcombobox.h>

// other TQt headers:
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqvgroupbox.h>
#include <tqgrid.h>
#include <tqpushbutton.h>
#include <tqslider.h>
#include <tqtooltip.h>

extern void wake_laptop_daemon();

ProfileConfig::ProfileConfig(TQWidget * parent, const char *name)
  : KCModule(parent, name)
{
    TQStringList performance_list;
    int current_performance;
    bool *active_list;
    bool has_performance = laptop_portable::get_system_performance(0, current_performance, performance_list, active_list);
    TQStringList throttle_list;
    int current_throttle;
    bool has_throttle = laptop_portable::get_system_throttling(0, current_throttle, throttle_list, active_list);

    KGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

    config =  new KConfig("kcmlaptoprc");

    TQVBoxLayout *top_layout = new TQVBoxLayout( this, KDialog::marginHint(),
					       KDialog::spacingHint() );

    TQHBoxLayout *ll = new TQHBoxLayout();

    TQVGroupBox *gb = new TQVGroupBox(i18n("Not Powered"), this);
    TQToolTip::add( gb, i18n( "Items in this box take effect whenever the laptop is unplugged from the wall" ) );
    if (laptop_portable::has_brightness()) {
    	TQWidget *wp = new TQWidget(gb);
    	TQHBoxLayout *xl = new TQHBoxLayout( wp);
    	poff = new TQCheckBox(i18n("Back panel brightness"), wp);
        TQToolTip::add( poff, i18n( "Enables the changing of the back panel brightness" ) );
	xl->addWidget(poff);
    	connect (poff, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(poff_changed(bool)));

    	xl->addWidget(new TQLabel("-", wp));
    	soff = new TQSlider(0, 255, 16, 160, Qt::Horizontal, wp);
    	soff->setEnabled(0);
        TQToolTip::add( soff, i18n( "How bright it should be when it is changed" ) );
    	connect (soff, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
    	xl->addWidget(soff);
    	xl->addWidget(new TQLabel("+", wp));
    	xl->addStretch(1);
    } else {
	poff = 0;
	soff = 0;
    }

    if (has_performance) {
    	TQWidget *wp = new TQWidget(gb);
    	TQHBoxLayout *xl = new TQHBoxLayout( wp);
    	performance_off = new TQCheckBox(i18n("System performance"), wp);
        TQToolTip::add( performance_off, i18n( "Enables the changing of the system performance profile" ) );
	xl->addWidget(performance_off);
    	connect (performance_off, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(performance_off_changed(bool)));

    	performance_val_off = new KComboBox(0, wp);
        TQToolTip::add( performance_val_off, i18n( "The new system performance profile to change to" ) );
	performance_val_off->insertStringList(performance_list);
    	performance_val_off->setEnabled(0);
    	connect (performance_val_off, TQT_SIGNAL(activated(int)), this, TQT_SLOT(configChanged()));
    	xl->addWidget(performance_val_off);
    	xl->addStretch(1);
    } else {
        performance_off = 0;
        performance_val_off = 0;	 
    }
    if (has_throttle) {
    	TQWidget *wp = new TQWidget(gb);
    	TQHBoxLayout *xl = new TQHBoxLayout( wp);
    	throttle_off = new TQCheckBox(i18n("CPU throttling"), wp);
        TQToolTip::add( throttle_off, i18n( "Enables the throttling of the CPU performance" ) );
	xl->addWidget(throttle_off);
    	connect (throttle_off, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(throttle_off_changed(bool)));

    	throttle_val_off = new KComboBox(0, wp);
	throttle_val_off->insertStringList(throttle_list);
    	throttle_val_off->setEnabled(0);
        TQToolTip::add( throttle_val_off, i18n( "How much to throttle the CPU by" ) );
    	connect (throttle_val_off, TQT_SIGNAL(activated(int)), this, TQT_SLOT(configChanged()));
    	xl->addWidget(throttle_val_off);
    	xl->addStretch(1);
    } else {
        throttle_off = 0;
        throttle_val_off = 0;	 
    }

    ll->addWidget(gb);

    gb = new TQVGroupBox(i18n("Powered"), this);
    TQToolTip::add( gb, i18n( "Items in this box take effect whenever the laptop is plugged into the wall" ) );
    if (laptop_portable::has_brightness()) {
    	TQWidget *wp = new TQWidget(gb);
    	TQHBoxLayout *xl = new TQHBoxLayout( wp);
    	pon = new TQCheckBox(i18n("Back panel brightness"), wp);
        TQToolTip::add( pon, i18n( "Enables the changing of the back panel brightness" ) );
	xl->addWidget(pon);
    	connect (pon, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(pon_changed(bool)));

    	xl->addWidget(new TQLabel("-", wp));
    	son = new TQSlider(0, 255, 16, 255, Qt::Horizontal, wp);
    	son->setEnabled(0);
        TQToolTip::add( son, i18n( "How bright it should be when it is changed" ) );
    	connect (son, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
    	xl->addWidget(son);
    	xl->addWidget(new TQLabel("+", wp));
    	xl->addStretch(1);
    } else {
	pon = 0;
	son = 0;
    }
    if (has_performance) {
    	TQWidget *wp = new TQWidget(gb);
    	TQHBoxLayout *xl = new TQHBoxLayout( wp);
    	performance_on = new TQCheckBox(i18n("System performance"), wp);
        TQToolTip::add( performance_on, i18n( "Enables the changing of the system performance profile" ) );
	xl->addWidget(performance_on);
    	connect (performance_on, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(performance_on_changed(bool)));

    	performance_val_on = new KComboBox(0, wp);
	performance_val_on->insertStringList(performance_list);
    	performance_val_on->setEnabled(0);
        TQToolTip::add( performance_val_on, i18n( "The new system performance profile to change to" ) );
    	connect (performance_val_on, TQT_SIGNAL(activated(int)), this, TQT_SLOT(configChanged()));
    	xl->addWidget(performance_val_on);
    	xl->addStretch(1);
    } else {
        performance_on = 0;
        performance_val_on = 0;	 
    }
    if (has_throttle) {
    	TQWidget *wp = new TQWidget(gb);
    	TQHBoxLayout *xl = new TQHBoxLayout( wp);
    	throttle_on = new TQCheckBox(i18n("CPU throttle"), wp);
        TQToolTip::add( throttle_on, i18n( "Enables the throttling of the CPU performance" ) );
	xl->addWidget(throttle_on);
    	connect (throttle_on, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(throttle_on_changed(bool)));

    	throttle_val_on = new KComboBox(0, wp);
	throttle_val_on->insertStringList(throttle_list);
    	throttle_val_on->setEnabled(0);
        TQToolTip::add( throttle_val_on, i18n( "How much to throttle the CPU by" ) );
    	connect (throttle_val_on, TQT_SIGNAL(activated(int)), this, TQT_SLOT(configChanged()));
    	xl->addWidget(throttle_val_on);
    	xl->addStretch(1);
    } else {
        throttle_on = 0;
        throttle_val_on = 0;	 
    }
    ll->addWidget(gb);

    ll->addStretch(1);

    top_layout->addLayout(ll);
    TQLabel *tmp_label = new TQLabel(i18n("This panel allows you to set default values for system attributes "
					"so that they change when the laptop is plugged in to the wall or "
					"running on batteries."), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );

    tmp_label = new TQLabel(i18n("You can also set options for these values that will be set by low battery "
				"conditions, or system inactivity in the other panels"), this );
    tmp_label->setAlignment( TQt::WordBreak );
    top_layout->addWidget( tmp_label );
    top_layout->addStretch(1);
    top_layout->addWidget( new TQLabel( i18n("Version: %1").tqarg(LAPTOP_VERSION), this), 0, TQt::AlignRight );


    load();      
}

ProfileConfig::~ProfileConfig()
{
  delete config;
}

void ProfileConfig::pon_changed(bool v)
{
	if (son)
		son->setEnabled(v);
	configChanged();
}

void ProfileConfig::poff_changed(bool v)
{
	if (soff)
		soff->setEnabled(v);
	configChanged();
}

void ProfileConfig::performance_on_changed(bool v)
{
	if (performance_val_on)
		performance_val_on->setEnabled(v);
	configChanged();
}

void ProfileConfig::performance_off_changed(bool v)
{
	if (performance_val_off)
		performance_val_off->setEnabled(v);
	configChanged();
}

void ProfileConfig::throttle_on_changed(bool v)
{
	if (throttle_val_on)
		throttle_val_on->setEnabled(v);
	configChanged();
}

void ProfileConfig::throttle_off_changed(bool v)
{
	if (throttle_val_off)
		throttle_val_off->setEnabled(v);
	configChanged();
}


void ProfileConfig::save()
{
        config->setGroup("ProfileDefault");

        config->writeEntry("EnableBrightnessOn", (pon?pon->isChecked():0));
        config->writeEntry("BrightnessOnLevel", (son?son->value():255));
        config->writeEntry("EnableBrightnessOff", (poff?poff->isChecked():0));
        config->writeEntry("BrightnessOffLevel", (soff?soff->value():160));
        config->writeEntry("EnablePerformanceOn", (performance_on?performance_on->isChecked():0));
        config->writeEntry("PerformanceOnLevel", (performance_val_on?performance_val_on->currentText():""));
        config->writeEntry("EnablePerformanceOff", (performance_off?performance_off->isChecked():0));
        config->writeEntry("PerformanceOffLevel", (performance_val_off?performance_val_off->currentText():""));
        config->writeEntry("EnableThrottleOn", (throttle_on?throttle_on->isChecked():0));
        config->writeEntry("ThrottleOnLevel", (throttle_val_on?throttle_val_on->currentText():""));
        config->writeEntry("EnableThrottleOff", (throttle_off?throttle_off->isChecked():0));
        config->writeEntry("ThrottleOffLevel", (throttle_val_off?throttle_val_off->currentText():""));
	config->sync();
        changed(false);
  	wake_laptop_daemon();
}

void ProfileConfig::load()
{
	load( false );
}

void ProfileConfig::load(bool useDefaults)
{
	config->setReadDefaults( useDefaults );

       	config->setGroup("ProfileDefault");

        bool v;
	if (pon) {
		v = config->readBoolEntry("EnableBrightnessOn", false);
		pon->setChecked(v);
	} else {
		v = 0;
	}
	int x;
	if (son) {
		x = config->readNumEntry("BrightnessOnLevel", 255);
		son->setValue(x);
		son->setEnabled(v);
	}
	if (poff) {
		v = config->readBoolEntry("EnableBrightnessOff", false);
		poff->setChecked(v);
	} else {
		v = 0;
	}
	if (soff) {
		x = config->readNumEntry("BrightnessOffLevel", 160);
		soff->setValue(x);
		soff->setEnabled(v);
	}
	if (performance_on) {
		v = config->readBoolEntry("EnablePerformanceOn", false);
		performance_on->setChecked(v);
	} else {
		v = 0;
	}
	if (performance_val_on) {
		TQString s = config->readEntry("PerformanceOnLevel", "");
		int ind = 0;
		for (int i = 0; i < performance_val_on->count(); i++)
		if (performance_val_on->text(i) == s) {
			ind = i;
			break;
		}
		performance_val_on->setCurrentItem(ind);
		performance_val_on->setEnabled(v);
	} 
	if (performance_off) {
		v = config->readBoolEntry("EnablePerformanceOff", false);
		performance_off->setChecked(v);
	} else {
		v = 0;
	}
	if (performance_val_off) {
		TQString s = config->readEntry("PerformanceOffLevel", "");
		int ind = 0;
		for (int i = 0; i < performance_val_off->count(); i++)
		if (performance_val_off->text(i) == s) {
			ind = i;
			break;
		}
		performance_val_off->setCurrentItem(ind);
		performance_val_off->setEnabled(v);
	} 
	if (throttle_on) {
		v = config->readBoolEntry("EnableThrottleOn", false);
		throttle_on->setChecked(v);
	} else {
		v = 0;
	}
	if (throttle_val_on) {
		TQString s = config->readEntry("ThrottleOnLevel", "");
		int ind = 0;
		for (int i = 0; i < throttle_val_on->count(); i++)
		if (throttle_val_on->text(i) == s) {
			ind = i;
			break;
		}
		throttle_val_on->setCurrentItem(ind);
		throttle_val_on->setEnabled(v);
	} 
	if (throttle_off) {
		v = config->readBoolEntry("EnableThrottleOff", false);
		throttle_off->setChecked(v);
	} else {
		v = 0;
	}
	if (throttle_val_off) {
		TQString s = config->readEntry("ThrottleOffLevel", "");
		int ind = 0;
		for (int i = 0; i < throttle_val_off->count(); i++)
		if (throttle_val_off->text(i) == s) {
			ind = i;
			break;
		}
		throttle_val_off->setCurrentItem(ind);
		throttle_val_off->setEnabled(v);
	} 
      emit changed( useDefaults );
}

void ProfileConfig::defaults()
{
	load( true );
}


void ProfileConfig::configChanged()
{
  emit changed(true);
}


TQString ProfileConfig::quickHelp() const
{
  return i18n("<h1>Laptop Power Profile Setup</h1>This module allows you to configure default values for static laptop "
	"system attributes that will change when the laptop is plugged in or unplugged from the wall.");
}


void ProfileConfig::slotStartMonitor()
{
  	wake_laptop_daemon();
}


#include "profile.moc"


