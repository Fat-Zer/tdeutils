/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999-2000 Espen Sand, espensa@online.no
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

#include <stdlib.h>

#include <qbitmap.h>
#include <qpainter.h>

#include <kurldrag.h>

#include "draglabel.h"


CDragLabel::CDragLabel( QWidget *parent )
  : QLabel( "draglabel", parent, "kde toolbar widget" )
{
  mValid = true;
  mDragPending = false;
  setBackgroundMode( Qt::PaletteButton );
}


CDragLabel::~CDragLabel( void )
{
}


void CDragLabel::mousePressEvent( QMouseEvent *e )
{
  if( mValid == false || e->button() != LeftButton || mUrl.isEmpty() == true )
  {
    return;
  }

  mDragOrigin = e->pos();
  mDragPending = true;
}


void CDragLabel::mouseMoveEvent( QMouseEvent *e )
{
  if( mDragPending == true )
  {
    if( abs(e->x() - mDragOrigin.x()) + abs(e->y() - mDragOrigin.y()) > 5 )
    {
      mDragPending = false;

      //
      // Make drag object, assign pixmap and grab keyboard. The grabbing
      // will allow ESC to abort the drag
      //
      KURL::List uris;
      uris.append(KURL(mUrl));
      KURLDrag *uriDrag = new KURLDrag( uris, this );
      if( uriDrag == 0 ) { return; }
      prepPixmap( *uriDrag );
      grabKeyboard();
      uriDrag->drag();
      releaseKeyboard();
    }
  }
}

void CDragLabel::mouseReleaseEvent( QMouseEvent * )
{
  mDragPending = false;
}


void CDragLabel::setUrl( const QString &url )
{
  mUrl = url;
}


void CDragLabel::setDragMask( const QPixmap pix )
{
  mDragMask = pix;
}


void CDragLabel::prepPixmap( KURLDrag &uriDrag )
{
  if( pixmap() == 0 )
  {
    return;
  }

  QString text;
  int index = mUrl.findRev( '/', mUrl.length() );
  if( index < 0 )
  {
    text = mUrl;
  }
  else
  {
    text = mUrl.right( mUrl.length() - index - 1 );
  }

  int sep = 2;
  QRect rect = fontMetrics().boundingRect(text);
  int w = pixmap()->width() + rect.width() + sep + 2;
  int h = fontMetrics().lineSpacing();
  if( pixmap()->height() > h ) { h = pixmap()->height(); }

  QBitmap mask( w, h, TRUE );
  QPixmap dragPixmap( w, h );
  dragPixmap.fill( black );

  QPainter p;
  p.begin( &mask );
  p.setPen( white );
  p.drawPixmap( 0, 0, mDragMask );
  p.drawText( pixmap()->width()+sep, 0, w-pixmap()->width()+sep,
	      h, AlignVCenter|AlignLeft, text );
  p.end();

  p.begin( &dragPixmap );
  p.drawPixmap( 0, 0, *pixmap() );
  p.end();

  dragPixmap.setMask( mask );

  QPoint hotspot( pixmap()->width(), pixmap()->height()/2 );
  uriDrag.setPixmap( dragPixmap, hotspot );
}





#include "draglabel.moc"
