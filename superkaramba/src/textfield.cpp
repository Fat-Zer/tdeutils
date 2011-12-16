/***************************************************************************
 *   Copyright (C) 2003 by Ralph M. Churchill                              *
 *   mrchucho@yahoo.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include "textfield.h"
#include <tqfontmetrics.h>
#include <kdebug.h>

TextField::TextField( )
{
  setFontSize(12);
  setColor(TQColor(192, 192, 192));
  setBGColor(TQColor(0, 0, 0));
  setFont("Helvetica");
  tqsetAlignment(TQt::AlignLeft);
  setFixedPitch(false);
  setShadow(0);
}

TextField::~TextField()
{
}

TextField::TextField( const TextField& def )
{
    setFontSize( def.getFontSize() );

    setColor(def.getColor());
    setBGColor(def.getBGColor());

    setFont( def.getFont() );
    tqsetAlignment( def.getAlignment() );
    setFixedPitch( def.getFixedPitch() );
    setShadow( def.getShadow() );
}

TextField& TextField::operator=(const TextField& rhs)
{
    if( this == &rhs)
        return *this;

    setFontSize( rhs.getFontSize() );

    setColor(rhs.getColor());
    setBGColor(rhs.getBGColor());

    setFont( rhs.getFont() );
    tqsetAlignment( rhs.getAlignment() );
    setFixedPitch( rhs.getFixedPitch() );
    setShadow( rhs.getShadow() );

    return *this;
}

void TextField::setColor(TQColor clr)
{
  color = clr;
}

TQColor TextField::getColor() const
{
    return color;
}

void TextField::setBGColor(TQColor clr)
{
    bgColor = clr;
}

TQColor TextField::getBGColor() const
{
    return bgColor;
}


void TextField::setFont(const TQString &f)
{
    font.setFamily(f);
    lineHeight = TQFontMetrics(font).height();
}


TQString TextField::getFont() const
{
    return font.family();
}

void TextField::setFontSize(int size)
{
    font.setPointSize(size);
    lineHeight = TQFontMetrics(font).height();
}

int TextField::getFontSize() const
{
    return font.pointSize();
}

void TextField::tqsetAlignment( const TQString &align )
{
    TQString a = align.upper();
    if( a == "LEFT" || a.isEmpty() )
        tqalignment = TQt::AlignLeft;
    if( a == "RIGHT" )
        tqalignment = TQt::AlignRight;
    if( a == "CENTER" )
        tqalignment = TQt::AlignHCenter;
}

void TextField::tqsetAlignment( int af )
{
    tqalignment = af;
}

int TextField::getAlignment() const
{
    return tqalignment;
}

TQString TextField::getAlignmentAsString() const
{
    if( tqalignment == TQt::AlignHCenter )
        return "CENTER";
    else if( tqalignment == TQt::AlignRight )
        return "RIGHT";
    else
        return "LEFT";
}

void TextField::setFixedPitch( bool fp)
{
    font.setFixedPitch( fp );
}

bool TextField::getFixedPitch() const
{
    return font.fixedPitch();
}

void TextField::setShadow ( int s )
{
    shadow = s;
}

int TextField::getShadow() const
{
    return shadow;
}

int TextField::getLineHeight() const
{
    return lineHeight;
}
