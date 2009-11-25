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

#include "pb_monitor.h"

#include <sys/types.h>
#include <unistd.h>

#include <kgenericfactory.h>

extern "C" {
#include <pbbipc.h>
}

namespace KMilo {

PowerBookMonitor::PowerBookMonitor(QObject *parent, const char *name, const QStringList& args)
: Monitor(parent, name, args) {
	_serverPort = -1;
	_clientPort = -1;
	_progress = 0;
}


PowerBookMonitor::~PowerBookMonitor() {
struct pbb_message msg;
	if (_serverPort >= 0) {
		msg.code = UNREGISTERCLIENT;
		msg.value = _clientPort;
		put_message(_serverPort, &msg);
	}

	if (_clientPort >= 0) {
		remove_msgport(_clientPort);
	}
}


bool PowerBookMonitor::init() {
	_serverPort = find_msgport(SERVERPORTKEY);
	if (_serverPort < 0) {
		return false;
	}

	_clientPort = create_msgport(getpid());
	if (_clientPort < 0) {
		_serverPort = -1;
		return false;
	}

	// Register
	struct pbb_message msg;
	msg.code = REGISTERCLIENT;
	msg.value = _clientPort;
	if (put_message(_serverPort, &msg) < 0) {
		remove_msgport(_clientPort);
		_clientPort = -1;
		_serverPort = -1;
		return false;
	}

return true;
}


Monitor::DisplayType PowerBookMonitor::poll() {
Monitor::DisplayType rc = None;
struct pbb_message msg;

	if (_serverPort < 0 || _clientPort < 0)
		return Error;

	// Drain the queue, send the last message
	while (0 == get_message(_clientPort, 0, &msg)) {
		switch (msg.code) {
			case DISPLAYVOLUME:
				rc = Monitor::Volume;
				_progress = msg.value;
				break;
			case DISPLAYMUTE:
				rc = Monitor::Mute;
				_progress = msg.value;
				break;
			case DISPLAYBRIGHTNESS:
				rc = Monitor::Brightness;
				_progress = msg.value*100/15;
				break;
			case REGFAILED:
			case CLIENTEXIT:
				rc = Monitor::Error;
				break;
			default:
				break;
		}
	}

return rc;
}


int PowerBookMonitor::progress() const {
	return _progress;
}

};

K_EXPORT_COMPONENT_FACTORY(kmilo_powerbook, KGenericFactory<PowerBookMonitor>("kmilo_powerbook"))

