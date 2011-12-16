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


#include "hexdrag.h"
static const char *mediaString = "application/octet-stream";


CHexDrag::CHexDrag( const TQByteArray &data, TQWidget *dragSource,
		    const char *name )
  :TQDragObject(dragSource,name)
{
  setData( data );
  prepPixmap();
}


CHexDrag::CHexDrag( TQWidget *dragSource, const char *name )
  :TQDragObject(dragSource,name)
{
  prepPixmap();
}


void CHexDrag::setData( const TQByteArray &data )
{
  mData = data;
}



void CHexDrag::prepPixmap(void)
{
  //
  // Wont use it yet,
  //
  /*
  KIconLoader &loader = *KGlobal::iconLoader();
  TQPixmap pix = loader.loadIcon( "binary.xpm" );

  TQPoint hotspot( pix.width()-20,pix.height()/2 );
  setPixmap( pix, hotspot ); 
  */
}


const char *CHexDrag::format( int i ) const
{
  if( i == 0 )
  {
    return( mediaString );
  }
  else
  {
    return( 0 );
  }
  return( i == 0 ? mediaString : 0 );
}


TQByteArray CHexDrag::tqencodedData( const char *fmt ) const
{
  if( fmt != 0 )
  {
    if( strcmp( fmt, mediaString) == 0 )
    {
      return( mData );
    }
  }

  TQByteArray buf;
  return( buf );
}


bool CHexDrag::canDecode( const TQMimeSource *e )
{
  return( e->provides(mediaString) );
}


bool CHexDrag::decode( const TQMimeSource *e, TQByteArray &dest )
{
  dest = e->tqencodedData(mediaString);
  return( dest.size() == 0 ? false : true );

  //
  // I get an 
  // "X Error: BadAtom (invalid Atom parameter) 5
  //   Major opcode:  17"
  //
  // if I try to use the code below on a source that has been 
  // collected from TQClipboard. It is the e->provides(mediaString)
  // that fail (TQt-2.0). Sometimes it works :(
  //
  // printf("0: %s\n", e->format(0) ); No problem. 
  // printf("1: %s\n", e->format(1) ); Crash. 
  //
  #if 0
  if( e->provides(mediaString) == true )
  {
    dest = e->tqencodedData(mediaString);
    return( true );
  }
  else
  {
    return( false );
  }
  #endif
}


#include "hexdrag.moc"
