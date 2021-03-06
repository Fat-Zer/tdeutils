/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef BAR_H
#define BAR_H

#include "meter.h"
#include <tqpixmap.h>
#include <tqstring.h>
#include <tqpainter.h>

class Bar : public Meter
{
Q_OBJECT
  
public:
    Bar(karamba* k,int ix,int iy,int iw,int ih );
    ~Bar();

    void mUpdate( TQPainter * );

    virtual void setMax( long m );
    virtual void setMin( long m );

public slots:
    bool setImage( TQString imagePath );
    TQString getImage() { return imagePath; };

    void setValue( long );
    long  getValue() { return barValue; };
    void setValue( TQString );
    void recalculateValue() {setValue(barValue); };

    void setVertical( bool );
    int  getVertical() { return vertical; };

private:
    long barValue;
    long value;

    int pixmapWidth;
    int pixmapHeight;

    bool vertical; // vertical bar?

    TQString imagePath;
    TQPixmap pixmap;
}
;
#endif // BAR_H
