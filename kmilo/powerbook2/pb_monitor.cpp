// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
/*
   This file is part of the KDE project

   Copyright (c) 2003 Zack Rusin <staikos@kde.org>
   Pretty much ripped of from :
	 George Staikos <staikos@kde.org> :)

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

#include <kgenericfactory.h>
#include <kdebug.h>

#include <sys/types.h>
#include <unistd.h>

extern "C" {
//Hack because of the fact that pbb author is using C++ keywords in the code
//among which is "template"...
#undef template
#include <pbb.h>

// TAG_BRIGHTNESS was renamed to TAG_LCDBRIGHTNESS in pbbuttons
// 0.6.1-2
#ifndef TAG_LCDBRIGHTNESS
#define TAG_LCDBRIGHTNESS TAG_BRIGHTNESS
#endif
}

#define BUFFERLEN	200
const QString tpmodes[] = { I18N_NOOP("No Tap"), I18N_NOOP("Tap"), I18N_NOOP("Drag"), I18N_NOOP("Lock") };

namespace KMilo {

PowerBookMonitor::PowerBookMonitor(QObject *parent, const char *name,
                                   const QStringList& args)
	: Monitor(parent, name, args),
		m_progress( 0 )
{
  	init_libpbb();
}

PowerBookMonitor::~PowerBookMonitor()
{
	ipc_exit();
}


bool PowerBookMonitor::init()
{
// pbbuttons 0.6.7 or newer
#ifdef CLIENT_REGISTER
	return ( ipc_init( "kmilo", LIBMODE_CLIENT, CLIENT_REGISTER ) == 0 );
#else
	return ( ipc_init( LIBMODE_CLIENT, 1 ) == 0 );
#endif
}


Monitor::DisplayType PowerBookMonitor::poll()
{
	Monitor::DisplayType rc = None;
	// Drain the queue, send the last message
	struct tagitem* tag = readMessage();
	m_message = QString::null;
	while ( tag && tag->tag != TAG_END ) {
		switch ( tag->tag ) {
		case TAG_VOLUME:
			rc = Monitor::Volume;
			m_progress = (int)tag->data;
			break;
		case TAG_MUTE:
			rc = Monitor::Mute;
			m_progress = (int)tag->data;
			break;
		case TAG_LCDBRIGHTNESS:
			rc = Monitor::Brightness;
			m_progress = ((int)tag->data)*100/15;
			break;
		case TAG_TPMODE:
			{
				rc = Monitor::Tap;
				QString marg =  tpmodes[ tag->data & 3 ];
				m_message = i18n( "Operating mode set to: %1." ).arg( marg );
			}
			break;
		default:
			break;
		}
		++tag;
	}

	if ( m_sleep ) {
		rc = Monitor::Sleep;
	}

	return rc;
}


int PowerBookMonitor::progress() const
{
	return m_progress;
}

QString PowerBookMonitor::message() const
{
	return m_message;
}

struct tagitem* PowerBookMonitor::readMessage()
{
	char buffer[BUFFERLEN];
	m_sleep = false;
	if ( (ipc_receive(buffer, BUFFERLEN)) >=0 ) {
		if ( buffer ) {
			struct pbbmessage *msg = reinterpret_cast<struct pbbmessage*>( buffer );
			switch ( msg->action ) {
			case REGFAILED:
				kdDebug()<<"PBB registration failed"<<endl;
				break;
			case CLIENTEXIT:
				kdDebug()<<"PBB client exited"<<endl;
				break;
			case CHANGEVALUE:
				return msg->taglist;
				break;
			case WARNING:
				if ( msg->taglist->data == 0 ) {
					m_message = i18n( "The computer will sleep now." );
				} else {
					m_message = i18n( "The computer will sleep in %n second.",
							  "The computer will sleep in %n seconds.",
							  msg->taglist->data );
				}
				m_sleep = true;
				kdDebug()<<"PBB Warning"<<endl;
				break;
			}
		}
	}
	return 0;
}

}

K_EXPORT_COMPONENT_FACTORY(kmilo_powerbook, KGenericFactory<KMilo::PowerBookMonitor>("kmilo_powerbook"))

