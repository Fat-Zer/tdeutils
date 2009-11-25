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

#include "hostconfig.h"

#include <kstringhandler.h>

#include <netdb.h>

using namespace KSim::Snmp;

static int defaultSnmpPort()
{
    servent *ent = getservbyname( "snmp", 0 );
    if ( !ent )
        return 161;
    return ent->s_port;
}

bool HostConfig::load( KConfigBase &config )
{
    name = config.readEntry( "Host" );
    if ( name.isEmpty() )
        return false;

    port = config.readNumEntry( "Port", defaultSnmpPort() );

    bool ok = false;
    version = stringToSnmpVersion( config.readEntry( "Version" ), &ok );
    if ( !ok )
        return false;

    if ( version != SnmpVersion3 ) {
        community = config.readEntry( "Community" );
        return true;
    }

    securityName = config.readEntry( "SecurityName" );

    securityLevel = stringToSecurityLevel( config.readEntry( "SecurityLevel" ), &ok );
    if ( !ok )
        return false;

    if ( securityLevel == NoAuthPriv )
        return true;

    authentication.protocol = stringToAuthenticationProtocol( config.readEntry( "AuthType" ), &ok );
    if ( !ok )
        return false;
    authentication.key = KStringHandler::obscure( config.readEntry( "AuthPassphrase" ) );

    if ( securityLevel == AuthNoPriv )
        return true;

    privacy.protocol = stringToPrivacyProtocol( config.readEntry( "PrivType" ), &ok );
    if ( !ok )
        return false;
    privacy.key = KStringHandler::obscure( config.readEntry( "PrivPassphrase" ) );

    return true;
}

void HostConfig::save( KConfigBase &config ) const
{
    if ( isNull() )
        return;

    config.writeEntry( "Host", name );
    if ( port != 0 )
        config.writeEntry( "Port", port );

    config.writeEntry( "Version", snmpVersionToString( version ) );

    if ( version != SnmpVersion3 ) {
        writeIfNotEmpty( config, "Community", community );
        return;
    }

    writeIfNotEmpty( config, "SecurityName", securityName );

    config.writeEntry( "SecurityLevel", securityLevelToString( securityLevel ) );

    if ( securityLevel == NoAuthPriv )
        return;

    writeIfNotEmpty( config, "AuthType", authenticationProtocolToString( authentication.protocol ) );
    writeIfNotEmpty( config, "AuthPassphrase", KStringHandler::obscure( authentication.key ) );

    if ( securityLevel == AuthNoPriv )
        return;

    writeIfNotEmpty( config, "PrivType", privacyProtocolToString( privacy.protocol ) );
    writeIfNotEmpty( config, "PrivPassphrase", KStringHandler::obscure( privacy.key ) );
}

void HostConfig::writeIfNotEmpty( KConfigBase &config, const QString &name, const QString &value )
{
    if ( value.isEmpty() )
        return;

    config.writeEntry( name, value );
}

void HostConfigMap::load( KConfigBase &config, const QStringList &hosts )
{
    clear();

    for ( QStringList::ConstIterator it = hosts.begin(); it != hosts.end(); ++it ) {
        config.setGroup( "Host " + *it );

        HostConfig src;
        if ( !src.load( config ) )
            continue;

        insert( *it, src );
    }
}

QStringList HostConfigMap::save( KConfigBase &config ) const
{
    QStringList hostList;

    for ( ConstIterator it = begin(); it != end(); ++it ) {
        QString host = it.key();

        hostList << host;

        config.setGroup( "Host " + host );
        ( *it ).save( config );
    }

    return hostList;
}

/* vim: et sw=4 ts=4
 */
