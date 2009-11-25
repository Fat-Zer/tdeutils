/*  ksim - a system monitor for kde
 *
 *  Copyright (C) 2001  Robbie Ward <linuxphreak@gmx.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <kstandarddirs.h>
#include <kconfig.h>
#include <kglobal.h>

#include "ksimconfig.h"
#include "themeloader.h"

KConfig *KSim::Config::mainConfig = 0;

KSim::Config::Config(KConfig *config)
{
  mainConfig = config;
}

KSim::Config::~Config()
{
}

KConfig *KSim::Config::config()
{
  return mainConfig;
}

int KSim::Config::width(int defaultWidth) const
{
  mainConfig->setGroup("General Options");
  int savedWidth = mainConfig->readNumEntry("Width");
  if (savedWidth == -1)
    return defaultWidth;

  return savedWidth;
}

void KSim::Config::setWidth(int width)
{
  mainConfig->setGroup("General Options");
  mainConfig->writeEntry("Width", width);
  mainConfig->sync();
}

bool KSim::Config::enabledMonitor(const QString &entry) const
{
  mainConfig->setGroup("Monitors");
  return mainConfig->readBoolEntry(entry, false);
}

void KSim::Config::setEnabledMonitor(const QString &entry, bool enabled)
{
  mainConfig->setGroup("Monitors");
  mainConfig->writeEntry(entry, enabled);
  mainConfig->sync();
}

QString KSim::Config::monitorCommand(const QString &entry) const
{
  mainConfig->setGroup("Monitors");
  return mainConfig->readPathEntry(entry + "_command");
}

void KSim::Config::setMonitorCommand(const QString &entry,
   const QString &command)
{
  mainConfig->setGroup("Monitors");
  mainConfig->writePathEntry(entry + "_command", command);
  mainConfig->sync();
}

int KSim::Config::monitorLocation(const QString &entry)
{
  mainConfig->setGroup("Monitors");
  return mainConfig->readNumEntry(entry + "_location", -1);
}

void KSim::Config::setMonitorLocation(const QString &entry, int location)
{
  mainConfig->setGroup("Monitors");
  mainConfig->writeEntry(entry + "_location", location);
  mainConfig->sync();
}

bool KSim::Config::displayFqdn() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("DisplayFqdn", true);
}

void KSim::Config::setDisplayFqdn(bool value)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("DisplayFqdn", value);
  mainConfig->sync();
}

bool KSim::Config::showDock() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("showDock", true);
}

void KSim::Config::setShowDock(bool showDock)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("showDock", showDock);
  mainConfig->sync();
}

bool KSim::Config::savePos() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("savePos", false);
}

void KSim::Config::setSavePos(bool savePos)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("savePos", savePos);
  mainConfig->sync();
}

bool KSim::Config::stayOnTop() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("stayOnTop", false);
}

void KSim::Config::setStayOnTop(bool stayOnTop)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("stayOnTop", stayOnTop);
  mainConfig->sync();
}

QPoint KSim::Config::position(const QPoint &defaultPos) const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readPointEntry("ksimPosition", &defaultPos);
}

void KSim::Config::setPosition(const QPoint &pos)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("ksimPosition", pos);
  mainConfig->sync();
}

QSize KSim::Config::graphSize() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readSizeEntry("GraphSize");
}

void KSim::Config::setGraphSize(const QSize &size)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("GraphSize", size);
  mainConfig->sync();
}

QString KSim::Config::uptimeFormat() const
{
  return uptimeFormatList()[uptimeItem()];
}

int KSim::Config::uptimeItem() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readNumEntry("UptimeItem", 0);
}

QStringList KSim::Config::uptimeFormatList() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readListEntry("UptimeFormat");
}

void KSim::Config::setUptimeFormat(const QStringList &uptimeFormat)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("UptimeFormat", uptimeFormat);
  mainConfig->sync();
}

void KSim::Config::setUptimeItem(int item)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("UptimeItem", item);
  mainConfig->sync();
}

QString KSim::Config::memoryFormat() const
{
  return memoryFormatList()[memoryItem()];
}

int KSim::Config::memoryItem() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readNumEntry("MemoryItem", 0);
}

QStringList KSim::Config::memoryFormatList() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readListEntry("MemoryFormat");
}

void KSim::Config::setMemoryFormat(const QStringList &memoryFormat)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("MemoryFormat", memoryFormat);
  mainConfig->sync();
}

void KSim::Config::setMemoryItem(int item)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("MemoryItem", item);
  mainConfig->sync();
}

QString KSim::Config::swapFormat() const
{
  mainConfig->setGroup("Misc");
  return swapFormatList()[swapItem()];
}

int KSim::Config::swapItem() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readNumEntry("SwapItem", 0);
}

QStringList KSim::Config::swapFormatList() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readListEntry("SwapFormat");
}

void KSim::Config::setSwapFormat(const QStringList &swapFormat)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("SwapFormat", swapFormat);
  mainConfig->sync();
}

void KSim::Config::setSwapItem(int item)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("SwapItem", item);
  mainConfig->sync();
}

bool KSim::Config::showTime() const
{                    
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("ShowTime", false);
}

void KSim::Config::setShowTime(bool time)
{ 
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("ShowTime", time);
  mainConfig->sync();
} 

bool KSim::Config::show24hour() const
{ 
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("Show24hour", true);
} 
  
void KSim::Config::setShow24hour(bool _24hour)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("Show24hour", _24hour);
  mainConfig->sync();
} 

bool KSim::Config::showDate() const
{ 
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("ShowDate", false);
} 

void KSim::Config::setShowDate(bool date)
{ 
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("ShowDate", date);
  mainConfig->sync();
}

bool KSim::Config::showUptime() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("ShowUptime", true);
}

void KSim::Config::setShowUptime(bool uptime)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("ShowUptime", uptime);
  mainConfig->sync();
}

bool KSim::Config::showMemory() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("ShowMemory", true);
}

void KSim::Config::setShowMemory(bool memory)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("ShowMemory", memory);
  mainConfig->sync();
}

bool KSim::Config::showSwap() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("ShowSwap", true);
}

void KSim::Config::setShowSwap(bool swap)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("ShowSwap", swap);
  mainConfig->sync();
}

bool KSim::Config::showProcs() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("ShowProcs", false);
}

void KSim::Config::setShowProcs(bool procs)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("ShowProcs", procs);
  mainConfig->sync();
}

QString KSim::Config::themeUrl() const
{
  return KSim::ThemeLoader::currentUrl();
}

QString KSim::Config::themeName() const
{
  return KSim::ThemeLoader::currentName();
}

void KSim::Config::setThemeName(const QString &name)
{
  mainConfig->setGroup("Theme");
  mainConfig->writeEntry("Name", name);
  mainConfig->sync();
}

int KSim::Config::themeAlt() const
{
  return KSim::ThemeLoader::currentAlternative();
}

void KSim::Config::setThemeAlt(int alt)
{
  mainConfig->setGroup("Theme");
  mainConfig->writeEntry("Alternative", alt);
  mainConfig->sync();
}

QFont KSim::Config::themeFont() const
{
  return KSim::ThemeLoader::currentFont();
}

void KSim::Config::setThemeFont(const QFont &font)
{
  mainConfig->setGroup("Theme");
  mainConfig->writeEntry("Font", font);
  mainConfig->sync();
}

int KSim::Config::themeFontItem() const
{
  return KSim::ThemeLoader::currentFontItem();
}

void KSim::Config::setThemeFontItem(int item)
{
  mainConfig->setGroup("Theme");
  mainConfig->writeEntry("FontItem", item);
  mainConfig->sync();
}

bool KSim::Config::reColourThemes() const
{
  mainConfig->setGroup("Misc");
  return mainConfig->readBoolEntry("ReColourTheme", false);
}

void KSim::Config::setReColourThemes(bool value)
{
  mainConfig->setGroup("Misc");
  mainConfig->writeEntry("ReColourTheme", value);
  mainConfig->sync();
}
