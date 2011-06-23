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
#ifndef __kwidgetstreamer
#define __kwidgetstreamer
#include <tqmap.h>
#include <tqptrlist.h>
#include <tqstringlist.h>
#include <tqobject.h>

/**
   This class defines methods for streaming widget data.

   For each widget type rules are defined telling which attributes should be
   streamed. If you need to stream other attributes or to avoid streaming
   certain attributes, then this may be obtained by editing the property
   map, which is obtained by calling @ref propertyMap. For further control
   inherit from this class and override @ref toStream and @ref fromStream.

   The following example shows how you can avoid streaming
   <tt>numDigits</tt> for a TQLCDNumber. The same approach applies if you
   want to add extra properties to be streamed.
   <pre>
   KWidgetStreamer streamer;
   KWidgetStreamer::PropertyMap& map = streamer.propertyMap();
   KWidgetStreamer::PropertyList& list = *map.tqfind(TQLCDNUMBER_OBJECT_NAME_STRING);
   list.remove("numDigits");
   </pre>
**/
class KWidgetStreamer
{

public:
  typedef TQStringList PropertyList;
  typedef TQMap< TQString, PropertyList > PropertyMap;
  typedef TQMap< TQString, PropertyList >::ConstIterator PropertyMapIt;
  typedef TQStringList::Iterator PropertyListIt;

  KWidgetStreamer();
  virtual ~KWidgetStreamer() {}

  virtual void toStream(const TQObject* from, TQDataStream& stream );
  virtual void fromStream(TQDataStream& stream, TQObject* to);

  PropertyMap& propertyMap() { return _map; }


protected:
  void propertyToStream( const TQObject* from, TQDataStream& stream );
  void propertyFromStream( TQDataStream& stream, TQObject* to );

private:
  PropertyMap _map;

};



#endif /* __kwidgetstreamer */

