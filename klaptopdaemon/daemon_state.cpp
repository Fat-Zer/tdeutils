/*
 * daemon_state.cpp
 * Copyright (C) 1999, 2003 Paul Campbell <paul@taniwha.com>
 *
 * this replaces kcmlaptop - there are 2 parts - one is the panels that
 *	put the setup configuration in the "kcmlaptop" configm, the other
 *	is the laptop_daemon (this guy) who watches the battery state
 *	and does stuff as a result
 *
 * This file contains the implementation of the main laptop battery monitoring daemon
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

#include <stdlib.h>
#include <fcntl.h>

#include <tqtimer.h>

#include <tdeconfig.h>
#include <klocale.h>
#include <tdecmdlineargs.h>
#include "daemon_state.h"
#include "portable.h"
#include <tdeaboutdata.h>
#include <kaudioplayer.h>
#include <sys/ioctl.h>
#include <kmessagebox.h>
#include <dcopclient.h>
#include <tqsocketnotifier.h>
#include <tqcursor.h>


#include <unistd.h>
#include <sys/time.h>

daemon_state::daemon_state()
{
	exists = laptop_portable::has_power_management();
	has_brightness = laptop_portable::has_brightness();
    	TQStringList profile_list;
    	int current_profile;
	bool *active_list;
    	has_performance = laptop_portable::get_system_performance(0, current_profile, profile_list, active_list);
    	TQStringList throttle_list;
    	int current_throttle;
    	has_throttle = laptop_portable::get_system_throttling(0, current_throttle, throttle_list, active_list);

}

daemon_state::~daemon_state()
{
}

void daemon_state::load()
{
	TDEConfig *config = new TDEConfig("kcmlaptoprc");

	if (config == 0) {
		::fprintf(stderr, "laptop_daemon: can't open kcmlaptop config files\n");
		::exit(2);
	}

	
	// acpi settings
       	config->setGroup("AcpiDefault");
        bool enablestandby = config->readBoolEntry("EnableStandby", false);
        bool enablesuspend = config->readBoolEntry("EnableSuspend", false);
        bool enablehibernate = config->readBoolEntry("EnableHibernate", false);
        bool enableperformance = config->readBoolEntry("EnablePerformance", false);
        bool enablethrottle = config->readBoolEntry("EnableThrottle", false);
	laptop_portable::acpi_set_mask(enablestandby, enablesuspend, enablehibernate, enableperformance, enablethrottle);

    	config->setGroup("ApmDefault");
    	enablestandby = config->readBoolEntry("EnableStandby", false);
    	enablesuspend = config->readBoolEntry("EnableSuspend", false);
    	laptop_portable::apm_set_mask(enablestandby, enablesuspend);

    	config->setGroup("SoftwareSuspendDefault");
    	enablehibernate = config->readBoolEntry("EnableHibernate", false);
    	laptop_portable::software_suspend_set_mask(enablehibernate);

	exists = laptop_portable::has_power_management();
	has_brightness = laptop_portable::has_brightness();
    	TQStringList profile_list;
    	int current_profile;
	bool *active_list;
    	has_performance = laptop_portable::get_system_performance(0, current_profile, profile_list, active_list);
    	TQStringList throttle_list;
    	int current_throttle;
    	has_throttle = laptop_portable::get_system_throttling(0, current_throttle, throttle_list, active_list);

	bool can_suspend = laptop_portable::has_suspend();
	bool can_hibernate = laptop_portable::has_hibernation();
	bool can_standby = laptop_portable::has_standby();

	// power control settings
	config->setGroup("LaptopPower");
	power_action[0] = config->readNumEntry("PowerSuspend", 0);
	switch (power_action[0]) {
	case 1:	if (!can_standby)   power_action[0] = 0; break;
	case 2:	if (!can_suspend)   power_action[0] = 0; break;
	case 3:	if (!can_hibernate) power_action[0] = 0; break;
	}
	power_action[1] = config->readNumEntry("NoPowerSuspend", 1);
	switch (power_action[1]) {
	case 1:	if (!can_standby)   power_action[1] = 0; break;
	case 2:	if (!can_suspend)   power_action[1] = 0; break;
	case 3:	if (!can_hibernate) power_action[1] = 0; break;
	}
	power_brightness_enabled[0] = config->readBoolEntry("PowerBrightnessEnabled", 0)&has_brightness;
	power_brightness_enabled[1] = config->readBoolEntry("NoPowerBrightnessEnabled", 0)&has_brightness;
	power_brightness[0] = config->readNumEntry("PowerBrightness", 255);
	power_brightness[1] = config->readNumEntry("NoPowerBrightness", 0);
	power_performance_enabled[0] = config->readBoolEntry("PowerPerformanceEnabled", 0)&has_performance;
	power_performance_enabled[1] = config->readBoolEntry("NoPowerPerformanceEnabled", 0)&has_performance;
	power_performance[0] = config->readEntry("PowerPerformance", "");
	power_performance[1] = config->readEntry("NoPowerPerformance", "");
	power_throttle_enabled[0] = config->readBoolEntry("PowerThrottleEnabled", 0)&has_throttle;
	power_throttle_enabled[1] = config->readBoolEntry("NoPowerThrottleEnabled", 0)&has_throttle;
	power_throttle[0] = config->readEntry("PowerThrottle", "");
	power_throttle[1] = config->readEntry("NoPowerThrottle", "");
  	power_wait[0] = 60*config->readNumEntry("PowerWait", 20);
  	power_wait[1] = 60*config->readNumEntry("NoPowerWait", 5);
	lav_enabled[0] = config->readBoolEntry("LavEnabled", 0);
	lav_enabled[1] = config->readBoolEntry("NoLavEnabled", 0);
	lav_val[0] = config->readDoubleNumEntry("PowerLav", -1);
	lav_val[1] = config->readDoubleNumEntry("NoPowerLav", -1);

	// General settings
	config->setGroup("BatteryDefault");
       	poll = config->readNumEntry("Poll", 20);
        enabled = config->readBoolEntry("Enable", true)&exists;
        notifyMeWhenFull = config->readBoolEntry("NotifyMe", false)&exists;
	useBlankSaver = config->readBoolEntry("BlankSaver", false);
        noBatteryIcon = config->readEntry("NoBatteryPixmap", "laptop_nobattery");
        noChargeIcon = config->readEntry("NoChargePixmap", "laptop_nocharge");
        chargeIcon = config->readEntry("ChargePixmap", "laptop_charge");


	config->setGroup("BatteryLow");
        time_based_action_low = config->readBoolEntry("TimeBasedAction", true);
        low[0] = config->readNumEntry("LowValTime", 15);
        low[1] = config->readNumEntry("LowValPercent", 7);
        runCommand[0] = config->readBoolEntry("RunCommand", false)&exists;
        runCommandPath[0] = config->readEntry("RunCommandPath");
        playSound[0] = config->readBoolEntry("PlaySound", false)&exists;
        playSoundPath[0] = config->readEntry("PlaySoundPath");
        systemBeep[0] = config->readBoolEntry("SystemBeep", true)&exists;
        logout[0] = config->readBoolEntry("Logout", false)&exists;
        shutdown[0] = config->readBoolEntry("Shutdown", false)&exists;
        notify[0] = config->readBoolEntry("Notify", true)&exists;
        do_hibernate[0] = config->readBoolEntry("Hibernate", false)&can_hibernate&exists;
        do_suspend[0] = config->readBoolEntry("Suspend", false)&can_suspend&exists;
        do_standby[0] = config->readBoolEntry("Standby", false)&can_standby&exists;
        do_brightness[0] = config->readBoolEntry("Brightness", false)&has_brightness&exists;
        val_brightness[0] = config->readNumEntry("BrightnessValue", 0);
        do_performance[0] = config->readBoolEntry("Performance", false)&has_performance&exists;
        val_performance[0] = config->readEntry("PerformanceValue", "");
        do_throttle[0] = config->readBoolEntry("Throttle", false)&has_throttle&exists;
        val_throttle[0] = config->readEntry("ThrottleValue", "");
        have_time = config->readNumEntry("HaveTime", 2);

        if (!have_time && laptop_portable::has_battery_time())
        	have_time = 1;

	config->setGroup("BatteryCritical");
        time_based_action_critical = config->readBoolEntry("TimeBasedAction", true);
        low[2] = config->readNumEntry("CriticalValTime", 5);
        low[3] = config->readNumEntry("CriticalValPercent", 3);
        runCommand[1] = config->readBoolEntry("RunCommand", false)&exists;
        runCommandPath[1] = config->readEntry("RunCommandPath");
        playSound[1] = config->readBoolEntry("PlaySound", false)&exists;
        playSoundPath[1] = config->readEntry("PlaySoundPath");
        systemBeep[1] = config->readBoolEntry("SystemBeep", true)&exists;
        logout[1] = config->readBoolEntry("Logout", false)&exists;
        shutdown[1] = config->readBoolEntry("Shutdown", false)&exists;
        notify[1] = config->readBoolEntry("Notify", (can_suspend?false:true))&exists;
        do_hibernate[1] = config->readBoolEntry("Hibernate", false)&can_hibernate&exists;
        do_suspend[1] = config->readBoolEntry("Suspend", (can_suspend?true:false))&can_suspend&exists;
        do_standby[1] = config->readBoolEntry("Standby", false)&can_standby&exists;
        do_brightness[1] = config->readBoolEntry("Brightness", false)&has_brightness&exists;
        val_brightness[1] = config->readNumEntry("BrightnessValue", 0);
        do_performance[1] = config->readBoolEntry("Performance", false)&has_performance&exists;
        val_performance[1] = config->readEntry("PerformanceValue", "");
        do_throttle[1] = config->readBoolEntry("Throttle", false)&has_throttle&exists;
        val_throttle[1] = config->readEntry("ThrottleValue", "");

	config->setGroup("ProfileDefault");
	bright_pon = config->readBoolEntry("EnableBrightnessOn", false)&has_brightness;
	bright_poff = config->readBoolEntry("EnableBrightnessOff", false)&has_brightness;
	performance_pon = config->readBoolEntry("EnablePerformanceOn", false)&has_performance;
	performance_poff = config->readBoolEntry("EnablePerformanceOff", false)&has_performance;
	throttle_pon = config->readBoolEntry("EnableThrottleOn", false)&has_throttle;
	throttle_poff = config->readBoolEntry("EnableThrottleOff", false)&has_throttle;
	bright_son = config->readNumEntry("BrightnessOnLevel", 255);
	bright_soff = config->readNumEntry("BrightnessOffLevel", 160);
	performance_val_on = config->readEntry("PerformanceOnLevel", "");
	performance_val_off = config->readEntry("PerformanceOffLevel", "");
	throttle_val_on = config->readEntry("ThrottleOnLevel", "");
	throttle_val_off = config->readEntry("ThrottleOffLevel", "");

  	config->setGroup("LaptopButtons");
	enable_lid_button = laptop_portable::has_button(laptop_portable::LidButton);
	enable_power_button = laptop_portable::has_button(laptop_portable::PowerButton);
  	button_lid = (enable_lid_button?config->readNumEntry("LidSuspend", 0):0);
	if (!enable_lid_button) {
		button_lid = 0;
	} else
	switch (button_lid) {
	case 1:	if (!can_standby)   button_lid = 0; break;
	case 2:	if (!can_suspend)   button_lid = 0; break;
	case 3:	if (!can_hibernate) button_lid = 0; break;
	}
  	button_power = (enable_power_button?config->readNumEntry("PowerSuspend", 0):0);
	if (!enable_power_button) {
		button_power = 0;
	} else
	switch (button_power) {
	case 1:	if (!can_standby)   button_power = 0; break;
	case 2:	if (!can_suspend)   button_power = 0; break;
	case 3:	if (!can_hibernate) button_power = 0; break;
	}
  	button_lid_bright_enabled = config->readBoolEntry("LidBrightnessEnabled", 0)&enable_lid_button&has_brightness;
  	button_power_bright_enabled = config->readBoolEntry("PowerBrightnessEnabled", 0)&enable_power_button&has_brightness;
  	button_lid_bright_val = config->readNumEntry("LidBrightness", 0);
  	button_power_bright_val = config->readNumEntry("PowerBrightness", 0);
  	button_lid_performance_enabled = config->readBoolEntry("LidPerformanceEnabled", 0)&enable_lid_button&has_performance;
  	button_power_performance_enabled = config->readBoolEntry("PowerPerformanceEnabled", 0)&enable_power_button&has_performance;
  	button_lid_performance_val = config->readEntry("LidPerformance", "");
  	button_power_performance_val = config->readEntry("PowerPerformance", "");
  	button_lid_throttle_enabled = config->readBoolEntry("LidThrottleEnabled", 0)&enable_lid_button&has_throttle;
  	button_power_throttle_enabled = config->readBoolEntry("PowerThrottleEnabled", 0)&enable_power_button&has_throttle;
  	button_lid_throttle_val = config->readEntry("LidThrottle", "");
  	button_power_throttle_val = config->readEntry("PowerThrottle", "");
	
	config->setGroup("SonyDefault");
	sony_enablescrollbar = config->readBoolEntry("EnableScrollBar", false);
	sony_middleemulation = config->readBoolEntry("EnableMiddleEmulation", false);

	delete config;
}

bool
daemon_state::need_to_run()
{
	//
	//	look for reasons NOT to run the daemon
	//
	if (!exists)
		return(0);
 	if (!enabled && !notifyMeWhenFull &&
		!runCommand[0] && !playSound[0] && !systemBeep[0] && !notify[0] && !do_suspend[0] &&
		!do_standby[0] && !logout[0] && !shutdown[0] && !do_hibernate[0] &&
		!runCommand[1] && !playSound[1] && !systemBeep[1] && !notify[1] && !do_suspend[1] &&
		!do_standby[1] && !logout[1] && !shutdown[1] && !do_hibernate[0] &&
		!sony_enablescrollbar && !sony_middleemulation && !do_brightness[0] && !do_brightness[1] && 
		button_lid==0 && button_power==0 && !button_lid_bright_enabled && !button_power_bright_enabled &&
		!button_lid_performance_enabled && !button_power_performance_enabled && !button_lid_throttle_enabled && !button_power_throttle_enabled &&
		!do_performance[0] && !do_performance[1] && !do_throttle[0] && !do_throttle[1] &&
		!bright_pon && !bright_poff && !performance_pon && !performance_poff && !throttle_pon && !throttle_poff &&
		!power_brightness_enabled[0] && !power_brightness_enabled[0] &&
		!power_performance_enabled[0] && !power_performance_enabled[1] && !power_throttle_enabled[0] && !power_throttle_enabled[1] &&
		power_action[0] == 0 && power_action[1] == 0) // if no reason to be running quit
		return(0);
	return(1);
}

