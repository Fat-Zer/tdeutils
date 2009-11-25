/*
   This file is part of the KDE project

   Copyright (c) 2004 Jonathan Riddell <jr@jriddell.org>

   Based on Demo kmilo service by George Staikos
   Copyright (c) 2003 George Staikos <staikos@kde.org>

   And tpb by Markus Braun
   Copyright (C) 2002,2003 Markus Braun <markus.braun@krawel.de>

   FreeBSD support by Markus Brueffer
   Copyright (C) 2005 Markus Brueffer <markus@FreeBSD.org>

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
#include <kconfig.h>
#include <krun.h>
#include <kurl.h>
#include "kmilointerface.h"

#ifdef Q_OS_FREEBSD
#include <sys/sysctl.h>
#endif

#include "thinkpad.h"

namespace KMilo {

ThinkPadMonitor::ThinkPadMonitor(QObject* parent, const char* name, const QStringList& args): Monitor(parent, name, args) {
	m_progress = 0;
	m_volume = 50;  //set in retrieveVolume()
}

ThinkPadMonitor::~ThinkPadMonitor() {
}

bool ThinkPadMonitor::init() {

	KConfig config("kmilodrc");
	reconfigure(&config);

	if (m_run) {
		clearStruct(thinkpad_state);
		clearStruct(last_thinkpad_state);
		if ( getNvramState(&thinkpad_state) == false )  {
			return false;
		}

		if (m_softwareVolume || m_volumeStep != defaultVolumeStep) {
			kmixClient = new DCOPRef("kmix", "Mixer0");
			kmixWindow = new DCOPRef("kmix", "kmix-mainwindow#1");
			retrieveVolume();
			setNvramVolume();
		}
	}

	return m_run;
}

Monitor::DisplayType ThinkPadMonitor::poll() {

	// save last state and get new one
	memcpy(&last_thinkpad_state, &thinkpad_state, sizeof(thinkpad_state_struct));
	getNvramState(&thinkpad_state);

	Monitor::DisplayType pollResult = None;

	// determine the state of the mute button
	if (thinkpad_state.mute_toggle != last_thinkpad_state.mute_toggle ||
	    (thinkpad_state.volume_toggle != last_thinkpad_state.volume_toggle
	     && last_thinkpad_state.mute_toggle == 1)) {

		showToggleMessage(i18n("Mute on"), i18n("Mute off"), thinkpad_state.mute_toggle == 1);
		if (m_softwareVolume || m_volumeStep != defaultVolumeStep) {
			kmixClient->send("setMasterMute", thinkpad_state.mute_toggle == 1);
		}
	}

	// determine the state of the Thinkpad button
	if (thinkpad_state.thinkpad_toggle != last_thinkpad_state.thinkpad_toggle &&
	    thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {
		_interface->displayText(i18n("Thinkpad Button Pressed"));

		KURL url(m_buttonThinkpad);
		(void) new KRun(url, 0, true, true);
	}

	// determine the state of ThinkLight
	if (thinkpad_state.thinklight_toggle != last_thinkpad_state.thinklight_toggle) {
		showToggleMessage(i18n("ThinkLight is on"), i18n("ThinkLight is off"), thinkpad_state.thinklight_toggle == 1);
	}

	// determine the state of the volume buttons
	if (thinkpad_state.volume_level != last_thinkpad_state.volume_level) {

		pollResult = Volume;

		if (m_volumeStep == defaultVolumeStep && m_softwareVolume == false) {
			//no need to write to nvram or set volume in software
			m_progress = thinkpad_state.volume_level * 100 / defaultVolumeStep;
		} else {
			if (thinkpad_state.volume_level > last_thinkpad_state.volume_level)  {
				m_progress = m_volume + m_volumeStep;
			} else {
				m_progress = m_volume - m_volumeStep;
			}
			setVolume(m_progress);
		}

	}

	// determine the state of the brightness buttons
	if (thinkpad_state.brightness_level != last_thinkpad_state.brightness_level) {
		pollResult = Brightness;
		m_progress = thinkpad_state.brightness_level * 100 / 7;
	}

	// Buttons below here are untested because they do not exist on my R31

	// determine the state of zoom
	if (thinkpad_state.zoom_toggle != last_thinkpad_state.zoom_toggle) {
		// showToggleMessage(i18n("Zoom is on"), i18n("Zoom is off"), thinkpad_state.zoom_toggle == 1);

		// Use as button since an Zooming is implemented
		_interface->displayText(i18n("Zoom button pressed"));

		KURL url(m_buttonZoom);
		(void) new KRun(url, 0, true, true);
	}

	// determine the state of the home button
	if (thinkpad_state.home_toggle != last_thinkpad_state.home_toggle &&
	    thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {
		_interface->displayText(i18n("Home button pressed"));
		KURL url(m_buttonHome);
		(void) new KRun(url, 0, true, true);
	}

	// determine the state of the search button
	if (thinkpad_state.search_toggle != last_thinkpad_state.search_toggle &&
	    thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {
		_interface->displayText(i18n("Search button pressed"));
		KURL url(m_buttonSearch);
		(void) new KRun(url, 0, true, true);
	}

	// determine the state of the mail button
	if (thinkpad_state.mail_toggle != last_thinkpad_state.mail_toggle &&
	    thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {
		_interface->displayText(i18n("Mail button pressed"));
		KURL url(m_buttonMail);
		(void) new KRun(url, 0, true, true);
	}

	// determine the state of display
	if (thinkpad_state.display_toggle != last_thinkpad_state.display_toggle &&
	   thinkpad_state.hibernate_toggle == last_thinkpad_state.hibernate_toggle) {

		// Some thinkpads have no hardware support to switch lcd/crt. They also
		// don't reflect the current state in thinkpad_state.display_state. So, if
		// thinkpad_state.display_toggle changes, but thinkpad_state.display_state does
		// not change, simulate the display state.

		unsigned int display_state = 1;
		if (thinkpad_state.display_state == last_thinkpad_state.display_state) {
			display_state = display_state % 3 + 1;
		} else {
			display_state = thinkpad_state.display_state;
		}

		switch (display_state & 0x03) {
		case 0x1:
			_interface->displayText(i18n("Display changed: LCD on, CRT off"));
			break;

		case 0x2:
			_interface->displayText(i18n("Display changed: LCD off, CRT on"));
			break;

		case 0x3:
			_interface->displayText(i18n("Display changed: LCD on, CRT on"));
			break;
		}
	}

	// determine the state of hv expansion
	if (thinkpad_state.expand_toggle != last_thinkpad_state.expand_toggle) {
		showToggleMessage(i18n("HV Expansion is on"), i18n("HV Expansion is off"), (thinkpad_state.expand_toggle & 0x01) == 1);
	}

	// determine power management mode AC
	if (thinkpad_state.powermgt_ac != last_thinkpad_state.powermgt_ac) {
		switch(thinkpad_state.powermgt_ac) {
		case 0x4:
			_interface->displayText(i18n("Power management mode AC changed: PM AC high"));
			break;

		case 0x2:
			_interface->displayText(i18n("Power management mode AC changed: PM AC auto"));
			break;

		case 0x1:
			_interface->displayText(i18n("Power management mode AC changed: PM AC manual"));
			break;

		default:
			_interface->displayText(i18n("Power management mode AC changed: PM AC unknown"));
			break;
		}
	}

	// determine power management mode battery
	if (thinkpad_state.powermgt_battery != last_thinkpad_state.powermgt_battery) {
		switch(thinkpad_state.powermgt_battery) {
		case 0x4:
			_interface->displayText(i18n("Power management mode battery changed: PM battery high"));
			break;

		case 0x2:
			_interface->displayText(i18n("Power management mode battery changed: PM battery auto"));
			break;

		case 0x1:
			_interface->displayText(i18n("Power management mode battery changed: PM battery manual"));
			break;

		default:
			_interface->displayText(i18n("Power management mode battery changed: PM battery unknown"));
			break;
		}
	}

	// determine the state of wireless lan
	if (thinkpad_state.wireless_toggle != last_thinkpad_state.wireless_toggle) {
		showToggleMessage(i18n("Wireless LAN is enabled"), i18n("Wireless LAN is disabled"), thinkpad_state.wireless_toggle == 1);
	}

	// determine the state of bluetooth
	if (thinkpad_state.bluetooth_toggle != last_thinkpad_state.bluetooth_toggle) {
		showToggleMessage(i18n("Bluetooth is enabled"), i18n("Bluetooth is disabled"), thinkpad_state.bluetooth_toggle == 1);
	}

	return pollResult;
}


int ThinkPadMonitor::progress() const {
	return m_progress;
}

QString ThinkPadMonitor::message() const {
	//unused
	//return i18n("yer maw!");
	return "";
}

bool ThinkPadMonitor::getNvramState(thinkpad_state_struct* thinkpad_state) {
#ifndef Q_OS_FREEBSD
	int file;
	unsigned char buffer[114];

	// open nvram for reading
	// must use open/close because seek is not supported by nvram
	if ((file=open(m_nvramFile.latin1(), O_RDONLY|O_NONBLOCK)) == -1) {
		kdError() << "Unable to open device: " << m_nvramFile << endl;
		return false;
	}

	// read nvram
	if (read(file, buffer, sizeof(buffer)) != sizeof(buffer)) {
		kdError() << "Unable to read from device: " << m_nvramFile << endl;
		return false;
	}

	// close nvram device
	if (close(file) == -1) {
		kdError() << "Unable to close device %s: " << m_nvramFile << endl;
		return false;
	}

	thinkpad_state->thinkpad_toggle
		= (thinkpad_state->thinkpad_toggle   & ~0x01) | (( buffer[0x57] & 0x08) >> 3);
	thinkpad_state->zoom_toggle
		= (thinkpad_state->zoom_toggle       & ~0x01) | ((~buffer[0x57] & 0x20) >> 5);
	thinkpad_state->display_toggle
		= (thinkpad_state->display_toggle    & ~0x01) | (( buffer[0x57] & 0x40) >> 6);
	thinkpad_state->home_toggle
		= (thinkpad_state->home_toggle       & ~0x01) | (( buffer[0x56] & 0x01)     );
	thinkpad_state->search_toggle
		= (thinkpad_state->search_toggle     & ~0x01) | (( buffer[0x56] & 0x02) >> 1);
	thinkpad_state->mail_toggle
		= (thinkpad_state->mail_toggle       & ~0x01) | (( buffer[0x56] & 0x04) >> 2);
	thinkpad_state->thinklight_toggle
		= (thinkpad_state->thinklight_toggle & ~0x01) | (( buffer[0x58] & 0x10) >> 4);
	thinkpad_state->hibernate_toggle
		= (thinkpad_state->hibernate_toggle  & ~0x01) | (( buffer[0x58] & 0x01)     );
	thinkpad_state->display_state
		=                                               (( buffer[0x59] & 0x03)     );
	thinkpad_state->expand_toggle
		= (thinkpad_state->expand_toggle     & ~0x01) | (( buffer[0x59] & 0x10) >> 4);
	thinkpad_state->brightness_level
		=                                               (( buffer[0x5E] & 0x07)     );
	thinkpad_state->brightness_toggle
		= (thinkpad_state->brightness_toggle & ~0x01) | (( buffer[0x5E] & 0x20) >> 5);
	thinkpad_state->volume_level
		=                                               (( buffer[0x60] & 0x0f)     );
	thinkpad_state->volume_toggle
		= (thinkpad_state->volume_toggle     & ~0x01) | (( buffer[0x60] & 0x80) >> 7);
	thinkpad_state->mute_toggle
		= (thinkpad_state->mute_toggle       & ~0x01) | (( buffer[0x60] & 0x40) >> 6);
	thinkpad_state->powermgt_ac
		=                                               (( buffer[0x39] & 0x07)     );
	thinkpad_state->powermgt_battery
		=                                               (( buffer[0x39] & 0x38) >> 3);
#else
	u_int n = 0;
	size_t len = sizeof(n);

	if ( sysctlbyname("dev.acpi_ibm.0.hotkey", &n, &len, NULL, 0) == -1 ) {
		kdError() << "Unable to read sysctl: dev.acpi_ibm.0.hotkey" << endl;
		return false;
	}

	thinkpad_state->thinkpad_toggle
		= (thinkpad_state->thinkpad_toggle   & ~0x01) | (( n & (1<<3)) >> 3);
	thinkpad_state->zoom_toggle
		= (thinkpad_state->zoom_toggle       & ~0x01) | (( n & (1<<4)) >> 4);
	thinkpad_state->display_toggle
		= (thinkpad_state->display_toggle    & ~0x01) | (( n & (1<<6)) >> 6);
	thinkpad_state->home_toggle
		= (thinkpad_state->home_toggle       & ~0x01) | (( n & (1<<0))     );
	thinkpad_state->search_toggle
		= (thinkpad_state->search_toggle     & ~0x01) | (( n & (1<<1)) >> 1);
	thinkpad_state->mail_toggle
		= (thinkpad_state->mail_toggle       & ~0x01) | (( n & (1<<2)) >> 2);
	thinkpad_state->hibernate_toggle
		= (thinkpad_state->hibernate_toggle  & ~0x01) | (( n & (1<<7)) >> 7);
	thinkpad_state->expand_toggle
		= (thinkpad_state->expand_toggle     & ~0x01) | (( n & (1<<9)) >> 9);
	thinkpad_state->brightness_toggle
		= (thinkpad_state->brightness_toggle & ~0x01) | (( n & (1<<10)) >> 10);
	thinkpad_state->volume_toggle
		= (thinkpad_state->volume_toggle     & ~0x01) | (( n & (1<<11)) >> 11);

	// Don't fail if the thinklight sysctl is not present. It is generated dynamically
	if ( sysctlbyname("dev.acpi_ibm.0.thinklight", &n, &len, NULL, 0) != -1 )
		thinkpad_state->thinklight_toggle = n;
	else
		kdWarning() << "Unable to read sysctl: dev.acpi_ibm.0.thinklight" << endl;

	if ( sysctlbyname("dev.acpi_ibm.0.lcd_brightness", &n, &len, NULL, 0) == -1 ) {
		kdError() << "Unable to read sysctl: dev.acpi_ibm.0.lcd_brightness" << endl;
		return false;
	}
	thinkpad_state->brightness_level = n;

	if ( sysctlbyname("dev.acpi_ibm.0.volume", &n, &len, NULL, 0) == -1 ) {
		kdError() << "Unable to read sysctl: dev.acpi_ibm.0.volume" << endl;
		return false;
	}
	thinkpad_state->volume_level = n;

	if ( sysctlbyname("dev.acpi_ibm.0.mute", &n, &len, NULL, 0) == -1 ) {
		kdError() << "Unable to read sysctl: dev.acpi_ibm.0.mute" << endl;
		return false;
	}
	thinkpad_state->mute_toggle = n;

	// Don't fail if wlan or bluetooth sysctls are not present. They are generated dynamically
	if ( sysctlbyname("dev.acpi_ibm.0.wlan", &n, &len, NULL, 0) != -1 )
		thinkpad_state->wireless_toggle = n;
	else
		kdWarning() << "Unable to read sysctl: dev.acpi_ibm.0.wlan" << endl;

	if ( sysctlbyname("dev.acpi_ibm.0.bluetooth", &n, &len, NULL, 0) != -1 )
		thinkpad_state->bluetooth_toggle = n;
	else
		kdWarning() << "Unable to read sysctl: dev.acpi_ibm.0.bluetooth" << endl;
#endif
	return true;
}

void ThinkPadMonitor::clearStruct(thinkpad_state_struct& thinkpad_state)  {
	thinkpad_state.thinkpad_toggle = 0;
	thinkpad_state.zoom_toggle = 0;
	thinkpad_state.display_toggle = 0;
	thinkpad_state.home_toggle = 0;
	thinkpad_state.search_toggle = 0;
	thinkpad_state.mail_toggle = 0;
	thinkpad_state.favorites_toggle = 0;
	thinkpad_state.reload_toggle = 0;
	thinkpad_state.abort_toggle = 0;
	thinkpad_state.backward_toggle = 0;
	thinkpad_state.forward_toggle = 0;
	thinkpad_state.fn_toggle = 0;
	thinkpad_state.thinklight_toggle = 0;
	thinkpad_state.hibernate_toggle = 0;
	thinkpad_state.display_state = 0;
	thinkpad_state.expand_toggle = 0;
	thinkpad_state.brightness_level = 0;
	thinkpad_state.brightness_toggle = 0;
	thinkpad_state.volume_level = 0;
	thinkpad_state.volume_toggle = 0;
	thinkpad_state.mute_toggle = 0;
	thinkpad_state.ac_state = 0;
	thinkpad_state.powermgt_ac = 0;
	thinkpad_state.powermgt_battery = 0;
	thinkpad_state.wireless_toggle = 0;
	thinkpad_state.bluetooth_toggle = 0;
}

void ThinkPadMonitor::showToggleMessage(QString onMessage, QString offMessage, bool state) {
		QString message;
		if (state)  {
			message = onMessage;
		} else {
			message = offMessage;
		}
		_interface->displayText(message);
}

void ThinkPadMonitor::reconfigure(KConfig* config) {
	config->setGroup("thinkpad");

	m_nvramFile = config->readEntry("nvram", "/dev/nvram");
	m_softwareVolume = config->readBoolEntry("softwareVolume", true);
	m_run = config->readBoolEntry("run", false);
	m_volumeStep = config->readNumEntry("volumeStep", defaultVolumeStep);
	m_buttonThinkpad = config->readEntry("buttonThinkpad", "/usr/bin/konsole");
	m_buttonHome = config->readEntry("buttonHome", "/usr/bin/konqueror");
	m_buttonSearch = config->readEntry("buttonSearch", "/usr/bin/kfind");
	m_buttonMail = config->readEntry("buttonMail", "/usr/bin/kmail");
	m_buttonZoom = config->readEntry("buttonZoom", "/usr/bin/ksnapshot");
}

bool ThinkPadMonitor::retrieveVolume() {
	bool kmix_error = false;

	DCOPReply reply = kmixClient->call("masterVolume");
	if (reply.isValid())  {
		m_volume = reply;
	} else {
		kmix_error = true;
	}

	if (kmix_error) { // maybe the error occurred because kmix wasn't running
		if (kapp->startServiceByDesktopName("kmix")==0) { // trying to start kmix
			// trying again
			reply = kmixClient->call("masterVolume");
			if (reply.isValid())  {
				m_volume = reply;
				kmix_error = false;
				kmixWindow->send("hide");
			}
		}
	}

	if (kmix_error) {
		kdError() << "KMilo: ThinkPadMonitor could not access kmix/Mixer0 via dcop" << endl;
		return false;
	} else {
		return true;
	}
}

void ThinkPadMonitor::setVolume(int volume) {
	if (!retrieveVolume())  {
		return;
	}

	if (volume > 100)  {
		m_volume = 100;
	} else if (volume < 0)  {
		m_volume = 0;
	} else {
		m_volume = volume;
	}
	kmixClient->send("setMasterVolume", m_volume);

	//write back to nvram but only if volume steps is not default
	if (m_volumeStep != defaultVolumeStep) {
		setNvramVolume();
	}
	m_progress = m_volume;
}

void ThinkPadMonitor::setNvramVolume() {
#ifndef Q_OS_FREEBSD
	int file;
	char buffer;

	//open nvram
	if ((file = open(m_nvramFile.latin1(), O_RDWR|O_NONBLOCK)) == -1) {
		kdError() << "Unable to open device " << m_nvramFile << endl;
		return;
	}

	// jump to volume section
	if (lseek(file, 0x60, SEEK_SET) == -1 ) {
		kdError() << "Unable to seek device " << m_nvramFile << endl;
		return;
	}

	// read nvram
	if (read(file, &buffer, sizeof(buffer)) != sizeof(buffer)) {
		kdError() << "Unable to read from device " << m_nvramFile << endl;
		return;
	}

	// set volume_level to the value we write back to nvram
	// taken from tpb, unfortunatly I'm not sure why it works
	thinkpad_state.volume_level = 0x07;
	buffer &= 0xf0;
	buffer |= thinkpad_state.volume_level;

	// jump to volume section
	if (lseek(file, 0x60, SEEK_SET) == -1 ) {
		kdError() << "Unable to seek device " << m_nvramFile << endl;
		return;
	}

	// write std value for volume
	if (write(file, &buffer, sizeof(buffer)) != sizeof(buffer)) {
		kdError() << "Unable to write to device " << m_nvramFile << endl;
		return;
	}

	close(file);
#else
	u_int n = thinkpad_state.volume_level;

	if (sysctlbyname("dev.acpi_ibm.0.volume", NULL, NULL, &n, sizeof(n)))
		kdError() << "Unable to write sysctl: dev.acpi_ibm.0.volume" << endl;
#endif
}

}

K_EXPORT_COMPONENT_FACTORY(kmilo_thinkpad, KGenericFactory<KMilo::ThinkPadMonitor>("kmilo_thinkpad"))
