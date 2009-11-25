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

#ifndef KSIM_SNMP_H
#define KSIM_SNMP_H

#include <kconfigbase.h>

#include <qmap.h>


namespace KSim
{

namespace Snmp
{

enum SnmpVersion { SnmpVersion1, SnmpVersion2c, SnmpVersion3, SnmpLastVersion };
QStringList allSnmpVersions();
QString snmpVersionToString( SnmpVersion version );
SnmpVersion stringToSnmpVersion( QString string, bool *ok = 0 );

enum SecurityLevel { NoAuthPriv, AuthNoPriv, AuthPriv, LastSecurityLevel };
QStringList allSecurityLevels();
QString securityLevelToString( SecurityLevel level );
SecurityLevel stringToSecurityLevel( QString string, bool *ok = 0 );

enum AuthenticationProtocol { MD5Auth, SHA1Auth, LastAuthenticationProtocol };
QStringList allAuthenticationProtocols();
QString authenticationProtocolToString( AuthenticationProtocol proto );
AuthenticationProtocol stringToAuthenticationProtocol( QString string, bool *ok = 0 );

enum PrivacyProtocol { DESPrivacy };
QStringList allPrivacyProtocols();
QString privacyProtocolToString( PrivacyProtocol proto );
PrivacyProtocol stringToPrivacyProtocol( QString string, bool *ok = 0 );

class ErrorInfo
{
public:
    enum ErrorType {
        NoError = 0,
        ErrUnknown, // ... if we forgot a mapping of error codes

        // api errors
        ErrGeneric,
        ErrInvalidLocalPort,
        ErrUnknownHost,
        ErrUnknownSession,
        ErrTooLong,
        ErrNoSocket,
        ErrCannotSendV2PDUOnV1Session,
        ErrCannotSendV1PDUOnV2Session,
        ErrBadValueForNonRepeaters,
        ErrBadValueForMaxRepetitions,
        ErrBuildingASN1Representation,
        ErrFailureInSendto,
        ErrBadParseOfASN1Type,
        ErrBadVersion,
        ErrBadSourceParty,
        ErrBadDestinationParty,
        ErrBadContext,
        ErrBadCommunity,
        ErrCannotSendNoAuthDesPriv,
        ErrBadACL,
        ErrBadParty,
        ErrSessionAbortFailure,
        ErrUnknownPDU,
        ErrTimeout,
        ErrFailureInRecvfrom,
        ErrUnableToDetermineContextEngineID,
        ErrNoSecurityName,
        ErrUnableToDetermineSecurityLevel,
        ErrASN1ParseError,
        ErrUnknownSecurityModel,
        ErrInvalidMessage,
        ErrUnknownEngineID,
        ErrUnknownUserName,
        ErrUnsupportedSecurityLevel,
        ErrAuthenticationFailure,
        ErrNotInTimeWindow,
        ErrDecryptionError,
        ErrSCAPIGeneralFailure,
        ErrSCAPISubSystemNotConfigured,
        ErrNoKeyTools,
        ErrUnknownReport,
        ErrUSMGenericError,
        ErrUSMUnknownSecurityName,
        ErrUSMUnsupportedSecurityLevel,
        ErrUSMEncryptionError,
        ErrUSMAuthenticationFailure,
        ErrUSMParseError,
        ErrUSMUnknownEngineID,
        ErrUSMNotInTimeWindow,
        ErrUSMDecryptionError,
        ErrMIBNotInitialized,
        ErrValueOutOfRange,
        ErrSubIdOutOfRange,
        ErrBadSubIdInOID,
        ErrOIDTooLong,
        ErrBadValueName,
        ErrBadValueNotation,
        ErrUnknownOID,
        ErrNullPDU,
        ErrMissingVariables,
        ErrBadVariableType,
        ErrOOM,
        ErrKerberos,

        // pdu response errors

        ErrResponseTooLarge,
        ErrNoSuchVariable,
        ErrBadValue,
        ErrReadOnly,
        ErrNoAccess,
        ErrWrongType,
        ErrWrongLength,
        ErrWrongEncoding,
        ErrWrongValue,
        ErrNoCreation,
        ErrInconsistentValue,
        ErrResourceUnavailable,
        ErrCommitFailed,
        ErrUndoFailed,
        ErrAuthorizationFailed,
        ErrNotWritable,
        ErrInconsistentName
    };

    ErrorInfo();
    ErrorInfo( int internalErrorCode );
    ErrorInfo( ErrorType error );

    ErrorType errorCode() const { return m_errorCode; }
    QString errorMessage() const { return m_errorMessage; }

private:
    ErrorType m_errorCode;
    QString m_errorMessage;
};

}

}

#endif // KSIMSNMP_H
/* vim: et sw=4 ts=4
 */
