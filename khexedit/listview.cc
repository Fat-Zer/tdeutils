/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#include <qheader.h>


#include "listview.h"


CListView::CListView( QWidget *parent, const char *name, int visibleItem )
  :KListView( parent, name ),  mVisibleItem(QMAX( 1, visibleItem ))
{
  setVisibleItem(visibleItem);
}

void CListView::setVisibleItem( int visibleItem, bool updateSize )
{
  mVisibleItem = QMAX( 1, visibleItem );
  if( updateSize == true )
  {
    QSize s = sizeHint();
    setMinimumSize( s.width() + verticalScrollBar()->sizeHint().width() + 
		    lineWidth() * 2, s.height() );
  }
}

QSize CListView::sizeHint( void ) const
{
  QSize s = QListView::sizeHint();
  
  int h = fontMetrics().height() + 2*itemMargin();
  if( h % 2 > 0 ) { h++; }
  
  s.setHeight( h*mVisibleItem + lineWidth()*2 + header()->sizeHint().height());
  return( s );
}
#include "listview.moc"
