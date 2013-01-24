/*
 * power.cpp
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (c) 2002 Marc Mutz <mutz@kde.org>
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

#include "power.h"
#include "portable.h"
#include "version.h"

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <knumvalidator.h>
#include <kdialog.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <knuminput.h>

#include <tqlayout.h>
#include <tqvbuttongroup.h>
#include <tqspinbox.h>
#include <tqradiobutton.h>
#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqslider.h>
#include <tqtooltip.h>
extern void wake_laptop_daemon();

PowerConfig::PowerConfig (TQWidget * parent, const char *name)
  : KCModule(parent, name),
    nopowerBox(0),
    nopowerStandby(0),
    nopowerSuspend(0),
    nopowerOff(0),
    nopowerHibernate(0),
    nopowerBrightness(0),
    nopowerValBrightness(0),
    nopowerThrottle(0),
    nopowerValThrottle(0),
    nopowerPerformance(0),
    nopowerValPerformance(0),

    powerBox(0),
    powerStandby(0),
    powerSuspend(0),
    powerOff(0),
    powerHibernate(0),
    powerBrightness(0),
    powerValBrightness(0),
    powerThrottle(0),
    powerValThrottle(0),
    powerPerformance(0),
    powerValPerformance(0),


    noeditwait(0),
    editwait(0),
    enablelav(0),
    noenablelav(0),
    noeditlav(0),
    editlav(0)
{
  TDEGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

  apm = laptop_portable::has_power_management();
  config =  new KConfig("kcmlaptoprc");
  int can_brightness = laptop_portable::has_brightness();
    TQStringList throttle_list;
    int current_throttle;
    bool *active_list;
    bool has_throttle = laptop_portable::get_system_throttling(0, current_throttle, throttle_list, active_list);
    TQStringList performance_list;
    int current_performance;
    bool has_performance = laptop_portable::get_system_performance(0, current_performance, performance_list, active_list);

  if (!apm && !can_brightness && !has_throttle && !has_performance) {
    TQVBoxLayout *top_layout = new TQVBoxLayout(this, KDialog::marginHint(),
					      KDialog::spacingHint());

    KActiveLabel* explain = laptop_portable::no_power_management_explanation(this);
    top_layout->addWidget(explain);

    top_layout->addStretch(1);
  } else {
    int can_standby = laptop_portable::has_standby();
    int can_suspend = laptop_portable::has_suspend();
    int can_hibernate = laptop_portable::has_hibernation();

    if (!can_standby && !can_suspend && !can_hibernate && !can_brightness && !has_throttle && !has_performance)
      apm = 0;
    if (!apm) {
      TQVBoxLayout *top_layout = new TQVBoxLayout(this, KDialog::marginHint(),
						KDialog::spacingHint());

      TQLabel* explain = laptop_portable::how_to_do_suspend_resume(this);
      top_layout->addWidget(explain);

      top_layout->addStretch(1);
    } else {
      TQVBoxLayout *top_layout = new TQVBoxLayout(this, KDialog::marginHint(),
						KDialog::spacingHint());
      TQHBoxLayout *hlay = new TQHBoxLayout( top_layout );

      nopowerBox = new TQVButtonGroup(i18n("Not Powered"), this);
      TQToolTip::add( nopowerBox, i18n( "Options in this box apply when the laptop is unplugged from the wall and has been idle for a while" ) );
      nopowerBox->layout()->setSpacing( KDialog::spacingHint() );
      hlay->addWidget( nopowerBox );

      if (can_standby) {
	nopowerStandby = new TQRadioButton(i18n("Standb&y"), nopowerBox);
        TQToolTip::add( nopowerStandby, i18n( "Causes the laptop to change to a standby temporary-low power state" ) );
      }
      if (can_suspend) {
	nopowerSuspend = new TQRadioButton(i18n("&Suspend"), nopowerBox);
        TQToolTip::add( nopowerSuspend, i18n( "Causes the laptop to change to a suspend 'save-to-ram' state" ) );
      }
      if (can_hibernate) {
	nopowerHibernate = new TQRadioButton(i18n("H&ibernate"), nopowerBox);
        TQToolTip::add( nopowerHibernate, i18n( "Causes the laptop to change to a hibernate 'save-to-disk' state" ) );
      }
      if (can_suspend||can_standby||can_hibernate)
        nopowerOff = new TQRadioButton(i18n("None"), nopowerBox);
      if (can_brightness) {
		nopowerBrightness = new TQCheckBox(i18n("Brightness"), nopowerBox);
        	TQToolTip::add( nopowerBrightness, i18n( "Enables changing the laptop's back panel brightness" ) );
		TQWidget *wp = new TQWidget(nopowerBox);
	        TQHBoxLayout *xl = new TQHBoxLayout( wp);
		xl->addWidget(new TQLabel("-", wp));
		nopowerValBrightness = new TQSlider(0, 255, 16, 255, Qt::Horizontal, wp);
        	TQToolTip::add( nopowerValBrightness, i18n( "How bright to change the back panel" ) );
		nopowerValBrightness->setEnabled(0);
		connect(nopowerValBrightness, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(changed()));
		connect(nopowerBrightness, TQT_SIGNAL(toggled(bool)), nopowerValBrightness, TQT_SLOT(setEnabled(bool)));
		xl->addWidget(nopowerValBrightness);
		xl->addWidget(new TQLabel("+", wp));
		xl->addStretch(1);
      } 
      if (has_performance) {
		nopowerPerformance = new TQCheckBox(i18n("System performance"), nopowerBox);
        	TQToolTip::add( nopowerPerformance, i18n( "Enables changing the laptop's performance profile" ) );

		TQWidget *wp = new TQWidget(nopowerBox);
	        TQHBoxLayout *xl = new TQHBoxLayout( wp);
    		nopowerValPerformance = new KComboBox(0, wp);
        	TQToolTip::add( nopowerValPerformance, i18n( "Which profile to change it to" ) );
		nopowerValPerformance->insertStringList(performance_list);
    		nopowerValPerformance->setEnabled(0);
		connect(nopowerValPerformance, TQT_SIGNAL(activated(int)), this, TQT_SLOT(changed()));
		connect(nopowerPerformance, TQT_SIGNAL(toggled(bool)), nopowerValPerformance, TQT_SLOT(setEnabled(bool)));
		xl->addWidget(nopowerValPerformance);
		xl->addStretch(1);
      } 
      if (has_throttle) {
		nopowerThrottle = new TQCheckBox(i18n("CPU throttle"), nopowerBox);
        	TQToolTip::add( nopowerThrottle, i18n( "Enables throttling the laptop's CPU" ) );

		TQWidget *wp = new TQWidget(nopowerBox);
	        TQHBoxLayout *xl = new TQHBoxLayout( wp);
    		nopowerValThrottle = new KComboBox(0, wp);
        	TQToolTip::add( nopowerValThrottle, i18n( "How much to throttle the laptop's CPU" ) );
		nopowerValThrottle->insertStringList(throttle_list);
    		nopowerValThrottle->setEnabled(0);
		connect(nopowerValThrottle, TQT_SIGNAL(activated(int)), this, TQT_SLOT(changed()));
		connect(nopowerThrottle, TQT_SIGNAL(toggled(bool)), nopowerValThrottle, TQT_SLOT(setEnabled(bool)));
		xl->addWidget(nopowerValThrottle);
		xl->addStretch(1);
      } 


      connect(nopowerBox, TQT_SIGNAL(clicked(int)), this, TQT_SLOT(changed()));

      bool can_lav = laptop_portable::has_lav();
      TQHBox *hbox;
      if (can_lav) {
      	  hbox = new TQHBox( nopowerBox );
          noenablelav = new TQCheckBox(i18n("Don't act if LAV is >"), hbox);
          connect(noenablelav, TQT_SIGNAL(clicked()), this, TQT_SLOT(changed()));
          noeditlav = new KDoubleSpinBox(0.0, 10.0, 0.0, 0.1, 1, hbox);
          TQToolTip::add( noeditlav, i18n( "If enabled and the system load average is greater than this value none of the above options will be applied" ) );
          connect(noeditlav, TQT_SIGNAL(valueChanged(double)), this, TQT_SLOT(changed()));
          connect(noenablelav, TQT_SIGNAL(toggled(bool)), noeditlav, TQT_SLOT(setEnabled(bool)));
      }

      hbox = new TQHBox( nopowerBox );
      TQLabel* noedlabel = new TQLabel(i18n("&Wait for:"), hbox);
      noeditwait = new TQSpinBox( 1, 60*24 /*1 day*/, 1, hbox );
      TQToolTip::add( noeditwait, i18n( "How long the computer has to be idle before these values take effect" ) );
      noeditwait->setSuffix( i18n("keep short, unit in spinbox", "min") );
      noedlabel->setBuddy( noeditwait );
      hbox->setStretchFactor( noeditwait, 1 );
      connect( noeditwait, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(changed()));


	///////////////////////////////////////////////////////////////


      powerBox = new TQVButtonGroup(i18n("Powered"), this);
      powerBox->layout()->setSpacing( KDialog::spacingHint() );
      TQToolTip::add( powerBox, i18n( "Options in this box apply when the laptop is plugged into the wall and has been idle for a while" ) );
      hlay->addWidget( powerBox );

      if (can_standby) {
	powerStandby = new TQRadioButton(i18n("Sta&ndby"), powerBox);
        TQToolTip::add( powerStandby, i18n( "Causes the laptop to change to a standby temporary-low power state" ) );
      }
      if (can_suspend) {
	powerSuspend = new TQRadioButton(i18n("S&uspend"), powerBox);
        TQToolTip::add( powerSuspend, i18n( "Causes the laptop to change to a suspend 'save-to-ram' state" ) );
      }
      if (can_hibernate) {
	powerHibernate = new TQRadioButton(i18n("Hi&bernate"), powerBox);
        TQToolTip::add( powerHibernate, i18n( "Causes the laptop to change to a hibernate 'save-to-disk' state" ) );
      }
      if (can_suspend||can_standby||can_hibernate)
      	powerOff = new TQRadioButton(i18n("None"), powerBox);
      if (can_brightness) {
		powerBrightness = new TQCheckBox(i18n("Brightness"), powerBox);
        	TQToolTip::add( powerBrightness, i18n( "Enables changing the laptop's back panel brightness" ) );
		TQWidget *wp = new TQWidget(powerBox);
	        TQHBoxLayout *xl = new TQHBoxLayout( wp);
		xl->addWidget(new TQLabel("-", wp));
		powerValBrightness = new TQSlider(0, 255, 16, 255, Qt::Horizontal, wp);
        	TQToolTip::add( powerValBrightness, i18n( "How bright to change the back panel" ) );
		powerValBrightness->setEnabled(0);
		connect(powerValBrightness, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(changed()));
		connect(powerBrightness, TQT_SIGNAL(toggled(bool)), powerValBrightness, TQT_SLOT(setEnabled(bool)));
		xl->addWidget(powerValBrightness);
		xl->addWidget(new TQLabel("+", wp));
		xl->addStretch(1);
      }
      if (has_performance) {
		powerPerformance = new TQCheckBox(i18n("System performance"), powerBox);
        	TQToolTip::add( powerPerformance, i18n( "Enables changing the laptop's performance profile" ) );

		TQWidget *wp = new TQWidget(powerBox);
	        TQHBoxLayout *xl = new TQHBoxLayout( wp);
    		powerValPerformance = new KComboBox(0, wp);
        	TQToolTip::add( powerValPerformance, i18n( "Which profile to change it to" ) );
		powerValPerformance->insertStringList(performance_list);
    		powerValPerformance->setEnabled(0);
		connect(powerValPerformance, TQT_SIGNAL(activated(int)), this, TQT_SLOT(changed()));
		connect(powerPerformance, TQT_SIGNAL(toggled(bool)), powerValPerformance, TQT_SLOT(setEnabled(bool)));
		xl->addWidget(powerValPerformance);
		xl->addStretch(1);
      } 
      if (has_throttle) {
		powerThrottle = new TQCheckBox(i18n("CPU throttle"), powerBox);
        	TQToolTip::add( powerThrottle, i18n( "Enables throttling the laptop's CPU" ) );

		TQWidget *wp = new TQWidget(powerBox);
	        TQHBoxLayout *xl = new TQHBoxLayout( wp);
    		powerValThrottle = new KComboBox(0, wp);
        	TQToolTip::add( powerValThrottle, i18n( "How much to throttle the laptop's CPU" ) );
		powerValThrottle->insertStringList(throttle_list);
    		powerValThrottle->setEnabled(0);
		connect(powerValThrottle, TQT_SIGNAL(activated(int)), this, TQT_SLOT(changed()));
		connect(powerThrottle, TQT_SIGNAL(toggled(bool)), powerValThrottle, TQT_SLOT(setEnabled(bool)));
		xl->addWidget(powerValThrottle);
		xl->addStretch(1);
      } 

      connect(powerBox, TQT_SIGNAL(clicked(int)), this, TQT_SLOT(changed()));

      if (can_lav) {
          hbox = new TQHBox( powerBox );
          enablelav = new TQCheckBox(i18n("Don't act if LAV is >"), hbox);
          connect( enablelav, TQT_SIGNAL(clicked()), this, TQT_SLOT(changed()));
          editlav = new KDoubleSpinBox(0.0, 10.0, 0.0, 0.1, 1, hbox);
          TQToolTip::add( editlav, i18n( "If enabled and the system load average is greater than this value none of the above options will be applied" ) );
          connect( editlav, TQT_SIGNAL(valueChanged(double)), this, TQT_SLOT(changed()));
          connect( enablelav, TQT_SIGNAL(toggled(bool)), editlav, TQT_SLOT(setEnabled(bool)) );
      }

      hbox = new TQHBox( powerBox );
      TQLabel* edlabel = new TQLabel(i18n("Wai&t for:"), hbox);
      editwait = new TQSpinBox( 1, 60*24 /*1 day*/, 1, hbox );
      TQToolTip::add( editwait, i18n( "How long the computer has to be idle before these values take effect" ) );
      editwait->setSuffix( i18n("keep short, unit in spinbox", "min") );
      edlabel->setBuddy( editwait );
      hbox->setStretchFactor( editwait, 1 );
      connect( editwait, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(changed()));

      hlay->addStretch(1);

      TQLabel* explain = new TQLabel(i18n("This panel configures the behavior of the automatic power-down feature - "
					"it works as a sort of extreme screen saver. You can configure different "
					"timeouts and types of behavior depending on whether or not your laptop is "
					"plugged in to the mains supply."), this );
      explain->setAlignment( TQt::WordBreak );
      top_layout->addWidget(explain);

      if (can_standby) {
	TQLabel* explain3 = new TQLabel(i18n("Different laptops may respond to 'standby' in different ways - in many "
					"it is only a temporary state and may not be useful for you."), this);
	explain3->setAlignment( TQt::WordBreak );
	top_layout->addWidget(explain3, 0, TQt::AlignLeft);
      }

      top_layout->addStretch(1);

      top_layout->addWidget( new TQLabel( i18n("Version: %1").arg(LAPTOP_VERSION), this ), 0, TQt::AlignRight );
    }
  }

  load();
}

PowerConfig::~PowerConfig()
{
  delete config;
}

void PowerConfig::save()
{
  if (editwait) {
      	power = getPower();
      	nopower = getNoPower();
	edit_wait = editwait->value();
	noedit_wait = noeditwait->value();
	nopower_bright_enabled = (nopowerBrightness?nopowerBrightness->isChecked():0);
	power_bright_enabled = (powerBrightness?powerBrightness->isChecked():0);
	nopower_bright_val = (nopowerValBrightness?nopowerValBrightness->value():0);
	power_bright_val = (powerValBrightness?powerValBrightness->value():255);
	nopower_performance_enabled = (nopowerPerformance?nopowerPerformance->isChecked():0);
	power_performance_enabled = (powerPerformance?powerPerformance->isChecked():0);
	nopower_performance_val = (nopowerValPerformance?nopowerValPerformance->currentText():"");
	power_performance_val = (powerValPerformance?powerValPerformance->currentText():"");
	nopower_throttle_enabled = (nopowerThrottle?nopowerThrottle->isChecked():0);
	power_throttle_enabled = (powerThrottle?powerThrottle->isChecked():0);
	nopower_throttle_val = (nopowerValThrottle?nopowerValThrottle->currentText():"");
	power_throttle_val = (powerValThrottle?powerValThrottle->currentText():"");
	edit_lav = (editlav?editlav->value():-1);
	noedit_lav = (noeditlav?noeditlav->value():-1);
	lav_enabled = (enablelav?enablelav->isChecked():0);
	nolav_enabled = (noenablelav?noenablelav->isChecked():0);
  }

  config->setGroup("LaptopPower");
  config->writeEntry("NoPowerSuspend", nopower);
  config->writeEntry("PowerSuspend", power);
  config->writeEntry("PowerWait", edit_wait);
  config->writeEntry("NoPowerWait", noedit_wait);
  config->writeEntry("PowerLav", edit_lav);
  config->writeEntry("NoPowerLav", noedit_lav);
  config->writeEntry("LavEnabled", lav_enabled);
  config->writeEntry("NoLavEnabled", nolav_enabled);
  config->writeEntry("PowerBrightnessEnabled", power_bright_enabled);
  config->writeEntry("NoPowerBrightnessEnabled", nopower_bright_enabled);
  config->writeEntry("PowerBrightness", power_bright_val);
  config->writeEntry("NoPowerBrightness", nopower_bright_val);
  config->writeEntry("PowerPerformanceEnabled", power_performance_enabled);
  config->writeEntry("NoPowerPerformanceEnabled", nopower_performance_enabled);
  config->writeEntry("PowerPerformance", power_performance_val);
  config->writeEntry("NoPowerPerformance", nopower_performance_val);
  config->writeEntry("PowerThrottleEnabled", power_throttle_enabled);
  config->writeEntry("NoPowerThrottleEnabled", nopower_throttle_enabled);
  config->writeEntry("PowerThrottle", power_throttle_val);
  config->writeEntry("NoPowerThrottle", nopower_throttle_val);
  config->sync();
  changed(false);
  wake_laptop_daemon();
}

void PowerConfig::load()
{
	load( false );
}

void PowerConfig::load(bool useDefaults)
{
  config->setReadDefaults( useDefaults );

  config->setGroup("LaptopPower");
  nopower = config->readNumEntry("NoPowerSuspend", (nopowerStandby?1:nopowerSuspend?2:0));
  power = config->readNumEntry("PowerSuspend", 0);
  edit_wait = config->readNumEntry("PowerWait", 20);
  noedit_wait = config->readNumEntry("NoPowerWait", 5);
  edit_lav = config->readDoubleNumEntry("PowerLav", -1);
  noedit_lav = config->readDoubleNumEntry("NoPowerLav", -1);
  lav_enabled = config->readBoolEntry("LavEnabled", 0);
  nolav_enabled = config->readBoolEntry("NoLavEnabled", 0);
  nopower_bright_enabled = config->readBoolEntry("NoPowerBrightnessEnabled", 0);
  power_bright_enabled = config->readBoolEntry("PowerBrightnessEnabled", 0);
  nopower_bright_val = config->readNumEntry("NoPowerBrightness", 0);
  power_bright_val = config->readNumEntry("PowerBrightness", 255);
  nopower_performance_enabled = config->readBoolEntry("NoPowerPerformanceEnabled", 0);
  power_performance_enabled = config->readBoolEntry("PowerPerformanceEnabled", 0);
  nopower_performance_val = config->readEntry("NoPowerPerformance", "");
  power_performance_val = config->readEntry("PowerPerformance", "");
  nopower_throttle_enabled = config->readBoolEntry("NoPowerThrottleEnabled", 0);
  power_throttle_enabled = config->readBoolEntry("PowerThrottleEnabled", 0);
  nopower_throttle_val = config->readEntry("NoPowerThrottle", "");
  power_throttle_val = config->readEntry("PowerThrottle", "");

  // the GUI should reflect the real values
  if (editwait) {
	editwait->setValue(edit_wait);
	noeditwait->setValue(noedit_wait);
	if (editlav) {
		editlav->setValue(edit_lav);
		editlav->setEnabled(lav_enabled);
	}
	if (noeditlav) {
		noeditlav->setValue(noedit_lav);
		noeditlav->setEnabled(nolav_enabled);
	}
	if (enablelav)
		enablelav->setChecked(lav_enabled);
	if (noenablelav)
		noenablelav->setChecked(nolav_enabled);
   	setPower(power, nopower);
	if (nopowerBrightness) 
		nopowerBrightness->setChecked(nopower_bright_enabled);
	if (powerBrightness) 
		powerBrightness->setChecked(power_bright_enabled);
	if (nopowerValBrightness) {
		nopowerValBrightness->setValue(nopower_bright_val);
		nopowerValBrightness->setEnabled(nopower_bright_enabled);
	}
	if (powerValBrightness) {
		powerValBrightness->setValue(power_bright_val);
		powerValBrightness->setEnabled(power_bright_enabled);
	}
	if (nopowerPerformance) 
		nopowerPerformance->setChecked(nopower_performance_enabled);
	if (powerPerformance) 
		powerPerformance->setChecked(power_performance_enabled);
	if (nopowerValPerformance) {
		int ind = 0;
		for (int i = 0; i < nopowerValPerformance->count(); i++)
		if (nopowerValPerformance->text(i) == nopower_performance_val) {
			ind = i;
			break;
		}
		nopowerValPerformance->setCurrentItem(ind);
		nopowerValPerformance->setEnabled(nopower_performance_enabled);
	}
	if (powerValPerformance) {
		int ind = 0;
		for (int i = 0; i < powerValPerformance->count(); i++)
		if (powerValPerformance->text(i) == power_performance_val) {
			ind = i;
			break;
		}
		powerValPerformance->setCurrentItem(ind);
		powerValPerformance->setEnabled(power_performance_enabled);
	}
	if (nopowerThrottle) 
		nopowerThrottle->setChecked(nopower_throttle_enabled);
	if (powerThrottle) 
		powerThrottle->setChecked(power_throttle_enabled);
	if (nopowerValThrottle) {
		int ind = 0;
		for (int i = 0; i < nopowerValThrottle->count(); i++)
		if (nopowerValThrottle->text(i) == nopower_throttle_val) {
			ind = i;
			break;
		}
		nopowerValThrottle->setCurrentItem(ind);
		nopowerValThrottle->setEnabled(nopower_throttle_enabled);
	}
	if (powerValThrottle) {
		int ind = 0;
		for (int i = 0; i < powerValThrottle->count(); i++)
		if (powerValThrottle->text(i) == power_throttle_val) {
			ind = i;
			break;
		}
		powerValThrottle->setCurrentItem(ind);
		powerValThrottle->setEnabled(power_throttle_enabled);
	}
  }
  emit changed( useDefaults );
}

void PowerConfig::defaults()
{
	load( true );
}

int  PowerConfig::getNoPower()
{
  if (!apm)
	return(nopower);
  if (nopowerHibernate && nopowerHibernate->isChecked())
    return 3;
  if (nopowerStandby && nopowerStandby->isChecked())
    return 1;
  if (nopowerSuspend && nopowerSuspend->isChecked())
    return 2;
  return(0);
}

int  PowerConfig::getPower()
{
  if (!apm || !powerOff)
	return(power);
  if (powerHibernate && powerHibernate->isChecked())
    return 3;
  if (powerStandby && powerStandby->isChecked())
    return 1;
  if (powerSuspend && powerSuspend->isChecked())
    return 2;
  return(0);
}

void PowerConfig::setPower(int p, int np)
{
  if (!apm || nopowerOff == 0)
	return;
  if (nopowerSuspend) {
	nopowerSuspend->setChecked(FALSE);
  } else {
	if (np == 2) np = 0;
  }
  if (nopowerStandby) {
	nopowerStandby->setChecked(FALSE);
  } else {
	if (np == 1) np = 0;
  }
  if (nopowerHibernate) {
	nopowerHibernate->setChecked(FALSE);
  } else {
	if (np == 3) np = 0;
  }
  if (nopowerOff)
	  nopowerOff->setChecked(FALSE);
  switch (np) {
  case 0: nopowerOff->setChecked(TRUE);break;
  case 1: nopowerStandby->setChecked(TRUE);break;
  case 2: nopowerSuspend->setChecked(TRUE);break;
  case 3: nopowerHibernate->setChecked(TRUE);break;
  }
  if (powerSuspend) {
	powerSuspend->setChecked(FALSE);
  } else {
	if (p == 2) p = 0;
  }
  if (powerStandby) {
	powerStandby->setChecked(FALSE);
  } else {
	if (p == 1) p = 0;
  }
  if (powerHibernate) {
	powerHibernate->setChecked(FALSE);
  } else {
	if (p == 3) p = 0;
  }
  if (powerOff)
  	powerOff->setChecked(FALSE);
  switch (p) {
  case 0: powerOff->setChecked(TRUE);break;
  case 1: powerStandby->setChecked(TRUE);break;
  case 2: powerSuspend->setChecked(TRUE);break;
  case 3: powerHibernate->setChecked(TRUE);break;
  }
}


TQString PowerConfig::quickHelp() const
{
  return i18n("<h1>Laptop Power Control</h1>This module allows you to "
	"control the power settings of your laptop and set timouts that will trigger "
	"state changes you can use to save power");

}

#include "power.moc"
