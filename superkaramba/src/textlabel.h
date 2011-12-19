/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef TEXTLABEL_H
#define TEXTLABEL_H
#include "meter.h"
#include <tqstring.h>
#include <tqpainter.h>
#include <tqcolor.h>
#include <tqfont.h>
#include <tqfontmetrics.h>
#include <tqstringlist.h>
#include <tqrect.h>

#include "textfield.h"

class TextLabel : public Meter
{
Q_OBJECT
  TQ_OBJECT
public:
    enum ScrollType { ScrollNone, ScrollNormal,
                      ScrollBackAndForth, ScrollOnePass };

    TextLabel(karamba *k, int x,int y,int w,int h);
    TextLabel(karamba *k);
    ~TextLabel();

    void setTextProps( TextField* );
    void setValue( TQString );
    void setValue( long );
    //virtual TQString getStringValue() const { return value.join("\n"); };
    TQString getStringValue() const { return value.join("\n"); };
    void setFontSize( int );
    void setBGColor(TQColor clr);
    void setFont( TQString );
    void setAlignment( TQString );
    void setFixedPitch( bool );
    void setShadow( int );
    void mUpdate( TQPainter * );

    virtual void show();
    virtual void hide();
    int getFontSize() const;
    TQColor getBGColor() const;
    TQString getFont() const;
    TQString getAlignment() const;
    bool getFixedPitch() const;
    int getShadow() const;
    void setScroll(ScrollType type, TQPoint speed, int gap, int pause);
    void setScroll(char* type, TQPoint speed, int gap, int pause);

    void attachClickArea(TQString leftMouseButton, TQString middleMouseButton,
                         TQString rightMouseButton);

    virtual bool click(TQMouseEvent*);

private:
    int tqalignment;
    int clip;
    TQStringList value;
    TQFont font;
    TQColor bgColor;
    int lineHeight;
    TQSize textSize;
    int shadow;
    TextField text;
    TQPoint scrollSpeed;
    TQPoint scrollPos;
    int scrollGap;
    int scrollPause;
    int pauseCounter;
    ScrollType scrollType;

    int drawText(TQPainter *p, int x, int y, int width, int height,
                 TQString text);
    bool calculateScrollCoords(TQRect meterRect, TQRect &textRect,
                               TQPoint &next, int &x, int &y);
    void calculateTextSize();
};

#endif // TEXTLABEL_H
