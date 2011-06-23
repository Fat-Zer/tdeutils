/*
 * ark: A program for modifying archives via a GUI.
 *
 * Copyright (C) 2004, Henrique Pinto <henrique.pinto@kdemail.net>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include "arkviewer.h"

#include <klocale.h>
#include <kparts/componentfactory.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kurl.h>
#include <kglobal.h>
#include <kiconloader.h>

#include <tqvbox.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqframe.h>
#include <tqurl.h>


ArkViewer::ArkViewer( TQWidget * tqparent, const char * name )
	: KDialogBase( tqparent, name, false, TQString(), Close ), m_part( 0 )
{
	m_widget = new TQVBox( this );
	m_widget->tqlayout()->setSpacing( 10 );

	connect( this, TQT_SIGNAL( finished() ), this, TQT_SLOT( slotFinished() ) );

	setMainWidget( m_widget );
}

ArkViewer::~ArkViewer()
{
	saveDialogSize( "ArkViewer" );
}

void ArkViewer::slotFinished()
{
	delete m_part;
	m_part = 0;
	delayedDestruct();
}

bool ArkViewer::view( const KURL& filename )
{
	KMimeType::Ptr mimetype = KMimeType::findByURL( filename, 0, true );

	setCaption( filename.fileName() );

	TQSize size = configDialogSize( "ArkViewer" );
	if (size.width() < 200)
		size = TQSize(560, 400);
	setInitialSize( size );

	TQFrame *header = new TQFrame( m_widget );
	TQHBoxLayout *headerLayout = new TQHBoxLayout( header );
	headerLayout->setAutoAdd( true );

	TQLabel *iconLabel = new TQLabel( header );
	iconLabel->setPixmap( mimetype->pixmap( KIcon::Desktop ) );
	iconLabel->tqsetSizePolicy( TQSizePolicy::Fixed, TQSizePolicy::Minimum );

	TQVBox *headerRight = new TQVBox( header );
	new TQLabel( TQString( "<qt><b>%1</b></qt>" )
	                     .tqarg( filename.fileName() ), headerRight
	          );
	new TQLabel( mimetype->comment(), headerRight );

	header->tqsetSizePolicy( TQSizePolicy::Expanding, TQSizePolicy::Maximum );

	m_part = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>( mimetype->name(), TQString(), m_widget, 0, TQT_TQOBJECT(this) );

	if ( m_part )
	{
		m_part->openURL( filename );
		show();
		return true;
	}
	else
	{
		return false;
	}
}

#include "arkviewer.moc"
