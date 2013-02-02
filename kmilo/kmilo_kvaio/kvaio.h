/* -*- C++ -*-

   This file declares the KVaio class.

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

#ifndef _KVAIO_H
#define _KVAIO_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqtimer.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <tdemainwindow.h>
#include "kvaiodriverinterface.h"
#include "monitor.h"
#include "kmilo_kvaio.h"

extern "C" {
#include <X11/Xlib.h>
//#include <X11/extensions/XTest.h>
}

class KVaio;
class KMiloKVaio;

/**
 * @short KVaio class
 * This class provides an interface between the Vaio driver and the
 * respective application (either the kmilod module or a single
 * application).
 * This class provides the application of the events created by the
 * KVaioDriverInterface class.
 * @author Mirko Boehm <mirko@hackerbuero.org>
 * @version 0.2
 */
class KVaio : public TQObject
{
    Q_OBJECT
  
public:
//    KVaio(TQObject *parent = 0, const char *name =0);
    KVaio(KMiloKVaio *parent = 0, const char *name =0);

    virtual ~KVaio();
    const KVaioDriverInterface *driver();
    void loadConfiguration(TDEConfig *);
    const int progress() { return m_progress; }

protected:
    KVaioDriverInterface *mDriver;
    Display *mDisp;
    DCOPClient mClient;
    bool isKScreensaverAvailable();
    bool isKMiloDAvailable();
    bool showTextMsg(const TQString& msg);
    bool showProgressMsg(const TQString& msg, int progress);
    bool showBatteryStatus ( bool force = false);
protected slots:
    void slotVaioEvent(int);
    void slotTimeout();
public slots:
    void blankScreen();
    void suspendToDisk();
    void BrightnessUp(int step);
    void BrightnessDown(int step);
    void VolumeUp(int step);
    void VolumeDown(int step);
    void mute();    
private:
    bool retrieveMute();
    bool retrieveVolume();
    void displayVolume();

    KMiloKVaio *myparent;

    DCOPRef *kmixClient, *kmixWindow;

    int m_progress;
    int m_volume, m_oldVolume, m_brightness;
    bool m_mute;

    int m_maxVolume, m_minVolume;
    int m_maxBright, m_minBright;

    int  m_VolumeStep;
    int  m_BrightnessStep;

    bool mReportUnknownEvents;
    bool mReportPowerStatus;
    bool mShowPowerStatusOnBackButton;
    TQTimer *mTimer;
};

#endif // _KVAIO_H
