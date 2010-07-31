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

#include "browsedialog.h"

#include <tqlabel.h>

#include <klistview.h>
#include <kguiitem.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <klineedit.h>

using namespace KSim::Snmp;

BrowseDialog::BrowseDialog( const HostConfig &hostConfig, const TQString &currentOid, TQWidget *parent, const char *name )
    : BrowseDialogBase( parent, name ), m_host( hostConfig )
{
    stop->setGuiItem( KGuiItem( i18n( "&Stop" ), "stop" ) );

    browserContents->setSorting( -1 );

    selectedObject->setText( currentOid );

    m_walker = 0;

    baseOids << "1.3.6.1.2" << "1.3.6.1.4";
    nextWalk();
}

TQString BrowseDialog::selectedObjectIdentifier() const
{
    return selectedObject->text();
}

void BrowseDialog::stopAllWalks()
{
    baseOids.clear();
    stopWalker();
}

void BrowseDialog::insertBrowseItem( const Walker::Result &result )
{
    if ( !result.success ||
         result.data.type() == Value::EndOfMIBView ||
         result.data.type() == Value::NoSuchInstance ||
         result.data.type() == Value::NoSuchObject ) {

        nextWalk();
        return;
    }

    TQListViewItem *i = new TQListViewItem( browserContents, browserContents->lastItem(), result.identifierString, result.dataString );
    applyFilter( i );
}

void BrowseDialog::applyFilter()
{
    for ( TQListViewItem *i = browserContents->firstChild();
          i; i = i->nextSibling() )
        applyFilter( i );
}

void BrowseDialog::nextWalk()
{
    stopWalker();

    if ( baseOids.isEmpty() )
        return;

    TQString baseOidString = baseOids.pop();
    Identifier id = Identifier::fromString( baseOidString );
    if ( id.isNull() )
        return;
    startWalk( id );
}

void BrowseDialog::startWalk( const Identifier &startOid )
{
    stopWalker();

    m_walker = new Walker( m_host, startOid, this );
    connect( m_walker, TQT_SIGNAL( resultReady( const Walker::Result & ) ),
             this, TQT_SLOT( insertBrowseItem( const Walker::Result & ) ) );
    connect( m_walker, TQT_SIGNAL( finished() ),
             this, TQT_SLOT( nextWalk() ) );

    stop->setEnabled( true );
}

void BrowseDialog::stopWalker()
{
    if ( !m_walker )
        return;

    disconnect( m_walker, TQT_SIGNAL( resultReady( const Walker::Result & ) ),
                this, TQT_SLOT( insertBrowseItem( const Walker::Result & ) ) );
    disconnect( m_walker, TQT_SIGNAL( finished() ),
                this, TQT_SLOT( nextWalk() ) );

    m_walker->deleteLater();
    m_walker = 0;

    stop->setEnabled( false );
}

void BrowseDialog::objectSelected( TQListViewItem *item )
{
    selectedObject->setText( item->text( 0 ) );
}

void BrowseDialog::applyFilter( TQListViewItem *item )
{
    TQString filterText = filter->text();

    if ( filterText.isEmpty() ) {
        item->setVisible( true );
        return;
    }

    if ( item->text( 0 ).find( filterText, 0 /*index*/, false /*case sensitive*/ ) == -1 ) {
        item->setVisible( false );
        return;
    }

    item->setVisible( true );
}

#include "browsedialog.moc"
/* vim: et sw=4 ts=4
 */
