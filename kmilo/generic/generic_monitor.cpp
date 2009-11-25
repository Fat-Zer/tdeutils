// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
/*
   This file is part of the KDE project

   Copyright (c) 2003 Willi Richert <w.richert@gmx.net>
   Pretty much ripped off from :
	 George Staikos <staikos@kde.org> :)

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include <kgenericfactory.h>
#include <kdebug.h>
#include <kconfig.h>

#include <sys/types.h>
#include <unistd.h>

#include "generic_monitor.h"
#include "kmilointerface.h"
#include <qmessagebox.h>
#include <qfile.h>

#define CONFIG_FILE "kmilodrc"


using namespace KMilo;

GenericMonitor::GenericMonitor(QObject *parent, const char *name, const QStringList& args)
: Monitor(parent, name, args)
{
	_poll = false;
	m_displayType = Monitor::None;

	m_mute = false;
	m_progress = 0;
	m_minVolume = 0;
	m_maxVolume = 100;
	m_volume = 50;
}

GenericMonitor::~GenericMonitor()
{
	if(ga) {
		ga->remove("FastVolumeUp");
		ga->remove("FastVolumeDown");
		ga->remove("SlowVolumeUp");
		ga->remove("SlowVolumeDown");
		ga->remove("Mute");
		delete ga;
	}
}

bool GenericMonitor::init()
{
	KConfig config(CONFIG_FILE);
	reconfigure(&config);

	if(!m_enabled)
		return false; // exit early if we are not supposed to run

	static const ShortcutInfo shortcuts[] = {
		{ "FastVolumeUp", Qt::Key_VolumeUp, SLOT(fastVolumeUp()) },
		{ "FastVolumeDown", Qt::Key_VolumeDown, SLOT(fastVolumeDown()) },
		{ "SlowVolumeUp", Qt::CTRL+Qt::Key_VolumeUp, SLOT(slowVolumeUp()) },
		{ "SlowVolumeDown", Qt::CTRL+Qt::Key_VolumeDown, SLOT(slowVolumeDown()) },
		{ "Mute", KShortcut("XF86AudioMute"), SLOT(mute()) }
	};

	ga = new KGlobalAccel(this, "miloGenericAccel");

	ShortcutInfo si;
	int len = (int)sizeof(shortcuts)/sizeof(ShortcutInfo);
	for (int i = 0; i < len; i++) {
		si = shortcuts[i];

		ga->insert(si.name, QString::null, QString::null,
							 si.symbol, si.symbol,
							 this,
							 si.slot, false);
	}

	ga->readSettings();
	ga->updateConnections();

	kmixClient = new DCOPRef("kmix", "Mixer0");
	kmixWindow = new DCOPRef("kmix", "kmix-mainwindow#1");

	return true;
}

void GenericMonitor::reconfigure(KConfig *config)
{
	config->setGroup("generic monitor");

	m_volumeDeviceIdx = config->readNumEntry("volumeDeviceIdx", -1);
	m_muteDeviceIdx = config->readNumEntry("muteDeviceIdx", m_volumeDeviceIdx);
	m_extraDeviceIdx = config->readNumEntry("extraDeviceIdx", -1);
	m_volumeStepFast = config->readNumEntry("volumeStepFast", 10);
	m_volumeStepSlow = config->readNumEntry("volumeStepSlow", 1);
	m_enabled = config->readBoolEntry("enabled", true);
}

bool GenericMonitor::retrieveKmixDevices()
{
	if(m_volumeDeviceIdx != -1 && m_muteDeviceIdx != -1)
		return true; // both indexes already set

	DCOPReply reply = kmixClient->call("masterDeviceIndex");
	if (!reply.isValid())
	{ // maybe the error occurred because kmix wasn't running
		_interface->displayText(i18n("Starting KMix..."));
		if (kapp->startServiceByDesktopName("kmix")==0) // trying to start kmix
		{
			reply = kmixClient->call("masterDeviceIndex");
			if (reply.isValid())
				kmixWindow->send("hide");
		}
	}
		
	if (!reply.isValid())
	{
		kdDebug() << "KMilo: GenericMonitor could not access kmix/Mixer0 via dcop" 
							<< endl;
		_interface->displayText(i18n("It seems that KMix is not running."));
		
		return false;
	} else {
		if (m_volumeDeviceIdx == -1)
			m_volumeDeviceIdx = reply;
		if (m_muteDeviceIdx == -1)
			m_muteDeviceIdx = m_volumeDeviceIdx; // this is the behaviour documented in README
		return true;
	}
}

bool GenericMonitor::retrieveVolume() 
{
	bool kmix_error = false;

	if(!retrieveKmixDevices())
		return false;

	DCOPReply reply = kmixClient->call("absoluteVolume", m_volumeDeviceIdx);
	if (reply.isValid())
		m_volume = reply;
	else
		kmix_error = true;

	if (kmix_error) // maybe the error occurred because kmix wasn't running
	{
		_interface->displayText(i18n("Starting KMix..."));
		if (kapp->startServiceByDesktopName("kmix")==0) // trying to start kmix
		{
			// trying again
			reply = kmixClient->call("absoluteVolume", m_volumeDeviceIdx);
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
		_interface->displayText(i18n("It seems that KMix is not running."));
		
		return false;
	} else {
		reply = kmixClient->call("absoluteVolumeMax", m_volumeDeviceIdx);
		m_maxVolume = reply;
		reply = kmixClient->call("absoluteVolumeMin", m_volumeDeviceIdx);
		m_minVolume = reply;
		return true;
	}
}

void GenericMonitor::volumeChange(int direction, int step)
{
	if (!retrieveVolume())
		return;

	/* Following snippet of code may seem to be overcomplicated, but it works for both devices with
	 * volume grain < 100 (32 tested) and devices with volume grain > 100 (256 tested) while preserving
	 * accuracy for devices with fine grain and preserving usability for devices with rough grain. */
	int userVisibleVolume = qRound(m_volume * 100.0 / (m_maxVolume - m_minVolume));
	userVisibleVolume += direction * step; // add requested volume step
	long previousVolume = m_volume;
	m_volume = qRound(m_minVolume + userVisibleVolume * (m_maxVolume - m_minVolume) / 100.0);
	if (m_volume == previousVolume) // if the change was rounded to zero
		m_volume += direction;

	if (m_volume > m_maxVolume)
		m_volume = m_maxVolume;
	if (m_volume < m_minVolume)
		m_volume = m_minVolume;

	displayVolume();
}

void GenericMonitor::slowVolumeUp()   { volumeChange( 1, m_volumeStepSlow); }
void GenericMonitor::slowVolumeDown() { volumeChange(-1, m_volumeStepSlow); }
void GenericMonitor::fastVolumeUp()   { volumeChange( 1, m_volumeStepFast); }
void GenericMonitor::fastVolumeDown() { volumeChange(-1, m_volumeStepFast); }

void GenericMonitor::displayVolume()
{
	_interface->displayProgress(i18n("Volume"), qRound(m_volume * 100.0 / (m_maxVolume - m_minVolume)));

	// If we got this far, the DCOP communication with kmix works,
	// so we don't have to test the result.
	// Also, device indexes are set to their proper values.
	kmixClient->send("setAbsoluteVolume", m_volumeDeviceIdx, m_volume);
	if(m_extraDeviceIdx != -1)
		// for simplicity, use relative volume rather that absolute (extra precision is not needed here)
		kmixClient->send("setVolume", m_extraDeviceIdx, qRound(m_volume * 100.0 / (m_maxVolume - m_minVolume)));

	// if mute then unmute
	if (m_mute)
	{
		m_mute = false;
		kmixClient->send("setMute", m_muteDeviceIdx, m_mute);
	}
}

bool GenericMonitor::retrieveMute() 
{
	bool kmix_error = false;

	if(!retrieveKmixDevices())
		return false;

	DCOPReply reply = kmixClient->call("mute", m_muteDeviceIdx);
	if (reply.isValid())
		m_mute = reply;
	else
		kmix_error = true;

	if (kmix_error)
	{
		// maybe the error occurred because kmix wasn't running
		_interface->displayText(i18n("Starting KMix..."));
		if (kapp->startServiceByDesktopName("kmix")==0) // trying to start kmix
		{
			// trying again
			reply = kmixClient->call("mute", m_muteDeviceIdx);
			if (reply.isValid()) 
			{
				m_mute = reply;
				kmix_error = false;
				kmixWindow->send("hide");
			}
		}	else 
		{
			kmixWindow->send("hide");
			kmix_error = true;
		}
	}

	if (kmix_error)
	{
		kdDebug() << "KMilo: GenericMonitor could not access kmix/Mixer0 via dcop" 
							<< endl;
		_interface->displayText(i18n("It seems that KMix is not running."));
		
		return false;
	} else {
		return true;
	}
}

void GenericMonitor::mute() 
{
	if (!retrieveMute())
		return;

	m_mute = !m_mute;
	QString muteText;
	if (m_mute)
	{
		muteText = i18n("Mute on");
	} else {
		muteText = i18n("Mute off");
	}

	kmixClient->send("setMute", m_muteDeviceIdx, m_mute);
	if(m_extraDeviceIdx != -1)
		kmixClient->send("setMute", m_extraDeviceIdx, m_mute);

	_interface->displayText(muteText);
}

int GenericMonitor::progress() const 
{
	return m_progress;
}

Monitor::DisplayType GenericMonitor::poll() 
{
	return m_displayType;
}


K_EXPORT_COMPONENT_FACTORY(kmilo_generic, KGenericFactory<GenericMonitor>("kmilo_generic"))

#include "generic_monitor.moc"
