/***************************************************************************
 *   Copyright (C) 2003 by Wilfried Huss                                   *
 *   Wilfried.Huss@gmx.at                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef RICHTEXTLABEL_H
#define RICHTEXTLABEL_H

#include "meter.h"
#include <tqstring.h>
#include <tqsimplerichtext.h>
#include <tqpainter.h>
#include <tqfont.h>
#include <tqfontmetrics.h>
#include <tqrect.h>
#include <tqsize.h>
#include "karamba.h"

class RichTextLabel : public Meter
{
    Q_OBJECT
  public:
    RichTextLabel(karamba*);
    RichTextLabel(karamba* k, int x, int y, int w, int h);
    ~RichTextLabel();

    void setText(TQString text, bool linkUnderline = false);
    void setValue(TQString text);
    void setValue(long v);
    TQString getStringValue() { return source; };

    void setFont(TQString font);
    TQString getFont() const;
    void setFontSize(int);
    int getFontSize() const;
    void setFixedPitch(bool);
    bool getFixedPitch() const;
    void setTextProps( TextField* t );
    void setColorGroup(const TQColorGroup &colorg);
    const TQColorGroup &getColorGroup() const;
    void setWidth(int width);

    virtual bool insideActiveArea(int, int);

    virtual bool click(TQMouseEvent*);
    virtual void mUpdate(TQPainter*);

    TQString anchorAt(int, int);

  private:
    TQSimpleRichText* text;
    TQString source;
    TQFont font;
    TQColorGroup colorGrp;
    bool underlineLinks;
    TQSize originalSize;
};

#endif
