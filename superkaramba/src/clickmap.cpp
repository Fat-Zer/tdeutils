/***************************************************************************
 *   Copyright (C) 2003 by Ralph M. Churchill                              *
 *   mrchucho@yahoo.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "clickmap.h"
#include <tqregexp.h>
#include <krun.h>

ClickMap::ClickMap(karamba* k, int x, int y, int w, int h )
    :Meter(k, x, y, w, h )
{
/*
    if( h != 0 || w != 0)
        clip = 0;
    else
        clip = TQt::DontClip;
*/

    if( h == 0 || w == 0)
    {
        setWidth(-1);
        setHeight(-1);
    }
}

ClickMap::~ClickMap()
{
}

void ClickMap::setTextProps( TextField *t )
{
    text = *t;
}

bool ClickMap::click( TQMouseEvent *e ) {

  //Don't load the web page if the click isn't for us
  if (boundingBox.contains(e->x(), e->y())) {

    int index = ((e -> y() - getY()) / text.getLineHeight()) + 1;
    if (index >= 1 && index <= (int)displays.count()) {
      // tqDebug( "You clicked item " + TQString::number( index ) + ", " +
      //  displays[index - 1] + " " + links[index - 1] );
      KRun::runCommand("konqueror " + links[index - 1]);
    }
  }
  return false;
}

void ClickMap::mUpdate( TQPainter *p )
{
    int i = 0; //text.getLineHeight();
    int row = 1;

    p->setFont(text.getFont());
    TQStringList::Iterator it = displays.begin();
    while( it != displays.end() && (row <= getHeight() || getHeight() == -1 )   )
    {
        p->setPen( text.getColor() );
        // p->drawText(x,y+i,width,height,  TQt::AlignCenter | TQt::ExpandTabs, *it);
        p->drawText(getX(), getY() + i + text.getLineHeight(), *it);
        i += text.getLineHeight();
        it++;
        row++;
    }
}

void ClickMap::setValue( TQString v )
{
    TQRegExp rx("^http://", false );
    if ( rx.search( v ) == -1 )
    {
        displays.append( v );
    }
    else
    {
        links.append( v );
    }
}

void ClickMap::setValue( long v )
{
    if ( v == 0 )
    {
        links.clear();
        displays.clear();
    }
}

#include "clickmap.moc"
