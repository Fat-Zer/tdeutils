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

#ifndef PLUGINMODULE_H
#define PLUGINMODULE_H

#include <qwidget.h>

#include <kdemacros.h>

class QPopupMenu;
class KConfig;

#define KSIM_INIT_PLUGIN(className) \
extern "C" { \
  KDE_EXPORT KSim::PluginObject *init_plugin(const char *name) { \
    return new className(name); \
  } \
}

namespace KSim
{
  class PluginView;
  class PluginPage;

  /**
   * The base class for ksim modules.
   *
   * To init a plugin so KSim can load your plugin just do:
   * <pre>
   *   // MyPluginModule is the class that inherits KSim::PluginObject
   *   KSIM_INIT_PLUGIN(MyPluginModule);
   * </pre>
   * @see KSim::PluginView KSim::PluginPage
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT PluginObject
  {
    public:
      /**
       * constructor for PluginObject
       */
      PluginObject(const QCString &name);
      /**
       * destructor for PluginObject
       */
      virtual ~PluginObject();

      /**
       * creates a new View page, re-implement this
       * to return your main view class
       */
      virtual KSim::PluginView *createView(const char *) = 0;
      /**
       * creates a new Config page, re-implement this
       * to return you config class
       */
      virtual KSim::PluginPage *createConfigPage(const char *) = 0;
      /**
       * re-implement this to show your about dialog
       */
      virtual void showAbout() = 0;
      /**
       * @return the instance name of the app
       */
      const char *instanceName() const;
      /**
       * @return the name of the plugin
       */
      const QCString &name() const;
      /**
       * sets the configuration file name to @p name
       */
      void setConfigFileName(const QString &name);
      /**
       * @return the config filename the plugin should use
       * or name() if the filename hasn't been set
       */
      const QString &configFileName() const;

    private:
      PluginObject();
      PluginObject(const PluginObject &);
      PluginObject &operator=(const PluginObject &);

      class Private;
      Private *d;
  };

  /**
   * Provides you with a config page
   *
   * re-implement readConfig() and saveConfig() for your
   * classes reading and saving functions and use
   * the config() to gain access to your config file
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT PluginPage : public QWidget
  {
    Q_OBJECT
    public:
      /**
       * constructor for PluginPage
       */
      PluginPage(KSim::PluginObject *parent, const char *name);
      /**
       * destructor for PluginPage
       */
      virtual ~PluginPage();

      /**
       * called when apply or ok button is clicked in KSimPref
       * use this to save your plugin options
       */
      virtual void saveConfig() = 0;
      /**
       * called when apply or ok button is clicked in KSimPref
       * use this to read your plugin options
       */
      virtual void readConfig() = 0;
      /*
       * use this to get a config object unique to the plugin name,
       * eg: the plugin foo would have the config file foorc
       */
      KConfig *config() const;

    signals:
      void pageChanged();

    protected:
      PluginObject *parentPlugin() const;

    private:
      class Private;
      Private *d;
  };

  /**
   * inherit from this class to get your base view
   *
   * use config() to get an instance of your config file,
   * reimplement reparseConfig() to recreate your view
   * when apply or ok gets clicked in the config dialog
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT PluginView : public QWidget
  {
    Q_OBJECT
    public:
      /**
       * constructor for PluginView
       */
      PluginView(KSim::PluginObject *parent, const char *name);
      /**
       * destructor for PluginView
       */
      virtual ~PluginView();

      /*
       * use this to get a config object unique to the plugin name,
       * eg: the plugin foo would have the config file foorc
       */
      KConfig *config() const;
      /**
       * the plugins popup menu
       */
      QPopupMenu *menu() const;
      /**
       * reimplement to recreate your view when KSim requests this
       */
      virtual void reparseConfig() = 0;
      void doCommand();

    signals:
      void runCommand(const QCString &);

    protected:
      virtual void mousePressEvent(QMouseEvent *);
      PluginObject *parentPlugin() const;

    private slots:
      void showAbout();

    private:
      class Private;
      Private *d;
  };
}
#endif
