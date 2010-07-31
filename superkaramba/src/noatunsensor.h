/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef NOATUNSENSOR_H
#define NOATUNSENSOR_H

#include "sensor.h"
#include <dcopclient.h>
#include <tqregexp.h>
#include <tqcstring.h>
#include <tqdatastream.h>
#include <tqstringlist.h>

/**
@author Hans Karlsson
*/
class NoatunSensor : public Sensor
{
public:
    NoatunSensor( int interval, DCOPClient *client);

    ~NoatunSensor();

    void update();
    void setMaxValue( SensorParams *);



private:
    TQCString noatunID;

    bool isRunning();
    TQString getTitle();
    int getTime();
    int getLength();


    DCOPClient *client;
    TQCString appId;
};

#endif
