/***************************************************************************
 *   Copyright (C) 2003 by Ralph M. Churchill                              *
 *   mrchucho@yahoo.com                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef RSSSENSOR_H
#define RSSSENSOR_H

#include <sensor.h>
#include <tqstring.h>
#include <tqtextcodec.h>

/**
 *
 * Ralph M. Churchill
 **/
class RssSensor : public Sensor
{
    Q_OBJECT
  
public:
    RssSensor( const TQString &source, int interval, const TQString &format, const TQString &encoding=TQString() );

    ~RssSensor();

    void update();
private:
    TQTextCodec *codec;
    TQString source;
    TQString format;
    TQString encoding;

};

#endif // RSSSENSOR_H
