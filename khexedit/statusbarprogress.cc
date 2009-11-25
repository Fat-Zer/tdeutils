/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *   This file is based on the work by Martynas Kunigelis (KProgress)
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

#include "statusbarprogress.h"

#include <qpainter.h>

#include <kapplication.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <qstyle.h>

CStatusBarProgress::CStatusBarProgress( QWidget *parent, const char *name )
  : QFrame(parent, name), QRangeControl(0, 100, 1, 10, 0),
    mOrientation( Horizontal )
{
  initialize();
}

CStatusBarProgress::CStatusBarProgress( Orientation orientation,
					QWidget *parent, const char *name )
  : QFrame(parent, name), QRangeControl(0, 100, 1, 10, 0),
    mOrientation( orientation )
{
  initialize();
}

CStatusBarProgress::CStatusBarProgress( int minValue, int maxValue, int value,
					Orientation orientation,
					QWidget *parent, const char *name )
  : QFrame(parent, name), QRangeControl(minValue, maxValue, 1, 10, value),
    mOrientation(orientation)
{
  initialize();
}

CStatusBarProgress::~CStatusBarProgress()
{
  delete mBarPixmap;
}

void CStatusBarProgress::advance( int offset )
{
  setValue(value() + offset);
}

void CStatusBarProgress::initialize( void )
{
  mBarPixmap    = 0;
  mBarStyle     = Solid;

  mBarColor     = palette().active().highlight();
  mBarTextColor = palette().active().highlightedText();
  mTextColor    = palette().active().text();
  setBackgroundColor( palette().active().background() );

  QFont font( KGlobalSettings::generalFont() );
  font.setBold( true );
  setFont( font );

  mTextEnabled = false;
  adjustStyle();
}


void CStatusBarProgress::setBarPixmap( const QPixmap &pixmap )
{
  if( pixmap.isNull() == true )
  {
    return;
  }

  delete mBarPixmap;
  mBarPixmap = new QPixmap( pixmap );
}

void CStatusBarProgress::setBarColor( const QColor &color )
{
  mBarColor = color;
  delete mBarPixmap;
  mBarPixmap = 0;
}

void CStatusBarProgress::setBarStyle( BarStyle style )
{
  if( mBarStyle != style )
  {
    mBarStyle = style;
    update();
  }
}

void CStatusBarProgress::setOrientation( Orientation orientation )
{
  if( mOrientation != orientation )
  {
    mOrientation = orientation;
    update();
  }
}

void CStatusBarProgress::setValue(int value)
{
  mCurPage = mMaxPage = -1;
  QRangeControl::setValue(value);
}

void CStatusBarProgress::setValue( int curPage, int maxPage )
{
  if( curPage <= 0 || maxPage <= 0 || curPage > maxPage )
  {
    mCurPage = mMaxPage = -1;
    QRangeControl::setValue(0);
  }
  else
  {
    mCurPage = curPage;
    mMaxPage = maxPage;
    float fraction = (float)curPage/(float)maxPage;
    QRangeControl::setValue( (int)(fraction*100.0) );
  }
}


void CStatusBarProgress::setTextEnabled( bool state )
{
  if( mTextEnabled != state )
  {
    mTextEnabled = state;
    update();
  }
}

void CStatusBarProgress::setText( const QString &msg )
{
  mMsg = msg;
  if( mTextEnabled == true )
  {
    update();
  }
}




const QColor & CStatusBarProgress::barColor( void ) const
{
  return( mBarColor );
}

const QPixmap * CStatusBarProgress::barPixmap( void ) const
{
  return( mBarPixmap );
}

bool CStatusBarProgress::textEnabled( void ) const
{
  return( mTextEnabled );
}

QSize CStatusBarProgress::sizeHint( void ) const
{
  QSize s( size() );

  if( orientation() == CStatusBarProgress::Vertical )
  {
    s.setWidth( fontMetrics().lineSpacing() );
  }
  else
  {
    s.setHeight( fontMetrics().lineSpacing() );
  }

  return( s );
}


CStatusBarProgress::Orientation CStatusBarProgress::orientation( void ) const
{
  return( mOrientation );
}

CStatusBarProgress::BarStyle CStatusBarProgress::barStyle() const
{
  return( mBarStyle );
}

int CStatusBarProgress::recalcValue(int range)
{
  int abs_value = value() - minValue();
  int abs_range = maxValue() - minValue();
  return range * abs_value / abs_range;
}

void CStatusBarProgress::valueChange()
{
  repaint(contentsRect(), FALSE);
  emit percentageChanged(recalcValue(100));
}

void CStatusBarProgress::rangeChange()
{
  repaint(contentsRect(), FALSE);
  emit percentageChanged(recalcValue(100));
}

void CStatusBarProgress::styleChange( GUIStyle )
{
  adjustStyle();
}

void CStatusBarProgress::adjustStyle( void )
{
  switch( style().SH_GUIStyle)
  {
    case WindowsStyle:
      setFrameStyle( QFrame::NoFrame );
    break;

    case MotifStyle:
    default:
      setFrameStyle( QFrame::Panel|QFrame::Sunken );
      setLineWidth( 1 );
    break;
  }
  update();
}

void CStatusBarProgress::paletteChange( const QPalette & )
{
  mBarColor     = palette().active().highlight();
  mBarTextColor = palette().active().highlightedText();
  mTextColor    = palette().active().text();
  setBackgroundColor( palette().active().background() );
}

void CStatusBarProgress::drawText( QPainter *p )
{
  QRect r(contentsRect());
  QString s;

  if( mMsg.isEmpty() == true )
  {
    s = QString("%1%").arg(recalcValue(100));
  }
  else
  {
    if( mCurPage > 0 )
    {
      s = i18n("%1... %2 of %3").arg(mMsg).arg(mCurPage).arg(mMaxPage);
    }
    else
    {
      s = i18n("%1... %2%").arg(mMsg).arg(recalcValue(100));
    }
  }

  p->setPen( mTextColor );
  p->drawText( r, AlignCenter, s );
  p->setClipRegion( fr );
  p->setPen( mBarTextColor );
  p->drawText( r, AlignCenter, s );
}

void CStatusBarProgress::drawContents(QPainter *p)
{
  QRect cr = contentsRect(), er = cr;
  fr = cr;
  QBrush fb(mBarColor), eb(backgroundColor() );

  if( mBarPixmap != 0 )
  {
    fb.setPixmap( *mBarPixmap );
  }

  if (backgroundPixmap())
    eb.setPixmap(*backgroundPixmap());

  switch( mBarStyle )
  {
    case Solid:
      if( mOrientation == Horizontal )
      {
	fr.setWidth(recalcValue(cr.width()));
	er.setLeft(fr.right() + 1);
      }
      else
      {
	fr.setTop(cr.bottom() - recalcValue(cr.height()));
	er.setBottom(fr.top() - 1);
      }

      p->setBrushOrigin(cr.topLeft());
      p->fillRect(fr, fb);
      p->fillRect(er, eb);

      if( mTextEnabled == true )
	drawText(p);
    break;

    case Blocked:
      const int margin = 2;
      int max, num, dx, dy;
      if( mOrientation == Horizontal )
      {
	fr.setHeight(cr.height() - 2 * margin);
	fr.setWidth((int)(0.67 * fr.height()));
	fr.moveTopLeft(QPoint(cr.left() + margin, cr.top() + margin));
	dx = fr.width() + margin;
	dy = 0;
	max = (cr.width() - margin) / (fr.width() + margin) + 1;
	num = recalcValue(max);
      }
      else
      {
	fr.setWidth(cr.width() - 2 * margin);
	fr.setHeight((int)(0.67 * fr.width()));
	fr.moveBottomLeft(QPoint(cr.left() + margin, cr.bottom() - margin));
	dx = 0;
	dy = - (fr.height() + margin);
	max = (cr.height() - margin) / (fr.height() + margin) + 1;
	num = recalcValue(max);
      }
      p->setClipRect(cr.x() + margin, cr.y() + margin,
		     cr.width() - margin, cr.height() - margin);
      for (int i = 0; i < num; i++)
      {
	p->setBrushOrigin(fr.topLeft());
	p->fillRect(fr, fb);
	fr.moveBy(dx, dy);
      }

      if (num != max)
      {
	if( mOrientation == Horizontal )
	  er.setLeft(fr.right() + 1);
	else
	  er.setBottom(fr.bottom() + 1);
	if (!er.isNull())
	{
	  p->setBrushOrigin(cr.topLeft());
	  p->fillRect(er, eb);
	}
      }
    break;
  }

}


void CStatusBarProgress::mousePressEvent( QMouseEvent */*e*/ )
{
  emit pressed();
}

#include "statusbarprogress.moc"
