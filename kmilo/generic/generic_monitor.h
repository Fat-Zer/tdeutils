// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
/*
   This file is part of the KDE project

   Copyright (c) 2003 Willi Richert <w.richert@gmx.net>
	 Pretty much ripped of from :
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
#ifndef _GENERICMONITOR_H_
#define _GENERICMONITOR_H_

#include <tdemainwindow.h>
#include <kglobalaccel.h>
#include <dcopref.h>
#include <kapplication.h>
#include <tdeconfig.h>

#include "kmilod.h"
#include "monitor.h"

namespace KMilo {

// now the key data (from kkeyserver_x11.h and $TQTDIR/include/tqnamespace.h)
struct ShortcutInfo
{
	const char* name;
	uint symbol;
	const char *slot;
};


class GenericMonitor : public Monitor
{
	Q_OBJECT
  

public:
	GenericMonitor(TQObject *parent, const char *name, const TQStringList&);
	virtual ~GenericMonitor();

	virtual bool init();
	virtual int progress() const;
	virtual DisplayType poll();
	virtual void reconfigure(TDEConfig*);

public slots:
  void slowVolumeUp();
  void slowVolumeDown();
  void fastVolumeUp();
  void fastVolumeDown();
  void mute();
  void launchMail();
  void launchBrowser();
  void launchSearch();
  void launchHomeFolder();
  void launchMusic();
  void launchCalculator();
  void launchTerminal();
  void launchHelp();
  void eject();
  void lightBulb();
  void pmBattery();

private:
	bool retrieveKmixDevices();
	void volumeChange(int direction, int step);
	bool retrieveMute();
	bool retrieveVolume();
	void displayVolume();
	void launch(TQString configKey, TQString defaultApplication);

	TDEGlobalAccel *ga;
	TDEConfig* config;

	DCOPRef *kmixClient, *kmixWindow;

	int m_progress;
	long m_volume;
	bool m_mute;

	long m_maxVolume, m_minVolume;

	// following properties are read from config file:
	int m_volumeStepFast, m_volumeStepSlow;
	int m_volumeDeviceIdx, m_muteDeviceIdx, m_extraDeviceIdx;
	bool m_enabled;

	Monitor::DisplayType m_displayType;
};

}

#endif
