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

#ifndef MONITOR_H
#define MONITOR_H

#include "monitorconfig.h"
#include "session.h"

#include <qthread.h>

namespace KSim
{

namespace Snmp
{

class Monitor : public QObject, public QThread
{
    Q_OBJECT
public:
    Monitor( const HostConfig &host, const Identifier &oid, int refresh, QObject *parent = 0, const char *name = 0 );
    virtual ~Monitor();

signals:
    void newData( const Value &data );
    void newData( const Identifier &oid, const Value &data );
    void error( const ErrorInfo &errorInfo );
    void error( const Identifier &oid, const ErrorInfo &errorInfo );

protected:
    virtual void run();
    virtual void customEvent( QCustomEvent *ev );
    virtual void timerEvent( QTimerEvent *ev );

private slots:
    void performSnmpRequest();
    bool performSyncSnmpRequest( Value &data, ErrorInfo *errorInfo = 0 );

private:
    struct AsyncSnmpQueryResult
    {
        AsyncSnmpQueryResult() : success( false ) {}
        Identifier oid;
        Value data;
        ErrorInfo error;
        bool success;
    };

    const Identifier m_oid;
    int m_timerId;
    Session m_session;
};

}

}

#endif // MONITOR_H
/* vim: et sw=4 ts=4
 */
