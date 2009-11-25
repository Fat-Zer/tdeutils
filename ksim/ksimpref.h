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

#ifndef KSIMPREF_H
#define KSIMPREF_H

#include <kdialogbase.h>
#include <kdelibs_export.h>

class QFrame;
class KDesktopFile;
namespace KSim
{
  class Config;
  class Plugin;
  class MonitorPrefs;
  class GeneralPrefs;
  class ClockPrefs;
  class UptimePrefs;
  class MemoryPrefs;
  class SwapPrefs;
  class ThemePrefs;

  class ChangedPlugin
  {
    public:
      ChangedPlugin() : m_name(0) {}
      ChangedPlugin(bool enabled, const QCString &libname,
          const QString &name, const QString &file) : m_enabled(enabled),
          m_libname(libname), m_name(name), m_file(file) {}
      ChangedPlugin(bool enabled, const QCString &libname,
          const QString &name, const QString &file, bool oldState)
          : m_enabled(enabled), m_libname(libname), m_name(name),
          m_file(file), m_oldState(oldState) {}

      bool isEnabled() const { return m_enabled; }
      bool isDifferent() const { return m_enabled != m_oldState; }
      const QCString &libName() const { return m_libname; }
      const QString &name() const { return m_name; }
      const QString &filename() const { return m_file; }

    private:
     bool m_enabled;
     QCString m_libname;
     QString m_name;
     QString m_file;
     bool m_oldState;
  };

  class ChangedPluginList : public QValueList<ChangedPlugin>
  {
    public:
      ChangedPluginList() {}
      ~ChangedPluginList() {}
  };

  class KDE_EXPORT ConfigDialog : public KDialogBase
  {
    Q_OBJECT
    public:
      ConfigDialog(KSim::Config *config, QWidget *parent, const char *name = 0);
      ~ConfigDialog();

    public slots:
      void removePage(const QCString &name);
      void createPage(const QCString &name); // overload
      void createPage(const KSim::Plugin &plugin);

    signals:
      void reparse(bool, const KSim::ChangedPluginList &);

    private slots:
      void reload();
      void savePrefs();
      void saveConfig(bool);
      void readConfig();
      void closePrefs();
      void loadPluginConfig();
      void enableButtons();
      void disableButtons();

    private:
      const KSim::ChangedPlugin &findPlugin(const QString &name) const;

      ChangedPluginList m_currentPlugins;
      KSim::MonitorPrefs *m_monPage;
      KSim::GeneralPrefs *m_generalPage;
      KSim::ClockPrefs *m_clockPage;
      KSim::UptimePrefs *m_uptimePage;
      KSim::MemoryPrefs *m_memoryPage;
      KSim::SwapPrefs *m_swapPage;
      KSim::ThemePrefs *m_themePage;
      KSim::Config *m_config;
  };
}
#endif
