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

#ifndef WALKER_H
#define WALKER_H

#include <tqthread.h>
#include <tqobject.h>
#include <tqmutex.h>
#include <tqvaluelist.h>

#include "session.h"

namespace KSim
{

namespace Snmp
{

class Walker : public TQObject, public TQThread
{
    Q_OBJECT
  TQ_OBJECT
public:
    Walker( const HostConfig &host, const Identifier &startOid, TQObject *tqparent, const char *name = 0 );
    virtual ~Walker();

    struct Result
    {
        Result() : success( false ) {}
        bool success;
        Identifier oid;
        TQString identifierString;
        Value data;
        TQString dataString;
        ErrorInfo error;
    };

signals:
    void resultReady( const Walker::Result &result );
    void finished();

protected:
    virtual void run();
    virtual void timerEvent( TQTimerEvent *ev );
    virtual void customEvent( TQCustomEvent *ev );

private:
    typedef TQValueList<Result *> ResultList;

    ResultList m_results;
    TQMutex m_resultGuard;

    bool m_stop;
    TQMutex m_stopGuard;

    Identifier m_oid;
    int m_timerId;
    Session m_session;
};

}
}

#endif // WALKER_H
/* vim: et sw=4 ts=4
 */
