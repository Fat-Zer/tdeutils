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
#include "drag.h"
#include "regexp.h"
#include "regexpconverter.h"
#include "widgetfactory.h"

RegExpWidgetDrag::RegExpWidgetDrag( RegExp* regexp, TQWidget* dragSource )
  : TQDragObject( dragSource ), _regexp( regexp->clone() )
{
}

RegExpWidgetDrag::~RegExpWidgetDrag()
{
  delete _regexp;
}


bool RegExpWidgetDrag::canDecode( TQDragMoveEvent* event )
{
  return event->provides( "KRegExpEditor/widgetdrag" );
}

RegExpWidget* RegExpWidgetDrag::decode(TQDropEvent* event, RegExpEditorWindow* window,
                                       TQWidget* parent)
{
  TQByteArray payload = event->encodedData("KRegExpEditor/widgetdrag" );
  TQTextStream stream( payload, IO_ReadOnly );
  TQString str = stream.read();
  RegExp* regexp = WidgetFactory::createRegExp( str );
  RegExpWidget* widget = WidgetFactory::createWidget( regexp, window, parent );
  delete regexp;
  return widget;
}

const char * RegExpWidgetDrag::format ( int i ) const
{
  if ( i == 0 )
    return "KRegExpEditor/widgetdrag";
  else if ( i == 1 )
    return "text/plain";
  else
    return 0;
}

TQByteArray RegExpWidgetDrag::encodedData ( const char* format ) const
{
  TQByteArray data;
  TQTextStream stream( data, IO_WriteOnly );
  if ( TQString::fromLocal8Bit( format ).startsWith(TQString::fromLocal8Bit( "KRegExpEditor/widgetdrag" ) ) ) {
    TQString xml = _regexp->toXmlString();
    stream << xml;
  }
  else if ( TQString::fromLocal8Bit( format ).startsWith(TQString::fromLocal8Bit( "text/plain" ) ) ) {
      stream << RegExpConverter::current()->toStr( _regexp, false );
  }
  else {
    tqWarning("Unexpected drag and drop format: %s", format );
  }
  return data;
}

