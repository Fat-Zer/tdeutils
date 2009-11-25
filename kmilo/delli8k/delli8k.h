/*
   This file is part of the KDE project

   Copyright (c) 2004 Barry O'Donovan <barry.odonovan@kdemail.net>

   The code in this module is heavily based on:
    - the Asus Laptop plug-in for KMilo
      Copyright (c) 2004 Chris Howells (howells@kde.org)
    - the Thinkpad plug-in
      Copyright (c) 2004 Jonathan Riddell (jr@jriddell.org)
    - the i8k kernel module and utilities (http://www.debian.org/~dz/i8k/)
      Copyright (c) 2001-2003  Massimo Dal Zotto <dz@debian.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef DELLI8K_H
#define DELLI8K_H

#include <kapplication.h>
#include <dcopref.h>

#include "monitor.h"

#define DELLI8K_VOLUME_STEPSIZE 5

namespace KMilo {


/**
 * KMilo plugin for Dell Laptops (conforming to the i8k driver)
 */
class DellI8kMonitor: public Monitor 
{
	public:
		DellI8kMonitor( QObject *parent, const char *name, const QStringList& );
		virtual ~DellI8kMonitor();

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
		 * Called by KMilo if poll() returns a volume change.
		 * Returns the new volume percentage.
		 */
		virtual int progress() const;

	private:

    void setVolume( int volume );
    void setMute( bool b );
    DCOPRef *kmixClient;
    DCOPRef *kmixWindow;
    bool retrieveVolume();
    bool retrieveMute();
    int m_status, m_volume;
    bool m_mute;

    int m_progress;
    int m_fd;

    int fn_status( int fd );
};

} //close namespace

#endif
