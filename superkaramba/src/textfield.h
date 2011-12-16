/***************************************************************************
 *   Copyright (C) 2003 by Ralph M. Churchill                              *
 *   mrchucho@yahoo.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef TEXTFIELD_H
#define TEXTFIELD_H
#include <tqstring.h>
#include <tqcolor.h>
#include <tqfont.h>

/**
 *
 * Ralph M. Churchill
 **/
class TextField
{
public:
    TextField();
    TextField( const TextField& );
    ~TextField();

    TextField& operator=(const TextField& );

    void setFontSize( int );
    void setColor(TQColor clr);
    void setBGColor(TQColor clr);
    void setFont( const TQString& );
    void tqsetAlignment( int );
    void tqsetAlignment( const TQString& );
    void setFixedPitch( bool );
    void setShadow( int );

    int getFontSize() const;
    TQColor getColor() const;
    TQColor getBGColor() const;
    TQString getFont() const;
    int getAlignment() const;
    TQString getAlignmentAsString() const;
    bool getFixedPitch() const;
    int getShadow() const;
    int getLineHeight() const;

protected:
    int tqalignment;
    TQFont font;
    TQColor color;
    TQColor bgColor;
    int shadow;
    int lineHeight;

}
;
#endif // TEXTFIELD_H
