/*
   This file is part of the KDE project

   Copyright (c) 2004 Barry O'Donovan (barry.odonovan@kdemail.net)

   The code in this module is heavily based on:
    - the Asus Laptop plug-in for KMilo
      Copyright (c) 2004 Chris Howells (howells@kde.org)
    - the Thinkpad plug-in
      Copyright (c) 2004 Jonathan Riddell (jr@jriddell.org)
    - the i8k kernel module and utilities (http://www.debian.org/~dz/i8k/)
      Copyright (c) 2001-2003  Massimo Dal Zotto <dz@debian.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <kgenericfactory.h>

#include "kmilointerface.h"

#include <fcntl.h>
#include <sys/ioctl.h>

#include "i8k.h"

/* For compatibilty with i8k kernel driver 1.1 */
#define I8K_VOL_MUTE_1_1 3

#include "delli8k.h"

namespace KMilo
{

DellI8kMonitor::DellI8kMonitor( QObject* parent, const char* name, const QStringList& args )
    : Monitor( parent, name, args )
{
}

DellI8kMonitor::~DellI8kMonitor()
{
}

bool DellI8kMonitor::init()
{
    kdDebug() << "KMilo: loading Dell I8k KMilo plugin" << endl;

    // Ensure the Dell I8k kernel module is installed/loaded
    if( ( m_fd = open( I8K_PROC, O_RDONLY ) ) < 0 )
    {
        kdDebug() << "KMilo: DellI8kMonitor: unable to open i8k proc file: " << I8K_PROC
                << " in init()" << endl;
        return false;
    }
	else
	{
        kmixClient = new DCOPRef( "kmix", "Mixer0" );
        kmixWindow = new DCOPRef( "kmix", "kmix-mainwindow#1" );
        retrieveVolume();
        retrieveMute();
	}

	return true;
}

Monitor::DisplayType DellI8kMonitor::poll()
{
    Monitor::DisplayType pollResult = None;

    if( ( m_status = fn_status( m_fd ) ) < 0 )
    {
        kdDebug() << "KMilo: DellI8kMonitor: unable to read fn status in poll()" << endl;
        return pollResult;
    }
    else
    {
        switch( m_status )
        {
            case I8K_VOL_UP:
                pollResult = Volume;
                retrieveVolume();
                setVolume( m_volume + DELLI8K_VOLUME_STEPSIZE );
                m_progress = m_volume;
                break;
            case I8K_VOL_DOWN:
                pollResult = Volume;
                retrieveVolume();
                setVolume( m_volume - DELLI8K_VOLUME_STEPSIZE );
                m_progress = m_volume;
                break;
            case I8K_VOL_MUTE:
            case I8K_VOL_MUTE_1_1:
                pollResult = None;
                retrieveMute();
                setMute( !m_mute );
                if( m_mute )
                    _interface->displayText( i18n( "Mute On" ) );
                else
                    _interface->displayText( i18n( "Mute Off" ) );
                break;
            case 0:
                break;
            default:
                kdDebug() << "KMile: DellI8kMonitor: invalid button status: " << m_status
                        << " in poll()" << endl;
                break;
        }
    }

	return pollResult;
}

int DellI8kMonitor::progress() const
{
    return m_progress;
}

bool DellI8kMonitor::retrieveVolume()
{
    bool kmix_error = false;

    DCOPReply reply = kmixClient->call( "masterVolume");

    if( reply.isValid() )
    {
        m_volume = reply;
    }
    else
    {
        kmix_error = true;
    }

    if (kmix_error)
    {
        // maybe the error occurred because kmix wasn't running
        if( kapp->startServiceByDesktopName( "kmix" ) == 0 )            // trying to start kmix
        {
            // trying again
            reply = kmixClient->call( "masterVolume");
            if( reply.isValid() )
            {
                m_volume = reply;
                kmix_error = false;
                kmixWindow->send( "hide" );
            }
        }
    }

    if (kmix_error)
    {
        kdError() << "KMilo: DellI8kMonitor could not access kmix/Mixer0 via dcop" << endl;
        return false;
    }

    return true;
}

void DellI8kMonitor::setVolume(int volume)
{
    if( !retrieveVolume() )
    {
        return;
    }

    if( volume > 100 )
    {
        m_volume = 100;
    }
    else if( volume < 0 )
    {
        m_volume = 0;
    }
    else
    {
        m_volume = volume;
    }

    kmixClient->send("setMasterVolume", m_volume);

    m_progress = m_volume;
}

bool DellI8kMonitor::retrieveMute()
{
    bool kmix_error = false;

    DCOPReply reply = kmixClient->call( "masterMute" );

    if( reply.isValid() )
    {
        m_mute = reply;
    }
    else
    {
        kmix_error = true;
    }

    if (kmix_error)
    {
        // maybe the error occurred because kmix wasn't running
        if( kapp->startServiceByDesktopName( "kmix" ) == 0 )            // trying to start kmix
        {
            // trying again
            reply = kmixClient->call( "masterVolume" );
            if( reply.isValid() )
            {
                m_volume = reply;
                kmix_error = false;
                kmixWindow->send( "hide" );
            }
        }
    }

    if (kmix_error)
    {
        kdError() << "KMilo: DellI8kMonitor could not access kmix/Mixer0 via dcop in isMute()" << endl;
        return false;
    }

    return true;
}

void DellI8kMonitor::setMute( bool b )
{
    m_mute = b;
    kmixClient->send( "setMasterMute", m_mute );
}

int DellI8kMonitor::fn_status( int fd )
{
    int args[1];
    int rc;

    if( ( rc = ioctl( fd, I8K_FN_STATUS, &args ) ) < 0 )
    {
        kdError() << "KMilo: DellI8kMonitor: ioctl failed in fn_status()" << endl;
        return rc;
    }

    return args[0];
}

} //close namespace

K_EXPORT_COMPONENT_FACTORY( kmilo_delli8k, KGenericFactory<KMilo::DellI8kMonitor>( "kmilo_delli8k" ) )
