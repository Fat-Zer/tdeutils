/*
   This file is part of the KDE project

   Copyright (c) 2004 Chris Howells <howells@kde.org>
   Much code and ideas stolen from Jonathan Riddell's ThinkPad plugin

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; version
   2 of the License.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/
#ifndef ASUS_H
#define ASUS_H

#include <kapplication.h>
#include <dcopref.h>

#include "monitor.h"

namespace KMilo {

typedef struct
{
	unsigned int brightness;
	unsigned int lcd;
	unsigned int display;
} asus_state_struct;

/**
 * KMilo plugin for Asus Laptops
 */
class AsusMonitor: public Monitor {
	public:
		AsusMonitor(QObject *parent, const char *name, const QStringList&);
		virtual ~AsusMonitor();

		/**
		 * Reimplemented from KMilo::Monitor.
		 * Loads configuration.
		 */
		virtual bool init();

		/**
		 * Reimplemented from KMilo::Monitor.
		 * Called by KMilo, this checks the /proc file system and acts on any changes
		 */
		virtual DisplayType poll();

		/**
		 * Reimplemented from KMilo::Monitor.
		 * Called by KMilo if poll() returns a brightness change.
		 * Returns the new brightness percentage.
		 */
		virtual int progress() const;

	private:

		int m_progress;

		asus_state_struct last_asus_state;
		asus_state_struct asus_state;

		bool readProc(asus_state_struct* );
		int readProcEntry(const QString &);

		void clearStruct(asus_state_struct& asus_state);

#ifdef Q_OS_FREEBSD
		int brightness_mib[4];
		int backlight_mib[4];
		int video_mib[4];
#endif
};

} // close namespace

#endif
