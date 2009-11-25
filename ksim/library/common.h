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

#ifndef KSIM__COMMON_H
#define KSIM__COMMON_H

#define KSIM_VERSION 110
#define KSIM_VERSION_STRING "1.1.0"

#include <qstring.h>
#include <kdemacros.h>

/**
 * namespace for common ksim entries
 *
 * @author Robbie Ward <linuxphreak@gmx.co.uk>
 */
namespace KSim
{
  // a few forward declares
  class ThemeLoader;
  class Theme;
  class PluginLoader;
  class PluginList;
  class Plugin;
  class PluginObject;
  class PluginView;
  class PluginPage;

  /**
   * class that all widgets (library dir only) HAVE to inherit
   * if they want to change items on ksim configuration reload
   *
   * any class that inherits this must re-implement
   * the configureObject() function.
   *
   * example:
   * <pre>
   *  #include &lt;qwidget.h&gt;
   *  #include &lt;ksim/common.h&gt;
   *
   *  class Test : public QWidget, virtual public KSim::Base
   *  {
   *    public:
   *      Test(QWidget *parent, const char *name) : QWidget(parent, name)
   *      {
   *      }
   *      ~Test() {}
   *
   *      void configureObject(bool repaintWidget)
   *      {
   *        // Re-Create view code here
   *      }
   *  };
   * </pre>
   * @short base class for widgets
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT Base
  {
    public:
      Base();
      Base(int type);
      virtual ~Base();

      /**
       * Reimplement this to change theme images/locations etc
       */
      virtual void configureObject(bool = true) = 0;
      /**
       * returns true if configureObject has been setup
       * to contain only theme configuration
       * @see #setThemeConfigOnly
       */
      bool isThemeConfigOnly() const;
      /**
       * sets the theme type
       */
      void setType(int type);
      /**
       * @return the theme type, eg host, mem, swap etc
       */
      int type() const;
      /**
       * Set the config entry key to look for when reading gkrellmrc
       */
      void setConfigString(const QString &string);
      /**
       * @return the config entry to look for when reading theme files
       */
      const QString &configString() const;

    protected:
      /**
       * reimplement this function if you want setType()
       * to call extra information
       */
      virtual void extraTypeCall();
      /**
       * @return the current KSim::ThemeLoader instance
       */
      KSim::ThemeLoader &themeLoader() const;
      /**
       * set this to false if you want KSim to call your KSim::Base
       * configureObject() when KSim's configuration has changed
       * even if the theme settings are the same. The default is true
       */
      void setThemeConfigOnly(bool);

    private:
      Base(const Base &rhs);
      Base &operator=(const Base &rhs);

      class Private;
      Private *d;
  };
}
#endif
