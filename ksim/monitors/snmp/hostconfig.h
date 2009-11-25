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
    HostConfig( KConfigBase &config )
    { load( config ); }

    QString name; // hostname
    ushort port;

    SnmpVersion version;

    QString community;

    QString securityName;

    SecurityLevel securityLevel;

    struct
    {
        AuthenticationProtocol protocol;
        QString key;
    } authentication;
    struct
    {
        PrivacyProtocol protocol;
        QString key;
    } privacy;

    bool load( KConfigBase &config );
    void save( KConfigBase &config ) const;

    bool isNull() const { return name.isEmpty(); }

    bool operator==( const HostConfig &rhs ) const
    { return name == rhs.name; }

private:
    static void writeIfNotEmpty( KConfigBase &config, const QString &name, const QString &value );
};

struct HostConfigMap : public QMap< QString, HostConfig >
{
    HostConfigMap() {}
    HostConfigMap( const QMap< QString, HostConfig > &rhs )
        : QMap< QString, HostConfig >( rhs ) {}

    void load( KConfigBase &config, const QStringList &hosts );
    QStringList save( KConfigBase &config ) const;
};

}
}

#endif // HOSTCONFIG_H
/* vim: et sw=4 ts=4
 */
