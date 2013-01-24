/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "disksensor.h"

#include <tqfile.h>
#include <tqtextstream.h>
#include <tqstring.h>
#include <tqregexp.h>
#include <kprocess.h>
#include <kprocio.h>

DiskSensor::DiskSensor( int msec ) : Sensor( msec )
{
    connect(&ksp, TQT_SIGNAL(receivedStdout(TDEProcess *, char *, int )),
            this,TQT_SLOT(receivedStdout(TDEProcess *, char *, int )));
    connect(&ksp, TQT_SIGNAL(processExited(TDEProcess *)),
            this,TQT_SLOT(processExited( TDEProcess * )));

    // update values on startup
    ksp.clearArguments();
    ksp << "df";
    ksp.start( KProcIO::Block,KProcIO::Stdout);
    init = 1;
}
DiskSensor::~DiskSensor()
{}

long DiskSensor::getFreeSpace(TQString mntPt) const
{
    TQRegExp rx( "^\\S*\\s*\\d+\\s+\\d+\\s+(\\d+)");
    rx.search(mntMap[mntPt]);
    return rx.cap(1).toLong();
}

long DiskSensor::getUsedSpace(TQString mntPt) const
{
    TQRegExp rx( "^\\S*\\s*\\d+\\s+(\\d+)\\s+\\d+");
    rx.search(mntMap[mntPt]);
    return rx.cap(1).toLong();
}

long DiskSensor::getTotalSpace(TQString mntPt) const
{

    TQRegExp rx( "^\\S*\\s*(\\d+)\\s+\\d+\\s+\\d+");
    rx.search(mntMap[mntPt]);

    return rx.cap(1).toLong();

}

int DiskSensor::getPercentUsed(TQString mntPt) const
{
    TQRegExp rx( "\\s+(\\d+)%\\s+");
    rx.search(mntMap[mntPt]);
    return rx.cap(1).toInt();
}

int DiskSensor::getPercentFree(TQString mntPt) const
{
    return ( 100 - getPercentUsed( mntPt ) );
}

void DiskSensor::receivedStdout(TDEProcess *, char *buffer, int len )
{

    buffer[len] = 0;
    sensorResult += TQString( TQCString(buffer) );

}

void DiskSensor::processExited(TDEProcess *)
{
    TQStringList stringList = TQStringList::split('\n',sensorResult);
    sensorResult = "";
    TQStringList::Iterator it = stringList.begin();
    //TQRegExp rx( "^(/dev/).*(/\\S*)$");
    TQRegExp rx( ".*\\s+(/\\S*)$");

    while( it != stringList.end())
    {
        rx.search( *it );
        if ( !rx.cap(0).isEmpty())
        {
            mntMap[rx.cap(1)] = *it;
        }
        it++;
    }
    stringList.clear();

    TQString format;
    TQString mntPt;
    SensorParams *sp;
    Meter *meter;

    TQObjectListIt lit( *objList );
    while (lit != 0)
    {
        sp = (SensorParams*)(*lit);
        meter = sp->getMeter();
        format = sp->getParam("FORMAT");
        mntPt = sp->getParam("MOUNTPOINT");
        if (mntPt.length() == 0)
            mntPt="/";

        if (format.length() == 0 )
        {
            format = "%u";
        }
        format.replace( TQRegExp("%fp", false),TQString::number(getPercentFree(mntPt)));
        format.replace( TQRegExp("%fg",false),
                        TQString::number(getFreeSpace(mntPt)/(1024*1024)));
        format.replace( TQRegExp("%fkb",false),
                        TQString::number(getFreeSpace(mntPt)*8) );
        format.replace( TQRegExp("%fk",false),
                        TQString::number(getFreeSpace(mntPt)) );
        format.replace( TQRegExp("%f", false),TQString::number(getFreeSpace(mntPt)/1024));
        
        format.replace( TQRegExp("%up", false),TQString::number(getPercentUsed(mntPt)));
        format.replace( TQRegExp("%ug",false),
                        TQString::number(getUsedSpace(mntPt)/(1024*1024)));
        format.replace( TQRegExp("%ukb",false),
                        TQString::number(getUsedSpace(mntPt)*8) );
        format.replace( TQRegExp("%uk",false),
                        TQString::number(getUsedSpace(mntPt)) );
        format.replace( TQRegExp("%u", false),TQString::number(getUsedSpace(mntPt)/1024));

        format.replace( TQRegExp("%tg",false),
                        TQString::number(getTotalSpace(mntPt)/(1024*1024)));
        format.replace( TQRegExp("%tkb",false),
                        TQString::number(getTotalSpace(mntPt)*8));
        format.replace( TQRegExp("%tk",false),
                        TQString::number(getTotalSpace(mntPt)));
        format.replace( TQRegExp("%t", false),TQString::number(getTotalSpace(mntPt)/1024));
        meter->setValue(format);
        ++lit;
    }
    if ( init == 1 )
    {
        emit initComplete();
        init = 0;
    }
}

void DiskSensor::update()
{
    ksp.clearArguments();
    ksp << "df";
    ksp.start( KProcIO::NotifyOnExit,KProcIO::Stdout);
}

void DiskSensor::setMaxValue( SensorParams *sp )
{
    Meter *meter;
    meter = sp->getMeter();
    const TQString mntPt = sp->getParam( "MOUNTPOINT" );

    TQString f;
    f = sp->getParam("FORMAT");
    if( f == "%fp" || f == "%up" )
        meter->setMax( 100 );
    else
        meter->setMax( getTotalSpace( mntPt ) / 1024 );
}



#include "disksensor.moc"
