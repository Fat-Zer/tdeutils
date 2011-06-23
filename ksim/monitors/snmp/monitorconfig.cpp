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

#include "monitorconfig.h"
#include "labelmonitor.h"
#include "chartmonitor.h"

#include <assert.h>

using namespace KSim::Snmp;

MonitorConfig::MonitorConfig()
{
    display = Label;
    refreshInterval.minutes = refreshInterval.seconds = 0;
    useCustomFormatString = false;
    displayCurrentValueInline = false;
}

bool MonitorConfig::load( KConfigBase &config, const HostConfigMap &hosts )
{
    TQString hostName = config.readEntry( "Host" );
    if ( hostName.isEmpty() )
        return false;

    HostConfigMap::ConstIterator hostIt = hosts.tqfind( hostName );
    if ( hostIt == hosts.end() )
        return false;

    host = *hostIt;

    name = config.readEntry( "MonitorName" );
    if ( name.isEmpty() )
        return false;

    oid = config.readEntry( "ObjectIdentifier" );
    if ( Identifier::fromString( oid ).isNull() )
        return false;

    bool ok = false;
    display = stringToMonitorDisplayType( config.readEntry( "DisplayType" ), &ok );
    if ( !ok )
        return false;

    refreshInterval.minutes = config.readUnsignedNumEntry( "RefreshIntervalMinutes" );
    refreshInterval.seconds = config.readUnsignedNumEntry( "RefreshIntervalSeconds" );

    if ( refreshInterval.minutes == 0 && refreshInterval.seconds == 0 )
        return false;

    if ( display == Label ) {
        useCustomFormatString = config.readBoolEntry( "UseCustomFormatString", useCustomFormatString );
        if ( useCustomFormatString )
            customFormatString = config.readEntry( "CustomFormatString" );
    } else
        displayCurrentValueInline = config.readBoolEntry( "DisplayCurrentValueInline", displayCurrentValueInline );

    return true;
}

void MonitorConfig::save( KConfigBase &config ) const
{
    if ( isNull() )
        return;

    config.writeEntry( "Host", host.name );
    config.writeEntry( "MonitorName", name );
    config.writeEntry( "ObjectIdentifier", oid );
    config.writeEntry( "DisplayType", monitorDisplayTypeToString( display ) );
    config.writeEntry( "RefreshIntervalMinutes", refreshInterval.minutes );
    config.writeEntry( "RefreshIntervalSeconds", refreshInterval.seconds );
    if ( display == Label ) {
        config.writeEntry( "UseCustomFormatString", useCustomFormatString );
        if ( useCustomFormatString )
            config.writeEntry( "CustomFormatString", customFormatString );
    } else
        config.writeEntry( "DisplayCurrentValueInline", displayCurrentValueInline );
}

TQWidget *MonitorConfig::createMonitorWidget( TQWidget *tqparent, const char *name )
{
    TQWidget *w;

    int refresh = refreshInterval.seconds * 1000 + refreshInterval.minutes * 60 * 1000;
    Identifier id = Identifier::fromString( oid );
    if ( id.isNull() )
        return 0;

    if ( display == Label )
        w = new LabelMonitor( *this, tqparent, name );
    else
        w = new ChartMonitor( *this, tqparent, name );

    Monitor *monitor = new Monitor( host, id, refresh, TQT_TQOBJECT(w) );
    TQObject::connect( monitor, TQT_SIGNAL( newData( const Value & ) ),
                      w, TQT_SLOT( setData( const Value & ) ) );
    return w;
}

TQString KSim::Snmp::monitorDisplayTypeToString( MonitorConfig::DisplayType type )
{
    switch ( type )
    {
        case MonitorConfig::Label: return TQString::tqfromLatin1( "Label" );
        case MonitorConfig::Chart: return TQString::tqfromLatin1( "Chart" );
        default: assert( false );
    };
    return TQString();
}

MonitorConfig::DisplayType KSim::Snmp::stringToMonitorDisplayType( TQString string, bool *ok )
{
    string = string.lower();
    if ( string == "chart" ) {
        if ( ok )
            *ok = true;
        return MonitorConfig::Chart;
    }
    if ( string == "label" ) {
        if ( ok )
            *ok = true;
        return MonitorConfig::Label;
    }
    if ( ok )
        *ok = false;
    return MonitorConfig::Chart;
}

TQStringList KSim::Snmp::allDisplayTypes()
{
    // !!! keep order with enum
    return TQStringList() << "Label" << "Chart";
}

void MonitorConfigMap::load( KConfigBase &config, const TQStringList &names, const HostConfigMap &hosts )
{
    clear();

    for ( TQStringList::ConstIterator it = names.begin(); it != names.end(); ++it ) {
        config.setGroup( "Monitor " + *it );

        MonitorConfig monitor;
        if ( !monitor.load( config, hosts ) )
            continue;

        insert( *it, monitor );
    }
}

TQStringList MonitorConfigMap::save( KConfigBase &config ) const
{
    TQStringList names;

    for ( ConstIterator it = begin(); it != end(); ++it ) {
        TQString name = it.key();

        names << name;

        config.setGroup( "Monitor " + name );
        ( *it ).save( config );
    }

    return names;
}

/* vim: et sw=4 ts=4
 */
