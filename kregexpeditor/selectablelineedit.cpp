/*
 *  Copyright (c) 2002-2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef TQT_ONLY
  #include "selectablelineedit.moc"
#endif

#include "selectablelineedit.h"

SelectableLineEdit::SelectableLineEdit( RegExpWidget* owner, TQWidget* parent, const char* name)
  : TQLineEdit( parent, name ), _owner(owner)
{
  connect( this, TQT_SIGNAL( textChanged( const TQString & ) ), this,
           TQT_SLOT( slotKeyPressed() ) );
  setAcceptDrops( false );
}

void SelectableLineEdit::setSelected( bool selected )
{
  if ( selected ) {
    TQPalette pal = TQPalette(palette());
    pal.setBrush( TQColorGroup::Base, gray );
    setPalette( pal );
  }
  else {
    unsetPalette();
  }
  repaint();
}

TQSize SelectableLineEdit::sizeHint() const
{
  int frameWidth = frame() ? 8 : 4; // from TQLineEdit source
  TQFontMetrics metrics = fontMetrics();
  int actualSize = metrics.width( text() );
  int charWidth = metrics.maxWidth();
  int height = TQLineEdit::sizeHint().height();

  int width;
  if ( hasFocus() )
    width = actualSize+6*charWidth + frameWidth;
  else
    width = TQMAX(actualSize, charWidth) + frameWidth;
  return TQSize( width , height );
}

void SelectableLineEdit::slotKeyPressed()
{
  int frameWidth = frame() ? 8 : 4; // from TQLineEdit source

  TQFontMetrics metrics = fontMetrics();
  int actualSize = metrics.width( text() );

  if ( actualSize > size().width()-frameWidth ) {
    repaint();
    emit parentPleaseUpdate();
  }
}
