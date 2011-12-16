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
#include "limitedcharlineedit.h"
#include <tqvalidator.h>

/**
   @internal
   A Validator for the @ref LimitedCharLineEdit
*/
class Validator :public TQValidator
{
public:
  Validator( LimitedCharLineEdit::Mode mode, TQWidget* parent )
    :TQValidator( TQT_TQOBJECT(parent), "Validator" ), _mode(mode)
  {
  }

  virtual TQValidator::State validate( TQString& txt, int & /*pos*/ ) const
  {
    if ( _mode == LimitedCharLineEdit::NORMAL ||
         (_mode == LimitedCharLineEdit::HEX &&
          TQRegExp(TQString::fromLocal8Bit("^[0-9A-Fa-f]*$")).search( txt ) != -1) ||
         (_mode == LimitedCharLineEdit::OCT &&
          TQRegExp(TQString::fromLocal8Bit("^[0-7]*$")).search( txt ) != -1 ) ) {
      return TQValidator::Acceptable;
    }
    else {
      return TQValidator::Invalid;
    }
  }

private:
  LimitedCharLineEdit::Mode _mode;
};


void LimitedCharLineEdit::keyPressEvent ( TQKeyEvent *event )
{
  TQLineEdit::keyPressEvent( event );
  if ( text().length() == _count && !event->text().isNull() )
    focusNextPrevChild(true);
}

LimitedCharLineEdit::LimitedCharLineEdit( Mode mode, TQWidget* parent, const char* name )
	:TQLineEdit( parent, name ), _mode(mode)
{
  if ( mode == NORMAL )
    _count = 1;
  else if ( mode == HEX )
    _count = 4;
  else
    _count = 4;

  setMaxLength( _count );
  setFixedSize( fontMetrics().width('A')*5+5, tqsizeHint().height());

  setValidator( new Validator( mode, this ) );
}


