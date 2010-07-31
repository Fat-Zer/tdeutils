/*

  ark -- archiver for the KDE project

  Copyright (C)

  1999: Francois-Xavier Duranceau duranceau@kde.org
  1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
  2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)
  2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>
  2005: Henrique Pinto <henrique.pinto@kdemail.net>

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
#ifndef FILELISTVIEW_H
#define FILELISTVIEW_H

#include <tqdatetime.h>
#include <tqpair.h>
#include <tqvaluelist.h>

#include <klistview.h>
#include <kio/global.h>

class QString;
class QStringList;
class QRect;
class QPainter;
class QColorGroup;
class QMouseEvent;
class QPoint;

enum columnName { sizeCol = 1 , packedStrCol, ratioStrCol, timeStampStrCol, otherCol };

class FileLVI : public KListViewItem
{
	public:
		FileLVI( KListView* lv );
		FileLVI( KListViewItem* lvi );

		TQString fileName() const { return m_entryName; }
		KIO::filesize_t fileSize() const { return m_fileSize; }
		KIO::filesize_t packedFileSize() const { return m_packedFileSize; }
		double ratio() const { return m_ratio; }
		TQDateTime timeStamp() const { return m_timeStamp; }

		int compare ( TQListViewItem * i, int col, bool ascending ) const;
		virtual TQString key( int column, bool ) const;
		virtual void setText( int column, const TQString &text );

	private:
		KIO::filesize_t m_fileSize;
		KIO::filesize_t m_packedFileSize;
		double    m_ratio;
		TQDateTime m_timeStamp;
		TQString   m_entryName;
};

typedef TQValueList< QPair< TQString, Qt::AlignmentFlags > > ColumnList;

class FileListView: public KListView
{
	Q_OBJECT
	public:
		FileListView( TQWidget *parent = 0, const char* name = 0 );

		FileLVI *currentItem() {return ((FileLVI *) KListView::currentItem());}

		/**
		 * Returns the full names of the selected files.
		 */
		TQStringList selectedFilenames();

		/**
		 * Return the full names of all files.
		 */
		TQStringList fileNames();

		/**
		 * Returns true if no file is selected
		 */
		bool isSelectionEmpty();

		virtual int addColumn( const TQString & label, int width = -1 );
		virtual void removeColumn( int index );
		columnName nameOfColumn( int index );

		/**
		 * Returns the file item, or 0 if not found.
		 * @param filename The filename in question to reference in the archive
		 * @return The requested file's FileLVI
		 */
		FileLVI* item(const TQString& filename) const;

		/**
		 * Adds a file and stats to the file listing
		 * @param entries A stringlist of the entries for each column of the list.
		 */
		void addItem( const TQStringList & entries );

		/**
		 * Returns the number of files in the archive.
		 */
		int totalFiles();

		/**
		 * Returns the number of selected files.
		 */
		int selectedFilesCount();

		/**
		 * Return the sum of the sizes of all files in the archive.
		 */
		KIO::filesize_t totalSize();

		/**
		 * Return the sum of the sizes of the selected files.
		 */
		KIO::filesize_t selectedSize();

		/**
		 * Adjust the size of all columns to fit their content.
		 */
		void adjustColumns() { for ( int i = 0; i < columns(); ++i ) adjustColumn( i ); }

	public slots:
		void selectAll();
		void unselectAll();
		void setHeaders( const ColumnList& columns );
		void clearHeaders();

	signals:
		void startDragRequest( const TQStringList & fileList );

	protected:
		virtual void contentsMouseReleaseEvent( TQMouseEvent *e );
		virtual void contentsMousePressEvent( TQMouseEvent *e );
		virtual void contentsMouseMoveEvent( TQMouseEvent *e );

	private:
		FileLVI* findParent( const TQString& fullname );
		TQStringList childrenOf( FileLVI* parent );

		TQMap<int, columnName> m_columnMap;
		bool m_pressed;
		TQPoint m_presspos;  // this will save the click pos to correctly recognize drag events
};

#endif
// kate: space-indent off; tab-width 4;
