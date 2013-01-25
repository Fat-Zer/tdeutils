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

#ifndef HOSTCONFIG_H
#define HOSTCONFIG_H

#include "snmp.h"

namespace KSim
{

namespace Snmp
{

struct HostConfig
{
    HostConfig() {}
    HostConfig( TDEConfigBase &config )
    { load( config ); }

    TQString name; // hostname
    ushort port;

    SnmpVersion version;

    TQString community;

    TQString securityName;

    SecurityLevel securityLevel;

    struct
    {
        AuthenticationProtocol protocol;
        TQString key;
    } authentication;
    struct
    {
        PrivacyProtocol protocol;
        TQString key;
    } privacy;

    bool load( TDEConfigBase &config );
    void save( TDEConfigBase &config ) const;

    bool isNull() const { return name.isEmpty(); }

    bool operator==( const HostConfig &rhs ) const
    { return name == rhs.name; }

private:
    static void writeIfNotEmpty( TDEConfigBase &config, const TQString &name, const TQString &value );
};

struct HostConfigMap : public TQMap< TQString, HostConfig >
{
    HostConfigMap() {}
    HostConfigMap( const TQMap< TQString, HostConfig > &rhs )
        : TQMap< TQString, HostConfig >( rhs ) {}

    void load( TDEConfigBase &config, const TQStringList &hosts );
    TQStringList save( TDEConfigBase &config ) const;
};

}
}

#endif // HOSTCONFIG_H
/* vim: et sw=4 ts=4
 */
