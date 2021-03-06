/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef SENSOR_H
#define SENSOR_H
#include <tqstring.h>
#include <tqobject.h>
#include <tqobjectlist.h>
#include <tqstringlist.h>
#include <tqmap.h>
#include <tqtimer.h>

#include "sensorparams.h"

class Sensor : public TQObject
{
    Q_OBJECT
  

public:
    Sensor( int msec = 1000 );
    void start();
    virtual ~Sensor();
    void addMeter( SensorParams *s );
    SensorParams* hasMeter( Meter *meter );
    void deleteMeter( Meter *meter );
    int isEmpty() { return objList->isEmpty(); };
    virtual void setMaxValue( SensorParams *s );

private:
    int msec;
    TQTimer timer;

protected:
    TQObjectList *objList;

public slots:
    virtual void update()=0;

};

#endif // SENSOR_H
