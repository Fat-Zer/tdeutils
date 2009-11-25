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
#include "session.h"
#include "snmp.h"
#include "snmp_p.h"
#include "snmplib.h"
#include "pdu.h"

#include <assert.h>

using namespace KSim::Snmp;

// ### clean me up
struct Session::Data
{
    Data() : session( 0 ) {}

    netsnmp_session defaultSession;
    netsnmp_session *session;

    bool initialized;

    HostConfig source;

    QCString host;
    QCString community;
    QCString securityName;
    QCString authPassPhrase;
    QCString privPassPhrase;
};

Session::Session( const HostConfig &source )
{
    d = new Data;
    d->initialized = false;

    d->source = source;

    // unicode madness
    d->host = source.name.ascii();
    d->community = source.community.ascii();
    d->securityName = source.securityName.ascii();
    d->authPassPhrase = source.authentication.key.ascii();
    d->privPassPhrase = source.privacy.key.ascii();

    SnmpLib::self()->snmp_sess_init( &d->defaultSession );
}

bool Session::snmpGetInternal( int getType, const IdentifierList &identifiers, ValueMap &variables, ErrorInfo *error )
{
    if ( !d->initialized && !initialize( error ) )
        return false;

    bool result = false;

    if ( getType != SNMP_MSG_GETNEXT && d->session ) {
        SnmpLib::self()->snmp_close( d->session );
        d->session = 0;
    }

    if ( getType != SNMP_MSG_GETNEXT ||
         !d->session ) {

        if ( ( d->session = SnmpLib::self()->snmp_open( &d->defaultSession ) ) == 0 ) {
            if ( error )
                *error = ErrorInfo( sessionErrorCode( d->defaultSession ) );
            return false;
        }
    }

    PDU request( getType );
    PDU response;

    request.addNullVariables( identifiers );

    int status = SnmpLib::self()->snmp_synch_response( d->session, request.release(), &response );

    if ( status == STAT_SUCCESS ) {

        if ( response.hasError() ) {

            if ( error )
                *error = ErrorInfo( response.errorCode() );

        } else {

            variables = response.variables();
            result = true;

            if ( error )
                *error = ErrorInfo( ErrorInfo::NoError );
        }

    } else if ( status == STAT_TIMEOUT ) {

        if ( error )
            *error = ErrorInfo( ErrorInfo::ErrTimeout );

    } else {

        if ( error )
            *error = ErrorInfo( sessionErrorCode( *d->session ) );

    }

    if ( getType != SNMP_MSG_GETNEXT ) {
        SnmpLib::self()->snmp_close( d->session );
        d->session = 0;
    }

    return result;

}

bool Session::initialize( ErrorInfo *error )
{
    if ( d->initialized ) {
        if ( error )
            *error = ErrorInfo( ErrorInfo::NoError );
        return true;
    }

    HostConfig &source = d->source;

    d->defaultSession.peername = d->host.data();

    d->defaultSession.version = snmpVersionToSnmpLibConstant( source.version );

    if ( source.version != SnmpVersion3 ) {
        d->defaultSession.community = reinterpret_cast<u_char *>( d->community.data() );
        d->defaultSession.community_len = d->community.length();
        d->initialized = true;
        return true;
    }

    d->defaultSession.securityName = d->securityName.data();
    d->defaultSession.securityNameLen = d->securityName.length();

    d->defaultSession.securityLevel = snmpSecurityLevelToSnmpLibConstant( source.securityLevel );

    // ### clean me up
    switch ( source.authentication.protocol ) {
        case MD5Auth: {
                          d->defaultSession.securityAuthProto = usmHMACMD5AuthProtocol;
                          d->defaultSession.securityAuthProtoLen = USM_AUTH_PROTO_MD5_LEN;
                          break;
                      }
        case SHA1Auth: {
                           d->defaultSession.securityAuthProto = usmHMACSHA1AuthProtocol;
                           d->defaultSession.securityAuthProtoLen = USM_AUTH_PROTO_SHA_LEN;
                           break;
                       }
        default: assert( false );
    }

    d->defaultSession.securityAuthKeyLen = USM_AUTH_KU_LEN;

    int result;
    if ( ( result = SnmpLib::self()->generate_Ku( d->defaultSession.securityAuthProto, d->defaultSession.securityAuthProtoLen,
                    reinterpret_cast<uchar *>( d->authPassPhrase.data() ), d->authPassPhrase.length(),
                    d->defaultSession.securityAuthKey, &d->defaultSession.securityAuthKeyLen ) )
            != SNMPERR_SUCCESS ) {

        if ( error )
            *error = ErrorInfo( result );

        return false;
    }

    switch ( source.privacy.protocol ) {
        case DESPrivacy: {
                             d->defaultSession.securityPrivProto = usmDESPrivProtocol;
                             d->defaultSession.securityPrivProtoLen = USM_PRIV_PROTO_DES_LEN;
                             break;
                         }
        default: assert( false );
    }

    d->defaultSession.securityPrivKeyLen = USM_PRIV_KU_LEN;

    if ( ( result = SnmpLib::self()->generate_Ku( d->defaultSession.securityAuthProto, d->defaultSession.securityAuthProtoLen,
                    reinterpret_cast<u_char *>( d->privPassPhrase.data() ), d->privPassPhrase.length(),
                    d->defaultSession.securityPrivKey, &d->defaultSession.securityPrivKeyLen ) )
            != SNMPERR_SUCCESS ) {

        if ( error )
            *error = ErrorInfo( result );


        return false;
    }

    d->initialized = true;
    return true;
}

bool Session::snmpGet( const QString &identifier, Value &value, ErrorInfo *error )
{
    bool ok = false;
    Identifier oid = Identifier::fromString( identifier, &ok );
    if ( !ok ) {
        if ( error )
            *error = ErrorInfo( ErrorInfo::ErrUnknownOID );
        return false;
    }

    return snmpGet( oid, value, error );
}

bool Session::snmpGet( const Identifier &identifier, Value &value, ErrorInfo *error )
{
    ValueMap vars;
    IdentifierList ids;

    ids << identifier;

    if ( !snmpGet( ids, vars, error ) )
        return false;

    ValueMap::ConstIterator it = vars.find( identifier );
    if ( it == vars.end() ) {
        if ( error )
            *error = ErrorInfo( ErrorInfo::ErrMissingVariables );
        return false;
    }

    value = it.data();

    return true;
}

bool Session::snmpGet( const IdentifierList &identifiers, ValueMap &variables, ErrorInfo *error )
{
    return snmpGetInternal( SNMP_MSG_GET, identifiers, variables, error );
}

bool Session::snmpGetNext( Identifier &identifier, Value &value, ErrorInfo *error )
{
    ValueMap vars;
    IdentifierList ids;

    ids << identifier;

    if ( !snmpGetInternal( SNMP_MSG_GETNEXT, ids, vars, error ) )
        return false;

    assert( vars.count() == 1 );

    ValueMap::ConstIterator it = vars.begin();
    identifier = it.key();
    value = it.data();

    return true;

}

Session::~Session()
{
    if ( d->session )
        SnmpLib::self()->snmp_close( d->session );
    delete d;
}

/* vim: et sw=4 ts=4
 */
