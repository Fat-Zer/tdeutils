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

#ifndef PLUGINLOADER_H
#define PLUGINLOADER_H

#include "pluginglobal.h"
#include <qobject.h>

#include <kdemacros.h>

namespace KSim
{
  class KDE_EXPORT PluginInfo
  {
    friend class PluginLoader;
    public:
      ~PluginInfo();

      /**
       * @return the name of the plugin
       */
      const QString &name() const;
      /**
       * @return the library name of the plugin
       */
      QCString libName(bool includePrefix = false) const;
      /**
       * @return the location of the desktop file
       */
      const QString &location() const;

    private:
      PluginInfo();

      QString m_name;
      QString m_location;
      QCString m_libName;

      class Private;
      Private *d;
  };

  /**
   * Provides a loader for the plugins
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT PluginLoader : public QObject
  {
    Q_OBJECT
    friend class MainView;
    public:
      enum SearchType { Name = 0, DesktopFile, LibName };
      enum ErrorCode { EmptyLibName = -3, LibNotFound = -2,
          UnSymbols = -1, LibLoaded = 0 };

      /**
       * @return a reference to the instance
       */
      static PluginLoader &self();
      /**
       * loads a plugin, example:
       * <pre>
       *   KDesktopFile deskfile("/home/user/foo.desktop");
       *   KSim::PluginLoader::self().loadPlugin(deskFile);
       * </pre>
       * you can then use @ref pluginList() to access the plugin, view,
       * config page and plugin information
       * @param file is the desktop file of the lib
       * @return true if the plugin is successfully loaded
       */
      bool loadPlugin(const KDesktopFile &file);
      /**
       * unloads a loaded plugin and removes plugin entries from pluginList()
       */
      bool unloadPlugin(const QCString &name);
      /**
       * unloads all loaded plugins
       */
      void unloadAllPlugins();
      /**
       * convenience function
       *
       * returns true if info is loaded
       */
      bool isLoaded(const KSim::Plugin &info) const;
      /**
       * returns true if library is loaded
       */
      bool isLoaded(const QCString &library) const;
      /**
       * finds the plugins desktopfile and returns information
       * on the plugin
       * @return a KSim::PluginInfo object
       * @see KSim::PluginInfo
       */
      KSim::PluginInfo findPluginInfo(const QString &name,
         SearchType type = DesktopFile) const;
      /**
       * looks through the list of loaded plugins and returns
       * the one that matches libName, or returns
       * KSim::Plugin::null if a plugin could not be found.
       *
       * if libName does not start with "ksim_" then the function
       * will prepend this automatically.
       * @return a KSim::Plugin object
       * @see KSim::Plugin
       */
      KSim::Plugin &find(const QCString &libName);
      /**
       * convenience function, see the above function for details.
       */
      const KSim::Plugin &find(const QCString &libName) const;
      /**
       * equivalent to find(info.libName());
       */
      KSim::Plugin &find(const KSim::PluginInfo &info);
      /**
       * convenience function, see the above function for details.
       */
      const KSim::Plugin &find(const KSim::PluginInfo &info) const;
      /**
       * provides plugin(), view(), config page and plugin information
       * @see KSim::Plugin KSim::PluginList
       */
      const KSim::PluginList &pluginList() const;
      /**
       * Overloaded member function, This behaves essentially like
       * the above function
       */
      KSim::PluginList &pluginList();
      /**
       * @return a reference to the last plugin loaded (or a null plugin
       * if the plugin was unable to load)
       */
      const KSim::Plugin &plugin() const;
      /**
       * Overloaded member function, This behaves essentially like
       * the above function
       */
      KSim::Plugin &plugin();

    signals:
      void pluginLoaded(const KSim::Plugin &);

    protected:
      /**
       * constructor for PluginLoader, use self() to get an instance
       */
      PluginLoader();
      ~PluginLoader();

    private:
      PluginLoader(const PluginLoader &);
      PluginLoader &operator=(const PluginLoader &);
      /**
       * Deletes the instance and cleans up after itself
       */
      static void cleanup();
      ErrorCode createPlugin(const KDesktopFile &file);

      class Private;
      Private *d;
      static PluginLoader *m_instance;
  };
}
#endif
