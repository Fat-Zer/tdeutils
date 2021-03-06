/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <tqglobal.h>

#if defined __FreeBSD__
#include <sys/time.h>
#include <sys/sysctl.h>
#endif

#if defined(Q_OS_NETBSD)
#include <sys/param.h>
#include <sys/time.h>
#include <sys/sysctl.h>
#endif

#include "uptimesensor.h"

UptimeSensor::UptimeSensor( int interval ) : Sensor( interval )
{}
UptimeSensor::~UptimeSensor()
{}

void UptimeSensor::update()
{
#if defined __FreeBSD__ || defined(Q_OS_NETBSD)
      struct timeval  boottime;
      time_t          now;            /* the current time of day */

      double avenrun[3];
      time_t uptime;
      int days, hours, i, mins, secs;
      int mib[2];
      size_t size;
      char buf[256];

        /*
         * Get time of day.
         */
        (void)time(&now);

        /*
         * Determine how long system has been up.
         * (Found by looking getting "boottime" from the kernel)
         */
        mib[0] = CTL_KERN;
        mib[1] = KERN_BOOTTIME;
        size = sizeof(boottime);
        if (sysctl(mib, 2, &boottime, &size, NULL, 0) != -1 &&
            boottime.tv_sec != 0) {
                uptime = now - boottime.tv_sec;
                if (uptime > 60)
                        uptime += 30;
                days = uptime / 86400;
                uptime %= 86400;
                hours = uptime / 3600;
                uptime %= 3600;
                mins = uptime / 60;
                secs = uptime % 60;
        }
#else
    TQFile file("/proc/uptime");
    TQString line;
    if ( file.open(IO_ReadOnly | IO_Translate) )
    {
        // file opened successfully
        TQTextStream t( &file );        // use a text stream
        line = t.readLine();         // line of text excluding '\n'
        file.close();

        TQRegExp rx( "^\\d+" );
        rx.search(line);
        int uptime = rx.cap(0).toInt();
        int days = uptime / 86400;
        uptime -= days * 86400;
        int hours = uptime  / 3600;
        uptime -= hours * 3600;
        int mins = uptime / 60;
        uptime -= mins * 60;
        int secs = uptime;
#endif

        TQString format;
        SensorParams *sp;
        Meter *meter;

        TQObjectListIt it( *objList );
        while (it != 0)
        {
            sp = (SensorParams*)(*it);
            meter = sp->getMeter();
            format = sp->getParam("FORMAT");

            if (format.length() == 0 )
            {
                format = "%dd %h:%M";
            }
            format.replace( TQRegExp("%d"), TQString::number(days));
            format.replace( TQRegExp("%H"), TQString::number(hours).rightJustify(2,'0'));
            format.replace( TQRegExp("%M"), TQString::number(mins).rightJustify(2,'0'));
            format.replace( TQRegExp("%S"), TQString::number(secs).rightJustify(2,'0'));
            format.replace( TQRegExp("%h"), TQString::number(hours));
            format.replace( TQRegExp("%m"), TQString::number(mins));
            format.replace( TQRegExp("%s"), TQString::number(secs));

            meter->setValue(format);
            ++it;
        }

#if !defined __FreeBSD__ && !defined(Q_OS_NETBSD)
  }
#endif
}
