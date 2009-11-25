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

#ifndef PLUGINGLOBAL_H
#define PLUGINGLOBAL_H

#include <qstring.h>
#include <qpixmap.h>
#include <qvaluelist.h>

#include <kdemacros.h>

class KDesktopFile;

namespace KSim
{
  class PluginObject;
  class PluginView;
  class PluginPage;

  /**
   * A class that holds various information about a plugin
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT Plugin
  {
    public:
      /**
       * Constructs a null Plugin
       */
      Plugin();
      /**
       * Constructs a Plugin
       */
      Plugin(KSim::PluginObject *plugin, const KDesktopFile &file);
      /**
       * Copy constructor
       */
      Plugin(const Plugin &rhs);
      /**
       * Destructor for Plugin
       */
      ~Plugin();

      /**
       * Assigns rhs to this Plugin and returns a reference to this Plugin.
       */
      Plugin &operator=(const Plugin &rhs);
      /**
       * @return true if the plugin is equal to rhs; otherwise returns false
       * @see #operator!=
       */
      bool operator==(const Plugin &rhs) const;
      /**
       * @return true if the plugin is different to rhs; otherwise returns false
       * @see #operator==
       */
      bool operator!=(const Plugin &rhs) const;
      /**
       * sets if the plugin is enabled or not
       */
      void setEnabled(bool enabled);
      /**
       * returns true if this plugin is enabled, else it returns false
       */
      bool isEnabled() const;
      /**
       * returns true if the last enabled state is different to the current
       * enabled state, else it returns false
       */
      bool isDifferent() const;
      /**
       * @return true if the Plugin object is empty
       */
      bool isNull() const;
      /**
       * @return the name of the plugin, NOT the library name
       * @see #libName
       */
      const QString &name() const;
      /**
       * @return the icon of the plugin
       */
      QPixmap icon() const;
      /**
       * @return library name of the plugin
       */
      QCString libName() const;
      /**
       * @return path to the .desktop file
       */
      const QString &fileName() const;
      /**
       * @return the plugin object
       */
      KSim::PluginObject *plugin() const;
      /**
       * @return the view object of the plugin
       */
      KSim::PluginView *view() const;
      /**
       * @return the config object of the plugin
       */
      KSim::PluginPage *configPage() const;
      /**
       * a null plugin, provided for convenience.
       * currently only used in KSim::PluginLoader
       */
      static KSim::Plugin null;
    private:
      void init(KSim::PluginObject *plugin, const KDesktopFile &file);

      class Private;
      Private *d;
  };

  /**
   * Please do not use this class directly,
   * use pluginList() from KSim::PluginLoader instead
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT PluginList : public QValueList<Plugin>
  {
    public:
      /**
       * constructs a null list
       */
      PluginList() : QValueList<Plugin>() {}
      /**
       * constructs a copy of @p list
       */
      PluginList(const PluginList &list) : QValueList<Plugin>(list) {}
      /**
       * constructs a copy of @p list
       */
      PluginList(const QValueList<Plugin> &list)
             : QValueList<Plugin>(list) {}
      /**
       * constructs a list with just one item
       */
      PluginList(const Plugin &plugin) { append(plugin); }
      ~PluginList() {}
  };
}
#endif
