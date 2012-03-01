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
#include "kwidgetstreamer.h"
#include "kmultiformlistbox.h"
#include <tqobjectlist.h>
#include <tqvariant.h>


void KWidgetStreamer::toStream(const TQObject* from, TQDataStream& stream )
{
  if ( from->inherits("KMultiFormListBox") ) {
    // Hmm, we'll trust TQt that this dynamic_cast won't fail!
    dynamic_cast<const KMultiFormListBox*>(from)->toStream( stream );
  }

  propertyToStream( from, stream );
}

void KWidgetStreamer::fromStream( TQDataStream& stream, TQObject* to )
{
  if ( to->inherits("KMultiFormListBox") ) {
    // Hmm, we'll trust TQt that this dynamic_cast won't fail!
    dynamic_cast<KMultiFormListBox*>(to)->fromStream( stream );
  }

  propertyFromStream( stream, to );
}


void KWidgetStreamer::propertyToStream( const TQObject* from, TQDataStream& stream )
{
  // Only handle widgets. Alternatives to widgets are layouts, validators, timers, etc.
  if ( ! from->inherits(TQWIDGET_OBJECT_NAME_STRING) )
    return;

  // Serializing all the children (if any).
  const TQObjectList children = from->childrenListObject();
  if ( !children.isEmpty() ) {
    stream <<  children.count();
    for ( TQObjectListIt it = TQObjectListIt(children); *it; ++it ) {
      toStream( *it, stream );
    }
  }
  else {
    stream << (unsigned int) 0;
  }

  // Now stream out properties
  for ( PropertyMapIt mapIt = _map.begin(); mapIt != _map.end(); mapIt++ ) {
    TQString tp = mapIt.key();
    PropertyList list = mapIt.data();
    if ( from->inherits( tp.latin1() ) ) {
      for ( PropertyListIt it = list.begin(); it != list.end(); ++it ) {
        TQVariant prop = from->property( (*it).latin1() );
        if ( ! prop.isValid() )
          tqWarning("Invalid property: %s:%s", tp.latin1(), (*it).latin1() );

        stream <<  prop ;
      }
    }
  }
}


void KWidgetStreamer::propertyFromStream( TQDataStream& stream, TQObject* to )
{
  // Only handle widgets. Alternatives to widgets are layouts, validators, timers, etc.
  if ( ! to->inherits(TQWIDGET_OBJECT_NAME_STRING) )
    return;

  // Stream in all the children (if any)
  const TQObjectList children = to->childrenListObject();
  unsigned int count;

  stream >> count;
  if ( !children.isEmpty() ) {
    Q_ASSERT( count == children.count() );
    for ( TQObjectListIt it = TQObjectListIt(children); *it; ++it )
      fromStream( stream, *it );
  }
  else {
    Q_ASSERT( count == 0 );
  }

  // Now stream in properties
  for ( PropertyMapIt mapIt = _map.begin(); mapIt != _map.end(); mapIt++ ) {
    TQString tp = mapIt.key();
    PropertyList list = mapIt.data();
    if ( to->inherits( tp.latin1() ) ) {
      for ( PropertyListIt it = list.begin(); it != list.end(); ++it ) {
         TQVariant value;
        stream >> value;
        to->setProperty((*it).latin1(), value);
      }
    }
  }
}

KWidgetStreamer::KWidgetStreamer ()
{
  TQStringList l;

  // TQCheckBox
  l.clear();
  l << TQString::fromLatin1("enabled")
    << TQString::fromLatin1("checked") << TQString::fromLatin1("tristate");
  _map.insert(TQString::fromLatin1(TQCHECKBOX_OBJECT_NAME_STRING), l);

  // TQComboBox
  l.clear();
  l << TQString::fromLatin1("enabled")
    << TQString::fromLatin1("editable") << TQString::fromLatin1("currentItem")
    << TQString::fromLatin1("maxCount") << TQString::fromLatin1("insertionPolicy")
    << TQString::fromLatin1("autoCompletion");
  _map.insert(TQString::fromLatin1(TQCOMBOBOX_OBJECT_NAME_STRING), l);

  // TQDial
  l.clear();
  l << TQString::fromLatin1("enabled")
    << TQString::fromLatin1("tracking") << TQString::fromLatin1("wrapping")
    << TQString::fromLatin1("value");
  _map.insert(TQString::fromLatin1(TQDIAL_OBJECT_NAME_STRING), l);

  // TQLCDNumber
  l.clear();
  l << TQString::fromLatin1("enabled")
    << TQString::fromLatin1("numDigits") << TQString::fromLatin1("mode")
    << TQString::fromLatin1("segmentStyle") << TQString::fromLatin1("value");
  _map.insert(TQString::fromLatin1(TQLCDNUMBER_OBJECT_NAME_STRING), l);

  // TQLineEdit
  l.clear();
  l << TQString::fromLatin1("enabled")
    << TQString::fromLatin1("text") << TQString::fromLatin1("maxLength")
    << TQString::fromLatin1("echoMode") << TQString::fromLatin1("alignment");
  _map.insert(TQString::fromLatin1(TQLINEEDIT_OBJECT_NAME_STRING), l);

  // TQMultiLineEdit
  l.clear();
  l << TQString::fromLatin1("enabled")
    << TQString::fromLatin1("text")
    << TQString::fromLatin1("alignment");
  _map.insert(TQString::fromLatin1(TQTEXTEDIT_OBJECT_NAME_STRING), l);

  // TQRadioButton
  l.clear();
  l << TQString::fromLatin1("enabled")
    << TQString::fromLatin1("checked");
  _map.insert(TQString::fromLatin1(TQRADIOBUTTON_OBJECT_NAME_STRING), l);

  // TQSlider
  l.clear();
  l << TQString::fromLatin1("enabled")
    << TQString::fromLatin1("value");
  _map.insert(TQString::fromLatin1(TQSLIDER_OBJECT_NAME_STRING), l);

  // TQSpinBox
  l.clear();
  l << TQString::fromLatin1("enabled")
    << TQString::fromLatin1("value");
  _map.insert(TQString::fromLatin1(TQSPINBOX_OBJECT_NAME_STRING), l);
}
