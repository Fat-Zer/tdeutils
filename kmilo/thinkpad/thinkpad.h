/*
   This file is part of the KDE project

   Copyright (c) 2004 Jonathan Riddell <jr@jriddell.org>

   Based on Demo kmilo service by George Staikos
   Copyright (c) 2003 George Staikos <staikos@kde.org>

   And tpb by Markus Braun http://www.nongnu.org/tpb/
   Copyright (C) 2002,2003 Markus Braun <markus.braun@krawel.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/
#ifndef _THINKPAD_H_
#define _THINKPAD_H_

#include <fcntl.h>
#include <unistd.h>

#include <kapplication.h>
#include <dcopref.h>

#include "monitor.h"

namespace KMilo {


const int defaultVolumeStep = 14;

// all nvram toggle values are stored in bit 0
// all xevents toggle values are stored in bit 1
// xevent and apm toggles are not used, kept for compatibility with tpb
typedef struct {
	unsigned int thinkpad_toggle;     // ThinkPad button
	unsigned int zoom_toggle;         // zoom toggle
	unsigned int display_toggle;      // display toggle
	unsigned int home_toggle;         // Home button
	unsigned int search_toggle;       // Search button
	unsigned int mail_toggle;         // Mail button
	unsigned int favorites_toggle;    // Favorites button (xevent)
	unsigned int reload_toggle;       // Reload button (xevent)
	unsigned int abort_toggle;        // Abort button (xevent)
	unsigned int backward_toggle;     // Backward button (xevent)
	unsigned int forward_toggle;      // Forward button (xevent)
	unsigned int fn_toggle;           // Fn button (xevent)
	unsigned int thinklight_toggle;   // ThinkLight
	unsigned int hibernate_toggle;    // hibernation/suspend toggle
	unsigned int display_state;       // display state
	unsigned int expand_toggle;       // hv expansion state
	unsigned int brightness_level;    // brightness level
	unsigned int brightness_toggle;   // brightness toggle
	unsigned int volume_level;        // volume level
	unsigned int volume_toggle;       // volume toggle
	unsigned int mute_toggle;         // mute toggle
	unsigned int ac_state;            // ac connected (apm)
	unsigned int powermgt_ac;         // power management mode ac
	unsigned int powermgt_battery;    // power management mode battery
	unsigned int wireless_toggle;     // wireless lan toggle
	unsigned int bluetooth_toggle;    // bluetooth toggle
} thinkpad_state_struct;

/**
 * KMilo plugin for IBM Thinkpads
 * Requires read access of nvram: insmod nvram; mknod /dev/nvram c 10 144; chmod 664 /dev/nvram
 *
 * Models R31 and R30 do not have hardware mixers, other models
 * probably want a smaller volume change than thinkpad's default, for
 * this you need write access to nvram: chmod 666 /dev/nvram
 */
class ThinkPadMonitor : public Monitor {
	public:
		ThinkPadMonitor(QObject *parent, const char *name, const QStringList&);
		virtual ~ThinkPadMonitor();

		/**
		 * Reimplemented from KMilo::Monitor.
		 * Loads configuration, clears struct, sets volume if necessary.
		 */
		virtual bool init();

		/**
		 * Reimplemented from KMilo::Monitor.
		 * Called by KMilo, this checks the nvram and acts on any changes
		 */
		virtual DisplayType poll();

		/**
		 * Reimplemented from KMilo::Monitor.
		 * Called by KMilo if poll() returns a volume or brightness change.
		 * Returns the new volume or brightness percentage.
		 */
		virtual int progress() const;

		/**
		 * Reimplemented from KMilo::Monitor.
		 * Not used, we just set the message ourselves using _interface->displayText()
		 */
		virtual QString message() const;
	protected:
		/**
		 * Reimplemented from KMilo::Monitor.
		 * Loads configuration from kmilorc
		 */
		virtual void reconfigure(KConfig*);
	private:
		/**
		 * get the current state from the nvram
		 */
		bool getNvramState(thinkpad_state_struct* thinkpad_state);

		/**
		 * get the current state from the nvram
		 */
		void setNvramVolume();

		/**
		 * set all values of thinkpad_state to zero
		 */
		void clearStruct(thinkpad_state_struct& thinkpad_state);

		/**
		 * Displays paramater message followed by 'on' or
		 * 'off' depending on state
		 */
		void showToggleMessage(QString onMessage, QString offMessage, bool state);

		/**
		 * Returned by progress()
		 */
		int m_progress;

		/**
		 * The file to read data from, default is /dev/nvram
		 */
		QString m_nvramFile;

		/**
		 * Some thinkpads change volume in hardware, others we
		 * have to do it ourselves
		 */
		bool m_softwareVolume;

		/**
		 * Config variable determines if this kmilo service is run or not
		 */
		bool m_run;

		/**
		 * Amount to change volume by each time, default is 14
		 * (as builtin to thinkpads)
		 */
		int m_volumeStep;

		/**
		 * Thinkpad button
		 */
		QString m_buttonThinkpad;

		/**
		 * Home button
		 */
		QString m_buttonHome;

		/**
		 * Search button
		 */
		QString m_buttonSearch;

		/**
		 * Mail button
		 */
		QString m_buttonMail;

		/**
		 * Zoom button
		 */
		QString m_buttonZoom;

		/**
		 * Used for changing the volume if m_softwareVolume is true
		 */
		void setVolume(int volume);
		DCOPRef* kmixClient;
		DCOPRef* kmixWindow;
		bool retrieveVolume();
		int m_volume, m_oldVolume;

		thinkpad_state_struct thinkpad_state;
		thinkpad_state_struct last_thinkpad_state;
};

} //close namespace

#endif
