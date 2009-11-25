#include <qlabel.h>
#include <kactivelabel.h>
#include <kconfig.h>
#include <qvgroupbox.h>
#include <qstringlist.h>

#ifndef PORTABLE_H
#define PORTABLE_H
/*
 * Copyright (c) 2002 Paul Campbell <paul@taniwha.com>
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

struct power_result {
	int	powered	;	// true if we're plugged into the wall
	int	percentage;	// value 0-100 percentage of battery left
	int	time;		// time in minutes left - -1 if this is not supported by the BIOS
};


// ATTENTION: if you change something in here, please update ALL of the
//            ported sections in the implementation file!

class KDE_EXPORT laptop_portable {
public:
	static void power_management_restart();	// reset internal state
	static int has_power_management();	// returns 1 if this computer has power management
	static int has_battery_time();		// returns 1 if this give BIOS battery time info

	/**
	 * Returns 1 if this computer can perform a suspend, i.e. supports some kind
	 * of suspend-to-ram energy saving mode (average wakeup time and energy 
	 * saving).
	 */
	static int has_suspend();

	/**
	 * Returns 1 if this computer can perform a standby, i.e. supports an energy
	 * saving mode with a very fast wakeup time (neither suspend-to-disk nor 
	 * suspend-to-ram).
	 */
	static int has_standby();

	/**
	 * Returns 1 if this computer can perform hibernatation, i.e. supports some
	 * kind of suspend-to-disk energy saving mode (highest energy saving, 
	 * slowest wakeup time).
	 */
	static int has_hibernation();

	static int has_apm(int);		// returns 1 if this computer has linux-style apm of some particular type
	static int has_apm() { return has_apm(0); }	// returns 1 if this computer has linux-style apm
	static int has_acpi(int);		// returns 1 if this computer has linux-style acpi of some particular type
	static int has_acpi() { return has_acpi(0); }	// returns 1 if this computer has linux-style acpi
	static bool has_software_suspend(int type); // // returns 1 if the system has software suspend available
	static bool has_software_suspend() { return has_software_suspend(0); }
	static int has_cpufreq();		// returns 1 if this computer can scale the cpu frequency
	static int has_brightness();		// returns 1 if this computer can set tyhe back panel brightness
	static void set_brightness(bool blank, int val);	// val = 0-255 255 brightest, 0 means dimmest (if !blank it must be still visible), 
	static int get_brightness();		// returns 0-255 brightness, -1 if you can't
	static QString cpu_frequency();		// Returns the cpu freq.
	static KActiveLabel *no_power_management_explanation(QWidget *parent);
	static QLabel *how_to_do_suspend_resume(QWidget *parent);
	static QLabel *pcmcia_info(int x, QWidget *parent);

	/**
	 * Put this computer into standby mode.
	 * @see has_standby()
	 */
	static void invoke_standby();

	/**
	 * Put this computer into suspend mode.
	 * @see has_suspend()
	 */
	static void invoke_suspend();

	/**
	 * Put this computer into hibernatation mode.
	 * @see has_hibernation()
	 */
	static void invoke_hibernation();

	static struct power_result poll_battery_state();
	static void get_battery_status(int &num_batteries, QStringList &names, QStringList &state, QStringList &values); // get multiple battery status
	static bool has_lav();			// true if the following returns a valid value
	static float get_load_average();	// current short term load average
	static void extra_config(QWidget *parent, KConfig *config, QVBoxLayout *layout);
	static void acpi_set_mask(bool standby, bool suspend, bool hibernate, bool perf, bool throttle);	
	static void apm_set_mask(bool standby, bool suspend);
	static void software_suspend_set_mask(bool hibernate);

	/**
	 * Get a list of available performance profiles.
	 * @param force - Force re-evaluation or use cached values. 
	 * @param current - Index of the currently active profile.
	 * @param s - A list of available profiles.
	 * @param active - Marks profiles as enabled or disabled (used e.g. for ACPI
	 * limits).
	 * @return True if this system provides performance profiles.
	 */
	static bool get_system_performance(bool force, int &current, QStringList &s, bool *&active);

	/**
	 * Set performance profile.
	 * @param val - Name of the performance profile as provided by 
	 * get_system_performance().
	 */
	static void set_system_performance(QString val);

	/**
	 * Get a list of available throttling levels.
	 * @param force - Force re-evaluation or use cached values. 
	 * @param current - Index of the currently active throttling level.
	 * @param s - A list of available throttling levels.
	 * @param active - Marks throttling labels as enabled or disabled (used e.g.
	 * for ACPI limits).
	 * @return True if this system provides throttling levels.
	 */
	static bool get_system_throttling(bool force, int &current, QStringList &s, bool *&active); 

	/**
	 * Set throttling level.
	 * @param val - Name of the throttling level as provided by 
	 * get_system_throttling().
	 */
	static void set_system_throttling(QString val);

	enum LaptopButton {LidButton=0, PowerButton};
	static bool has_button(LaptopButton p);	// true if we have support for a particular button
	static bool get_button(LaptopButton p);	// true if a button is pressed
};

#endif
