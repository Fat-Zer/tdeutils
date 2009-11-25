/*
   This file is part of the KDE project

   Copyright (c) 2004 Chris Howells <howells@kde.org>
   Much code and ideas stolen from Jonathan Riddell's ThinkPad plugin

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; version
   2 of the License.

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

#include <qfile.h>
#include <qdir.h>

#include "kmilointerface.h"

#include "asus.h"

#ifdef Q_OS_FREEBSD
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#define ROUND(x) (int(x + 0.5))

namespace KMilo {

AsusMonitor::AsusMonitor(QObject* parent, const char* name, const QStringList& args): Monitor(parent, name, args)
{
}

AsusMonitor::~AsusMonitor()
{
}

bool AsusMonitor::init()
{
	kdDebug() << "loading asus kmilo plugin" << endl;

#ifdef Q_OS_FREEBSD
	/*
	 * Check if the sysctls are in place by looking up their MIBs.
	 * Caching the MIBs results in a major speedup.
	 */
	size_t len = 4;
	if ( sysctlnametomib("hw.acpi.asus.lcd_brightness", brightness_mib, &len) == -1 ||
	     sysctlnametomib("hw.acpi.asus.lcd_backlight", backlight_mib, &len) == -1 ||
	     sysctlnametomib("hw.acpi.asus.video_output", video_mib, &len) == -1 )
	{
		kdDebug() << "The driver's sysctls don't seem to exist, check that the acpi_asus module is loaded" << endl;
		return false;
	}
#else
	// we need to read the /proc file system and store the current values into a struct
	QDir dir("/proc/acpi/asus");
	if (!dir.exists())
	{
		kdDebug() << "/proc/acpi/asus doesn't exist, check that the asus_acpi module is loaded" << endl;
		return false;
	}
#endif
	else
	{
		clearStruct(asus_state);
		clearStruct(last_asus_state);
		if (!readProc(&asus_state))
		{
			return false;
		}
	}

	return true;
}

Monitor::DisplayType AsusMonitor::poll()
{

	// save last state and get new one
	memcpy(&last_asus_state, &asus_state, sizeof(asus_state_struct));
	readProc(&asus_state);

	Monitor::DisplayType pollResult = None;

	if (last_asus_state.brightness != asus_state.brightness)
	{
		pollResult = Brightness;
		float value = (float)asus_state.brightness / 15;
		m_progress = ROUND(value * 100);
	}

	/*if (last_asus_state.lcd != asus_state.lcd)
	{
		if (asus_state.lcd == 0)
		{
			_interface->displayText(i18n("Display changed: LCD off"));
		}
		else
		{
			_interface->displayText(i18n("Display changed: LCD on"));
		}
	}*/

	if (last_asus_state.display != asus_state.display)
	{
		switch (asus_state.display)
		{
			case 0:
				_interface->displayText(i18n("Display changed: off"));
				break;
			case 1:
				_interface->displayText(i18n("Display changed: LCD on"));
				break;
			case 2:
				_interface->displayText(i18n("Display changed: CRT on"));
				break;
			case 3:
				_interface->displayText(i18n("Display changed: LCD and CRT on"));
				break;
			case 4:
				_interface->displayText(i18n("Display changed: TV out on"));
				break;
			case 5:
				_interface->displayText(i18n("Display changed: LCD and TV out on"));
				break;
			case 6:
				_interface->displayText(i18n("Display changed: CRT and TV out on"));
				break;
			case 7:
				_interface->displayText(i18n("Display changed: LCD, CRT and TV out on"));
				break;
		}
	}

	return pollResult;
}


int AsusMonitor::progress() const
{
	return m_progress;
}

bool AsusMonitor::readProc(asus_state_struct* asus_state)
{
#ifdef Q_OS_FREEBSD
	int value;
	size_t value_len = sizeof(value);

	if ( sysctl(brightness_mib, 4, &value, &value_len, NULL, 0) != -1 )
		asus_state->brightness = value;

	if ( sysctl(backlight_mib, 4, &value, &value_len, NULL, 0) != -1 )
		asus_state->lcd = value;

	if ( sysctl(video_mib, 4, &value, &value_len, NULL, 0) != -1 )
		asus_state->display = value;
#else
	asus_state->brightness = readProcEntry(QString("brn"));
	//asus_state->lcd = readProcEntry(QString("lcd"));
	//disabled because it does not yet work on my S5200N (asus_acpi bug)
	//asus_state->display = readProcEntry(QString("disp"));
	//FIXME
#endif
	return true;
}

int AsusMonitor::readProcEntry(const QString &name)
{
	QFile f(QString("/proc/acpi/asus/%1").arg(name).local8Bit());

	if (f.open(IO_ReadOnly))
	{
		QString line;
		if (f.readLine(line, 1024) > 0)
		{
			line = line.stripWhiteSpace();
			int value = line.section(' ', 0, 0).toInt();
			if (value > 0)
			{
				return value;
			}
		}
	}
	return 0;
}

void AsusMonitor::clearStruct(asus_state_struct& asus_state)
{
	asus_state.brightness = 0;
	asus_state.lcd = 0;
	asus_state.display = 0;
}

}

K_EXPORT_COMPONENT_FACTORY(kmilo_asus, KGenericFactory<KMilo::AsusMonitor>("kmilo_asus"))
