/* -*- C++ -*-

   This file implements the KVaio class.

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
   
   * Modified by Toan Nguyen <nguyenthetoan@gmail.com> 
   * to include support for 
   *		Fn+F2,F3,F4 (Volume)
   * 		Fn+F5,F6 (Brightness)
   * 		Fn+F1 (blankscreen)
   * 		Fn+F12 (suspend to disk)
*/

#include <kconfig.h>

#include "kvaio.h"
#include "kmilointerface.h"

#include <tqlabel.h>
#include <tqcstring.h>
#include <tqevent.h>
#include <tqdatastream.h>
#include <kmainwindow.h>
#include <klocale.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <dcopclient.h>
#include <kapplication.h>

extern "C" {
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <getopt.h>
#include <errno.h>

#include "./sonypi.h"
}


KVaio::KVaio(KMiloKVaio *parent, const char* name)
    : TQObject(parent, name),
      mDisp(0),
      mTimer (new TQTimer (this) )
{
    myparent = parent;
    
    mDriver = new KVaioDriverInterface(this);

    if(!mDriver->connectToDriver())
    {
        delete mDriver; mDriver = 0L;
        kdDebug() << "KVaio: Cannot connect to driver." << endl;
    } else {
        kdDebug() << "KVaio: Connected to SonyPI driver." << endl;
        connect(mDriver, TQT_SIGNAL(vaioEvent(int)), TQT_SLOT(slotVaioEvent(int)));
	connect (mTimer, TQT_SIGNAL (timeout ()), TQT_SLOT (slotTimeout() ) );
	mTimer->start (10000, true);
    }

    mDisp = XOpenDisplay(NULL);
    if(!mDisp)
    {
        kdDebug() << "KVaio ctor: Failed to open display. Very strange."
		  << endl;
    }

    if(!mClient.attach())
    {
       kdDebug() << "KVaio ctor: cannot attach to DCOP server." << endl;
    }

    KConfig config("kmilodrc");

    loadConfiguration(&config);

    m_mute = false;
    m_progress = 0;
    m_minVolume = 0;
    m_maxVolume = 100;
    m_volume = 50;

    m_VolumeStep = 10;
    
    m_brightness = 128;
    m_minBright = 0;
    m_maxBright = 255;
    m_BrightnessStep = 16;    

    kmixClient = new DCOPRef("kmix", "Mixer0");
    kmixWindow = new DCOPRef("kmix", "kmix-mainwindow#1");
//    retrieveVolume();
//    retrieveMute();

}

KVaio::~KVaio()
{
    kdDebug() << "KVaio dtor: shutting down." << endl;
    if(mDriver!=0)
    {
        mDriver->disconnectFromDriver();
    }
    if(mClient.isAttached())
    {
        mClient.detach();
    }
}

void KVaio::slotVaioEvent(int event)
{
    TQString text;
    TQTextStream stream(text, IO_WriteOnly);

    switch(event)
    {
	case SONYPI_EVENT_FNKEY_RELEASED:
	    break;
	case SONYPI_EVENT_FNKEY_F1:
	    blankScreen();
	    break;
	case SONYPI_EVENT_FNKEY_F2:
	    mute();
    	    break;
        case SONYPI_EVENT_FNKEY_F3:
	    VolumeDown(m_VolumeStep); 
    	    break;
        case SONYPI_EVENT_FNKEY_F4:
	    VolumeUp(m_VolumeStep); 
    	    break;
        case SONYPI_EVENT_FNKEY_F5:
	    BrightnessDown(m_BrightnessStep);
    	    break;
        case SONYPI_EVENT_FNKEY_F6:
	    BrightnessUp(m_BrightnessStep);
	    break;
	case SONYPI_EVENT_FNKEY_F12:
	    suspendToDisk();
	    break;
	case SONYPI_EVENT_MEMORYSTICK_INSERT:
	    showTextMsg( i18n ("Memory Stick inserted") );
	    break;
	case SONYPI_EVENT_MEMORYSTICK_EJECT:
	    showTextMsg( i18n ("Memory Stick ejected") );
	    break;
	case SONYPI_EVENT_BACK_PRESSED:
	    if (mShowPowerStatusOnBackButton)
	    {
		showBatteryStatus (true);
	    }
	    break;
	default:
	    stream << i18n("Unhandled event: ") << event;
	    if(mReportUnknownEvents) showTextMsg(text);
	    kdDebug() << "KVaio::slotVaioEvent: event not handled."
		      << endl;
    }
}

bool KVaio::showTextMsg(const TQString& msg)
{
    return myparent->showTextMsg(msg);    
}



bool KVaio::showProgressMsg(const TQString& msg, int value)
{
    m_progress = value;
    return myparent->showProgressMsg(msg,value);
}

void KVaio::blankScreen()
{
    bool blankonly;
    
    if (isKScreensaverAvailable()) {
    
        TQByteArray data, replyData;
        TQDataStream arg(data, IO_WriteOnly);
        TQCString replyType;

	/* Set the screensave to BlankOnly mode */
	blankonly = true;
        arg << blankonly;
	mClient.call("kdesktop","KScreensaverIface","setBlankOnly(bool)",
			data, replyType, replyData) ;

	/* Save the screen */
	if ( !mClient.call("kdesktop", "KScreensaverIface", "save()",
			data, replyType, replyData) )
        {
            kdDebug() << "KVaio::blankScreen: there was some error "
                      << "using DCOP." << endl;
        }

	/* Set the screensave to its original mode */
	blankonly = false;
        arg << blankonly;
	mClient.call("kdesktop","KScreensaverIface","setBlankOnly(bool)",
			data, replyType, replyData) ;

    }
    
}

void KVaio::suspendToDisk()
{
        TQByteArray data, replyData;
        TQDataStream arg(data, IO_WriteOnly);
        TQCString replyType;

	mClient.call("kpowersave","KPowersaveIface","do_suspendToDisk()",
			data, replyType, replyData) ;
}


bool KVaio::isKScreensaverAvailable()
{
    if(mClient.isAttached())
        {
        // kdDebug() << "KVaio::showTextMsg: attached to DCOP server." << endl;
        if(mClient.isApplicationRegistered("kdesktop"))
        {
            QCStringList objects;

            // kdDebug() << "KVaio::showTextMsg: kded is registered at dcop server."
            //           << endl;
            objects = mClient.remoteObjects("kdesktop");
            if(objects.contains("KScreensaverIface"))
            {
                // kdDebug() << "KVaio::showTextMsg: kmilod is available at kded."
                //           << endl;
                return true;
            } else {
                kdDebug() << "KVaio::isKScreensaverAvailable: "
                          << "KScreensaverIface is NOT available at kdesktop." << endl;
            return false;
            }
        } else {
            kdDebug() << "KVaio::isKScreensaverAvailable: "
                      << "kdesktop is NOT registered at dcop server." << endl;
            return false;
        }
    } else {
        kdDebug() << "KVaio::isKScreensaverAvailable: "
                  << "kdesktop is NOT registered at dcop server." << endl;
        return false;
    }
}

bool KVaio::isKMiloDAvailable()
{
    if(mClient.isAttached())
    {
        // kdDebug() << "KVaio::showTextMsg: attached to DCOP server." << endl;
        if(mClient.isApplicationRegistered("kded"))
        {
            QCStringList objects;

            // kdDebug() << "KVaio::showTextMsg: kded is registered at dcop server."
            //           << endl;
            objects = mClient.remoteObjects("kded");
            if(objects.contains("kmilod"))
            {
                // kdDebug() << "KVaio::showTextMsg: kmilod is available at kded."
                //           << endl;
                return true;
            } else {
                kdDebug() << "KVaio::isKMiloDAvailable: "
                          << "kmilod is NOT available at kded." << endl;
                return false;
            }
        } else {
            kdDebug() << "KVaio::isKMiloDAvailable: "
                      << "kded is NOT registered at dcop server." << endl;
            return false;
        }
    } else {
        kdDebug() << "KVaio::isKMiloDAvailable: "
                  << "kded is NOT registered at dcop server." << endl;
        return false;
    }
}

void KVaio::loadConfiguration(KConfig *k)
{
    k->setGroup("KVaio");

    mReportUnknownEvents =
	k->readBoolEntry("Report_Unknown_Events", false);
    mReportPowerStatus =
	k->readBoolEntry("PeriodicallyReportPowerStatus", false);
    mShowPowerStatusOnBackButton =
	k->readBoolEntry("PowerStatusOnBackButton", true);

    kdDebug() << "KVaio::loadConfiguration: " << endl
              << "       mReportUnknownEvents:      "
	      << mReportUnknownEvents << endl
	      << "       mReportPowerStatus:        "
	      << mReportPowerStatus << endl
	      << "mShowPowerStatusOnBackButton:     "
	      << mShowPowerStatusOnBackButton << endl;
}

const KVaioDriverInterface* KVaio::driver()
{
    return mDriver;
}

void KVaio::slotTimeout ()
{
    showBatteryStatus ();
    mTimer->start (4000, true);
}

bool KVaio::showBatteryStatus ( bool force )
{
    static bool acConnectedCache  = false;
    static int previousChargeCache = -1;
    bool bat1Avail = false, bat2Avail = false, acConnected = false;
    int bat1Remaining = 0, bat1Max = 0, bat2Remaining = 0, bat2Max = 0;
    bool displayBatteryMsg = false;
    bool displayACStatus = false;

    TQString text, acMsg;
    TQTextStream stream(text, IO_WriteOnly);

    // -----
    // only display on startup if mReportPowerStatus is true:
    if (mReportPowerStatus==false || !force)
    {
        return true;
    }

    // query all necessary information:
    (void) mDriver->getBatteryStatus(bat1Avail, bat1Remaining, bat1Max,
                                 bat2Avail, bat2Remaining, bat2Max,
                                 acConnected);

    int remaining;
    if ( bat1Avail || bat2Avail )
        remaining = (int)(100.0*(bat1Remaining+bat2Remaining)
                               / (bat1Max+bat2Max));
    else
        remaining = -1; // no battery available

    if (acConnectedCache != acConnected || force)
    {
	displayACStatus = true;
	acConnectedCache = acConnected;
    }

    displayBatteryMsg = ( previousChargeCache * 100 / remaining > 1000 )
	|| ( previousChargeCache * 100 / remaining > 200 && remaining < 10 )
	|| force;


    if (displayBatteryMsg)
    {
	previousChargeCache = remaining;
    }

    // ----- prepare text messages
    if (displayACStatus || displayBatteryMsg)
    {

	if (displayACStatus)
	{
	    acMsg = acConnected ? i18n ("AC Connected") : i18n ("AC Disconnected");
	}

	switch (remaining)
	{
	    case 100:
		stream << i18n("Battery is Fully Charged. ");
		break;
	    case 5:
	    case 4:
	    case 3:
	    case 2:
	    case 1:
		stream << i18n("Caution: Battery is Almost Empty (%1% remaining).").tqarg(remaining);
		break;
	    case 0:
		stream << i18n("Alert: Battery is Empty!");
		break;
            case -1:
                stream << i18n("No Battery Inserted.");
                break;
	    default:
		stream << i18n("Remaining Battery Capacity: %1%").tqarg( remaining );
	};

	// show a message if the battery status changed by more then 10% or on startup
	if (displayACStatus)
	{
	    stream << endl << acMsg;
	}

	return showTextMsg (text);
    } else {
	return true;
    }
}

void KVaio::BrightnessUp(int step)
{
	m_brightness = mDriver->brightness();
	
	m_brightness += step;
	if(m_brightness > m_maxBright) {
		m_brightness = m_maxBright;
	}
	
	mDriver->setBrightness(m_brightness);
	showProgressMsg( i18n("Brightness"), m_brightness*100/255);
}

void KVaio::BrightnessDown(int step)
{
	m_brightness = mDriver->brightness();
	
	m_brightness -= step;
	if(m_brightness < m_minBright) {
		m_brightness = m_minBright;
	}
	
	mDriver->setBrightness(m_brightness);
	showProgressMsg( i18n("Brightness"), m_brightness*100/255);
}


void KVaio::displayVolume()
{
//        _interface->displayProgress(i18n("Volume"), m_volume);

	showProgressMsg(i18n("Volume"), m_volume);
        // If we got this far, the DCOP communication with kmix works,
	// so we don't have to test the result.
        kmixClient->send("setMasterVolume", m_volume);

        // if mute then unmute
        if (m_mute)
        {
                m_mute = false;
                kmixClient->send("setMasterMute",  m_mute);
        }
}


bool KVaio::retrieveVolume() {
       bool kmix_error = false;

        DCOPReply reply = kmixClient->call("masterVolume");
        if (reply.isValid())
                m_volume = reply;
        else
                kmix_error = true;

        if (kmix_error) // maybe the error occurred because kmix wasn't running
        {
//                _interface->displayText
		showTextMsg(i18n("Starting KMix..."));
                if (kapp->startServiceByDesktopName("kmix")==0) // trying to start kmix
                {
                        // trying again
                        reply = kmixClient->call("masterVolume");
                        if (reply.isValid())
                        {
                                m_volume = reply;
                                kmix_error = false;
                                kmixWindow->send("hide");
                        }
                }
        }

        if (kmix_error)
        {
                kdDebug() << "KMilo: GenericMonitor could not access kmix/Mixer0 via dcop"
                                                        << endl;
                //_interface->displayText
		showTextMsg(i18n("It seems that KMix is not running."));

                return false;
        } else {
                return true;
        }
}

void KVaio::VolumeUp(int step)
{
        if (!retrieveVolume())
                return;

        // FIXME if the mixer doesn't support steps of the specified size it
        // could get stuck at one position
        m_volume += step;
        if (m_volume > m_maxVolume)
                m_volume = m_maxVolume;

        displayVolume();
}

void KVaio::VolumeDown(int step)
{
        if (!retrieveVolume())
                return;

        m_volume -= step;
        if (m_volume < m_minVolume)
                m_volume = m_minVolume;

        displayVolume();
}

bool KVaio::retrieveMute() 
{
        bool kmix_error = false;

        DCOPReply reply = kmixClient->call("masterMute");
        if (reply.isValid())
                m_volume = reply;
        else
                kmix_error = true;

        if (kmix_error)
        {
                // maybe the error occurred because kmix wasn't running
                //_interface->displayText
		showTextMsg(i18n("Starting KMix..."));
                if (kapp->startServiceByDesktopName("kmix")==0) // trying to start kmix
                {
                        // trying again
                        reply = kmixClient->call("masterMute");
                        if (reply.isValid())
                        {
                                m_mute = reply;
                                kmix_error = false;
                                kmixWindow->send("hide");
                        }
                }       else
                {
                        kmixWindow->send("hide");
                        kmix_error = true;
                }
        }

        if (kmix_error)
        {
                kdDebug() << "KMilo: GenericMonitor could not access kmix/Mixer0 via dcop"
                                                        << endl;
                //_interface->displayText
		showTextMsg(i18n("It seems that KMix is not running."));

                return false;
        } else {
                return true;
        }
}

void KVaio::mute() 
{
    if (!retrieveMute())
	return;

    m_mute = !m_mute;

    int newVolume;
    TQString muteText;
    if (m_mute)
    {
            m_oldVolume = m_volume;
            newVolume = 0;
            muteText = i18n("Mute on");
    } else {
            newVolume = m_oldVolume;
            muteText = i18n("Mute off");
    }

    kmixClient->send("setMasterMute", m_mute);

    //_interface->displayText(muteText);
    showTextMsg(muteText);

}


#include "kvaio.moc"
