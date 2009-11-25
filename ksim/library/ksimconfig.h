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

#ifndef KSIMCONFIG_H
#define KSIMCONFIG_H

#include <qstringlist.h>
#include <qpoint.h>
#include <qfont.h>

#include <kdemacros.h>

class KConfig;

namespace KSim
{
  /**
   * @internal
   */
  class KDE_EXPORT Config
  {
    public:
      Config(KConfig *config);
      ~Config();

      static KConfig *config();

      int width(int defaultWidth) const;
      void setWidth(int width);
      bool enabledMonitor(const QString &) const;
      void setEnabledMonitor(const QString &, bool);
      QString monitorCommand(const QString &) const;
      void setMonitorCommand(const QString &, const QString &);
      int monitorLocation(const QString &);
      void setMonitorLocation(const QString &, int);
      bool displayFqdn() const;
      void setDisplayFqdn(bool);
      bool showDock() const;
      void setShowDock(bool);
      bool savePos() const;
      void setSavePos(bool);
      bool stayOnTop() const;
      void setStayOnTop(bool);
      QPoint position(const QPoint &) const;
      void setPosition(const QPoint &);
      QSize graphSize() const;
      void setGraphSize(const QSize &);
      QString uptimeFormat() const;
      void setUptimeFormat(const QStringList &);
      int uptimeItem() const;
      void setUptimeItem(int);
      QStringList uptimeFormatList() const;
      QString memoryFormat() const;
      void setMemoryFormat(const QStringList &);
      int memoryItem() const;
      void setMemoryItem(int);
      QStringList memoryFormatList() const;
      QString swapFormat() const;
      void setSwapFormat(const QStringList &);
      int swapItem() const;
      void setSwapItem(int);
      QStringList swapFormatList() const;
      bool showTime() const;
      void setShowTime(bool);
      bool show24hour() const;
      void setShow24hour(bool);
      bool showDate() const;
      void setShowDate(bool);
      bool showUptime() const;
      void setShowUptime(bool);
      bool showMemory() const;
      void setShowMemory(bool);
      bool showSwap() const;
      void setShowSwap(bool);
      bool showProcs() const;
      void setShowProcs(bool);
      QString themeUrl() const;
      QString themeName() const;
      void setThemeName(const QString &);
      int themeAlt() const;
      void setThemeAlt(int);
      QFont themeFont() const;
      void setThemeFont(const QFont &);
      int themeFontItem() const;
      void setThemeFontItem(int);
      bool reColourThemes() const;
      void setReColourThemes(bool);

    private:
      static KConfig *mainConfig;
  };
}
#endif
