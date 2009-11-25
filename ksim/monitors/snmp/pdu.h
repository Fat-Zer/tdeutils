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

#ifndef PDU_H
#define PDU_H

#include "identifier.h"
#include "snmplib.h"

namespace KSim
{

namespace Snmp
{

class PDU
{
public:
    PDU();
    PDU( int requestType );
    ~PDU();

    void addNullVariables( const IdentifierList &oids );
    void addNullVariable( Identifier oid );

    netsnmp_pdu **operator&() { return &d; }

    netsnmp_pdu *release();

    bool hasError() const;
    int errorCode() const;

    ValueMap variables() const;

private:
    netsnmp_pdu *d;
};

}
}

#endif // PDU_H
/* vim: et sw=4 ts=4
 */
