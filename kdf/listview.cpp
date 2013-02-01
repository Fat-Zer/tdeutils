/*
 *   Copyright (C) 1999  Espen Sand, espen@kde.org
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

//
// 1999-11-28 Espen Sand
// The purpose of this class is:
// 1) Easily set minimum number of visible items and to adjust the sizeHint()
// 2) Provide a pixmap collection
//

#include <tqbitmap.h>
#include <tqheader.h>
#include <tqpainter.h>

#include <kiconloader.h>

#include "listview.h"

template class TQDict<TQPixmap>;

CListView::CListView( TQWidget *parent, const char *name, int visibleItem )
  :TDEListView( parent, name ),  mVisibleItem(TQMAX( 1, visibleItem ))
{
  setVisibleItem(visibleItem);
  mPixDict.setAutoDelete(true);
}

void CListView::setVisibleItem( int visibleItem, bool updateSize )
{
  mVisibleItem = TQMAX( 1, visibleItem );
  if( updateSize == true )
  {
    TQSize s = sizeHint();
    setMinimumSize( s.width() + verticalScrollBar()->sizeHint().width() +
		    lineWidth() * 2, s.height() );
  }
}

TQSize CListView::sizeHint( void ) const
{
  TQSize s = TQListView::sizeHint();

  int h = fontMetrics().height() + 2*itemMargin();
  if( h % 2 > 0 ) { h++; }

  s.setHeight( h*mVisibleItem + lineWidth()*2 + header()->sizeHint().height());
  return( s );
}



const TQPixmap &CListView::icon( const TQString &iconName, bool drawBorder )
{
  TQPixmap *pix = mPixDict[ iconName ];
  if( pix == 0 )
  {
    pix = new TQPixmap( SmallIcon( iconName ) );

    if( drawBorder == true )
    {
      //
      // 2000-01-23 Espen Sand
      // Careful here: If the mask has not been defined we can
      // not use TQPixmap::mask() because it returns 0 => segfault
      //
      if( pix->mask() != 0 )
      {
	TQBitmap *bm = new TQBitmap(*(pix->mask()));
	if( bm != 0 )
	{
	  TQPainter qp(bm);
	  qp.setPen(TQPen(white,1));
	  qp.drawRect(0,0,bm->width(),bm->height());
	  qp.end();
	  pix->setMask(*bm);
	}

	TQPainter qp(pix);
	qp.setPen(TQPen(red,1));
	qp.drawRect(0,0,pix->width(),pix->height());
	qp.end();
        delete bm;

      }
    }
    mPixDict.replace( iconName, pix );
  }

  return( *pix );
}


#include "listview.moc"




