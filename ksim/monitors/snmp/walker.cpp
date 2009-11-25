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

#include "walker.h"

#include <qapplication.h>

#include <functional>
#include <algorithm>

using namespace KSim::Snmp;

namespace
{
    template <class T>
    struct Deleter : public std::unary_function<T, void>
    {
        void operator()( T arg )
        { delete arg; }
    };
}

static const uint walkerRefresh = 0;

Walker::Walker( const HostConfig &host, const Identifier &startOid, QObject *parent, const char *name )
    : QObject( parent, name ), m_stop( false ), m_oid( startOid ), m_session( host )
{
    m_timerId = startTimer( walkerRefresh );
    start();
}

Walker::~Walker()
{
    m_stopGuard.lock();
    m_stop = true;
    m_stopGuard.unlock();

    if ( QThread::running() )
        QThread::wait();

    std::for_each( m_results.begin(), m_results.end(), Deleter<Result *>() );
}

void Walker::run()
{
    while ( !m_stop ) {
        Result *result = new Result;

        result->success = m_session.snmpGetNext( m_oid, result->data, &result->error );
        result->oid = m_oid;

        // do it here, because it sometimes is slow and can block for a bit
        if ( result->success ) {
            result->identifierString = result->oid.toString();
            result->dataString = result->data.toString();
        }

        m_stopGuard.lock();
        if ( !m_stop )
            m_stop = !result->success;
        m_stopGuard.unlock();

        m_resultGuard.lock();
        m_results << result;
        m_resultGuard.unlock();
    }

    // cause finished signal to be emitted
    QApplication::postEvent( this, new QCustomEvent( QEvent::User ) );
}

void Walker::timerEvent( QTimerEvent *ev )
{
    if ( ev->timerId() != m_timerId )
        return;

    Result *result = 0;

    m_resultGuard.lock();
    if ( !m_results.isEmpty() ) {
        result = m_results.first();
        m_results.remove( m_results.begin() );
    }
    m_resultGuard.unlock();

    if ( result ) {
        emit resultReady( *result );

        delete result;
    }

    if ( !QThread::running() && !result ) {
        killTimer( m_timerId );
        m_timerId = 0;
    }
}

void Walker::customEvent( QCustomEvent * )
{
    emit finished();
}

#include "walker.moc"
/* vim: et sw=4 ts=4
 */
