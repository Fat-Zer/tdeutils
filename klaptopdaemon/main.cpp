/*
 * main.cpp
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
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


#include <klocale.h>
#include <kprocess.h>
#include <kconfig.h>
#include <kglobal.h>
#include <qlayout.h>
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

  KDE_EXPORT KCModule *create_pcmcia(QWidget *parent, const char *)
  {
    return new PcmciaConfig(parent, "kcmlaptop");
  }

  KDE_EXPORT KCModule *create_bwarning(QWidget *parent, const char *)
  {
    return new WarningConfig(0, parent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_cwarning(QWidget *parent, const char *)
  {
    return new WarningConfig(1, parent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_battery(QWidget *parent, const char *)
  {
    return new BatteryConfig(parent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_power(QWidget *parent, const char *)
  {
    return new PowerConfig(parent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_acpi(QWidget *parent, const char *)
  {
    return new AcpiConfig(parent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_apm(QWidget *parent, const char *)
  {
    return new ApmConfig(parent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_Profile(QWidget *parent, const char *)
  {
    return new ProfileConfig(parent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_sony(QWidget *parent, const char *)
  {
    return new SonyConfig(parent, "kcmlaptop");
  }
  KDE_EXPORT KCModule *create_buttons(QWidget *parent, const char *)
  {
    return new ButtonsConfig(parent, "kcmlaptop");
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

  KDE_EXPORT KCModule *create_laptop(QWidget *parent, const char *)
  {
	return new LaptopModule(parent, "kcmlaptop");
  }

  KDE_EXPORT void init_laptop()
  {
	init_battery();
  }
}



LaptopModule::LaptopModule(QWidget *parent, const char *)
  : KCModule(parent, "kcmlaptop")
{
  {	// export ACPI options
    KConfig config("kcmlaptoprc", true /*readonly*/, false /*no globals*/);
    config.setGroup("AcpiDefault");

    bool enablestandby = config.readBoolEntry("EnableStandby", false);
    bool enablesuspend = config.readBoolEntry("EnableSuspend", false);
    bool enablehibernate = config.readBoolEntry("EnableHibernate", false);
    bool enableperformance = config.readBoolEntry("EnablePerformance", false);
    bool enablethrottle = config.readBoolEntry("EnableThrottle", false);
    laptop_portable::acpi_set_mask(enablestandby, enablesuspend, enablehibernate, enableperformance, enablethrottle);

    config.setGroup("ApmDefault");

    enablestandby = config.readBoolEntry("EnableStandby", false);
    enablesuspend = config.readBoolEntry("EnableSuspend", false);
    laptop_portable::apm_set_mask(enablestandby, enablesuspend);
    config.setGroup("SoftwareSuspendDefault");
    enablehibernate = config.readBoolEntry("EnableHibernate", false);
    laptop_portable::software_suspend_set_mask(enablehibernate);
  }
  QVBoxLayout *layout = new QVBoxLayout(this);
  tab = new QTabWidget(this);
  layout->addWidget(tab);

  battery = new BatteryConfig(parent, "kcmlaptop");
  tab->addTab(battery, i18n("&Battery"));
  connect(battery, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  power = new PowerConfig(parent, "kcmlaptop");
  tab->addTab(power, i18n("&Power Control"));
  connect(power, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  warning = new WarningConfig(0, parent, "kcmlaptop");
  tab->addTab(warning, i18n("Low Battery &Warning"));
  connect(warning, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  critical = new WarningConfig(1, parent, "kcmlaptop");
  tab->addTab(critical, i18n("Low Battery &Critical"));
  connect(critical, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

    QStringList profile_list;
    int current_profile;
    bool *active_list;
    bool has_profile = laptop_portable::get_system_performance(0, current_profile, profile_list, active_list);
    QStringList throttle_list;
    int current_throttle;
    bool has_throttling = laptop_portable::get_system_throttling(0, current_throttle, throttle_list, active_list);
  if (laptop_portable::has_brightness() || has_profile || has_throttling) {
  	profile = new ProfileConfig(parent, "kcmlaptop");
  	tab->addTab(profile, i18n("Default Power Profiles"));
  	connect(profile, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
  } else {
	profile = 0;
  }
  if (laptop_portable::has_button(laptop_portable::LidButton) || laptop_portable::has_button(laptop_portable::PowerButton)) {
  	buttons = new ButtonsConfig(parent, "kcmlaptop");
  	tab->addTab(buttons, i18n("Button Actions"));
  	connect(buttons, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
  } else {
        buttons = 0;
  }
  if (laptop_portable::has_acpi()) {
  	acpi = new AcpiConfig(parent, "kcmlaptop");
  	tab->addTab(acpi, i18n("&ACPI Config"));
  	connect(acpi, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
  } else {
        acpi = 0;
  }
  if (laptop_portable::has_apm()) {
  	apm = new ApmConfig(parent, "kcmlaptop");
  	tab->addTab(apm, i18n("&APM Config"));
  	connect(apm, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
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
  		sony = new SonyConfig(parent, "kcmlaptop");
  		tab->addTab(sony, i18n("&Sony Laptop Config"));
  		connect(sony, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
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

QString LaptopModule::quickHelp() const
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
