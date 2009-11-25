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
#include "snmplib.h"
#include "snmp_p.h"

#include <kstaticdeleter.h>

using namespace KSim::Snmp;

SnmpLib *SnmpLib::s_self;

::KStaticDeleter<KSim::Snmp::SnmpLib> sd;

SnmpLib::SnmpLib()
{
    m_lockHelper = new ClassLocker<SnmpLib>( &m_guard, this );

    init_snmp( "ksim" );
}

SnmpLib::~SnmpLib()
{
    delete m_lockHelper;
}

ClassLocker<SnmpLib> &SnmpLib::self()
{
    if ( !s_self ) {
        static QMutex singletonGuard;

        QMutexLocker locker( &singletonGuard );
        if ( !s_self )
            sd.setObject( s_self, new SnmpLib );
    }
    return *s_self->m_lockHelper;
}

oid *SnmpLib::snmp_parse_oid( const char *name, oid *oid, size_t *length )
{
    return ::snmp_parse_oid( name, oid, length );
}

int SnmpLib::netsnmp_ds_set_int( int storeid, int which, int value )
{
    return ::netsnmp_ds_set_int( storeid, which, value );
}

int SnmpLib::netsnmp_ds_get_int( int storeid, int which )
{
    return ::netsnmp_ds_get_int( storeid, which );
}

struct tree *SnmpLib::netsnmp_sprint_realloc_objid_tree( u_char ** buf,
                                                         size_t * buf_len,
                                                         size_t * out_len,
                                                         int allow_realloc,
                                                         int *buf_overflow,
                                                         const oid * objid,
                                                         size_t objidlen )
{
    return ::netsnmp_sprint_realloc_objid_tree( buf, buf_len, out_len, allow_realloc, buf_overflow, objid, objidlen );
}

int SnmpLib::netsnmp_oid_equals( const oid *firstOid, size_t firstOidLen, const oid *secondOid, size_t secondOidLen )
{
    return ::netsnmp_oid_equals( firstOid, firstOidLen, secondOid, secondOidLen );
}

int SnmpLib::snmp_oid_compare( const oid *firstOid, size_t firstOidLen, const oid *secondOid, size_t secondOidLen )
{
    return ::snmp_oid_compare( firstOid, firstOidLen, secondOid, secondOidLen );
}

void SnmpLib::snmp_error( netsnmp_session *session, int *p_errno, int *p_snmp_errno, char **p_str )
{
    ::snmp_error( session, p_errno, p_snmp_errno, p_str );
}

const char *SnmpLib::snmp_api_errstring( int code )
{
    return ::snmp_api_errstring( code );
}

const char *SnmpLib::snmp_errstring( int code )
{
    return ::snmp_errstring( code );
}

netsnmp_pdu *SnmpLib::snmp_pdu_create( int pduType )
{
    return ::snmp_pdu_create( pduType );
}

void SnmpLib::snmp_free_pdu( netsnmp_pdu *pdu )
{
    ::snmp_free_pdu( pdu );
}

netsnmp_variable_list *SnmpLib::snmp_add_null_var( netsnmp_pdu *pdu, oid *var, size_t varlen )
{
    return ::snmp_add_null_var( pdu, var, varlen );
}

void SnmpLib::snmp_sess_init( netsnmp_session *session )
{
    ::snmp_sess_init( session );
}

netsnmp_session *SnmpLib::snmp_open( netsnmp_session *session )
{
    return ::snmp_open( session );
}

int SnmpLib::snmp_close( netsnmp_session *session )
{
    return ::snmp_close( session );
}

int SnmpLib::snmp_synch_response( netsnmp_session *session, netsnmp_pdu *pdu,
                                  netsnmp_pdu **response )
{
    return ::snmp_synch_response( session, pdu, response );
}

int SnmpLib::generate_Ku( const oid * hashtype, u_int hashtype_len,
                          u_char * P, size_t pplen,
                          u_char * Ku, size_t * kulen )
{
    return ::generate_Ku( hashtype, hashtype_len, P, pplen, Ku, kulen );
}

/* vim: et sw=4 ts=4
 */
