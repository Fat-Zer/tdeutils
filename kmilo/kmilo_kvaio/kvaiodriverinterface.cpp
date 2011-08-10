/* -*- C++ -*-

   This file implements the KVaioDriverInterface class.
   It provides an event-oriented wrapper for the kernel sonypi driver.

   $ Author: Mirko Boehm $
   $ Copyright: (C) 1996-2003, Mirko Boehm $
   $ Contact: mirko@kde.org
         http://www.kde.org
         http://www.hackerbuero.org $
   $ License: LGPL with the following explicit clarification:
         This code may be linked against any version of the TQt toolkit
         from Troll Tech, Norway. $

   $Id$

   * Portions of this code are
   * (C) 2001-2002 Stelian Pop <stelian@popies.net> and
   * (C) 2001-2002 Alcove <www.alcove.com>.
   * Thanks to Stelian for the implementation of the sonypi driver.
*/

#include "kvaiodriverinterface.h"

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <X11/Xlib.h>
//#include <X11/extensions/XTest.h>

#include "./sonypi.h"
}

#include <tqsocketnotifier.h>
#include <kdebug.h>

// Taken from Stelian Pop's spicctrl utility:

/* Have our own definition of ioctls... */

/* get/set brightness */
#define SONYPI_IOCGBRT		_IOR('v', 0, __u8)
#define SONYPI_IOCSBRT		_IOW('v', 0, __u8)

/* get battery full capacity/remaining capacity */
#define SONYPI_IOCGBAT1CAP	_IOR('v', 2, __u16)
#define SONYPI_IOCGBAT1REM	_IOR('v', 3, __u16)
#define SONYPI_IOCGBAT2CAP	_IOR('v', 4, __u16)
#define SONYPI_IOCGBAT2REM	_IOR('v', 5, __u16)

/* get battery flags: battery1/battery2/ac adapter present */
#define SONYPI_BFLAGS_B1	0x01
#define SONYPI_BFLAGS_B2	0x02
#define SONYPI_BFLAGS_AC	0x04
#define SONYPI_IOCGBATFLAGS	_IOR('v', 7, __u8)

/* get/set bluetooth subsystem state on/off */
#define SONYPI_IOCGBLUE         _IOR('v', 8, __u8)
#define SONYPI_IOCSBLUE         _IOW('v', 9, __u8)

KVaioDriverInterface::KVaioDriverInterface(TQObject *parent)
    : TQObject(parent),
      mFd(0),
      mNotifier(0)
{
}

bool KVaioDriverInterface::connectToDriver(bool listen)
{
    const char* DriverFile = "/dev/sonypi";

    mFd = open(DriverFile, O_RDONLY);

    // mFd = open(DriverFile, O_RDWR);

    if(mFd == -1)
    {
        kdDebug() << "KVaio: Failed to open /dev/sonypi: "
                  << strerror(errno) << "." << endl;
        return false;
    }

    fcntl(mFd, F_SETFL, fcntl(mFd, F_GETFL) | O_ASYNC);

    if(listen)
        {
            mNotifier = new TQSocketNotifier(mFd,  TQSocketNotifier::Read, this);
            connect(mNotifier, TQT_SIGNAL(activated(int)), TQT_SLOT(socketActivated(int)));
        }
    return true;
}

void KVaioDriverInterface::disconnectFromDriver()
{
    delete mNotifier;
    mNotifier = 0;

    if(mFd!=0)
    {
        close(mFd);
        mFd = 0;
    }
}

void KVaioDriverInterface::socketActivated(int)
{
    unsigned char events[8];
    int count;

    do {
        count = read(mFd, &events, sizeof(events));
        for(int index = 0; index<count; index++)
        {
            emit(vaioEvent(events[index]));
        }
    } while(count==sizeof(events));
}

int KVaioDriverInterface::brightness()
{
    unsigned char value = 0;
    int result=0;

    result = ioctl(mFd, SONYPI_IOCGBRT, &value);

    if (result >= 0)
    {
        return value;
    } else {
        return -1;
    }

}

void KVaioDriverInterface::setBrightness(int value)
{
    static unsigned char cache; // to avoid unnecessary updates
    int result;
    unsigned char value8 = 0;

    if(value<0) value=0;
    if(value>255) value=255;

    value8 = __u8(value);

    if(value8 != cache)
    {
        result = ioctl(mFd, SONYPI_IOCSBRT, &value8);

        if(result<0)
        {
            kdDebug() << "KVaioDriverInterface::setBrightness: ioctl failed."
                      << endl;
        }
        cache = value8;
    }
}

bool KVaioDriverInterface::getBatterytqStatus(
    bool& bat1Avail, int& bat1Remaining, int& bat1Max,
    bool& bat2Avail, int& bat2Remaining, int& bat2Max,
    bool& acConnected)
{
    unsigned char batFlags = 0;

    bool retval  = true;

    if(ioctl(mFd, SONYPI_IOCGBATFLAGS, &batFlags)<0)
    {
        retval = false;
    } else {
        __u16 rem1 = 0, rem2 = 0, cap1 = 0, cap2 = 0;

        bat1Avail = batFlags & SONYPI_BFLAGS_B1;
        bat2Avail = batFlags & SONYPI_BFLAGS_B2;
        acConnected = batFlags & SONYPI_BFLAGS_AC;

//         kdDebug() << "KVaioDriverInterface::getBatterytqStatus: battery 1:  "
//                   << (bat1Avail ? "available" : "not available") << endl
//                   << "                                        battery 2:  "
//                   << (bat2Avail ? "available" : "not available") << endl
//                   << "                                        AC adapter: "
//                   << (acConnected ? "connected" : "connected")
//                   << "." << endl;

        if(bat1Avail
           && ioctl(mFd, SONYPI_IOCGBAT1CAP, &cap1) >= 0
           && ioctl(mFd, SONYPI_IOCGBAT1REM, &rem1) >= 0)
        {
            bat1Max = cap1;
            bat1Remaining = rem1;
        } else {
            bat1Remaining = 0;
            bat1Max = 0;
            retval = false;
        }

        if(bat2Avail
           && ioctl(mFd, SONYPI_IOCGBAT2CAP, &cap2) >= 0
           && ioctl(mFd, SONYPI_IOCGBAT2REM, &rem2) >= 0)
        {
            bat2Max = cap2;
            bat2Remaining = rem2;
        } else {
            bat2Remaining = 0;
            bat2Max = 0;
            retval = false;
        }
    }

    return retval;
}

#include "kvaiodriverinterface.moc"

