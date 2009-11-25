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

#include "config.h"
#include "pdu.h"
#include "snmp_p.h"
#include "value_p.h"

#include <algorithm>
#include <functional>

#include <assert.h>

using namespace KSim::Snmp;

PDU::PDU()
    : d( 0 )
{
}

PDU::PDU( int requestType )
{
    d = SnmpLib::self()->snmp_pdu_create( requestType );
}

PDU::~PDU()
{
    if ( d )
        SnmpLib::self()->snmp_free_pdu( d );
}

void PDU::addNullVariables( const IdentifierList &oids )
{
    std::for_each( oids.begin(), oids.end(),
                   std::bind1st( std::mem_fun( &PDU::addNullVariable ), this ) );
}

void PDU::addNullVariable( Identifier oid )
{
    assert( d );
    SnmpLib::self()->snmp_add_null_var( d, oid.d->data, oid.d->length );
}

netsnmp_pdu *PDU::release()
{
    netsnmp_pdu *res = d;
    d = 0;
    return res;
}

bool PDU::hasError() const
{
    if ( !d )
        return false;

    return d->errstat != SNMP_ERR_NOERROR;
}

int PDU::errorCode() const
{
    return d->errstat;
}

ValueMap PDU::variables() const
{
    if ( !d )
        return ValueMap();

    ValueMap result;

    for ( variable_list *var = d->variables; var; var = var->next_variable ) {
        Identifier oid( new Identifier::Data( var->name, var->name_length ) );
        Value value( new ValueImpl( var ) );

        result.insert( oid, value );
    }

    return result;
}

/* vim: et sw=4 ts=4
 */
