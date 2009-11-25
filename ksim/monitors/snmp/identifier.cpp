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

#include "identifier.h"
#include "snmp_p.h"
#include "snmplib.h"

#include <assert.h>

using namespace KSim::Snmp;

Identifier::Data::Data( const oid *d, size_t length )
{
    memcpy( data, d, length * sizeof( oid ) );
    this->length = length;
}

Identifier::Identifier()
    : d( new Data )
{
}

Identifier::Identifier( Data *data )
    : d( data )
{
    assert( d );
}

Identifier::Identifier( const Identifier &rhs )
    : d( new Data( *rhs.d ) )
{
}

Identifier &Identifier::operator=( const Identifier &rhs )
{
    if ( d == rhs.d )
        return *this;

    *d = *rhs.d;

    return *this;
}

Identifier::~Identifier()
{
    delete d;
}

Identifier Identifier::fromString( const QString &name, bool *ok )
{
    Identifier res;
    res.d->length = MAX_OID_LEN;
    // if ( !get_node( name.ascii(), res.d->data, &res.d->length ) ) {
    if ( name.isEmpty() || !SnmpLib::self()->snmp_parse_oid( name.ascii(), res.d->data, &res.d->length ) ) {
        if ( ok )
            *ok = false;
        return Identifier();
    }

    if ( ok )
        *ok = true;

    return res;
}

QString Identifier::toString( PrintFlags flags ) const
{
    size_t buflen = 256;
    size_t outlen = 0;
    int overflow = 0;

    u_char *buf = ( u_char* )calloc( buflen, 1 );
    if ( !buf )
        return QString::null;

    int oldOutpuFormat = SnmpLib::self()->netsnmp_ds_get_int( NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT );
    SnmpLib::self()->netsnmp_ds_set_int( NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT, 
                                         flags == PrintAscii ? NETSNMP_OID_OUTPUT_MODULE : NETSNMP_OID_OUTPUT_NUMERIC );

    SnmpLib::self()->netsnmp_sprint_realloc_objid_tree( &buf, &buflen, &outlen, 1 /* alloc realloc */,
                                                        &overflow, d->data, d->length );

    SnmpLib::self()->netsnmp_ds_set_int( NETSNMP_DS_LIBRARY_ID, NETSNMP_DS_LIB_OID_OUTPUT_FORMAT, oldOutpuFormat );

    if ( overflow ) {
        free( buf );
        return QString::null;
    }

    QString result = QString::fromAscii( ( char* )buf );

    free( buf );

    return result;
}

bool Identifier::operator==( const Identifier &rhs ) const
{
    return SnmpLib::self()->netsnmp_oid_equals( d->data, d->length, rhs.d->data, rhs.d->length ) == 0;
}

bool Identifier::operator<( const Identifier &rhs ) const
{
    return SnmpLib::self()->snmp_oid_compare( d->data, d->length, rhs.d->data, rhs.d->length ) < 0;
}

bool Identifier::isNull() const
{
    return d->length == 0;
}

/* vim: et sw=4 ts=4
 */
