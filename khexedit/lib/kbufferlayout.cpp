/***************************************************************************
                          kbufferlayout.cpp  -  description
                             -------------------
    begin                : Thu Jun 12 2003
    copyright            : (C) 2003 by Friedrich W. H. Kossebau
    email                : Friedrich.W.H@Kossebau.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/


#include "kbufferlayout.h"

using namespace KHE;



TDEBufferLayout::TDEBufferLayout( int NoBpL, int SO, int L )
 : NoOfBytesPerLine( NoBpL ),
   StartOffset( SO ),
   Length( L )
{
  calcStart();
  calcEnd();
}


TDEBufferLayout::~TDEBufferLayout()
{
}


bool TDEBufferLayout::setStartOffset( int SO )
{
  // rejecting <0
  if( SO < 0 )
    SO = 0;

  if( StartOffset == SO )
    return false;

  StartOffset = SO;

  calcStart();
  calcEnd();
  return true;
}


bool TDEBufferLayout::setNoOfBytesPerLine( int N )
{
  // rejecting <1
  if( N < 1 )
    N = 1;

  // no changes?
  if( NoOfBytesPerLine == N )
    return false;

  NoOfBytesPerLine = N;

  calcStart();
  calcEnd();
  return true;
}


bool TDEBufferLayout::setLength( int L )
{
  // rejecting < 0
  if( L < 0 )
    L = 0;

  // no changes?
  if( Length == L )
    return false;

  Length = L;

  calcEnd();
  return true;
}


void TDEBufferLayout::setNoOfLinesPerPage( int N )
{
  NoOfLinesPerPage = N;
}


void TDEBufferLayout::calcStart()
{
  ContentCoords.setStart( TDEBufferCoord(StartOffset,NoOfBytesPerLine,false) );
}


void TDEBufferLayout::calcEnd()
{
  ContentCoords.setEnd( (Length>0)?TDEBufferCoord(Length-1+StartOffset,NoOfBytesPerLine,false):
                                   TDEBufferCoord(-1,ContentCoords.start().line()) );
}


int TDEBufferLayout::indexAtCLineStart( int L ) const
{
  return ( L <= ContentCoords.start().line() ) ? 0:
         ( L > ContentCoords.end().line() ) ?    Length-1:
                                                 L * NoOfBytesPerLine - StartOffset;
}


int TDEBufferLayout::indexAtCLineEnd( int L ) const
{
  return ( L < ContentCoords.start().line() ) ? 0:
         ( L >= ContentCoords.end().line() ) ?  Length-1:
                                                (L+1)*NoOfBytesPerLine-StartOffset-1;
}


int TDEBufferLayout::indexAtCCoord( const TDEBufferCoord &C ) const
{
  int Index = indexAtCoord( C );

  return ( Index <= 0 ) ?      0:
         ( Index >= Length ) ? Length-1:
                               Index;
}


int TDEBufferLayout::lineAtCIndex( int Index ) const
{
  return ( Index <= 0 ) ?      ContentCoords.start().line():
         ( Index >= Length ) ? ContentCoords.end().line():
                               lineAtIndex(Index);
}


TDEBufferCoord TDEBufferLayout::coordOfCIndex( int Index ) const
{
  return ( Index <= 0 ) ?      ContentCoords.start():
         ( Index >= Length ) ? ContentCoords.end():
                               coordOfIndex(Index);
}


int TDEBufferLayout::indexAtLineStart( int L ) const
{
  return ( L == ContentCoords.start().line() ) ? 0 : L*NoOfBytesPerLine-StartOffset;
}


int TDEBufferLayout::indexAtLineEnd( int L ) const
{
  return ( L == ContentCoords.end().line() ) ? Length-1 : (L+1)*NoOfBytesPerLine-StartOffset-1;
}


int TDEBufferLayout::indexAtCoord( const TDEBufferCoord &C ) const
{
  return C.indexByLineWidth( NoOfBytesPerLine ) - StartOffset;
}

int TDEBufferLayout::lineAtIndex( int Index ) const
{
  return (Index+StartOffset)/NoOfBytesPerLine;
}

TDEBufferCoord TDEBufferLayout::coordOfIndex( int Index ) const
{
  return TDEBufferCoord( Index+StartOffset, NoOfBytesPerLine, false );
}



int TDEBufferLayout::correctIndex( int I ) const
{
  return ( I <= 0 ) ?      0:
         ( I >= Length ) ? Length-1:
                           I;
}


TDEBufferCoord TDEBufferLayout::correctCoord( const TDEBufferCoord &C ) const
{
  return ( C <= ContentCoords.start() ) ?  ContentCoords.start():
         ( C >= ContentCoords.end() ) ?    ContentCoords.end():
         ( C.pos() >= NoOfBytesPerLine ) ? TDEBufferCoord( NoOfBytesPerLine-1, C.line() ):
                                           C;
}


bool TDEBufferLayout::atLineStart( const TDEBufferCoord &C ) const
{
  return ( C.line() == ContentCoords.start().line() ) ? C.pos() == ContentCoords.start().pos():
                                                        C.pos() == 0;
}

bool TDEBufferLayout::atLineEnd( const TDEBufferCoord &C ) const
{
  return ( C.line() == ContentCoords.end().line() ) ? C.pos() == ContentCoords.end().pos():
                                                      C.pos() == NoOfBytesPerLine-1;
}


KSection TDEBufferLayout::positions( int Line ) const
{
  return KSection( firstPos(Line), lastPos(Line) );
}


int TDEBufferLayout::firstPos( const TDEBufferCoord &C ) const
{
  return ( ContentCoords.start().isLaterInLineThan(C) ) ? ContentCoords.start().pos() : C.pos();
}

int TDEBufferLayout::lastPos( const TDEBufferCoord &C ) const
{
  return ( ContentCoords.end().isPriorInLineThan(C) ) ? ContentCoords.end().pos() : C.pos();
}

int TDEBufferLayout::firstPos( int Line ) const
{
  return Line == ContentCoords.start().line() ? ContentCoords.start().pos() : 0;
}

int TDEBufferLayout::lastPos( int Line ) const
{
  return ( Line == ContentCoords.end().line() ) ? ContentCoords.end().pos() : NoOfBytesPerLine-1;
}

bool TDEBufferLayout::hasContent( int Line ) const
{
  return ContentCoords.includesLine( Line );
}
