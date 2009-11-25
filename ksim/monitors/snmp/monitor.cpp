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

#include <qtimer.h>
#include <qapplication.h>

using namespace KSim::Snmp;

Monitor::Monitor( const HostConfig &host, const Identifier &oid, int refresh, QObject *parent, const char *name )
    : QObject( parent, name ), m_oid( oid ), m_session( host )
{
    if ( refresh > 0 )
        m_timerId = startTimer( refresh );
    else
        m_timerId = -1;

    QTimer::singleShot( 0, this, SLOT( performSnmpRequest() ) );
}

Monitor::~Monitor()
{
    if ( QThread::running() )
        QThread::wait();
}

void Monitor::run()
{
    AsyncSnmpQueryResult *result = new AsyncSnmpQueryResult;

    result->oid = m_oid;
    result->success = performSyncSnmpRequest( result->data, &result->error );

    QCustomEvent *ev = new QCustomEvent( QEvent::User, result );
    QApplication::postEvent( this, ev );
}

void Monitor::customEvent( QCustomEvent *ev )
{
    if ( ev->type() != QEvent::User )
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

void Monitor::timerEvent( QTimerEvent *ev )
{
    if ( ev->timerId() != m_timerId )
        return;

    performSnmpRequest();
}

void Monitor::performSnmpRequest()
{
    if ( QThread::running() )
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
