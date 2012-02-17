/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef XMMSSENSOR_H
#define XMMSSENSOR_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqdatetime.h>
#include <tqregexp.h>
#include <tqtextcodec.h>

#include "sensor.h"

class XMMSSensor :  public Sensor
{
    Q_OBJECT
  
public:
    XMMSSensor( int interval, const TQString &encoding=TQString() );
    ~XMMSSensor();
    void update();
    void setMaxValue( SensorParams *);
    bool hasXMMS() const;

private:
    TQTextCodec *codec;

    class XMMS;
    XMMS *xmms;
};


#endif // XMMSSENSOR_H
