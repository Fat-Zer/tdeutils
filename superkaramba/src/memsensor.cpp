/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include "memsensor.h"
#include <tqfile.h>
#include <tqglobal.h>
#include <tqtextstream.h>
#include <tqstring.h>
#include <tqregexp.h>

#ifdef Q_OS_FREEBSD
#include <sys/time.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/resource.h>
#include <unistd.h>
#include <kvm.h>
#include <sys/file.h>
#include <osreldate.h>
#endif

#if defined(Q_OS_NETBSD)
#include <sys/param.h>
#include <sys/sysctl.h>
#include <sys/sched.h>
#include <sys/swap.h>
#endif

#include <kprocio.h>

#if defined Q_OS_FREEBSD || defined(Q_OS_NETBSD)
/* define pagetok in terms of pageshift */
#define pagetok(size) ((size) << pageshift)
#endif

MemSensor::MemSensor(int msec) : Sensor(msec)
{
#if defined Q_OS_FREEBSD || defined(Q_OS_NETBSD)
   /* get the page size with "getpagesize" and calculate pageshift from it */
    int pagesize = getpagesize();
    pageshift = 0;
    while (pagesize > 1)
    {
        pageshift++;
        pagesize >>= 1;
    }

    /* we only need the amount of log(2)1024 for our conversion */
    pageshift -= 10;
# if defined(Q_OS_FREEBSD) && defined(__FreeBSD_version) && __FreeBSD_version >= 500018
    kd = kvm_open("/dev/null", "/dev/null", "/dev/null", O_RDONLY, "kvm_open");
# elif defined Q_OS_FREEBSD
    connect(&ksp, TQT_SIGNAL(receivedStdout(TDEProcess *, char *, int )),
            this,TQT_SLOT(receivedStdout(TDEProcess *, char *, int )));
    connect(&ksp, TQT_SIGNAL(processExited(TDEProcess *)),
            this,TQT_SLOT(processExited( TDEProcess * )));

    swapTotal = swapUsed = 0;

    MaxSet = false;

    readValues();
# endif
#else
    readValues();
#endif
}

MemSensor::~MemSensor()
{}

#ifdef Q_OS_FREEBSD
void MemSensor::receivedStdout(TDEProcess *, char *buffer, int len )
{
    buffer[len] = 0;
    sensorResult += TQString( TQCString(buffer) );
}
#else
void MemSensor::receivedStdout(TDEProcess *, char *, int)
{
}
#endif

void MemSensor::processExited(TDEProcess *)
{
#ifdef Q_OS_FREEBSD
    TQStringList stringList = TQStringList::split('\n',sensorResult);
    sensorResult = "";
    TQStringList itemsList = TQStringList::split(' ', stringList[1]);

    swapUsed = itemsList[2].toInt();
    swapTotal = itemsList[1].toInt();
#endif
}

int MemSensor::getMemTotal()
{
#if defined Q_OS_FREEBSD || defined(Q_OS_NETBSD)
    static int mem = 0;
    size_t size = sizeof(mem);

    sysctlbyname("hw.physmem", &mem, &size, NULL, 0);
    return (mem / 1024);
#else
    TQRegExp rx( "MemTotal:\\s*(\\d+)" );
    rx.search( meminfo );
    return ( rx.cap(1).toInt() );
#endif
}

int MemSensor::getMemFree()
{
#ifdef Q_OS_FREEBSD
    static int mem = 0;
    size_t size = sizeof(mem);

    sysctlbyname("vm.stats.vm.v_free_count", &mem, &size, NULL, 0);
    return (pagetok(mem));
#elif defined(Q_OS_NETBSD)
    struct uvmexp_sysctl uvmexp;
    int mib[2];
    size_t ssize;
    mib[0] = CTL_VM;
    mib[1] = VM_UVMEXP2;
    ssize = sizeof(uvmexp);
    sysctl(mib,2,&uvmexp,&ssize,NULL,0);
    return pagetok(uvmexp.free);
#else
    TQRegExp rx( "MemFree:\\s*(\\d+)" );
    rx.search( meminfo );
    return ( rx.cap(1).toInt() );
#endif
}

int MemSensor::getBuffers()
{
#ifdef Q_OS_FREEBSD
    static int mem = 0;
    size_t size = sizeof(mem);

    sysctlbyname("vfs.bufspace", &mem, &size, NULL, 0);
    return (mem / 1024);
#elif defined(Q_OS_NETBSD)
    static int buf_mem = 0;
    size_t size = sizeof(buf_mem);

    sysctlbyname("vm.bufmem", &buf_mem, &size, NULL, 0);
    return (buf_mem / 1024);
#else
    TQRegExp rx( "Buffers:\\s*(\\d+)" );
    rx.search( meminfo );
    return ( rx.cap(1).toInt() );
#endif
}

int MemSensor::getCached()
{
#ifdef Q_OS_FREEBSD
    static int mem = 0;
    size_t size = sizeof(mem);

    sysctlbyname("vm.stats.vm.v_cache_count", &mem, &size, NULL, 0);
    return (pagetok(mem));
#elif defined(Q_OS_NETBSD)
    return 0;
#else
    TQRegExp rx1( "Cached:\\s*(\\d+)" );
    TQRegExp rx2( "SwapCached:\\s*(\\d+)" );
    rx1.search( meminfo );
    rx2.search( meminfo );
    return ( rx1.cap(1).toInt() + rx2.cap(1).toInt() );
#endif
}


int MemSensor::getSwapTotal()
{
#ifdef Q_OS_FREEBSD
# if defined(__FreeBSD_version) && __FreeBSD_version >= 500018
    int n = -1;
    int pagesize = getpagesize();
    int retavail = 0;

    if (kd != NULL)
        n = kvm_getswapinfo(kd, &swapinfo, 1, 0);

    if (n < 0 || swapinfo.ksw_total == 0)
        return(0);

    retavail = swapinfo.ksw_total * pagesize / 1024;

    return(retavail);
#else
    return(swapTotal);
# endif
#elif defined(Q_OS_NETBSD)
    struct uvmexp_sysctl uvmexp;
    int STotal = 0;
    int pagesize = 1;
    int mib[2];
    size_t ssize;
    mib[0] = CTL_VM;
    mib[1] = VM_UVMEXP;
    ssize = sizeof(uvmexp);

    if (sysctl(mib,2,&uvmexp,&ssize,NULL,0) != -1) {
	pagesize = uvmexp.pagesize;
	STotal = (pagesize*uvmexp.swpages) >> 10;
    }
    return STotal;
#else
    TQRegExp rx( "SwapTotal:\\s*(\\d+)" );
    rx.search( meminfo );
    return ( rx.cap(1).toInt() );
#endif
}

int MemSensor::getSwapFree()
{
#ifdef Q_OS_FREEBSD
# if defined(__FreeBSD_version) && __FreeBSD_version >= 500018
    int n = -1;
    int pagesize = getpagesize();
    int retfree = 0;

    if (kd != NULL)
        n = kvm_getswapinfo(kd, &swapinfo, 1, 0);
    if (n < 0 || swapinfo.ksw_total == 0)
        return(0);

    retfree = (swapinfo.ksw_total - swapinfo.ksw_used) * pagesize / 1024;

    return(retfree);
# else
    return(swapTotal - swapUsed);
# endif
#elif defined(Q_OS_NETBSD)
    struct uvmexp_sysctl uvmexp;
    int STotal = 0;
    int SFree = 0;
    int SUsed = 0;
    int pagesize = 1;
    int mib[2];
    size_t ssize;
    mib[0] = CTL_VM;
    mib[1] = VM_UVMEXP;
    ssize = sizeof(uvmexp);

    if (sysctl(mib,2,&uvmexp,&ssize,NULL,0) != -1) {
	pagesize = uvmexp.pagesize;
	STotal = (pagesize*uvmexp.swpages) >> 10;
	SUsed = (pagesize*uvmexp.swpginuse) >> 10;
	SFree = STotal - SUsed;
    }
    return SFree;
#else
    TQRegExp rx( "SwapFree:\\s*(\\d+)" );
    rx.search( meminfo );
    return ( rx.cap(1).toInt() );
#endif
}

void MemSensor::readValues()
{
#if defined Q_OS_FREEBSD || defined(Q_OS_NETBSD)
# if defined(Q_OS_FREEBSD) && !(defined(__FreeBSD_version) && __FreeBSD_version >= 500018)
    ksp.clearArguments();
    ksp << "swapinfo";
    ksp.start( TDEProcess::NotifyOnExit,KProcIO::Stdout);
# endif
#else
    TQFile file("/proc/meminfo");
    TQString line;
    if ( file.open(IO_ReadOnly | IO_Translate) )
    {
        TQTextStream t( &file );        // use a text stream
        meminfo = t.read();
        file.close();
    }
#endif
}

void MemSensor::update()
{
    readValues();
    TQString format;
    SensorParams *sp;
    Meter *meter;
    TQObjectListIt it( *objList );
#if defined(Q_OS_FREEBSD) && !(defined(__FreeBSD_version) && __FreeBSD_version >= 500018)
    bool set = false;
#endif
    int totalMem = getMemTotal();
    int usedMem = totalMem - getMemFree();
    int usedMemNoBuffers =  usedMem - getBuffers() - getCached();
    int totalSwap = getSwapTotal();
    int usedSwap = totalSwap - getSwapFree();

    while (it != 0)
    {
        sp = (SensorParams*)(*it);
#if defined(Q_OS_FREEBSD) && !(defined(__FreeBSD_version) && __FreeBSD_version >= 500018)
        if ( (!MaxSet) && (totalSwap > 0) ) {
           setMaxValue(sp);
           bool set = true;
        }
#endif
        meter = sp->getMeter();
        format = sp->getParam("FORMAT");
        if (format.length() == 0 )
        {
            format = "%um";
        }

        format.replace( TQRegExp("%fmb", false), TQString::number( (int)(( totalMem - usedMemNoBuffers)/1024.0+0.5)));
        format.replace( TQRegExp("%fm", false), TQString::number( (int)( ( totalMem - usedMem  )/1024.0+0.5) ));

        format.replace( TQRegExp("%umb", false), TQString::number( (int)((usedMemNoBuffers)/1024.0+0.5)));
        format.replace( TQRegExp("%um", false), TQString::number( (int)((usedMem)/1024.0+0.5 )));

        format.replace( TQRegExp("%tm", false), TQString::number( (int)( (totalMem)/1024.0+0.5)));

        format.replace( TQRegExp("%fs", false), TQString::number( (int)((totalSwap - usedSwap)/1024.0+0.5)));
        format.replace( TQRegExp("%us", false), TQString::number( (int)(usedSwap/1024.0+0.5)));
        format.replace( TQRegExp("%ts", false), TQString::number( (int)(totalSwap/1024.0+0.5)));

        meter->setValue(format);
        ++it;
    }
#if defined(Q_OS_FREEBSD) && !(defined(__FreeBSD_version) && __FreeBSD_version >= 500018)
    if (set)
        MaxSet = true;
#endif
}

void MemSensor::setMaxValue( SensorParams *sp )
{
    Meter *meter;
    meter = sp->getMeter();
    TQString f;
    f = sp->getParam("FORMAT");

    if (f.length() == 0 )
    {
        f = "%um";
    }
    if( f=="%fm" || f== "%um" || f=="%fmb" || f=="%umb" )
        meter->setMax( getMemTotal() / 1024 );
    if( f=="%fs" || f== "%us" )
        meter->setMax( getSwapTotal() / 1024 );
}

#include "memsensor.moc"
