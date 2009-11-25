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

#include "view.h"
#include "plugin.h"
#include "monitorconfig.h"

#include <label.h>

#include <qlayout.h>

#include <kconfig.h>

using namespace KSim::Snmp;

View::View( Plugin *parent, const char *name )
    : KSim::PluginView( parent, name )
{
    m_layout = new QVBoxLayout( this );

    reparseConfig();
}

View::~View()
{
}

void View::reparseConfig()
{
    m_widgets.setAutoDelete( true );
    m_widgets.clear();
    m_widgets.setAutoDelete( false );

    KConfig &cfg = *config();
    cfg.setGroup( "General" );
    QStringList allHosts = cfg.readListEntry( "Hosts" );
    QStringList monitors = cfg.readListEntry( "Monitors" );

    HostConfigMap hosts;
    hosts.load( cfg, allHosts );

    MonitorConfigMap monitorConfigs;
    monitorConfigs.load( cfg, monitors, hosts );

    for ( MonitorConfigMap::ConstIterator it = monitorConfigs.begin();
          it != monitorConfigs.end(); ++it ) {

        MonitorConfig monitorConfig = *it;

        QWidget *w = monitorConfig.createMonitorWidget( this );
        if ( !w )
            continue;

        m_widgets.append( w );
        m_layout->addWidget( w );

        w->show();
    }
}

#include "view.moc"

/* vim: et sw=4 ts=4
 */
