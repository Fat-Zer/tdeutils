#ifndef __DAEMON_STATE_H
#define __DAEMON_STATE_H 1
/*
 * daemon_state.h
 * Copyright (C) 1999, 2003 Paul Campbell <paul@taniwha.com>
 *
 * This file contains the saved config state for the laptop daemon
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


#include <qdir.h>
#include <unistd.h>
#include <time.h>
#include <qmovie.h>
#include <qptrlist.h>
#include <qfileinfo.h>
#include <qimage.h>
#include <kdelibs_export.h>

class KDE_EXPORT daemon_state
{
public:
    	daemon_state();
    	~daemon_state();
	void	load();
	bool	need_to_run();

	bool 	exists;
	bool	has_brightness;
	bool	has_performance;
	bool	has_throttle;

	QString noBatteryIcon;
	QString chargeIcon;
	QString noChargeIcon;

	//
	//	power out actions
	//

	int	power_wait[2];		// how close to the end when we trigger the action
	int	power_action[2];	// what to do when this action is triggered
	int	power_brightness[2];	// which brightness level to switch to
	bool	power_brightness_enabled[2];	// and whether to switch
	QString	power_performance[2];	// which performance level to switch to
	bool	power_performance_enabled[2];	// and whether to switch
	QString	power_throttle[2];	// which throttling level to switch to
	bool	power_throttle_enabled[2];	// and whether to switch
	bool	lav_enabled[2];		// check LAV first
	float	lav_val[2];		// don't act if higher than this

	//
	//	power out warnings
	//

	bool    systemBeep[2];
	bool	logout[2];
	bool	shutdown[2];
	bool    runCommand[2];
	QString runCommandPath[2];
	bool    playSound[2];
	QString playSoundPath[2];
	bool    notify[2];
	bool    do_suspend[2];
	bool    do_hibernate[2];
	bool    do_standby[2];
	bool	do_brightness[2];
	int	val_brightness[2];
	bool	do_performance[2];
	QString	val_performance[2];
	bool	do_throttle[2];
	QString	val_throttle[2];
        bool    time_based_action_low, time_based_action_critical;
	int	low[4];
	int	poll;			// how often to pol

	

	bool    enabled;
	bool	notifyMeWhenFull;
	bool	useBlankSaver;


	int	have_time;

	// sony jog-bar support
	
	bool sony_enablescrollbar;	// enable jog bar support
	bool sony_middleemulation;	// press on jog bar emulates mouse middle button

	// brightness 
	
	bool bright_pon;		// auto set brightness when powered
	int  bright_son;		// value to set
	bool bright_poff;		// auto set brightness when on battery
	int  bright_soff;		// value to set

	// performance
	
	bool performance_pon, performance_poff;
	QString performance_val_on, performance_val_off;

	// throttle
	
	bool throttle_pon, throttle_poff;
	QString throttle_val_on, throttle_val_off;

	// button triggered stuff
  	int button_lid;
  	int button_power;
  	bool button_lid_bright_enabled;
  	bool button_power_bright_enabled;
  	int button_lid_bright_val;
  	int button_power_bright_val;
  	bool button_lid_performance_enabled;
  	bool button_power_performance_enabled;
  	QString button_lid_performance_val;
  	QString button_power_performance_val;
  	bool button_lid_throttle_enabled;
  	bool button_power_throttle_enabled;
  	QString button_lid_throttle_val;
  	QString button_power_throttle_val;
	bool enable_lid_button;
	bool enable_power_button;

};
#endif
