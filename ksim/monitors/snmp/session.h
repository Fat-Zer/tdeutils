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

#ifndef SESSION_H
#define SESSION_H

#include "hostconfig.h"
#include "value.h"
#include "identifier.h"

namespace KSim
{

namespace Snmp
{

class Session
{
public:
    struct Data;

    Session( const HostConfig &snmpHostConfig );
    ~Session();

    bool snmpGet( const QString &identifier, Value &value, ErrorInfo *error = 0 );
    bool snmpGet( const Identifier &identifier, Value &value, ErrorInfo *error = 0 );
    bool snmpGet( const IdentifierList &identifiers, ValueMap &variables, ErrorInfo *error = 0 );

    bool snmpGetNext( Identifier &identifier, Value &value, ErrorInfo *error = 0 );

private:
    Data *d;

    bool snmpGetInternal( int getType, const IdentifierList &identifiers, ValueMap &variables, ErrorInfo *error = 0 );

    bool initialize( ErrorInfo *error );

    Session( const Session & );
    Session &operator=( const Session & );
};

}

}

#endif // SESSION_H
/* vim: et sw=4 ts=4
 */
