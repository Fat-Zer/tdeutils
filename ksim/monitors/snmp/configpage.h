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

#ifndef CONFIGPAGE_H
#define CONFIGPAGE_H

#include <pluginmodule.h>
#include <klistview.h>

#include "monitorconfig.h"
#include "hostconfig.h"

namespace KSim
{

namespace Snmp
{

class HostItem : public QListViewItem
{
public:
    HostItem( QListView *parent, const KSim::Snmp::HostConfig &src )
        : QListViewItem( parent, QString::null, QString::null, QString::null )
    {
        setFromHostConfig( src );
    }

    HostItem( QListView *parent )
        : QListViewItem( parent, QString::null, QString::null, QString::null )
    {}

    void setFromHostConfig( const KSim::Snmp::HostConfig &src )
    {
        setText( 0, src.name );
        setText( 1, QString::number( src.port ) );
        setText( 2, snmpVersionToString( src.version ) );
    }
};

class MonitorItem : public QListViewItem
{
public:
    MonitorItem( QListView *parent, const KSim::Snmp::MonitorConfig &monitor )
        : QListViewItem( parent, QString::null, QString::null, QString::null )
    {
        setFromMonitor( monitor );
    }

    MonitorItem( QListView *parent )
        : QListViewItem( parent, QString::null, QString::null, QString::null )
    {}

    void setFromMonitor( const KSim::Snmp::MonitorConfig &monitor )
    {
        setText( 0, monitor.name );
        setText( 1, monitorDisplayTypeToString( monitor.display ) );
    }
};

class ConfigWidget;
class Plugin;

class ConfigPage : public KSim::PluginPage
{
    Q_OBJECT
public:
    ConfigPage( Plugin *parent, const char *name );
    ~ConfigPage();

    virtual void saveConfig();
    virtual void readConfig();

private slots:
    void addNewHost();
    void modifyHost();
    void removeHost();
    void addNewMonitor();
    void modifyMonitor();
    void removeMonitor();

    void disableOrEnableSomeWidgets();

private:
    void removeMonitors( QStringList monitors );

    void removeAllHostGroups();
    void removeAllMonitorGroups();
    void removeConfigGroups( const QString &prefix );
    void fillGui();

    QStringList monitorsForHost( const HostConfig &host ) const;

    ConfigWidget *m_page;
    KSim::Snmp::HostConfigMap m_hosts;
    KSim::Snmp::MonitorConfigMap m_monitors;
};

}

}

#endif // CONFIGPAGE_H
/* vim: et sw=4 ts=4
 */
