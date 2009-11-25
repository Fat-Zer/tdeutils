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

#ifndef MONITORCONFIG_H
#define MONITORCONFIG_H

#include "hostconfig.h"

namespace KSim
{

namespace Snmp
{

struct MonitorConfig
{
    enum DisplayType { Label = 0, Chart };

    MonitorConfig();
    MonitorConfig( KConfigBase &config, const HostConfigMap &hosts )
    { load( config, hosts ); }

    bool load( KConfigBase &config, const HostConfigMap &hosts );
    void save( KConfigBase &config ) const;

    bool isNull() const { return name.isEmpty() || host.isNull(); }

    QWidget *createMonitorWidget( QWidget *parent, const char *name = 0 );

    HostConfig host;
    QString name;
    QString oid;
    struct
    {
        uint minutes;
        uint seconds;
    } refreshInterval;
    DisplayType display;

    bool useCustomFormatString;
    QString customFormatString;

    bool displayCurrentValueInline;
};

struct MonitorConfigMap : public QMap< QString, MonitorConfig >
{
    MonitorConfigMap() {}
    MonitorConfigMap( const QMap< QString, MonitorConfig > &rhs )
        : QMap< QString, MonitorConfig >( rhs ) {}

    void load( KConfigBase &config, const QStringList &names, const HostConfigMap &hosts );
    QStringList save( KConfigBase &config ) const;
};

typedef QValueList<MonitorConfig> MonitorConfigList;

QString monitorDisplayTypeToString( MonitorConfig::DisplayType type );
MonitorConfig::DisplayType stringToMonitorDisplayType( QString string, bool *ok = 0 );
QStringList allDisplayTypes();

}

}

#endif // MONITORCONFIG_H
/* vim: et sw=4 ts=4
 */
