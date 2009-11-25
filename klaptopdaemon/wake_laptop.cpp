/*
 * wake_laptop.cpp
 * Copyright (C) 2003 Paul Campbell <paul@taniwha.com>
 *
 * send restart message to laptop daemon thru kded so that the daemon
 * 	gets started if required
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

#include <kprocess.h>
#include <kconfig.h>
#include <kdatastream.h>
#include <dcopclient.h>
#include <kapplication.h>

KDE_EXPORT void
wake_laptop_daemon()
{
	DCOPClient      *dclient = kapp->dcopClient();
        if (!dclient || (!dclient->isAttached() && !dclient->attach())) 
		return;

	QByteArray data;
	QDataStream arg( data, IO_WriteOnly );
	(void) dclient->send( "kded", "klaptopdaemon", "restart()", data );
}
