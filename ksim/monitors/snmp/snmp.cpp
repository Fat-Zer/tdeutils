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
#include "snmp.h"
#include "snmp_p.h"
#include "snmplib.h"
#include "value_p.h"

#include <qmutex.h>

#include <assert.h>

using namespace KSim::Snmp;

static const struct EnumStringMapInfo
{
    const int enumValue;
    const char *stringValue;
    const char snmpLibConstant;
} snmpVersionStrings[ 4 ] = {
    { SnmpVersion1, "1", SNMP_VERSION_1 },
    { SnmpVersion2c, "2c", SNMP_VERSION_2c },
    { SnmpVersion3, "3", SNMP_VERSION_3 },
    { 0, 0, 0 }
}, securityLevelStrings[ 4 ] = {
    { NoAuthPriv, "NoAuthPriv", SNMP_SEC_LEVEL_NOAUTH },
    { AuthNoPriv, "AuthNoPriv", SNMP_SEC_LEVEL_AUTHNOPRIV },
    { AuthPriv, "AuthPriv", SNMP_SEC_LEVEL_AUTHPRIV },
    { 0, 0, 0 }
}, authenticationProtocolStrings[ 3 ] = {
    { MD5Auth, "MD5", 0 },
    { SHA1Auth, "SHA1", 0 },
    { 0, 0, 0 }
}, privacyProtocolStrings[ 2 ] = {
    { DESPrivacy, "DES", 0 },
    { 0, 0, 0 }
};

static QStringList allStrings( const EnumStringMapInfo *array )
{
    QStringList result;
    for ( uint i = 0; array[ i ].stringValue; ++i )
        result << QString::fromLatin1( array[ i ].stringValue );
    return result;
}

static QString enumToString( const EnumStringMapInfo *array, int value )
{
    for ( uint i = 0; array[ i ].stringValue; ++i )
        if ( array[ i ].enumValue == value )
            return QString::fromLatin1( array[ i ].stringValue );

    assert( false );
    return QString::null;
}

static int stringToEnum( const EnumStringMapInfo *array, QString string, bool *ok )
{
    string = string.lower();
    uint i;
    for ( i = 0; array[ i ].stringValue; ++i )
        if ( QString::fromLatin1( array[ i ].stringValue ).lower() == string ) {
            if ( ok ) *ok = true;
            return array[ i ].enumValue;
        }

    if ( ok )
        *ok = false;

    // something..
    return array[ 0 ].enumValue;
}

static int extractSnmpLibConstant( const EnumStringMapInfo *array, int enumValue )
{
    for ( uint i = 0; array[ i ].stringValue; ++i )
        if ( array[ i ].enumValue == enumValue )
            return array[ i ].snmpLibConstant;

    assert( false );
    return 0;
}

int KSim::Snmp::snmpVersionToSnmpLibConstant( SnmpVersion version )
{
    return extractSnmpLibConstant( snmpVersionStrings, version );
}

int KSim::Snmp::snmpSecurityLevelToSnmpLibConstant( SecurityLevel secLevel )
{
    return extractSnmpLibConstant( securityLevelStrings, secLevel );
}

QStringList KSim::Snmp::allSnmpVersions()
{
    return allStrings( snmpVersionStrings );
}

QString KSim::Snmp::snmpVersionToString( SnmpVersion version )
{
    return enumToString( snmpVersionStrings, version );
}

SnmpVersion KSim::Snmp::stringToSnmpVersion( QString string, bool *ok )
{
    return static_cast<SnmpVersion>( stringToEnum( snmpVersionStrings, string, ok ) );
}

QStringList KSim::Snmp::allSecurityLevels()
{
    return allStrings( securityLevelStrings );
}

QString KSim::Snmp::securityLevelToString( SecurityLevel level )
{
    return enumToString( securityLevelStrings, level );
}

SecurityLevel KSim::Snmp::stringToSecurityLevel( QString string, bool *ok )
{
    return static_cast<SecurityLevel>( stringToEnum( securityLevelStrings, string, ok ) );
}

QStringList KSim::Snmp::allAuthenticationProtocols()
{
    return allStrings( authenticationProtocolStrings );
}

QString KSim::Snmp::authenticationProtocolToString( AuthenticationProtocol proto )
{
    return enumToString( authenticationProtocolStrings, proto );
}

AuthenticationProtocol KSim::Snmp::stringToAuthenticationProtocol( QString string, bool *ok )
{
    return static_cast<AuthenticationProtocol>( stringToEnum( authenticationProtocolStrings, string, ok ) );
}

QStringList KSim::Snmp::allPrivacyProtocols()
{
    return allStrings( privacyProtocolStrings );
}

QString KSim::Snmp::privacyProtocolToString( PrivacyProtocol proto )
{
    return enumToString( privacyProtocolStrings, proto );
}

PrivacyProtocol KSim::Snmp::stringToPrivacyProtocol( QString string, bool *ok )
{
    return static_cast<PrivacyProtocol>( stringToEnum( privacyProtocolStrings, string, ok ) );
}

// I'm afraid of them changing the order in the error constants or the like, hence the
// slow list instead of a fast lookup table
static const struct ErrorMapInfo
{
    int errorCode;
    ErrorInfo::ErrorType enumValue;
} errorMap[] = 
{
    // API Errors
    { SNMPERR_GENERR, ErrorInfo::ErrGeneric },
    { SNMPERR_BAD_LOCPORT, ErrorInfo::ErrInvalidLocalPort },
    { SNMPERR_BAD_ADDRESS, ErrorInfo::ErrUnknownHost },
    { SNMPERR_BAD_SESSION, ErrorInfo::ErrUnknownSession },
    { SNMPERR_TOO_LONG, ErrorInfo::ErrTooLong },
    { SNMPERR_NO_SOCKET, ErrorInfo::ErrNoSocket },
    { SNMPERR_V2_IN_V1, ErrorInfo::ErrCannotSendV2PDUOnV1Session },
    { SNMPERR_V1_IN_V2, ErrorInfo::ErrCannotSendV1PDUOnV2Session },
    { SNMPERR_BAD_REPEATERS, ErrorInfo::ErrBadValueForNonRepeaters },
    { SNMPERR_BAD_REPETITIONS, ErrorInfo::ErrBadValueForMaxRepetitions },
    { SNMPERR_BAD_ASN1_BUILD, ErrorInfo::ErrBuildingASN1Representation },
    { SNMPERR_BAD_SENDTO, ErrorInfo::ErrFailureInSendto },
    { SNMPERR_BAD_PARSE, ErrorInfo::ErrBadParseOfASN1Type },
    { SNMPERR_BAD_VERSION, ErrorInfo::ErrBadVersion },
    { SNMPERR_BAD_SRC_PARTY, ErrorInfo::ErrBadSourceParty },
    { SNMPERR_BAD_DST_PARTY, ErrorInfo::ErrBadDestinationParty },
    { SNMPERR_BAD_CONTEXT, ErrorInfo::ErrBadContext },
    { SNMPERR_BAD_COMMUNITY, ErrorInfo::ErrBadCommunity },
    { SNMPERR_NOAUTH_DESPRIV, ErrorInfo::ErrCannotSendNoAuthDesPriv },
    { SNMPERR_BAD_ACL, ErrorInfo::ErrBadACL },
    { SNMPERR_BAD_PARTY, ErrorInfo::ErrBadParty },
    { SNMPERR_ABORT, ErrorInfo::ErrSessionAbortFailure },
    { SNMPERR_UNKNOWN_PDU, ErrorInfo::ErrUnknownPDU },
    { SNMPERR_TIMEOUT, ErrorInfo::ErrTimeout },
    { SNMPERR_BAD_RECVFROM, ErrorInfo::ErrFailureInRecvfrom },
    { SNMPERR_BAD_ENG_ID, ErrorInfo::ErrUnableToDetermineContextEngineID },
    { SNMPERR_BAD_SEC_NAME, ErrorInfo::ErrNoSecurityName },
    { SNMPERR_BAD_SEC_LEVEL, ErrorInfo::ErrUnableToDetermineSecurityLevel },
    { SNMPERR_ASN_PARSE_ERR, ErrorInfo::ErrASN1ParseError },
    { SNMPERR_UNKNOWN_SEC_MODEL, ErrorInfo::ErrUnknownSecurityModel },
    { SNMPERR_INVALID_MSG, ErrorInfo::ErrInvalidMessage },
    { SNMPERR_UNKNOWN_ENG_ID, ErrorInfo::ErrUnknownEngineID },
    { SNMPERR_UNKNOWN_USER_NAME, ErrorInfo::ErrUnknownUserName },
    { SNMPERR_UNSUPPORTED_SEC_LEVEL, ErrorInfo::ErrUnsupportedSecurityLevel },
    { SNMPERR_AUTHENTICATION_FAILURE, ErrorInfo::ErrAuthenticationFailure },
    { SNMPERR_NOT_IN_TIME_WINDOW, ErrorInfo::ErrNotInTimeWindow },
    { SNMPERR_DECRYPTION_ERR, ErrorInfo::ErrDecryptionError },
    { SNMPERR_SC_GENERAL_FAILURE, ErrorInfo::ErrSCAPIGeneralFailure },
    { SNMPERR_SC_NOT_CONFIGURED, ErrorInfo::ErrSCAPISubSystemNotConfigured },
    { SNMPERR_KT_NOT_AVAILABLE, ErrorInfo::ErrNoKeyTools },
    { SNMPERR_UNKNOWN_REPORT, ErrorInfo::ErrUnknownReport },
    { SNMPERR_USM_GENERICERROR, ErrorInfo::ErrUSMGenericError },
    { SNMPERR_USM_UNKNOWNSECURITYNAME, ErrorInfo::ErrUSMUnknownSecurityName },
    { SNMPERR_USM_UNSUPPORTEDSECURITYLEVEL, ErrorInfo::ErrUSMUnsupportedSecurityLevel },
    { SNMPERR_USM_ENCRYPTIONERROR, ErrorInfo::ErrUSMEncryptionError },
    { SNMPERR_USM_AUTHENTICATIONFAILURE, ErrorInfo::ErrUSMAuthenticationFailure },
    { SNMPERR_USM_PARSEERROR, ErrorInfo::ErrUSMParseError },
    { SNMPERR_USM_UNKNOWNENGINEID, ErrorInfo::ErrUSMUnknownEngineID },
    { SNMPERR_USM_NOTINTIMEWINDOW, ErrorInfo::ErrUSMNotInTimeWindow },
    { SNMPERR_USM_DECRYPTIONERROR, ErrorInfo::ErrUSMDecryptionError },
    { SNMPERR_NOMIB, ErrorInfo::ErrMIBNotInitialized },
    { SNMPERR_RANGE, ErrorInfo::ErrValueOutOfRange },
    { SNMPERR_MAX_SUBID, ErrorInfo::ErrSubIdOutOfRange },
    { SNMPERR_BAD_SUBID, ErrorInfo::ErrBadSubIdInOID },
    { SNMPERR_LONG_OID, ErrorInfo::ErrOIDTooLong },
    { SNMPERR_BAD_NAME, ErrorInfo::ErrBadValueName },
    { SNMPERR_VALUE, ErrorInfo::ErrBadValueNotation },
    { SNMPERR_UNKNOWN_OBJID, ErrorInfo::ErrUnknownOID },
    { SNMPERR_NULL_PDU, ErrorInfo::ErrNullPDU },
    { SNMPERR_NO_VARS, ErrorInfo::ErrMissingVariables },
    { SNMPERR_VAR_TYPE, ErrorInfo::ErrBadVariableType },
    { SNMPERR_MALLOC, ErrorInfo::ErrOOM },
    { SNMPERR_KRB5, ErrorInfo::ErrKerberos },

    // PDU response errors
    { SNMP_ERR_TOOBIG, ErrorInfo::ErrResponseTooLarge },
    { SNMP_ERR_NOSUCHNAME, ErrorInfo::ErrNoSuchVariable },
    { SNMP_ERR_BADVALUE, ErrorInfo::ErrBadValue },
    { SNMP_ERR_READONLY, ErrorInfo::ErrReadOnly },
    { SNMP_ERR_GENERR, ErrorInfo::ErrGeneric },
    { SNMP_ERR_NOACCESS, ErrorInfo::ErrNoAccess },
    { SNMP_ERR_WRONGTYPE, ErrorInfo::ErrWrongType },
    { SNMP_ERR_WRONGLENGTH, ErrorInfo::ErrWrongLength },
    { SNMP_ERR_WRONGENCODING, ErrorInfo::ErrWrongEncoding },
    { SNMP_ERR_WRONGVALUE, ErrorInfo::ErrWrongValue },
    { SNMP_ERR_NOCREATION, ErrorInfo::ErrNoCreation },
    { SNMP_ERR_INCONSISTENTVALUE, ErrorInfo::ErrInconsistentValue },
    { SNMP_ERR_RESOURCEUNAVAILABLE, ErrorInfo::ErrResourceUnavailable },
    { SNMP_ERR_COMMITFAILED, ErrorInfo::ErrCommitFailed },
    { SNMP_ERR_UNDOFAILED, ErrorInfo::ErrUndoFailed },
    { SNMP_ERR_AUTHORIZATIONERROR, ErrorInfo::ErrAuthorizationFailed },
    { SNMP_ERR_NOTWRITABLE, ErrorInfo::ErrNotWritable },
    { SNMP_ERR_INCONSISTENTNAME, ErrorInfo::ErrInconsistentName },

    { SNMPERR_SUCCESS, ErrorInfo::NoError }
};

ErrorInfo::ErrorType KSim::Snmp::convertSnmpLibErrorToErrorInfo( int error )
{
    for ( uint i = 0; errorMap[ i ].errorCode != SNMPERR_SUCCESS; ++i )
        if ( errorMap[ i ].errorCode == error )
            return errorMap[ i ].enumValue;
    return ErrorInfo::ErrUnknown;
}

int KSim::Snmp::convertErrorInfoToSnmpLibError( ErrorInfo::ErrorType error )
{
    for ( uint i = 0; errorMap[ i ].errorCode != SNMPERR_SUCCESS; ++i )
        if ( errorMap[ i ].enumValue == error )
            return errorMap[ i ].errorCode;
    assert( false );
    return SNMPERR_SUCCESS;
}

int KSim::Snmp::sessionErrorCode( netsnmp_session &session )
{
    int errorCode = 0;
    SnmpLib::self()->snmp_error( &session, 0, &errorCode, 0 );
    return errorCode;
}

static QString messageForErrorCode( int errorCode )
{
    if ( errorCode >= SNMPERR_MAX && errorCode <= SNMPERR_GENERR )
        return QString::fromLatin1( SnmpLib::self()->snmp_api_errstring( errorCode ) );
    if ( errorCode >= SNMP_ERR_NOERROR && errorCode <= MAX_SNMP_ERR )
        return QString::fromLatin1( SnmpLib::self()->snmp_errstring( errorCode ) );

    return QString::null;
}

ErrorInfo::ErrorInfo()
{
    m_errorCode = NoError;
}

ErrorInfo::ErrorInfo( int internalErrorCode )
{
    m_errorCode = convertSnmpLibErrorToErrorInfo( internalErrorCode );
    m_errorMessage = messageForErrorCode( internalErrorCode );
}

ErrorInfo::ErrorInfo( ErrorType error )
{
    m_errorCode = error;
    if ( error != NoError && error != ErrUnknown )
        m_errorMessage = messageForErrorCode( convertErrorInfoToSnmpLibError( error ) );
}

/* vim: et sw=4 ts=4
 */
