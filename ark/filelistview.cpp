/*

  ark -- archiver for the KDE project

  Copyright (C)

  2005: Henrique Pinto <henrique.pinto@kdemail.net>
  2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>
  2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)
  1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
  1999: Francois-Xavier Duranceau duranceau@kde.org

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

// TQt includes
#include <tqpainter.h>
#include <tqwhatsthis.h>
#include <tqheader.h>

// KDE includes
#include <tdelocale.h>
#include <tdeglobal.h>
#include <kdebug.h>
#include <tdeglobalsettings.h>
#include <kmimetype.h>

#include "filelistview.h"
#include "arch.h"

/////////////////////////////////////////////////////////////////////
// FileLVI implementation
/////////////////////////////////////////////////////////////////////

FileLVI::FileLVI( TDEListView* lv )
	: TDEListViewItem( lv ), m_fileSize( 0 ), m_packedFileSize( 0 ),
	  m_ratio( 0 ), m_timeStamp( TQDateTime() ), m_entryName( TQString() )
{

}

FileLVI::FileLVI( TDEListViewItem* lvi )
	: TDEListViewItem( lvi ), m_fileSize( 0 ), m_packedFileSize( 0 ),
	  m_ratio( 0 ), m_timeStamp( TQDateTime() ), m_entryName( TQString() )
{
}

TQString FileLVI::key( int column, bool ascending ) const
{
	if ( column == 0 )
		return fileName();
	else
		return TQListViewItem::key( column, ascending );
}

int FileLVI::compare( TQListViewItem * i, int column, bool ascending ) const
{
	FileLVI * item = static_cast< FileLVI * >( i );

	if ( ( this->childCount() > 0 ) && ( item->childCount() == 0 ) )
		return -1;
	
	if ( ( this->childCount() == 0 ) && ( item->childCount() > 0 ) )
		return 1;

	if ( column == 0 )
		return TDEListViewItem::compare( i, column, ascending );
	
	columnName colName = ( static_cast< FileListView * > ( listView() ) )->nameOfColumn( column );
	switch ( colName )
	{
		case sizeCol:
		{
			return ( m_fileSize < item->fileSize() ? -1 :
			           ( m_fileSize > item->fileSize() ? 1 : 0 )
			       );
			break;
		}

		case ratioStrCol:
		{
			return ( m_ratio < item->ratio() ? -1 :
			           ( m_ratio > item->ratio() ? 1 : 0 )
			       );
			break;
		}

		case packedStrCol:
		{
			return ( m_packedFileSize < item->packedFileSize() ? -1 :
			           ( m_packedFileSize > item->packedFileSize() ? 1 : 0 )
			       );
			break;
		}

		case timeStampStrCol:
		{
			return ( m_timeStamp < item->timeStamp() ? -1 :
			           ( m_timeStamp > item->timeStamp() ? 1 : 0 )
			       );
			break;
		}

		default:
			return TDEListViewItem::compare( i, column, ascending );
	}
}

void FileLVI::setText( int column, const TQString &text )
{
	columnName colName = ( static_cast< FileListView * > ( listView() ) )->nameOfColumn( column );
	if ( column == 0 )
	{
		TQString name = text;
		if ( name.endsWith( "/" ) )
			name = name.left( name.length() - 1 );
		if ( name.startsWith( "/" ) )
			name = name.mid( 1 );
		int pos = name.findRev( '/' );
		if ( pos != -1 )
			name = name.right( name.length() - pos - 1 );
		TQListViewItem::setText( column, name );
		m_entryName = text;
	}
	else if ( colName == sizeCol )
	{
		m_fileSize = text.toULongLong();
		TQListViewItem::setText( column, TDEIO::convertSize( m_fileSize ) );
	}
	else if ( colName == packedStrCol )
	{
		m_packedFileSize = text.toULongLong();
		TQListViewItem::setText( column, TDEIO::convertSize( m_packedFileSize ) );
	}
	else if ( colName == ratioStrCol )
	{
		int l = text.length() - 1;
		if ( l>0 && text[l] == '%' )
			m_ratio = text.left(l).toDouble();
		else
			m_ratio = text.toDouble();
		TQListViewItem::setText( column, i18n( "Packed Ratio", "%1 %" )
		                                .arg(TDEGlobal::locale()->formatNumber( m_ratio, 1 ) )
		                      );
	}
	else if ( colName == timeStampStrCol )
	{
		if ( text.isEmpty() )
			TQListViewItem::setText(column, text);
		else
		{
			m_timeStamp = TQDateTime::fromString( text, Qt::ISODate );
			TQListViewItem::setText( column, TDEGlobal::locale()->formatDateTime( m_timeStamp ) );
		}
	}
	else
		TQListViewItem::setText(column, text);
}

static FileLVI* folderLVI( TDEListViewItem *parent, const TQString& name )
{
	FileLVI *folder = new FileLVI( parent );

	folder->setText( 0, name );

	folder->setPixmap( 0, KMimeType::mimeType( "inode/directory" )->pixmap( TDEIcon::Small ) );

	return folder;
}

static FileLVI* folderLVI( TDEListView *parent, const TQString& name )
{
	FileLVI *folder = new FileLVI( parent );
	folder->setText( 0, name );
	folder->setPixmap( 0, KMimeType::mimeType( "inode/directory" )->pixmap( TDEIcon::Small ) );
	return folder;
}

/////////////////////////////////////////////////////////////////////
// FileListView implementation
/////////////////////////////////////////////////////////////////////


FileListView::FileListView(TQWidget *parent, const char* name)
	: TDEListView(parent, name)
{
	TQWhatsThis::add( this,
	                 i18n( "This area is for displaying information about the files contained within an archive." )
	               );

	setMultiSelection( true );
	setSelectionModeExt( FileManager );
	setItemsMovable( false );
	setRootIsDecorated( true );
	setShowSortIndicator( true );
	setItemMargin( 3 );
	header()->hide(); // Don't show the header until there is something to be shown in it

	m_pressed = false;
}

int FileListView::addColumn ( const TQString & label, int width )
{
	int index = TDEListView::addColumn( label, width );
	if ( label == SIZE_COLUMN.first )
	{
		m_columnMap[ index ] = sizeCol;
	}
	else if ( label == PACKED_COLUMN.first )
	{
		m_columnMap[ index ] = packedStrCol;
	}
	else if ( label == RATIO_COLUMN.first )
	{
		m_columnMap[ index ] = ratioStrCol;
	}
	else if ( label == TIMESTAMP_COLUMN.first )
	{
		m_columnMap[ index ] = timeStampStrCol;
	}
	else
	{
		m_columnMap[ index ] = otherCol;
	}
	return index;
}

void FileListView::removeColumn( int index )
{
	for ( unsigned int i = index; i < m_columnMap.count() - 2; i++ )
	{
		m_columnMap.replace( i, m_columnMap[ i + 1 ] );
	}

	m_columnMap.remove( m_columnMap[ m_columnMap.count() - 1 ] );
	TDEListView::removeColumn( index );
}

columnName FileListView::nameOfColumn( int index )
{
	return m_columnMap[ index ];
}

TQStringList FileListView::selectedFilenames()
{
	TQStringList files;

	FileLVI *item = static_cast<FileLVI*>( firstChild() );

	while ( item )
	{
		if ( item->isSelected() )
		{
			// If the item has children, add each child and the item
			if ( item->childCount() > 0 )
			{
				files += item->fileName();
				files += childrenOf( item );

				/* If we got here, then the logic for "going to the next item"
				 * is a bit different: as we already dealt with all the children,
				 * the "next item" is the next sibling of the current item, not
				 * its first child. If the current item has no siblings, then
				 * the next item is the next sibling of its parent, and so on.
				 */
				FileLVI *nitem = static_cast<FileLVI*>( item->nextSibling() );
				while ( !nitem && item->parent() )
				{
					item = static_cast<FileLVI*>( item->parent() );
					if ( item->parent() )
						nitem = static_cast<FileLVI*>( item->parent()->nextSibling() );
				}
				item = nitem;
				continue;
			}
			else
			{
				// If the item has no children, just add it to the list
				files += item->fileName();
			}
		}
		// Go to the next item
		item = static_cast<FileLVI*>( item->itemBelow() );
	}

	return files;
}

TQStringList FileListView::fileNames()
{
	TQStringList files;

	TQListViewItemIterator it( this );
	while ( it.current() )
	{
		FileLVI *item = static_cast<FileLVI*>( it.current() );
		files += item->fileName();
		++it;
	}

	return files;
}

bool FileListView::isSelectionEmpty()
{
	FileLVI * flvi = (FileLVI*)firstChild();

	while (flvi)
	{
		if( flvi->isSelected() )
			return false;
		else
		flvi = (FileLVI*)flvi->itemBelow();
	}
	return true;
}

void
FileListView::contentsMousePressEvent(TQMouseEvent *e)
{
	if( e->button()==Qt::LeftButton )
	{
		m_pressed = true;
		m_presspos = e->pos();
	}

	TDEListView::contentsMousePressEvent(e);
}

void
FileListView::contentsMouseReleaseEvent(TQMouseEvent *e)
{
	m_pressed = false;
	TDEListView::contentsMouseReleaseEvent(e);
}

void
FileListView::contentsMouseMoveEvent(TQMouseEvent *e)
{
	if(!m_pressed)
	{
		TDEListView::contentsMouseMoveEvent(e);
	}
	else if( ( m_presspos - e->pos() ).manhattanLength() > TDEGlobalSettings::dndEventDelay() )
	{
		m_pressed = false;	// Prevent triggering again
		if(isSelectionEmpty())
		{
			return;
		}
		TQStringList dragFiles = selectedFilenames();
		emit startDragRequest( dragFiles );
		TDEListView::contentsMouseMoveEvent(e);
	}
}

FileLVI*
FileListView::item(const TQString& filename) const
{
	FileLVI * flvi = (FileLVI*) firstChild();

	while (flvi)
	{
		TQString curFilename = flvi->fileName();
		if (curFilename == filename)
			return flvi;
		flvi = (FileLVI*) flvi->nextSibling();
	}

	return 0;
}

void FileListView::addItem( const TQStringList & entries )
{
	FileLVI *flvi, *parent = findParent( entries[0] );
	if ( parent )
		flvi = new FileLVI( parent );
	else
		flvi = new FileLVI( this );


	int i = 0;

	for (TQStringList::ConstIterator it = entries.begin(); it != entries.end(); ++it)
	{
		flvi->setText(i, *it);
		++i;
	}

	KMimeType::Ptr mimeType = KMimeType::findByPath( entries.first(), 0, true );
	flvi->setPixmap( 0, mimeType->pixmap( TDEIcon::Small ) );
}

void FileListView::selectAll()
{
	TQListView::selectAll( true );
}

void FileListView::unselectAll()
{
	TQListView::selectAll( false );
}

void FileListView::setHeaders( const ColumnList& columns )
{
	clearHeaders();

	for ( ColumnList::const_iterator it = columns.constBegin();
	      it != columns.constEnd();
	      ++it )
	{
		TQPair< TQString, TQt::AlignmentFlags > pair = *it;
		int colnum = addColumn( pair.first );
		setColumnAlignment( colnum, pair.second );
	}
	
	setResizeMode( TQListView::LastColumn );

	header()->show();
}

void FileListView::clearHeaders()
{
	header()->hide();
	while ( columns() > 0 )
	{
		removeColumn( 0 );
	}
}

int FileListView::totalFiles()
{
	int numFiles = 0;

	TQListViewItemIterator it( this );
	while ( it.current() )
	{
		if ( it.current()->childCount() == 0 )
			++numFiles;
		++it;
	}

	return numFiles;
}

int FileListView::selectedFilesCount()
{
	int numFiles = 0;

	TQListViewItemIterator it( this, TQListViewItemIterator::Selected );
	while ( it.current() )
	{
		++numFiles;
		++it;
	}

	return numFiles;
}

TDEIO::filesize_t FileListView::totalSize()
{
	TDEIO::filesize_t size = 0;

	TQListViewItemIterator it(this);
	while ( it.current() )
	{
		FileLVI *item = static_cast<FileLVI*>( it.current() );
		size += item->fileSize();
		++it;
	}

	return size;
}

TDEIO::filesize_t FileListView::selectedSize()
{
	TDEIO::filesize_t size = 0;

	TQListViewItemIterator it( this, TQListViewItemIterator::Selected );
	while ( it.current() )
	{
		FileLVI *item = static_cast<FileLVI*>( it.current() );
		size += item->fileSize();
		++it;
	}

	return size;
}

FileLVI* FileListView::findParent( const TQString& fullname )
{
	TQString name = fullname;

	if ( name.endsWith( "/" ) )
		name = name.left( name.length() -1 );
	if ( name.startsWith( "/" ) )
		name = name.mid( 1 );
	// Checks if this entry needs a parent
	if ( !name.contains( '/' ) )
		return static_cast< FileLVI* >( 0 );

	// Get a list of ancestors
	TQString parentFullname = name.left( name.findRev( '/' ) );
	TQStringList ancestorList = TQStringList::split( '/', parentFullname );

	// Checks if the listview contains the first item in the list of ancestors
	TQListViewItem *item = firstChild();
	while ( item )
	{
		if ( item->text( 0 ) == ancestorList[0] )
			break;
		item = item->nextSibling();
	}

	// If the list view does not contain the item, create it
	if ( !item )
	{
		item = folderLVI( this, ancestorList[0] );
	}

	// We've already dealt with the first item, remove it
	ancestorList.pop_front();

	while ( ancestorList.count() > 0 )
	{
		TQString name = ancestorList[0];

		FileLVI *parent = static_cast< FileLVI*>( item );
		item = parent->firstChild();
		while ( item )
		{
			if ( item->text(0) == name )
				break;
			item = item->nextSibling();
		}

		if ( !item )
		{
			item = folderLVI( parent, name );
		}

		ancestorList.pop_front();
	}

	item->setOpen( true );
	return static_cast< FileLVI* >( item );
}

TQStringList FileListView::childrenOf( FileLVI* parent )
{
	Q_ASSERT( parent );
	TQStringList children;

	FileLVI *item = static_cast<FileLVI*>( parent->firstChild() );

	while ( item )
	{
		if ( item->childCount() == 0 )
		{
			children += item->fileName();
		}
		else
		{
			children += item->fileName();
			children += childrenOf( item );
		}
		item = static_cast<FileLVI*>( item->nextSibling() );
	}

	return children;
}

#include "filelistview.moc"
// kate: space-indent off; tab-width 4;
