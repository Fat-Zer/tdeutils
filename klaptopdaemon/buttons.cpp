/*
 * buttons.cpp
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (c) 2002 Marc Mutz <mutz@kde.org>
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

#include "buttons.h"
#include "portable.h"
#include "version.h"

#include <kglobal.h>
#include <klocale.h>
#include <kconfig.h>
#include <knumvalidator.h>
#include <kdialog.h>
#include <kapplication.h>
#include <kcombobox.h>

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qspinbox.h>
#include <qradiobutton.h>
#include <qcheckbox.h>
#include <qhbox.h>
#include <qslider.h>
#include <qtooltip.h>
extern void wake_laptop_daemon();

ButtonsConfig::ButtonsConfig (QWidget * parent, const char *name)
  : KCModule(parent, name),
    lidBox(0),
    lidStandby(0),
    lidSuspend(0),
    lidOff(0),
    lidHibernate(0),
    lidShutdown(0),
    lidLogout(0),
    lidBrightness(0),
    lidValBrightness(0),
    lidThrottle(0),
    lidValThrottle(0),
    lidPerformance(0),
    lidValPerformance(0),

    powerBox(0),
    powerStandby(0),
    powerSuspend(0),
    powerOff(0),
    powerHibernate(0),
    powerShutdown(0),
    powerLogout(0),
    powerBrightness(0),
    powerValBrightness(0),
    powerThrottle(0),
    powerValThrottle(0),
    powerPerformance(0),
    powerValPerformance(0)
{
  int can_shutdown = 1; // fix me
  int can_logout = 1; // fix me

  KGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

  apm = laptop_portable::has_power_management();
  config =  new KConfig("kcmlaptoprc");
  int can_brightness = laptop_portable::has_brightness();
    QStringList throttle_list;
    int current_throttle;
    bool *active_list;
    bool has_throttle = laptop_portable::get_system_throttling(0, current_throttle, throttle_list, active_list);
    QStringList performance_list;
    int current_performance;
    bool has_performance = laptop_portable::get_system_performance(0, current_performance, performance_list, active_list);

  if (!apm && !can_brightness && !has_throttle && !has_performance) {
    QVBoxLayout *top_layout = new QVBoxLayout(this, KDialog::marginHint(),
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
      QVBoxLayout *top_layout = new QVBoxLayout(this, KDialog::marginHint(),
						KDialog::spacingHint());

      QLabel* explain = laptop_portable::how_to_do_suspend_resume(this);
      top_layout->addWidget(explain);

      top_layout->addStretch(1);
    } else {
      QVBoxLayout *top_layout = new QVBoxLayout(this, KDialog::marginHint(),
						KDialog::spacingHint());
      QHBoxLayout *hlay = new QHBoxLayout( top_layout );

      if (laptop_portable::has_button(laptop_portable::LidButton)) {
      	lidBox = new QVButtonGroup(i18n("Lid Switch Closed"), this);
      	lidBox->layout()->setSpacing( KDialog::spacingHint() );
        QToolTip::add( lidBox, i18n( "Select which actions will occur when the laptop's lid is closed" ) );
      	hlay->addWidget( lidBox );

      	if (can_standby) {
		lidStandby = new QRadioButton(i18n("Standb&y"), lidBox);
        	QToolTip::add( lidStandby, i18n( "Causes the laptop to move into the standby temporary low-power state" ) );
	}
      	if (can_suspend) {
		lidSuspend = new QRadioButton(i18n("&Suspend"), lidBox);
        	QToolTip::add( lidSuspend, i18n( "Causes the laptop to move into the suspend 'save-to-ram' state" ) );
	}
      	if (can_hibernate) {
		lidHibernate = new QRadioButton(i18n("H&ibernate"), lidBox);
        	QToolTip::add( lidHibernate, i18n( "Causes the laptop to move into the hibernate 'save-to-disk' state" ) );
	}
      	if (can_shutdown) {
		lidShutdown = new QRadioButton(i18n("System power off"), lidBox);
        	QToolTip::add( lidShutdown, i18n( "Causes the laptop to power down" ) );
	}
      	if (can_logout) {
		lidLogout = new QRadioButton(i18n("Logout"), lidBox);
        	QToolTip::add( lidShutdown, i18n( "Causes you to be logged out" ) );
	}
      	if (can_suspend||can_standby||can_hibernate||can_shutdown||can_logout)
		lidOff = new QRadioButton(i18n("&Off"), lidBox);
      	if (can_brightness) {
		lidBrightness = new QCheckBox(i18n("Brightness"), lidBox);
        	QToolTip::add( lidBrightness, i18n( "Causes the back panel brightness to be set" ) );
		QWidget *wp = new QWidget(lidBox);
	        QHBoxLayout *xl = new QHBoxLayout( wp);
		xl->addWidget(new QLabel("-", wp));
		lidValBrightness = new QSlider(0, 255, 16, 255, Qt::Horizontal, wp);
		lidValBrightness->setEnabled(0);
        	QToolTip::add( lidValBrightness, i18n( "How bright the back panel will be set to" ) );
		connect (lidValBrightness, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
		xl->addWidget(lidValBrightness);
		xl->addWidget(new QLabel("+", wp));
		xl->addStretch(1);
      	} 
      	if (has_performance) {
		lidPerformance = new QCheckBox(i18n("System performance"), lidBox);
        	QToolTip::add( lidPerformance, i18n( "Causes the performance profile to be changed" ) );

		QWidget *wp = new QWidget(lidBox);
	        QHBoxLayout *xl = new QHBoxLayout( wp);
    		lidValPerformance = new KComboBox(0, wp);
        	QToolTip::add( lidValPerformance, i18n( "The performance profile to switch to" ) );
		lidValPerformance->insertStringList(performance_list);
    		lidValPerformance->setEnabled(0);
    		connect (lidValPerformance, SIGNAL(activated(int)), this, SLOT(configChanged()));
		xl->addWidget(lidValPerformance);
		xl->addStretch(1);
      	} 
      	if (has_throttle) {
		lidThrottle = new QCheckBox(i18n("CPU throttle"), lidBox);
        	QToolTip::add( lidThrottle, i18n( "Causes the CPU to be throttled back" ) );

		QWidget *wp = new QWidget(lidBox);
	        QHBoxLayout *xl = new QHBoxLayout( wp);
    		lidValThrottle = new KComboBox(0, wp);
        	QToolTip::add( lidValThrottle, i18n( "How much to throttle back the CPU" ) );
		lidValThrottle->insertStringList(throttle_list);
    		lidValThrottle->setEnabled(0);
    		connect (lidValThrottle, SIGNAL(activated(int)), this, SLOT(configChanged()));
		xl->addWidget(lidValThrottle);
		xl->addStretch(1);


      	} 
        connect(lidBox, SIGNAL(clicked(int)), this, SLOT(configChanged()));
      }

      if (laptop_portable::has_button(laptop_portable::PowerButton)) {

      	powerBox = new QVButtonGroup(i18n("Power Switch Pressed"), this);
      	powerBox->layout()->setSpacing( KDialog::spacingHint() );
        QToolTip::add( powerBox, i18n( "Select which actions will occur when the laptop's power button is pressed" ) );
      	hlay->addWidget( powerBox );

      	if (can_standby) {
		powerStandby = new QRadioButton(i18n("Sta&ndby"), powerBox);
        	QToolTip::add( powerStandby, i18n( "Causes the laptop to move into the standby temporary low-power state" ) );
	}
      	if (can_suspend) {
		powerSuspend = new QRadioButton(i18n("S&uspend"), powerBox);
        	QToolTip::add( powerSuspend, i18n( "Causes the laptop to move into the suspend 'save-to-ram' state" ) );
	}
      	if (can_hibernate) {
		powerHibernate = new QRadioButton(i18n("Hi&bernate"), powerBox);
        	QToolTip::add( powerHibernate, i18n( "Causes the laptop to move into the hibernate 'save-to-disk' state" ) );
	}
      	if (can_shutdown) {
		powerShutdown = new QRadioButton(i18n("System power off"), powerBox);
        	QToolTip::add( powerShutdown, i18n( "Causes the laptop to power down" ) );
	}
      	if (can_logout) {
		powerLogout = new QRadioButton(i18n("Logout"), powerBox);
        	QToolTip::add( powerShutdown, i18n( "Causes you to be logged out" ) );
	}
      	if (can_suspend||can_standby||can_hibernate||can_shutdown||can_logout)
		powerOff = new QRadioButton(i18n("O&ff"), powerBox);
      	if (can_brightness) {
		powerBrightness = new QCheckBox(i18n("Brightness"), powerBox);
        	QToolTip::add( powerBrightness, i18n( "Causes the back panel brightness to be set" ) );
		QWidget *wp = new QWidget(powerBox);
	        QHBoxLayout *xl = new QHBoxLayout( wp);
		xl->addWidget(new QLabel("-", wp));
		powerValBrightness = new QSlider(0, 255, 16, 255, Qt::Horizontal, wp);
        	QToolTip::add( powerValBrightness, i18n( "How bright the back panel will be set to" ) );
		powerValBrightness->setEnabled(0);
		connect (powerValBrightness, SIGNAL(valueChanged(int)), this, SLOT(configChanged()));
		xl->addWidget(powerValBrightness);
		xl->addWidget(new QLabel("+", wp));
		xl->addStretch(1);
      	}
      	if (has_performance) {
		powerPerformance = new QCheckBox(i18n("System performance"), powerBox);
        	QToolTip::add( powerPerformance, i18n( "Causes the performance profile to be changed" ) );

		QWidget *wp = new QWidget(powerBox);
	        QHBoxLayout *xl = new QHBoxLayout( wp);
    		powerValPerformance = new KComboBox(0, wp);
        	QToolTip::add( powerValPerformance, i18n( "The performance profile to switch to" ) );
		powerValPerformance->insertStringList(performance_list);
    		powerValPerformance->setEnabled(0);
    		connect (powerValPerformance, SIGNAL(activated(int)), this, SLOT(configChanged()));
		xl->addWidget(powerValPerformance);
		xl->addStretch(1);
      	} 
      	if (has_throttle) {
		powerThrottle = new QCheckBox(i18n("CPU throttle"), powerBox);
        	QToolTip::add( powerThrottle, i18n( "Causes the CPU to be throttled back" ) );

		QWidget *wp = new QWidget(powerBox);
	        QHBoxLayout *xl = new QHBoxLayout( wp);
    		powerValThrottle = new KComboBox(0, wp);
        	QToolTip::add( powerValThrottle, i18n( "How much to throttle back the CPU" ) );
		powerValThrottle->insertStringList(throttle_list);
    		powerValThrottle->setEnabled(0);
    		connect (powerValThrottle, SIGNAL(activated(int)), this, SLOT(configChanged()));
		xl->addWidget(powerValThrottle);
		xl->addStretch(1);
      	} 
      	connect(powerBox, SIGNAL(clicked(int)), this, SLOT(configChanged()));
      }

      hlay->addStretch(1);

      QLabel* explain = new QLabel(i18n("This panel enables actions that are triggered when the lid closure switch "
			    	        "or power switch on your laptop is pressed. Some laptops may already "
					"automatically do things like this, if you cannot disable them in your BIOS "
					"you probably should not enable anything in this panel."), this);
      explain->setAlignment( Qt::WordBreak );
      top_layout->addWidget(explain);

      top_layout->addStretch(1);

      top_layout->addWidget( new QLabel( i18n("Version: %1").arg(LAPTOP_VERSION), this ), 0, Qt::AlignRight );
    }
  }

  load();
}

ButtonsConfig::~ButtonsConfig()
{
  delete config;
}

void ButtonsConfig::save()
{
      	power = getPower();
      	lid = getLid();
	lid_bright_enabled = (lidBrightness?lidBrightness->isChecked():0);
	power_bright_enabled = (powerBrightness?powerBrightness->isChecked():0);
	lid_bright_val = (lidValBrightness?lidValBrightness->value():0);
	power_bright_val = (powerValBrightness?powerValBrightness->value():0);
	lid_performance_enabled = (lidPerformance?lidPerformance->isChecked():0);
	power_performance_enabled = (powerPerformance?powerPerformance->isChecked():0);
	lid_performance_val = (lidValPerformance?lidValPerformance->currentText():"");
	power_performance_val = (powerValPerformance?powerValPerformance->currentText():"");
	lid_throttle_enabled = (lidThrottle?lidThrottle->isChecked():0);
	power_throttle_enabled = (powerThrottle?powerThrottle->isChecked():0);
	lid_throttle_val = (lidValThrottle?lidValThrottle->currentText():"");
	power_throttle_val = (powerValThrottle?powerValThrottle->currentText():"");

  config->setGroup("LaptopButtons");
  config->writeEntry("LidSuspend", lid);
  config->writeEntry("PowerSuspend", power);
  config->writeEntry("PowerBrightnessEnabled", power_bright_enabled);
  config->writeEntry("LidBrightnessEnabled", lid_bright_enabled);
  config->writeEntry("PowerBrightness", power_bright_val);
  config->writeEntry("LidBrightness", lid_bright_val);
  config->writeEntry("PowerPerformanceEnabled", power_performance_enabled);
  config->writeEntry("LidPerformanceEnabled", lid_performance_enabled);
  config->writeEntry("PowerPerformance", power_performance_val);
  config->writeEntry("LidPerformance", lid_performance_val);
  config->writeEntry("PowerThrottleEnabled", power_throttle_enabled);
  config->writeEntry("LidThrottleEnabled", lid_throttle_enabled);
  config->writeEntry("PowerThrottle", power_throttle_val);
  config->writeEntry("LidThrottle", lid_throttle_val);
  config->sync();
  changed(false);
  wake_laptop_daemon();
}

void ButtonsConfig::load()
{
	load( false );
}

void ButtonsConfig::load(bool useDefaults)
{
  config->setReadDefaults( useDefaults );

  config->setGroup("LaptopButtons");
  lid = config->readNumEntry("LidSuspend", 0);
  power = config->readNumEntry("PowerSuspend", 0);
  lid_bright_enabled = config->readBoolEntry("LidBrightnessEnabled", 0);
  power_bright_enabled = config->readBoolEntry("PowerBrightnessEnabled", 0);
  lid_bright_val = config->readNumEntry("LidBrightness", 0);
  power_bright_val = config->readNumEntry("PowerBrightness", 0);
  lid_performance_enabled = config->readBoolEntry("LidPerformanceEnabled", 0);
  power_performance_enabled = config->readBoolEntry("PowerPerformanceEnabled", 0);
  lid_performance_val = config->readEntry("LidPerformance", "");
  power_performance_val = config->readEntry("PowerPerformance", "");
  lid_throttle_enabled = config->readBoolEntry("LidThrottleEnabled", 0);
  power_throttle_enabled = config->readBoolEntry("PowerThrottleEnabled", 0);
  lid_throttle_val = config->readEntry("LidThrottle", "");
  power_throttle_val = config->readEntry("PowerThrottle", "");

  // the GUI should reflect the real values
   	setPower(power, lid);
	if (lidBrightness) 
		lidBrightness->setChecked(lid_bright_enabled);
	if (powerBrightness) 
		powerBrightness->setChecked(power_bright_enabled);
	if (lidValBrightness) {
		lidValBrightness->setValue(lid_bright_val);
		lidValBrightness->setEnabled(lid_bright_enabled);
	}
	if (powerValBrightness) {
		powerValBrightness->setValue(power_bright_val);
		powerValBrightness->setEnabled(power_bright_enabled);
	}
	if (lidPerformance) 
		lidPerformance->setChecked(lid_performance_enabled);
	if (powerPerformance) 
		powerPerformance->setChecked(power_performance_enabled);
	if (lidValPerformance) {
		int ind = 0;
		for (int i = 0; i < lidValPerformance->count(); i++)
		if (lidValPerformance->text(i) == lid_performance_val) {
			ind = i;
			break;
		}
		lidValPerformance->setCurrentItem(ind);
		lidValPerformance->setEnabled(lid_performance_enabled);
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
	if (lidThrottle) 
		lidThrottle->setChecked(lid_throttle_enabled);
	if (powerThrottle) 
		powerThrottle->setChecked(power_throttle_enabled);
	if (lidValThrottle) {
		int ind = 0;
		for (int i = 0; i < lidValThrottle->count(); i++)
		if (lidValThrottle->text(i) == lid_throttle_val) {
			ind = i;
			break;
		}
		lidValThrottle->setCurrentItem(ind);
		lidValThrottle->setEnabled(lid_throttle_enabled);
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
  emit changed( useDefaults );
}

void ButtonsConfig::defaults()
{
  setPower(0, 0);
  lid_bright_enabled = 0;
  power_bright_enabled = 0;
  lid_bright_val = 0;
  power_bright_val = 0;
  lid_performance_enabled = 0;
  power_performance_enabled = 0;
  lid_performance_val = "";
  power_performance_val = "";
  lid_throttle_enabled = 0;
  power_throttle_enabled = 0;
  lid_throttle_val = "";
  power_throttle_val = "";
	if (lidBrightness) 
		lidBrightness->setChecked(lid_bright_enabled);
	if (powerBrightness) 
		powerBrightness->setChecked(power_bright_enabled);
	if (lidValBrightness) {
		lidValBrightness->setValue(lid_bright_val);
		lidValBrightness->setEnabled(lid_bright_enabled);
	}
	if (powerValBrightness) {
		powerValBrightness->setValue(power_bright_val);
		powerValBrightness->setEnabled(power_bright_enabled);
	}


	if (lidPerformance) 
		lidPerformance->setChecked(lid_performance_enabled);
	if (powerPerformance) 
		powerPerformance->setChecked(power_performance_enabled);
	if (lidValPerformance) {
		lidValPerformance->setCurrentItem(0);
		lidValPerformance->setEnabled(lid_performance_enabled);
	}
	if (powerValPerformance) {
		powerValPerformance->setCurrentItem(0);
		powerValPerformance->setEnabled(power_performance_enabled);
	}
	if (lidThrottle) 
		lidThrottle->setChecked(lid_throttle_enabled);
	if (powerThrottle) 
		powerThrottle->setChecked(power_throttle_enabled);
	if (lidValThrottle) {
		lidValThrottle->setCurrentItem(0);
		lidValThrottle->setEnabled(lid_throttle_enabled);
	}
	if (powerValThrottle) {
		powerValThrottle->setCurrentItem(0);
		powerValThrottle->setEnabled(power_throttle_enabled);
	}
}

int  ButtonsConfig::getLid()
{
  if (!apm)
	return(lid);
  if (lidHibernate && lidHibernate->isChecked())
    return 3;
  if (lidStandby && lidStandby->isChecked())
    return 1;
  if (lidSuspend && lidSuspend->isChecked())
    return 2;
  if (lidLogout && lidLogout->isChecked())
    return 4;
  if (lidShutdown && lidShutdown->isChecked())
    return 5;
  return(0);
}

int  ButtonsConfig::getPower()
{
  if (!apm)
	return(power);
  if (powerHibernate && powerHibernate->isChecked())
    return 3;
  if (powerStandby && powerStandby->isChecked())
    return 1;
  if (powerSuspend && powerSuspend->isChecked())
    return 2;
  if (powerLogout && powerLogout->isChecked())
    return 4;
  if (powerShutdown && powerShutdown->isChecked())
    return 5;
  return(0);
}

void ButtonsConfig::setPower(int p, int np)
{
  if (!apm)
	return;
  if (lidSuspend) {
	lidSuspend->setChecked(FALSE);
  } else {
	if (np == 2) np = 0;
  }
  if (lidShutdown) {
	lidShutdown->setChecked(FALSE);
  } else {
	if (np == 5) np = 0;
  }
  if (lidLogout) {
	lidLogout->setChecked(FALSE);
  } else {
	if (np == 4) np = 0;
  }
  if (lidStandby) {
	lidStandby->setChecked(FALSE);
  } else {
	if (np == 1) np = 0;
  }
  if (lidHibernate) {
	lidHibernate->setChecked(FALSE);
  } else {
	if (np == 3) np = 0;
  }
  if (lidOff)
	  lidOff->setChecked(FALSE);
  switch (np) {
  case 0: if (lidOff)
		  lidOff->setChecked(TRUE);
	  break;
  case 1: lidStandby->setChecked(TRUE);break;
  case 2: lidSuspend->setChecked(TRUE);break;
  case 3: lidHibernate->setChecked(TRUE);break;
  case 4: lidLogout->setChecked(TRUE);break;
  case 5: lidShutdown->setChecked(TRUE);break;
  }
  if (powerSuspend) {
	powerSuspend->setChecked(FALSE);
  } else {
	if (p == 2) p = 0;
  }
  if (powerLogout) {
	powerLogout->setChecked(FALSE);
  } else {
	if (p == 4) p = 0;
  }
  if (powerShutdown) {
	powerShutdown->setChecked(FALSE);
  } else {
	if (p == 5) p = 0;
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
  case 0: if (powerOff)
		  powerOff->setChecked(TRUE);
	  break;
  case 1: powerStandby->setChecked(TRUE);break;
  case 2: powerSuspend->setChecked(TRUE);break;
  case 3: powerHibernate->setChecked(TRUE);break;
  case 4: powerLogout->setChecked(TRUE);break;
  case 5: powerShutdown->setChecked(TRUE);break;
  }
}



void ButtonsConfig::configChanged()
{
  if (powerBrightness) 
	powerValBrightness->setEnabled(powerBrightness->isChecked());
  if (lidBrightness) 
	lidValBrightness->setEnabled(lidBrightness->isChecked());

  if (powerPerformance) 
	powerValPerformance->setEnabled(powerPerformance->isChecked());
  if (lidPerformance) 
	lidValPerformance->setEnabled(lidPerformance->isChecked());

  if (powerThrottle) 
	powerValThrottle->setEnabled(powerThrottle->isChecked());
  if (lidThrottle) 
	lidValThrottle->setEnabled(lidThrottle->isChecked());

  emit changed(true);
}


QString ButtonsConfig::quickHelp() const
{
  return i18n("<h1>Laptop Power Control</h1>This module allows you to "
	"configure the power switch or lid closure switch on your laptop "
	"so they can trigger system actions");

}

#include "buttons.moc"
