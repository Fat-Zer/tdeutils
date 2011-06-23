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

#include <tqheader.h>


#include "listview.h"


CListView::CListView( TQWidget *tqparent, const char *name, int visibleItem )
  :KListView( tqparent, name ),  mVisibleItem(TQMAX( 1, visibleItem ))
{
  setVisibleItem(visibleItem);
}

void CListView::setVisibleItem( int visibleItem, bool updateSize )
{
  mVisibleItem = TQMAX( 1, visibleItem );
  if( updateSize == true )
  {
    TQSize s = tqsizeHint();
    setMinimumSize( s.width() + verticalScrollBar()->tqsizeHint().width() + 
		    lineWidth() * 2, s.height() );
  }
}

TQSize CListView::tqsizeHint( void ) const
{
  TQSize s = TQListView::tqsizeHint();
  
  int h = fontMetrics().height() + 2*itemMargin();
  if( h % 2 > 0 ) { h++; }
  
  s.setHeight( h*mVisibleItem + lineWidth()*2 + header()->tqsizeHint().height());
  return( s );
}
#include "listview.moc"
