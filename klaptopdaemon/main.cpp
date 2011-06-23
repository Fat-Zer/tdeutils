/*
 * main.cpp
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


#include <klocale.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kglobal.h>
#include <tqlayout.h>
#include <unistd.h>
#include <fcntl.h>

#include "main.h"
#include "version.h"
#include "warning.h"
#include "power.h"
#include "battery.h"
#include "buttons.h"
#include "pcmcia.h"
#include "acpi.h"
#include "apm.h"
#include "sony.h"
#include "profile.h"
#include "portable.h"
extern void wake_laptop_daemon();


extern "C"
{

  KDE_EXPORT KCModule *create_pcmcia(TQWidget *tqparent, const char *)
  {
    return new PcmciaConfig(tqparent, "kcmlaptop");
  }

  KDE_EXPORT KCModule *create_bwarning(TQWidget *tqparent, const char *)
  {
    return new WarningConfig(0, tqparent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_cwarning(TQWidget *tqparent, const char *)
  {
    return new WarningConfig(1, tqparent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_battery(TQWidget *tqparent, const char *)
  {
    return new BatteryConfig(tqparent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_power(TQWidget *tqparent, const char *)
  {
    return new PowerConfig(tqparent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_acpi(TQWidget *tqparent, const char *)
  {
    return new AcpiConfig(tqparent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_apm(TQWidget *tqparent, const char *)
  {
    return new ApmConfig(tqparent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_Profile(TQWidget *tqparent, const char *)
  {
    return new ProfileConfig(tqparent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_sony(TQWidget *tqparent, const char *)
  {
    return new SonyConfig(tqparent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_buttons(TQWidget *tqparent, const char *)
  {
    return new ButtonsConfig(tqparent, "kcmlaptop");
  }

  KDE_EXPORT void init_battery()
  {
    KConfig config("kcmlaptoprc", true /*readonly*/, false /*no globals*/);
    config.setGroup("BatteryDefault");
    bool enable = false;
    if (!config.hasKey("Enable")) {  // if they have APM or PCMCIA, Enable=true
	struct power_result pr = laptop_portable::poll_battery_state();
	if ((laptop_portable::has_power_management() &&
             !(pr.powered &&
              (pr.percentage < 0 || pr.percentage == 0xff)))||
	    0 == access("/var/run/stab", R_OK|F_OK) ||
	    0 == access("/var/lib/pcmcia/stab", R_OK|F_OK))
		enable = true;
    } else {
	    enable = config.readBoolEntry("Enable", false);
    }
    if (!enable)
      return;
     wake_laptop_daemon();
  }

  KDE_EXPORT KCModule *create_laptop(TQWidget *tqparent, const char *)
  {
	return new LaptopModule(tqparent, "kcmlaptop");
  }

  KDE_EXPORT void init_laptop()
  {
	init_battery();
  }
}



LaptopModule::LaptopModule(TQWidget *tqparent, const char *)
  : KCModule(tqparent, "kcmlaptop")
{
  {	// export ACPI options
    KConfig config("kcmlaptoprc", true /*readonly*/, false /*no globals*/);
    config.setGroup("AcpiDefault");

    bool enablestandby = config.readBoolEntry("EnableStandby", false);
    bool enablesuspend = config.readBoolEntry("EnableSuspend", false);
    bool enablehibernate = config.readBoolEntry("EnableHibernate", false);
    bool enableperformance = config.readBoolEntry("EnablePerformance", false);
    bool enablethrottle = config.readBoolEntry("EnableThrottle", false);
    laptop_portable::acpi_set_tqmask(enablestandby, enablesuspend, enablehibernate, enableperformance, enablethrottle);

    config.setGroup("ApmDefault");

    enablestandby = config.readBoolEntry("EnableStandby", false);
    enablesuspend = config.readBoolEntry("EnableSuspend", false);
    laptop_portable::apm_set_tqmask(enablestandby, enablesuspend);
    config.setGroup("SoftwareSuspendDefault");
    enablehibernate = config.readBoolEntry("EnableHibernate", false);
    laptop_portable::software_suspend_set_tqmask(enablehibernate);
  }
  TQVBoxLayout *tqlayout = new TQVBoxLayout(this);
  tab = new TQTabWidget(this);
  tqlayout->addWidget(tab);

  battery = new BatteryConfig(tqparent, "kcmlaptop");
  tab->addTab(battery, i18n("&Battery"));
  connect(battery, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));

  power = new PowerConfig(tqparent, "kcmlaptop");
  tab->addTab(power, i18n("&Power Control"));
  connect(power, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));

  warning = new WarningConfig(0, tqparent, "kcmlaptop");
  tab->addTab(warning, i18n("Low Battery &Warning"));
  connect(warning, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));

  critical = new WarningConfig(1, tqparent, "kcmlaptop");
  tab->addTab(critical, i18n("Low Battery &Critical"));
  connect(critical, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));

    TQStringList profile_list;
    int current_profile;
    bool *active_list;
    bool has_profile = laptop_portable::get_system_performance(0, current_profile, profile_list, active_list);
    TQStringList throttle_list;
    int current_throttle;
    bool has_throttling = laptop_portable::get_system_throttling(0, current_throttle, throttle_list, active_list);
  if (laptop_portable::has_brightness() || has_profile || has_throttling) {
  	profile = new ProfileConfig(tqparent, "kcmlaptop");
  	tab->addTab(profile, i18n("Default Power Profiles"));
  	connect(profile, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));
  } else {
	profile = 0;
  }
  if (laptop_portable::has_button(laptop_portable::LidButton) || laptop_portable::has_button(laptop_portable::PowerButton)) {
  	buttons = new ButtonsConfig(tqparent, "kcmlaptop");
  	tab->addTab(buttons, i18n("Button Actions"));
  	connect(buttons, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));
  } else {
        buttons = 0;
  }
  if (laptop_portable::has_acpi()) {
  	acpi = new AcpiConfig(tqparent, "kcmlaptop");
  	tab->addTab(acpi, i18n("&ACPI Config"));
  	connect(acpi, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));
  } else {
        acpi = 0;
  }
  if (laptop_portable::has_apm()) {
  	apm = new ApmConfig(tqparent, "kcmlaptop");
  	tab->addTab(apm, i18n("&APM Config"));
  	connect(apm, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));
  } else {
        apm = 0;
  }
  if (::access("/dev/sonypi", F_OK) == 0) {
	bool do_sony = 1;
	if (::access("/dev/sonypi", R_OK) == 0) {
		int fd = ::open("/dev/sonypi", O_RDONLY);	// make sure the driver's there as well as the /dev inode
		if (fd >= 0) {
			::close(fd);
		} else {
			do_sony = 0;
		}
	}
	if (do_sony) {
  		sony = new SonyConfig(tqparent, "kcmlaptop");
  		tab->addTab(sony, i18n("&Sony Laptop Config"));
  		connect(sony, TQT_SIGNAL(changed(bool)), this, TQT_SLOT(moduleChanged(bool)));
	} else {
		sony = 0;
	}
  } else {
        sony = 0;
  }
  
  KAboutData* about = 
  new KAboutData("kcmlaptop", I18N_NOOP("Laptop Battery Configuration"), LAPTOP_VERSION,
       I18N_NOOP("Battery Control Panel Module"),
      KAboutData::License_GPL,
       I18N_NOOP("(c) 1999 Paul Campbell"), 0, 0);
  //about->addAuthor("NAME", 0, "e-mail addy");
  setAboutData( about );
}

void LaptopModule::load()
{
  battery->load();
  warning->load();
  critical->load();
  power->load();
  if (apm)
	  apm->load();
  if (acpi)
	  acpi->load();
  if (profile)
	  profile->load();
  if (sony)
	  sony->load();
  if (buttons)
	  buttons->load();
}

void LaptopModule::save()
{
  battery->save();
  warning->save();
  critical->save();
  power->save();
  if (profile)
	  profile->save();
  if (acpi)
	  acpi->save();
  if (apm)
	  apm->save();
  if (sony)
	  sony->save();
  if (buttons)
	  buttons->save();
}


void LaptopModule::defaults()
{
  battery->defaults();
  warning->defaults();
  critical->defaults();
  power->defaults();
  if (acpi)
	  acpi->defaults();
  if (apm)
	  apm->defaults();
  if (profile)
	  profile->defaults();
  if (sony)
	  sony->defaults();
  if (buttons)
	  buttons->defaults();
}

TQString LaptopModule::quickHelp() const
{
  return i18n("<h1>Laptop Battery</h1>This module allows you to monitor "
        "your batteries. To make use of this module, you must have power management software "
        "installed. (And, of course, you should have batteries in your "
        "machine.)");
}


void LaptopModule::moduleChanged(bool state)
{
  emit changed(state);
}


#include "main.moc"
