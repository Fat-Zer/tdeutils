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

#ifdef QT_ONLY
#include "compat.h"
#else
#include <klocale.h>
#include "charselector.moc"
#endif

#include "charselector.h"
#include "limitedcharlineedit.h"
#include "regexpconverter.h"
#include <tqlayout.h>
#include <tqwidgetstack.h>
#include <tqcombobox.h>

/**
   In the class CharSelector, three LimitedCharLineEdit are used.
   These widgets are all used in a TQWidgetStack. The LimitedCharLineEdit
   class is basically a TQLineEdit, which is limited to a certain
   number of characters. This conflicts with the TQWidgetStack, as this
   class expects the widgets on the stack to take up all space.
   StackContainer fills in this gab.
*/
class StackContainer :public QWidget
{
public:
    StackContainer( TQWidget* child, TQWidget* parent ) : TQWidget( parent )
        {
            TQHBoxLayout* layout = new TQHBoxLayout( this );
            child->reparent( this, TQPoint(0,0), false );
            layout->addWidget( child );
            layout->addStretch( 1 );
        }
};

CharSelector::CharSelector( TQWidget* parent, const char* name )
    :TQWidget( parent, name ), _oldIndex(0)
{
  TQStringList items;
  TQHBoxLayout* layout = new TQHBoxLayout( this, 0, 6 );

  _type = new TQComboBox( this, "_type" );
  items << i18n("Normal Character")
        << i18n("Unicode Char in Hex.")
        << i18n("Unicode Char in Oct.")
        << TQString::fromLatin1("----")
        << i18n("The Bell Character (\\a)")
        << i18n("The Form Feed Character (\\f)")
        << i18n("The Line Feed Character (\\n)")
        << i18n("The Carriage Return Character (\\r)")
        << i18n("The Horizontal Tab Character (\\t)")
        << i18n("The Vertical Tab Character (\\v)");
  _type->insertStringList( items );
  layout->addWidget( _type );

  _stack = new TQWidgetStack( this, "_stack" );
  layout->addWidget( _stack );

  _normal = new LimitedCharLineEdit( LimitedCharLineEdit::NORMAL, 0, "_normal" );
  _stack->addWidget( new StackContainer( _normal, _stack ), 0 );

  _hex = new LimitedCharLineEdit( LimitedCharLineEdit::HEX, _stack, "_hex" );
  _stack->addWidget( new StackContainer( _hex, _stack ), 1 );

  _oct = new LimitedCharLineEdit( LimitedCharLineEdit::OCT, _stack, "_oct" );
  _stack->addWidget( new StackContainer( _oct, _stack ), 2 );

  _stack->raiseWidget( 0 );

  connect( _type, TQT_SIGNAL( activated( int ) ), this, TQT_SLOT(slotNewItem( int ) ) );
}

void CharSelector::slotNewItem( int which )
{
  _type->setCurrentItem( which );
  if ( which <= 2 ) {
    _stack->raiseWidget( which );
    _normal->setEnabled( true );
    _hex->setEnabled( true );
    _oct->setEnabled( true );
  }
  else if ( which == 3 ) {
    _type->setCurrentItem( _oldIndex );
    slotNewItem(_oldIndex);
    return;
  }
  else {
    _normal->setEnabled( false );
    _hex->setEnabled( false );
    _oct->setEnabled( false );
  }

    _oldIndex = which;
}

void CharSelector::setText( TQString text )
{
    // This is the best I can do about missing character range features, unless all of
    // textrangeregexp is to be reworked. The problem is that textrangeregexp only allows to
    // get the characters, which would be something like \a, but \a does not work with say Emacs
    // style regexps -- ko28 Sep. 2003 10:55 -- Jesper K. Pedersen
    bool enabled = ( RegExpConverter::current()->features() & RegExpConverter::ExtRange );
    _type->setEnabled( enabled );

  if ( text.at(0) == TQChar('\\') ) {
    if ( text.at(1) == TQChar('x') ) {
      _hex->setText(text.mid(2));
      slotNewItem( 1 );
    }
    else if ( text.at(1) == TQChar('0') ) {
      _oct->setText(text.mid(2));
      slotNewItem( 2 );
    }
    else if ( text.at(1) == TQChar('a') )
      slotNewItem(4);
    else if ( text.at(1) == TQChar('f') )
      slotNewItem(5);
    else if ( text.at(1) == TQChar('n') )
      slotNewItem(6);
    else if ( text.at(1) == TQChar('r') )
      slotNewItem(7);
    else if ( text.at(1) == TQChar('t') )
      slotNewItem(8);
    else if ( text.at(1) == TQChar('v') )
      slotNewItem(9);
    else {
      qWarning("Warning %s:%d Unknown escape %s", __FILE__, __LINE__, text.latin1() );
    }
  }
  else {
    slotNewItem(0);
    _normal->setText(text);
  }
}

bool CharSelector::isEmpty() const
{
  return ( _type->currentItem() == 0 && _normal->text().isEmpty() ) ||
    ( _type->currentItem() == 1 && _hex->text().isEmpty() ) ||
    ( _type->currentItem() == 2 && _oct->text().isEmpty() );
}

TQString CharSelector::text() const
{
  switch ( _type->currentItem() ) {
  case 0: // Normal Character
    return _normal->text();
  case 1: // Hex
    return TQString::fromLocal8Bit("\\x") + _hex->text();
  case 2: // Oct
      return TQString::fromLocal8Bit("\\0") + _oct->text();
  case 3: // The seperator
    break;
  case 4:
    return TQString::fromLatin1("\\a");
  case 5:
    return TQString::fromLatin1("\\f");
  case 6:
    return TQString::fromLatin1("\\n");
  case 7:
    return TQString::fromLatin1("\\r");
  case 8:
    return TQString::fromLatin1("\\t");
  case 9:
    return TQString::fromLatin1("\\v");
  }
  return TQString::null;
}
