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

#ifndef KSIMVIEW_H
#define KSIMVIEW_H

#include <tqwidget.h>
#include <tqtimer.h>

#include <dcopobject.h>

#include <kpanelextension.h>
#include <tdelibs_export.h>

class TQBoxLayout;
class TQHBoxLayout;
class TQVBoxLayout;
class TQPopupMenu;
class KDesktopFile;
class TDEConfig;

namespace KSim
{
  class Sysinfo;
  class Config;
  class Plugin;
  class Label;
  class Frame;
  class PanelExtension;
  class ConfigDialog;
  class ChangedPluginList;

  class KDE_EXPORT MainView : public TQWidget, virtual public DCOPObject
  {
    Q_OBJECT
//    
    K_DCOP
    public:
      MainView(TDEConfig *config, bool loadPlugins,
         KSim::PanelExtension *topLevel,
         const char *name);

      ~MainView();

      virtual void show();

      KSim::Config *config() const;
      void makeDirs();

      TQSize sizeHint(KPanelExtension::Position, TQSize maxSize) const;
      void positionChange(Qt::Orientation);

    k_dcop:
      const TQString &hostname() const;
      virtual void maskMainView();

    signals:
      void reload();

    public slots:
      void reparseConfig(bool, const KSim::ChangedPluginList &);
      void addPlugins();
      void addPlugin(const KDesktopFile &, bool force = false);
      void removePlugin(const KDesktopFile &);
      void addMonitor(const KSim::Plugin &);
      void runCommand(const TQCString &);
      void preferences();
      void slotMaskMainView();

    protected:
      virtual void resizeEvent(TQResizeEvent *);
      virtual void paletteChange(const TQPalette &);

    private slots:
      void destroyPref();

    private:
      void cleanup();
	  
      KSim::Sysinfo *m_sysinfo;
      KSim::Label *m_hostLabel;
      KSim::Frame *m_leftFrame;
      KSim::Frame *m_rightFrame;
      KSim::Frame *m_topFrame;
      KSim::Frame *m_bottomFrame;
      KSim::PanelExtension *m_topLevel;
      KSim::Config *m_config;
      KSim::ConfigDialog *m_prefDialog;
      TQHBoxLayout *m_sizeLayout;
      TQVBoxLayout *m_subLayout;
      TQBoxLayout *m_pluginLayout;
      TQPoint m_mousePoint;
      int m_oldLocation;
      TQTimer m_maskTimer;
  };
}
#endif // KSIMVIEW_H
