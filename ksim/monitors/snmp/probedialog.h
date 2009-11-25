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

#ifndef PROBEDIALOG_H
#define PROBEDIALOG_H

#include <qvaluestack.h>

#include <kprogress.h>

#include "hostconfig.h"
#include "identifier.h"

namespace KSim
{

namespace Snmp
{

class Monitor;

class ProbeDialog : public KProgressDialog
{
    Q_OBJECT
public:
    ProbeDialog( const HostConfig &hostConfig, QWidget *parent, const char *name = 0 );

    struct ProbeResult
    {
        ProbeResult() : success( false ) {}
        ProbeResult( const Identifier &_oid, const Value &_value )
            : oid( _oid ), value( _value ), success( true )
        {}
        ProbeResult( const Identifier &_oid, const ErrorInfo &_errorInfo )
            : oid( _oid ), success( false ), errorInfo( _errorInfo )
        {}

        Identifier oid;
        Value value;
        bool success;
        ErrorInfo errorInfo;
    };
    typedef QValueList<ProbeResult> ProbeResultList;

    ProbeResultList probeResults() const { return m_results; }

protected:
    virtual void done( int code );

private slots:
    void probeOne();
    void probeResult( const Identifier &oid, const Value &value );
    void probeError( const Identifier &oid, const ErrorInfo &errorInfo );

private:
    void nextProbe();

    HostConfig m_host;
    QValueStack<Identifier> m_probeOIDs;
    Monitor *m_currentMonitor;

    ProbeResultList m_results;
    bool m_canceled;
};

}
}

#endif // PROBEDIALOG_H
/* vim: et sw=4 ts=4
 */
