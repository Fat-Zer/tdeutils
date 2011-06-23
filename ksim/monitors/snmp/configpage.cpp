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

#include "configpage.h"
#include "plugin.h"

#include "configwidget.h"
#include "hostdialog.h"
#include "monitordialog.h"

#include <tqlayout.h>
#include <tqgroupbox.h>
#include <tqpushbutton.h>

#include <kconfig.h>
#include <klistview.h>
#include <kmessagebox.h>
#include <klocale.h>

using namespace KSim::Snmp;

static bool listViewHasSelection( TQListView *lv )
{
    for ( TQListViewItem *i = lv->firstChild(); i; i = i->itemBelow() )
        if ( i->isSelected() )
            return true;
    return false;
}

ConfigPage::ConfigPage( Plugin *tqparent, const char *name )
    : KSim::PluginPage( tqparent, name )
{
    ( new TQVBoxLayout( this ) )->setAutoAdd( true );

    m_page = new ConfigWidget( this );

    connect( m_page->addHost, TQT_SIGNAL( clicked() ),
             this, TQT_SLOT( addNewHost() ) );
    connect( m_page->modifyHost, TQT_SIGNAL( clicked() ),
             this, TQT_SLOT( modifyHost() ) );
    connect( m_page->removeHost, TQT_SIGNAL( clicked() ),
             this, TQT_SLOT( removeHost() ) );

    connect( m_page->addMonitor, TQT_SIGNAL( clicked() ),
             this, TQT_SLOT( addNewMonitor() ) );
    connect( m_page->modifyMonitor, TQT_SIGNAL( clicked() ),
             this, TQT_SLOT( modifyMonitor() ) );
    connect( m_page->removeMonitor, TQT_SIGNAL( clicked() ),
             this, TQT_SLOT( removeMonitor() ) );

    connect( m_page->hosts, TQT_SIGNAL( selectionChanged() ),
             this, TQT_SLOT( disableOrEnableSomeWidgets() ) );
    connect( m_page->monitors, TQT_SIGNAL( selectionChanged() ),
             this, TQT_SLOT( disableOrEnableSomeWidgets() ) );
}

ConfigPage::~ConfigPage()
{
}

void ConfigPage::saveConfig()
{
    KConfig &cfg = *config();

    // collect garbage
    removeAllHostGroups();
    removeAllMonitorGroups();

    TQStringList hosts = m_hosts.save( cfg );
    TQStringList monitors = m_monitors.save( cfg );

    cfg.setGroup( "General" );
    cfg.writeEntry( "Hosts", hosts );
    cfg.writeEntry( "Monitors", monitors );
}

void ConfigPage::readConfig()
{
    KConfig &cfg = *config();

    cfg.setGroup( "General" );
    TQStringList hosts = cfg.readListEntry( "Hosts" );
    TQStringList monitors = cfg.readListEntry( "Monitors" );

    m_hosts.load( cfg, hosts );
    m_monitors.load( cfg, monitors, m_hosts );

    fillGui();
}

void ConfigPage::addNewHost()
{
    HostDialog dlg( this );
    if ( dlg.exec() ) {
        HostConfig src = dlg.settings();
        m_hosts.insert( src.name, src );

        ( void )new HostItem( m_page->hosts, src );
    }

    disableOrEnableSomeWidgets();
}

void ConfigPage::modifyHost()
{
    HostItem *currentItem = dynamic_cast<HostItem *>( m_page->hosts->currentItem() );
    if ( !currentItem )
        return;

    HostConfigMap::Iterator hostIt = m_hosts.tqfind( currentItem->text( 0 ) );
    if ( hostIt == m_hosts.end() )
        return;

    HostDialog dlg( *hostIt, this );
    if ( dlg.exec() ) {
        HostConfig newHost = dlg.settings();

        if ( newHost.name != hostIt.key() ) {
            m_hosts.remove( hostIt );
            hostIt = m_hosts.insert( newHost.name, newHost );
        } else
            *hostIt = newHost;

        currentItem->setFromHostConfig( newHost );
    }
}

void ConfigPage::removeHost()
{
    HostItem *currentItem = dynamic_cast<HostItem *>( m_page->hosts->currentItem() );
    if ( !currentItem )
        return;

    HostConfigMap::Iterator hostIt = m_hosts.tqfind( currentItem->text( 0 ) );
    if ( hostIt == m_hosts.end() )
        return;

    TQStringList monitors = monitorsForHost( *hostIt );
    if ( !monitors.isEmpty() ) {
        int answer = KMessageBox::warningContinueCancelList(
            this,
            i18n( "This host has the following monitor associated. Do you really want to delete this host entry?",
                  "This host has the following %n monitors associated. Do you really want to delete this host entry?",
                  monitors.count() ), 
            monitors,
            i18n( "Delete Host Entry" ),
            i18n( "Delete" ) );

        if ( answer != KMessageBox::Continue )
            return;

        removeMonitors( monitors );
    }

    m_hosts.remove( hostIt );
    delete currentItem;

    disableOrEnableSomeWidgets();
}

void ConfigPage::addNewMonitor()
{
    MonitorDialog dlg( m_hosts, this );
    if ( dlg.exec() ) {
        MonitorConfig monitor = dlg.monitorConfig();
        m_monitors.insert( monitor.name, monitor );

        ( void )new MonitorItem( m_page->monitors, monitor );
    }
}

void ConfigPage::modifyMonitor()
{
    MonitorItem *currentItem = dynamic_cast<MonitorItem *>( m_page->monitors->currentItem() );
    if ( !currentItem )
        return;

    MonitorConfigMap::Iterator monitorIt = m_monitors.tqfind( currentItem->text( 0 ) );
    if ( monitorIt == m_monitors.end() )
        return;

    MonitorDialog dlg( *monitorIt, m_hosts, this );
    if ( dlg.exec() ) {
        MonitorConfig newMonitor = dlg.monitorConfig();

        if ( newMonitor.name != monitorIt.key() ) {
            m_monitors.remove( monitorIt );
            monitorIt = m_monitors.insert( newMonitor.name, newMonitor );
        } else
            *monitorIt = newMonitor;

        currentItem->setFromMonitor( newMonitor );
    }
}

void ConfigPage::removeMonitor()
{
    MonitorItem *currentItem = dynamic_cast<MonitorItem *>( m_page->monitors->currentItem() );
    if ( !currentItem )
        return;

    MonitorConfigMap::Iterator monitorIt = m_monitors.tqfind( currentItem->text( 0 ) );
    if ( monitorIt == m_monitors.end() )
        return;

    m_monitors.remove( monitorIt );
    delete currentItem;
}

void ConfigPage::disableOrEnableSomeWidgets()
{
    bool hostSelected = listViewHasSelection( m_page->hosts );
    bool monitorSelected = listViewHasSelection( m_page->monitors );

    m_page->modifyHost->setEnabled( hostSelected );
    m_page->removeHost->setEnabled( hostSelected );

    m_page->modifyMonitor->setEnabled( monitorSelected );
    m_page->removeMonitor->setEnabled( monitorSelected );

    m_page->monitorGroup->setEnabled( !m_hosts.isEmpty() );
}

void ConfigPage::removeMonitors( TQStringList monitors )
{
    for ( TQStringList::ConstIterator it = monitors.begin();
          it != monitors.end(); ++it )
        m_monitors.remove( *it );

    TQListViewItem *item = m_page->monitors->firstChild();
    while ( item ) {
        TQListViewItem *nextItem = item->itemBelow();

        for ( TQStringList::Iterator it = monitors.begin();
              it != monitors.end(); ++it )
            if ( item->text( 0 ) == *it ) {

                monitors.remove( it );

                delete item;

                break;
            }

        item = nextItem;
    }
}

void ConfigPage::removeAllHostGroups()
{
    removeConfigGroups( "Host " );
}

void ConfigPage::removeAllMonitorGroups()
{
    removeConfigGroups( "Monitor " );
}

void ConfigPage::removeConfigGroups( const TQString &prefix )
{
    KConfig &cfg = *config();

    TQStringList groups = cfg.groupList();
    for ( TQStringList::ConstIterator it = groups.begin(); it != groups.end(); ++it )
        if ( ( *it ).startsWith( prefix ) )
            cfg.deleteGroup( *it, true /* deep */ );
}

void ConfigPage::fillGui()
{
    m_page->hosts->clear();
    m_page->monitors->clear();

    for ( HostConfigMap::ConstIterator it = m_hosts.begin(); it != m_hosts.end(); ++it )
        ( void )new HostItem( m_page->hosts, *it );

    for ( MonitorConfigMap::ConstIterator it = m_monitors.begin(); it != m_monitors.end(); ++it )
        ( void )new MonitorItem( m_page->monitors, *it );

    disableOrEnableSomeWidgets();
}

TQStringList ConfigPage::monitorsForHost( const HostConfig &host ) const
{
    TQStringList monitors;

    for ( MonitorConfigMap::ConstIterator it = m_monitors.begin();
          it != m_monitors.end(); ++it )
        if ( ( *it ).host == host )
            monitors << ( *it ).name;

    return monitors;
}

#include "configpage.moc"
/* vim: et sw=4 ts=4
 */
