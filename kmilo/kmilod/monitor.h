// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
/*
   This file is part of the KDE project

   Copyright (c) 2003 George Staikos <staikos@kde.org>

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

#ifndef _MONITOR_H_
#define _MONITOR_H_

#include <qstring.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qobject.h>

#include <kdemacros.h>

class KConfig;

namespace KMilo {
class KMiloInterface;

class KDE_EXPORT Monitor : public QObject {
	public:
		Monitor(QObject *parent, const char *name, const QStringList&);
		virtual ~Monitor();

		/**
		 *  Called by kmilod to initialise this plugin.  The plugin
		 *  must return true if its hardware is present and it should
		 *  be loaded.  If it returns false, it will be unloaded.
		 */
		virtual bool init();

		enum DisplayType { None,    //!
				 Error,   //! Error
				 Volume,  //! Volume level changed
				 Mute,    //! Volume has been muted
				 Brightness, //! Brighteness of the screen has changed
				 Tap,  //! Mousttracker tap mode has changed
				 Sleep //! laptop will sleep in progress() seconds
		};

		/**
		 *  This is called by KMiloD when it polls.  Must return
		 *  the type of event that has occurred, or None.
		 */
		virtual DisplayType poll();

		/**
		 *  If poll() returns stating that a value has changed, this
		 *  will be called by kmilod to determine the value (0..100)
		 *  for the slider in the display, if necessary.
		 */
		virtual int progress() const;

		/**
		 * Message displayed for DisplayType's that are not int
		 * measurable ( DisplayType::Tap for example )
		 */
		virtual QString message() const;

		/**
		 * Custom pixmap that can be displayed be the themes
		 * when displaying the DisplayType.
		 */
		virtual QPixmap customPixmap() const;

		bool shouldPoll() const { return _poll; }

		virtual void reconfigure(KConfig*);

	protected:
		bool _poll;  // set to false to disable polling.
		friend class KMiloD;
		void setInterface(KMiloInterface *i) { _interface = i; }

		KMiloInterface *_interface;
};

}

#endif
