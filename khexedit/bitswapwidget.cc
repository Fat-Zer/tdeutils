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

#include <tqpainter.h>
 
#include <tdeglobalsettings.h>

#include "bitswapwidget.h"

CDigitLabel::CDigitLabel( TQWidget *parent, uint digit, const char *name )
  :TQLabel(parent, name), mDigit( digit ), mDotPosition( 0 )
{
  setFocusPolicy( TQ_StrongFocus );
  initialize();
}

CDigitLabel::~CDigitLabel( void )
{

}

void CDigitLabel::initialize( void )
{
  TQFont font( TDEGlobalSettings::generalFont() );
  font.setBold( true );
  setFont( font );
}


void CDigitLabel::paletteChange( const TQPalette & )
{
  update();
}


void CDigitLabel::setValue( uint digit, bool notify )
{
  if( mDigit != digit )
  {
    if( notify == true )
    {
      emit valueChanged( TQT_TQOBJECT(this), digit, false );
    }

    mDigit = digit;
    update();

    if( notify == true )
    {
      emit valueChanged( TQT_TQOBJECT(this), mDigit, true );
    }
  }
}


void CDigitLabel::setDotPosition( uint dotPosition )
{
  if( mDotPosition != dotPosition )
  {
    mDotPosition = dotPosition;
    update();
  }
}



TQSize CDigitLabel::sizeHint( void ) const
{
  int h = fontMetrics().height();
  TQSize s( h, h ); // Retangular
  return( s );
}
 

#include <stdio.h>


void CDigitLabel::drawContents( TQPainter *p )
{
  TQRect cr = contentsRect();

  if( hasFocus() == true )
  {
    p->fillRect( cr, palette().active().highlight() );
    p->setPen( palette().active().highlightedText() );
  }
  else
  {
    p->fillRect( cr, palette().active().base() );
    p->setPen( palette().active().text() );
  }

  if( mDotPosition != 0 )
  {
    p->fillRect( cr.x()+2, cr.y()+2, 5, 5, TQt::red ); // UL    
    /*
    if( mDotPosition == 1 )
    {
      p->fillRect( cr.x()+2, cr.y()+2, 5, 5, red ); // UL
    }
    else if( mDotPosition == 2 )
    {
      p->fillRect( cr.width()-7, cr.y()+2, 5, 5, red ); // UR
    }
    else if( mDotPosition == 3 ) 
    {
      p->fillRect( cr.x()+2, cr.height()-7, 5, 5, red ); // LL
    }
    else if( mDotPosition == 4 )
    {
      p->fillRect( cr.width()-7, cr.height()-7, 5, 5, red ); // LR
    } 
    */
  }

  TQString text;
  text.setNum( mDigit );
  p->drawText( 0, 0, cr.width(), cr.height(), alignment(), text );

  

}


void CDigitLabel::keyPressEvent( TQKeyEvent *e )  
{
  switch ( e->key() ) 
  {
    case Key_Left:
    case Key_Up: 
      emit stepCell( TQT_TQOBJECT(this), false );
    break;

    case Key_Right:
    case Key_Down:  
      emit stepCell( TQT_TQOBJECT(this), true );
    break;
    
    case Key_Escape:
      e->ignore(); // Allow dialog or whatever use this one
      return;
    break;

    default:
    {
      int val =  e->text()[0].digitValue();
      if( val >= 0 && val <= 7 )
      {
	setValue( val, true );
      }
    }
    break;
  }
  
  e->accept();
}



CByteWidget::CByteWidget( TQWidget *parent, const char *name )
  :TQWidget(parent, name)
{
  mHBox = new TQHBoxLayout( this, 0 );
  
  for( uint i=0; i<8; i++ )
  {
    mDigit[i] = new CDigitLabel( this, 7-i );
    mDigit[i]->setLineWidth( 1 );
    mDigit[i]->setFixedSize( mDigit[i]->sizeHint()*2 );
    mDigit[i]->setFrameStyle( TQFrame::Panel | TQFrame::Sunken );
    mDigit[i]->setAlignment( AlignCenter );
    connect( mDigit[i], TQT_SIGNAL(stepCell(const TQObject *, bool )),
	     this, TQT_SLOT(stepCell(const TQObject *, bool )));
    connect( mDigit[i], TQT_SIGNAL(valueChanged(const TQObject *, uint, bool )),
	     this, TQT_SLOT(valueChanged(const TQObject *, uint, bool )));
    mHBox->addWidget( mDigit[i] );
  }

  mHBox->addStretch();  
}

CByteWidget::~CByteWidget( void )
{
  for( uint i=0; i<8; i++ ) { delete mDigit[i]; }
  delete mHBox;
}

void CByteWidget::stepCell( const TQObject *obj, bool next )
{
  for( uint i=0; i<8; i++ )
  {
    if( TQT_BASE_OBJECT_CONST(obj) == TQT_BASE_OBJECT_CONST(mDigit[i]) )
    {
      if( next == true )
      {
	mDigit[(i+1)%8]->setFocus();
      }
      else
      {
	mDigit[(i+7)%8]->setFocus();
      }
    }
  }
}



void CByteWidget::valueChanged( const TQObject *obj, uint val, bool after )
{
  if( after == false )
  {
    for( uint i=0; i<8; i++ )
    {
      if( TQT_BASE_OBJECT_CONST(obj) == TQT_BASE_OBJECT_CONST(mDigit[i]) )
      {
	uint tmp = 7-mDigit[i]->value();
	mDigit[tmp]->setValue( mDigit[i]->value(), false );
	break;
      }
    }


    if( mDigit[7-val]->value() != val )
    {
      uint tmp = mDigit[7-val]->value();
      mDigit[7-tmp]->setValue( tmp, false );
      mDigit[7-val]->setValue( val, false );
    }

  }
  else
  {
    setBuddy( obj );

    uint pos = 1;
    for( uint i=0; i<8; i++ )
    {
      if( mDigit[i]->value() < (7-i) )
      {
	mDigit[i]->setDotPosition( pos );
	mDigit[7-mDigit[i]->value()]->setDotPosition( pos );
	pos++;
      }
      else if( mDigit[i]->value() == (7-i) )
      {
	mDigit[i]->setDotPosition( 0 );
      }
    }

  }
}


void CByteWidget::reset( void )
{
  for( uint i=0; i<8; i++ )
  {
    mDigit[i]->setValue( 7-i, false );
    mDigit[i]->setDotPosition( 0 );
  }
}


void CByteWidget::setBuddy( const TQObject *obj )
{
  for( uint i=0; i<8; i++ )
  {
    if( TQT_BASE_OBJECT_CONST(obj) == TQT_BASE_OBJECT_CONST(mDigit[i]) )
    {
      uint val = mDigit[i]->value();
      if( val < 8 )
      {
	mDigit[7-val]->setValue( 7-i, false );
      }
      break;
    }
  }
}





bool CByteWidget::flag( TQByteArray &buf )
{
  bool swapPresent = false;
  uint i, j;
  
  buf.resize( 8 );
  for( i=0; i<8; buf[i++] = 0 );

  for( i=j=0; i<8; i++ )
  {
    if( mDigit[i]->value() < (7-i) )
    {
      buf[j] = 7 - i; j++;
      buf[j] = mDigit[i]->value(); j++;
      swapPresent = true;
    }
  }

  return( swapPresent );
}


#include "bitswapwidget.moc"
