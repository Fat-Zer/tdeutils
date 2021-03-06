/*
 * warning.cpp
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
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

#include "warning.h"
#include "portable.h"
#include "version.h"

#include <tdelocale.h>
#include <tdefiledialog.h>
#include <tdemessagebox.h>
#include <tdeconfig.h>
#include <kurlrequester.h>
#include <tdeapplication.h>
#include <kcombobox.h>

#include <tqlayout.h>
#include <tqcheckbox.h>
#include <tqspinbox.h>
#include <tqslider.h>
#include <tqhbox.h>
#include <tqvbuttongroup.h>
#include <tqradiobutton.h>
#include <tqtooltip.h>

extern void wake_laptop_daemon();

WarningConfig::WarningConfig (int t, TQWidget * parent, const char *name)
  : TDECModule(parent, name),
    checkSuspend(0),
    checkStandby(0),
    checkHibernate(0)
{
  TDEGlobal::locale()->insertCatalogue("klaptopdaemon"); // For translation of klaptopdaemon messages

  type = t;
  apm = laptop_portable::has_power_management();
  config =  new TDEConfig("kcmlaptoprc");

  my_load(0);

  if (!apm) {
    TQVBoxLayout *top_layout = new TQVBoxLayout( this, KDialog::marginHint(),
					       KDialog::spacingHint() );
	
    KActiveLabel* explain = laptop_portable::no_power_management_explanation(this);
    top_layout->addWidget(explain, 0);

    top_layout->addStretch(1);
  } else {
    TQGridLayout *grid = new TQGridLayout( this, 11, 2, /* rows x cols */
					 KDialog::marginHint(),
					 KDialog::spacingHint() );
    grid->setColStretch( 1, 1 );

    int curRow = 0;
    // setup the trigger stuff:
    if (type) { 
        checkCriticalTime = new TQCheckBox( i18n("Critical &trigger:"), this );
        checkCriticalPercent = new TQCheckBox( i18n("Critical &trigger:"), this );
        editCriticalTime = new TQSpinBox(1, 60*24, 1, this);
        editCriticalTime->setSuffix(i18n("keep short, unit in spinbox", "min"));
        TQToolTip::add(editCriticalTime, i18n("When this amount of battery life is left the actions below will be triggered"));
        editCriticalPercent = new TQSpinBox(1, 100, 1, this);
        editCriticalPercent->setSuffix(i18n("keep short, unit in spinbox", "%"));
        TQToolTip::add(editCriticalPercent, i18n("When this amount of battery life is left the actions below will be triggered"));
        grid->addWidget(checkCriticalTime, curRow, 0);
        grid->addWidget(editCriticalTime, curRow++, TQt::AlignLeft);
        grid->addWidget(checkCriticalPercent, curRow, 0);
        grid->addWidget(editCriticalPercent, curRow++, TQt::AlignLeft);

        connect(editCriticalTime, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
        connect(editCriticalPercent, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
        connect(checkCriticalTime, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(configChanged()));
        connect(checkCriticalPercent, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(configChanged()));
        connect(checkCriticalTime, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(checkCriticalTimeChanged(bool)));
        connect(checkCriticalPercent, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(checkCriticalPercentChanged(bool)));
    } else {
        checkLowTime = new TQCheckBox( i18n("Low &trigger:"), this );
        checkLowPercent = new TQCheckBox( i18n("Low &trigger:"), this );        
        editLowTime = new TQSpinBox(1, 60*24, 1, this);
        editLowTime->setSuffix(i18n("keep short, unit in spinbox", "min"));
        TQToolTip::add(editLowTime, i18n("When this amount of battery life is left the actions below will be triggered"));
        editLowPercent = new TQSpinBox(1, 100, 1, this);
        editLowPercent->setSuffix(i18n("keep short, unit in spinbox", "%"));
        TQToolTip::add(editLowPercent, i18n("When this amount of battery life is left the actions below will be triggered"));
        grid->addWidget(checkLowTime, curRow, 0);
        grid->addWidget(editLowTime, curRow++, TQt::AlignLeft);
        grid->addWidget(checkLowPercent, curRow, 0);
        grid->addWidget(editLowPercent, curRow++, TQt::AlignLeft);
        
        connect(editLowTime, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
        connect(editLowPercent, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
        connect(checkLowTime, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(configChanged()));
        connect(checkLowPercent, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(configChanged()));
        connect(checkLowTime, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(checkLowTimeChanged(bool)));
        connect(checkLowPercent, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(checkLowPercentChanged(bool)));
    }

    
    // setup the Run Command stuff
    checkRunCommand = new TQCheckBox(i18n("Run &command:"), this);
    grid->addWidget(checkRunCommand, curRow, 0);

    editRunCommand = new KURLRequester( this );
    editRunCommand->setEnabled(false);
    connect( checkRunCommand, TQT_SIGNAL(toggled(bool)),
	     editRunCommand,  TQT_SLOT(setEnabled(bool)) );
    connect( checkRunCommand, TQT_SIGNAL(clicked()),
	     this, TQT_SLOT(configChanged()) );
    connect( editRunCommand, TQT_SIGNAL(textChanged(const TQString&)),
	     this, TQT_SLOT(configChanged()) );
    grid->addWidget(editRunCommand, curRow++, 1);
    TQToolTip::add( editRunCommand, i18n( "This command will be run when the battery gets low" ) );

    // setup the Play Sound stuff
    checkPlaySound = new TQCheckBox(i18n("&Play sound:"), this);
    grid->addWidget(checkPlaySound, curRow, 0);

    editPlaySound = new KURLRequester( this );
    editPlaySound->setEnabled(false);
    connect( checkPlaySound, TQT_SIGNAL(toggled(bool)),
	     editPlaySound,  TQT_SLOT(setEnabled(bool)) );
    connect( checkPlaySound, TQT_SIGNAL(clicked()),
	     this, TQT_SLOT(configChanged()) );
    connect( editPlaySound, TQT_SIGNAL(textChanged(const TQString&)),
	     this, TQT_SLOT(configChanged()) );
    grid->addWidget(editPlaySound, curRow++, 1);
    TQToolTip::add( editPlaySound, i18n( "This sound will play when the battery gets low" ) );

    // setup the System Sound stuff
    checkBeep = new TQCheckBox(i18n("System &beep"), this);
    grid->addWidget(checkBeep, curRow++, 0);
    connect(checkBeep, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()));
    TQToolTip::add( checkBeep, i18n( "The system will beep if this is enabled" ) );

    checkNotify = new TQCheckBox(i18n("&Notify"), this);
    grid->addWidget(checkNotify, curRow++, 0);
    connect(checkNotify, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()));

    int can_suspend = laptop_portable::has_suspend();
    int can_standby = laptop_portable::has_standby();
    int can_hibernate = laptop_portable::has_hibernation();
    int can_brightness = laptop_portable::has_brightness();

    if (can_brightness) {
      checkBrightness = new TQCheckBox(i18n("Panel b&rightness"), this);
      checkBrightness->setMinimumSize(checkBrightness->sizeHint());
      TQToolTip::add( checkBrightness, i18n( "If enabled the back panel brightness will change" ) );
      grid->addWidget(checkBrightness, curRow, 0);
      connect(checkBrightness, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(brightness_changed(bool)));
      TQHBoxLayout *v = new TQHBoxLayout();
      v->addWidget(new TQLabel("-", this));
      valueBrightness = new TQSlider(0, 255, 16, 160, Qt::Horizontal, this);
      TQToolTip::add( valueBrightness, i18n( "How bright or dim to make the back panel" ) );
      valueBrightness->setMaximumWidth(70);
      v->addWidget(valueBrightness);
      v->addWidget(new TQLabel("+", this));
      v->addStretch(1);
      grid->addLayout(v, curRow, 1);
      valueBrightness->setEnabled(0);
      connect(valueBrightness, TQT_SIGNAL(valueChanged(int)), this, TQT_SLOT(configChanged()));
      ++curRow;
    } else {
      checkBrightness = 0;
      valueBrightness = 0;
    }
    TQStringList performance_list;
    int current_performance;
    bool *active_list;
    bool has_performance = laptop_portable::get_system_performance(0, current_performance, performance_list, active_list);
    if (has_performance) {
    	performance = new TQCheckBox(i18n("System performance"), this);
        TQToolTip::add( performance, i18n( "If enabled the laptop's power performance profile will change" ) );
        grid->addWidget(performance, curRow, 0);
    	connect (performance, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(performance_changed(bool)));

        TQHBoxLayout *v = new TQHBoxLayout();
    	performance_val = new KComboBox(0, this);
	performance_val->insertStringList(performance_list);
    	performance_val->setEnabled(0);
    	connect (performance_val, TQT_SIGNAL(activated(int)), this, TQT_SLOT(configChanged()));
        TQToolTip::add( performance_val, i18n( "The performance profile to change to" ) );
	v->addWidget(performance_val);
	v->addStretch(1);
        grid->addLayout(v, curRow, 1);
	++curRow;
    } else {
        performance = 0;
        performance_val = 0;
    }
    TQStringList throttle_list;
    int current_throttle;
    bool has_throttle = laptop_portable::get_system_throttling(0, current_throttle, throttle_list, active_list);
    if (has_throttle) {
    	throttle = new TQCheckBox(i18n("CPU throttling"), this);
        TQToolTip::add( throttle, i18n( "If enabled the CPU performance will be throttled" ) );
        grid->addWidget(throttle, curRow, 0);
    	connect (throttle, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(throttle_changed(bool)));

        TQHBoxLayout *v = new TQHBoxLayout();
    	throttle_val = new KComboBox(0, this);
	throttle_val->insertStringList(throttle_list);
    	throttle_val->setEnabled(0);
    	connect (throttle_val, TQT_SIGNAL(activated(int)), this, TQT_SLOT(configChanged()));
        TQToolTip::add( throttle_val, i18n( "How much to throttle the CPU performance by" ) );
	v->addWidget(throttle_val);
	v->addStretch(1);
        grid->addLayout(v, curRow, 1);
	++curRow;
    } else {
        throttle = 0;
        throttle_val = 0;
    }



    TQVButtonGroup *b = new TQVButtonGroup(i18n("System State Change"), this);
    TQToolTip::add( b, i18n( "You may choose one of the following to occur when the battery gets low" ) );
    b->layout()->setSpacing( KDialog::spacingHint() );
    if (can_standby) {
      checkStandby = new TQRadioButton(i18n("Standb&y"), b);
      TQToolTip::add( checkStandby, i18n( "Move the system into the standby state - a temporary lower power state" ) );
      checkStandby->setMinimumSize(checkStandby->sizeHint());
      connect(checkStandby, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()));
    }
    if (can_suspend) {
      checkSuspend = new TQRadioButton(i18n("&Suspend"), b);
      TQToolTip::add( checkSuspend, i18n( "Move the system into the suspend state - also known as 'save-to-ram'" ) );
      checkSuspend->setMinimumSize(checkSuspend->sizeHint());
      connect(checkSuspend, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()));
    }
    if (can_hibernate) {
      checkHibernate = new TQRadioButton(i18n("H&ibernate"), b);
      TQToolTip::add( checkHibernate, i18n( "Move the system into the hibernate state - also known as 'save-to-disk'" ) );
      checkHibernate->setMinimumSize(checkHibernate->sizeHint());
      connect(checkHibernate, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()));
    }
    // setup the logout option
    checkLogout = new TQRadioButton(i18n("&Logout"), b);
    connect(checkLogout, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()));
    // setup the shutdown option
    checkShutdown = new TQRadioButton(i18n("System power off"), b);
    TQToolTip::add( checkShutdown, i18n( "Power the laptop off" ) );
    connect(checkShutdown, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()));
    
    checkNone = new TQRadioButton(i18n("&None"), b);
    connect(checkNone, TQT_SIGNAL(clicked()), this, TQT_SLOT(configChanged()));

    grid->addMultiCellWidget(b, curRow, curRow, 0, 1, TQt::AlignLeft|TQt::AlignTop);
    curRow++;


    TQLabel* explain;
    if (type) {
      explain = new TQLabel(i18n("This panel controls how and when you receive warnings that your battery power is going to run out VERY VERY soon."), this);
    } else {
      explain = new TQLabel(i18n("This panel controls how and when you receive warnings that your battery power is about to run out"), this);
    }
    explain->setAlignment( TQt::WordBreak );
    grid->addMultiCellWidget(explain, curRow, curRow, 0, 1);
    ++curRow;

    if (!can_suspend && !can_standby && !can_hibernate) {
      // display help text:
      TQLabel* note = laptop_portable::how_to_do_suspend_resume(this);
      grid->addMultiCellWidget(note, curRow, curRow, 0, 1);
      ++curRow;
    }
    grid->setRowStretch(curRow++, 1);

    grid->addWidget(new TQLabel( i18n("Version: %1").arg(LAPTOP_VERSION), this),
		    curRow, 1, TQt::AlignRight);

  }
  my_load(1);
}

WarningConfig::~WarningConfig()
{
  delete config;
}

void WarningConfig::brightness_changed(bool v)
{
  valueBrightness->setEnabled(v);
  configChanged();
}

void WarningConfig::throttle_changed(bool v)
{
  throttle_val->setEnabled(v);
  configChanged();
}

void WarningConfig::performance_changed(bool v)
{
  performance_val->setEnabled(v);
  configChanged();
}

void WarningConfig::checkLowTimeChanged(bool state)
{
  checkLowPercent->setChecked(state ? false : true);
  editLowPercent->setEnabled(state ? false : true);
}

void WarningConfig::checkLowPercentChanged(bool state)
{
  checkLowTime->setChecked(state ? false : true);
  editLowTime->setEnabled(state ? false : true);
}

void WarningConfig::checkCriticalTimeChanged(bool state)
{
  checkCriticalPercent->setChecked(state ? false : true);
  editCriticalPercent->setEnabled(state ? false : true);
}

void WarningConfig::checkCriticalPercentChanged(bool state)
{
  checkCriticalTime->setChecked(state ? false: true);
  editCriticalTime->setEnabled(state ? false : true);
}


void WarningConfig::save()
{
    if (apm) {
    	runcommand = checkRunCommand->isChecked();
    	playsound = checkPlaySound->isChecked();
    	logout = checkLogout->isChecked();
    	shutdown = checkShutdown->isChecked();
    	beep = checkBeep->isChecked();
    	notify = checkNotify->isChecked();
    	do_suspend = (checkSuspend? checkSuspend->isChecked() : false);
    	do_standby = (checkStandby? checkStandby->isChecked() : false);
    	do_hibernate = (checkHibernate? checkHibernate->isChecked() : false);
    	do_brightness = (checkBrightness? checkBrightness->isChecked() : false);
    	val_brightness = (valueBrightness? valueBrightness->value() : 255);
    	do_performance = (performance? performance->isChecked() : false);
    	val_performance = (performance_val? performance_val->currentText() : "");
    	do_throttle = (throttle? throttle->isChecked() : false);
    	val_throttle = (throttle_val? throttle_val->currentText() : "");
    	runcommand_val = editRunCommand->url();
        if (type) {
          time_based_action_critical = checkCriticalTime->isChecked();
          critical_val_time = editCriticalTime->value();
          critical_val_percent = editCriticalPercent->value();
        } else {
          time_based_action_low = checkLowTime->isChecked();
          low_val_time = editLowTime->value();
          low_val_percent = editLowPercent->value();
        }
        
    	sound_val = editPlaySound->url();
    }
    config->setGroup((type?"BatteryCritical":"BatteryLow"));

    if (config->group() == "BatteryLow") {
        config->writeEntry("TimeBasedAction", time_based_action_low);
        config->writeEntry("LowValTime", low_val_time);
        config->writeEntry("LowValPercent", low_val_percent);
    } else {
        config->writeEntry("TimeBasedAction", time_based_action_critical);
        config->writeEntry("CriticalValTime", critical_val_time);
        config->writeEntry("CriticalValPercent", critical_val_percent);
    }
    config->writeEntry("RunCommand", runcommand);
    config->writeEntry("PlaySound", playsound);
    config->writeEntry("Logout", logout);
    config->writeEntry("Shutdown", shutdown);
    config->writeEntry("SystemBeep", beep);
    config->writeEntry("Notify", notify);
    config->writeEntry("Suspend", do_suspend);
    config->writeEntry("Standby", do_standby);
    config->writeEntry("Hibernate", do_hibernate);
    config->writeEntry("Brightness", do_brightness);
    config->writeEntry("BrightnessValue", val_brightness);
    config->writeEntry("Performance", do_performance);
    config->writeEntry("PerformanceValue", val_performance);
    config->writeEntry("Throttle", do_throttle);
    config->writeEntry("ThrottleValue", val_throttle);
    config->writeEntry("RunCommandPath", runcommand_val);
    config->writeEntry("PlaySoundPath", sound_val);
    config->sync();
    wake_laptop_daemon();
}

void WarningConfig::load()
{
	load( false );
}

void WarningConfig::load(bool useDefaults)
{
	if (config == NULL)
		return;
	my_load(0, useDefaults);
	my_load(1, useDefaults);
}

void WarningConfig::my_load(int x, bool useDefaults )
{
	config->setReadDefaults( useDefaults );

	// open the config file
	if (!x) {
		config->setGroup((type?"BatteryCritical":"BatteryLow"));

                if (config->group() == "BatteryLow") {
                    time_based_action_low = config->readBoolEntry("TimeBasedAction", true);
                    low_val_time = config->readNumEntry("LowValTime", 15);
                    low_val_percent = config->readNumEntry("LowValPercent", 7);
                } else {
                    time_based_action_critical = config->readBoolEntry("TimeBasedAction", true);
                    critical_val_time = config->readNumEntry("CriticalValTime", 5);
                    critical_val_percent = config->readNumEntry("CriticalValPercent", 3);
                }
		runcommand = config->readBoolEntry("RunCommand", false);
		playsound = config->readBoolEntry("PlaySound", false);
		logout = config->readBoolEntry("Logout", false);
		shutdown = config->readBoolEntry("Shutdown", false);
		beep = config->readBoolEntry("SystemBeep", true);
		notify = config->readBoolEntry("Notify", (type && checkSuspend ? false : true));
		do_suspend = config->readBoolEntry("Suspend", (type && checkSuspend ? true :false));
		do_standby = config->readBoolEntry("Standby", false);
		do_hibernate = config->readBoolEntry("Hibernate", false);
		do_brightness = config->readBoolEntry("Brightness", false);
		val_brightness = config->readNumEntry("BrightnessValue", 255);
		do_performance = config->readBoolEntry("Performance", false);
		val_performance = config->readEntry("PerformanceValue", "");
		do_throttle = config->readBoolEntry("Throttle", false);
		val_throttle = config->readEntry("ThrottleValue", "");
		runcommand_val = config->readEntry("RunCommandPath");
		sound_val = config->readEntry("PlaySoundPath");
		have_time = config->readNumEntry("HaveTime", 2);
		if (laptop_portable::has_power_management())
			have_time = laptop_portable::has_battery_time();
	
	} else
	if (apm) {
		checkRunCommand->setChecked(runcommand);
		checkPlaySound->setChecked(playsound);
		checkBeep->setChecked(beep);
		if (checkBrightness)
			checkBrightness->setChecked(do_brightness);
		if (valueBrightness) {
			valueBrightness->setValue(val_brightness);
  			valueBrightness->setEnabled(do_brightness);
		}
		if (performance)
			performance->setChecked(do_performance);
		if (performance_val) {
			int ind = 0;
			for (int i = 0; i < performance_val->count(); i++)
			if (performance_val->text(i) == val_performance) {
				ind = i;
				break;
			}
			performance_val->setCurrentItem(ind);
  			performance_val->setEnabled(do_performance);
		}
		if (throttle)
			throttle->setChecked(do_throttle);
		if (throttle_val) {
			int ind = 0;
			for (int i = 0; i < throttle_val->count(); i++)
			if (throttle_val->text(i) == val_throttle) {
				ind = i;
				break;
			}
			throttle_val->setCurrentItem(ind);
  			throttle_val->setEnabled(do_throttle);
		}
		
		checkLogout->setChecked(logout);
		checkNotify->setChecked(notify);
		checkShutdown->setChecked(shutdown);
		if (checkHibernate) {
			checkHibernate->setChecked(do_hibernate);
		} else {
			do_hibernate = 0;
		}
		if (checkStandby) {
			checkStandby->setChecked(do_standby);
		} else {
			do_standby = 0;
		}
		if (checkSuspend) {
			checkSuspend->setChecked(do_suspend);
		} else {
			do_suspend = 0;
		}
		checkNone->setChecked(!do_suspend&!do_standby&!do_hibernate&!logout&!shutdown);
		editRunCommand->setURL(runcommand_val);

                if(type) {
                checkCriticalTime->setChecked(time_based_action_critical ? true : false);
                editCriticalTime->setValue(critical_val_time);
                checkCriticalPercent->setChecked(time_based_action_critical ? false : true);
                editCriticalPercent->setValue(critical_val_percent);
                } else {
                  checkLowTime->setChecked(time_based_action_low ? true : false);
                  editLowTime->setValue(low_val_time);
                  checkLowPercent->setChecked(time_based_action_low ? false : true);
                  editLowPercent->setValue(low_val_percent);
                }
                
                editPlaySound->setURL(sound_val);
	}
	emit changed(useDefaults);
}

void WarningConfig::defaults()
{
	load( true );
}


void WarningConfig::configChanged()
{
  emit changed(true);
}


#if 0
void WarningConfig::enableRunCommand(bool enable)
{
	editRunCommand->setEnabled(enable);
	buttonBrowseRunCommand->setEnabled(enable);
	configChanged();
}

void WarningConfig::enablePlaySound(bool enable)
{
	editPlaySound->setEnabled(enable);
	buttonBrowsePlaySound->setEnabled(enable);
	configChanged();
}

void WarningConfig::browseRunCommand()
{
  KURL url = KFileDialog::getOpenURL(TQString(), TQString(), this  );
  
  if( url.isEmpty() )
    return;
  
  if( !url.isLocalFile() )
  {
    KMessageBox::sorry( 0L, i18n( "Only local files are currently supported." ) );
    return;
  }
  
  editRunCommand->setText( url.path() );
  configChanged();
}

void WarningConfig::browsePlaySound()
{
  KURL url = KFileDialog::getOpenURL(TQString(), TQString(), this  );
  
  if( url.isEmpty() )
    return;
  
  if( !url.isLocalFile() )
  {
    KMessageBox::sorry( 0L, i18n( "Only local files are currently supported." ) );
    return;
  }
  
  editPlaySound->setText( url.path() );
  configChanged();
}
#endif

TQString WarningConfig::quickHelp() const
{
  return i18n("<h1>Low battery Warning</h1>This module allows you to "
	"set an alarm in case your battery's charge is about to run out.");
}

#include "warning.moc"
