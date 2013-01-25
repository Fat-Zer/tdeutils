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

#include <tqstringlist.h>
#include <tqpoint.h>
#include <tqfont.h>

#include <kdemacros.h>

class TDEConfig;

namespace KSim
{
  /**
   * @internal
   */
  class KDE_EXPORT Config
  {
    public:
      Config(TDEConfig *config);
      ~Config();

      static TDEConfig *config();

      int width(int defaultWidth) const;
      void setWidth(int width);
      bool enabledMonitor(const TQString &) const;
      void setEnabledMonitor(const TQString &, bool);
      TQString monitorCommand(const TQString &) const;
      void setMonitorCommand(const TQString &, const TQString &);
      int monitorLocation(const TQString &);
      void setMonitorLocation(const TQString &, int);
      bool displayFqdn() const;
      void setDisplayFqdn(bool);
      bool showDock() const;
      void setShowDock(bool);
      bool savePos() const;
      void setSavePos(bool);
      bool stayOnTop() const;
      void setStayOnTop(bool);
      TQPoint position(const TQPoint &) const;
      void setPosition(const TQPoint &);
      TQSize graphSize() const;
      void setGraphSize(const TQSize &);
      TQString uptimeFormat() const;
      void setUptimeFormat(const TQStringList &);
      int uptimeItem() const;
      void setUptimeItem(int);
      TQStringList uptimeFormatList() const;
      TQString memoryFormat() const;
      void setMemoryFormat(const TQStringList &);
      int memoryItem() const;
      void setMemoryItem(int);
      TQStringList memoryFormatList() const;
      TQString swapFormat() const;
      void setSwapFormat(const TQStringList &);
      int swapItem() const;
      void setSwapItem(int);
      TQStringList swapFormatList() const;
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
      TQString themeUrl() const;
      TQString themeName() const;
      void setThemeName(const TQString &);
      int themeAlt() const;
      void setThemeAlt(int);
      TQFont themeFont() const;
      void setThemeFont(const TQFont &);
      int themeFontItem() const;
      void setThemeFontItem(int);
      bool reColourThemes() const;
      void setReColourThemes(bool);

    private:
      static TDEConfig *mainConfig;
  };
}
#endif
