/* This file is part of the KDE project
   Copyright (C) 2003 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "monitor.h"

#include <tqtimer.h>
#include <tqapplication.h>

using namespace KSim::Snmp;

Monitor::Monitor( const HostConfig &host, const Identifier &oid, int refresh, TQObject *parent, const char *name )
    : TQObject( parent, name ), m_oid( oid ), m_session( host )
{
    if ( refresh > 0 )
        m_timerId = startTimer( refresh );
    else
        m_timerId = -1;

    TQTimer::singleShot( 0, this, TQT_SLOT( performSnmpRequest() ) );
}

Monitor::~Monitor()
{
    if ( TQThread::running() )
        TQThread::wait();
}

void Monitor::run()
{
    AsyncSnmpQueryResult *result = new AsyncSnmpQueryResult;

    result->oid = m_oid;
    result->success = performSyncSnmpRequest( result->data, &result->error );

    TQCustomEvent *ev = new TQCustomEvent( TQEvent::User, result );
    TQApplication::postEvent( this, ev );
}

void Monitor::customEvent( TQCustomEvent *ev )
{
    if ( ev->type() != TQEvent::User )
        return;

    AsyncSnmpQueryResult *result = reinterpret_cast<AsyncSnmpQueryResult *>( ev->data() );
    if ( result->success ) {
        emit newData( result->data );
        emit newData( result->oid, result->data );
    } else {
        emit error( result->error );
        emit error( result->oid, result->error );
    }

    delete result;
}

void Monitor::timerEvent( TQTimerEvent *ev )
{
    if ( ev->timerId() != m_timerId )
        return;

    performSnmpRequest();
}

void Monitor::performSnmpRequest()
{
    if ( TQThread::running() )
        return;

    start();
}

bool Monitor::performSyncSnmpRequest( Value &data, ErrorInfo *errorInfo )
{
    return m_session.snmpGet( m_oid, data, errorInfo );
}

#include "monitor.moc"

/* vim: et sw=4 ts=4
 */
