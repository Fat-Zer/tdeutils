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

  // Serializing all the tqchildren (if any).
  const TQObjectList tqchildren = from->childrenListObject();
  if ( !tqchildren.isEmpty() ) {
    stream <<  tqchildren.count();
    for ( TQObjectListIt it = TQObjectListIt(tqchildren); *it; ++it ) {
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
          qWarning("Invalid property: %s:%s", tp.latin1(), (*it).latin1() );

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

  // Stream in all the tqchildren (if any)
  const TQObjectList tqchildren = to->childrenListObject();
  unsigned int count;

  stream >> count;
  if ( !tqchildren.isEmpty() ) {
    Q_ASSERT( count == tqchildren.count() );
    for ( TQObjectListIt it = TQObjectListIt(tqchildren); *it; ++it )
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
  l << TQString::tqfromLatin1("enabled")
    << TQString::tqfromLatin1("checked") << TQString::tqfromLatin1("tristate");
  _map.insert(TQString::tqfromLatin1(TQCHECKBOX_OBJECT_NAME_STRING), l);

  // TQComboBox
  l.clear();
  l << TQString::tqfromLatin1("enabled")
    << TQString::tqfromLatin1("editable") << TQString::tqfromLatin1("currentItem")
    << TQString::tqfromLatin1("maxCount") << TQString::tqfromLatin1("insertionPolicy")
    << TQString::tqfromLatin1("autoCompletion");
  _map.insert(TQString::tqfromLatin1(TQCOMBOBOX_OBJECT_NAME_STRING), l);

  // TQDial
  l.clear();
  l << TQString::tqfromLatin1("enabled")
    << TQString::tqfromLatin1("tracking") << TQString::tqfromLatin1("wrapping")
    << TQString::tqfromLatin1("value");
  _map.insert(TQString::tqfromLatin1(TQDIAL_OBJECT_NAME_STRING), l);

  // TQLCDNumber
  l.clear();
  l << TQString::tqfromLatin1("enabled")
    << TQString::tqfromLatin1("numDigits") << TQString::tqfromLatin1("mode")
    << TQString::tqfromLatin1("segmentStyle") << TQString::tqfromLatin1("value");
  _map.insert(TQString::tqfromLatin1(TQLCDNUMBER_OBJECT_NAME_STRING), l);

  // TQLineEdit
  l.clear();
  l << TQString::tqfromLatin1("enabled")
    << TQString::tqfromLatin1("text") << TQString::tqfromLatin1("maxLength")
    << TQString::tqfromLatin1("echoMode") << TQString::tqfromLatin1("tqalignment");
  _map.insert(TQString::tqfromLatin1(TQLINEEDIT_OBJECT_NAME_STRING), l);

  // TQMultiLineEdit
  l.clear();
  l << TQString::tqfromLatin1("enabled")
    << TQString::tqfromLatin1("text")
    << TQString::tqfromLatin1("tqalignment");
  _map.insert(TQString::tqfromLatin1(TQTEXTEDIT_OBJECT_NAME_STRING), l);

  // TQRadioButton
  l.clear();
  l << TQString::tqfromLatin1("enabled")
    << TQString::tqfromLatin1("checked");
  _map.insert(TQString::tqfromLatin1(TQRADIOBUTTON_OBJECT_NAME_STRING), l);

  // TQSlider
  l.clear();
  l << TQString::tqfromLatin1("enabled")
    << TQString::tqfromLatin1("value");
  _map.insert(TQString::tqfromLatin1(TQSLIDER_OBJECT_NAME_STRING), l);

  // TQSpinBox
  l.clear();
  l << TQString::tqfromLatin1("enabled")
    << TQString::tqfromLatin1("value");
  _map.insert(TQString::tqfromLatin1(TQSPINBOX_OBJECT_NAME_STRING), l);
}
