/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "programsensor.h"

#include <tqstringlist.h>
ProgramSensor::ProgramSensor(const TQString &progName, int interval, TQString encoding )
        : Sensor( interval )
{
     if( !encoding.isEmpty())
    {
        codec = TQTextCodec::codecForName( encoding.ascii() );
        if ( codec == 0)
            codec = TQTextCodec::codecForLocale();
    }
    else
        codec = TQTextCodec::codecForLocale();


    programName = progName;
    //update();
    connect(&ksp, TQT_SIGNAL(receivedStdout(TDEProcess *, char *, int )),
            this,TQT_SLOT(receivedStdout(TDEProcess *, char *, int )));
    connect(&ksp, TQT_SIGNAL(processExited(TDEProcess *)),
            this,TQT_SLOT(processExited( TDEProcess * )));
}

ProgramSensor::~ProgramSensor()
{}

void ProgramSensor::receivedStdout(TDEProcess *, char *buffer, int len)
{
    buffer[len] = 0;
    sensorResult += codec->toUnicode( TQCString(buffer) );
}

void ProgramSensor::processExited(TDEProcess *)
{
    int lineNbr;
    SensorParams *sp;
    Meter *meter;
    TQValueVector<TQString> lines;
    TQStringList stringList = TQStringList::split('\n',sensorResult,true);
    TQStringList::ConstIterator end( stringList.end() );
    for ( TQStringList::ConstIterator it = stringList.begin(); it != end; ++it )
    {
        lines.push_back(*it);
    }

    int count = (int) lines.size();
    TQObjectListIt it( *objList );
    while (it != 0)
    {
        sp = (SensorParams*)(*it);
        meter = sp->getMeter();
        if( meter != 0)
        {
            lineNbr = (sp->getParam("LINE")).toInt();
            if ( lineNbr >= 1  && lineNbr <=  (int) count )
            {
                meter->setValue(lines[lineNbr-1]);
            }
            if ( -lineNbr >= 1 && -lineNbr <= (int) count )
            {
                meter->setValue(lines[count+lineNbr]);
            }
            if (lineNbr == 0)
            {
                meter->setValue(sensorResult);
            }
        }
        ++it;
    }

    sensorResult = "";
}

void ProgramSensor::update()
{
    ksp.clearArguments();
    ksp << programName;


    ksp.start( KProcIO::NotifyOnExit,KProcIO::Stdout);
}

#include "programsensor.moc"
