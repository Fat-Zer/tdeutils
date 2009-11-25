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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kmilod.h"
#include "monitor.h"

#include <qfile.h>

#include <klocale.h>
#include <kdebug.h>
#include <kservicetype.h>
#include <kservice.h>
#include <kconfig.h>
#include <kparts/componentfactory.h>

#include "kmilointerface.h"
#include "defaultskin.h"

using namespace KMilo;

extern "C" {
   KDE_EXPORT KDEDModule *create_kmilod(const QCString &name) {
	   return new KMiloD(name);
   }
}


KMiloD::KMiloD(const QCString &name) : KDEDModule(name), _interval(100)
{
	_monitors.setAutoDelete(true);
	_miface = new KMiloInterface(this);

	// Create the display skin object
	_display = new DefaultSkin;

	bool shouldPoll = false;

	// Load the modules
	KService::List plugs = KServiceType::offers("KMilo Plugin");
	for (KService::List::ConstIterator it = plugs.begin();
						it != plugs.end(); ++it) {
		KService::Ptr service = *it;
		KMilo::Monitor *m = KParts::ComponentFactory::createInstanceFromService<KMilo::Monitor>(service, 0, service->desktopEntryName().latin1());
		if (m) {
			m->setInterface(_miface);
			if (m->init()) {
				_monitors.append(m);
				kdDebug() << "KMilo loaded module "
					<< service->property("Name").toString()
					<< endl;
				shouldPoll = shouldPoll || m->shouldPoll();
			} else {
				delete m;
			}
		}
	}

	// Start the timer
	QObject::connect(&_timer, SIGNAL(timeout()), this, SLOT(doTimer()));
	if (shouldPoll) {
		_timer.start(_interval);
	}
}


KMiloD::~KMiloD()
{
	_timer.stop();

	// Modules are automatically cleaned up when this is done.  It has
	// to be done before delete _display to avoid a race though.
	_monitors.clear();

	delete _display;
	_display = 0L;

	delete _miface;
	_miface = 0L;
}


void KMiloD::setEnabled(bool enabled) {
	if (enabled) {
		enable();
	} else {
		disable();
	}
}


void KMiloD::enable() {
	if (!_monitors.isEmpty()) {
		_timer.start(_interval, false);
	}
}


void KMiloD::disable() {
	_timer.stop();
}


bool KMiloD::enabled() const {
	return _timer.isActive();
}


int KMiloD::pollMilliSeconds() const {
	return _interval;
}


bool KMiloD::setPollMilliSeconds(int ms) {
	if (ms > 1000 || ms < 0) {
		return false;
	}

	if (!_monitors.isEmpty()) {
		_timer.start(_interval, false);
	}

	_interval = ms;

return true;
}


void KMiloD::doTimer() {
	// Iterate through all the modules we have and poll
	for (KMilo::Monitor *m = _monitors.first(); m; m = _monitors.next()) {
		if (!m->shouldPoll()) {
			continue;
		}

		KMilo::Monitor::DisplayType dt = m->poll();
		switch (dt) {
			case KMilo::Monitor::Volume:
				displayProgress(i18n("Volume"), m->progress());
				break;
			case KMilo::Monitor::Brightness:
				displayProgress(i18n("Brightness"), m->progress());
				break;
			case KMilo::Monitor::Mute:
				displayText(i18n("Muted"));
				break;
			case KMilo::Monitor::Tap:
				displayText(m->message());
				break;
			case KMilo::Monitor::Sleep:
				displayText(m->message());
				break;
			case KMilo::Monitor::None:
				// Do nothing
				break;
			case KMilo::Monitor::Error:
				{
				// On error, remove the monitor and continue
				KMilo::Monitor *n = _monitors.next();
				_monitors.remove(m); // deletes m
				m = n;
				}
				break;
			default:
				kdWarning() << "Error in KMiloD.  Please report." << endl;
				break;
		}
	}
}


void KMiloD::displayText(const QString& text) {
	_display->displayText(text, QPixmap());
}


void KMiloD::displayText(const QString& text, const QPixmap& customPixmap) {
	_display->displayText(text, customPixmap);
}


void KMiloD::displayProgress(const QString& text, int progress) {
	_display->displayProgress(text, progress, QPixmap());
}

void KMiloD::displayProgress(const QString& text, int progress, const QPixmap& customPixmap ) {
	_display->displayProgress(text, progress, customPixmap);
}

void KMiloD::reconfigure()
{
	// load the kmilo configuration file:
	KConfig config("kmilodrc");

	KMilo::Monitor *monitor;

	for(monitor = _monitors.first(); monitor; monitor = _monitors.next())
	{
		monitor->reconfigure(&config);
	}
}

#include "kmilod.moc"

