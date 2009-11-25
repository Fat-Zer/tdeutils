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

#include "pluginloader.h"
#include "pluginloader.moc"

#include <kdebug.h>
#include <klibloader.h>
#include <kdesktopfile.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>

#include <qstringlist.h>

class KSim::PluginInfo::Private
{
  public:
};

KSim::PluginInfo::~PluginInfo()
{
}

const QString &KSim::PluginInfo::name() const
{
  return m_name;
}

QCString KSim::PluginInfo::libName(bool includePrefix) const
{
  return (includePrefix ? "ksim_" + m_libName : m_libName);
}

const QString &KSim::PluginInfo::location() const
{
  return m_location;
}

KSim::PluginInfo::PluginInfo()
{
}

class KSim::PluginLoader::Private
{
  public:
    typedef KSim::PluginObject *(PluginPtr)(const char *);
    KSim::PluginList pluginList;
    QString error;
    QString lib;
    bool lastLoaded;
    static const char *const ksimString;
};

const char *const KSim::PluginLoader::Private::ksimString = "ksim_";

KSim::PluginLoader *KSim::PluginLoader::m_instance = 0; // initialize pointer
KSim::PluginLoader &KSim::PluginLoader::self()
{
  if (!m_instance) // is it the first call?
    m_instance = new PluginLoader; // create sole instance

  return *m_instance; // address of sole instance
}

KSim::PluginLoader::~PluginLoader()
{
  unloadAllPlugins();
  delete d;
}

bool KSim::PluginLoader::loadPlugin(const KDesktopFile &file)
{
  switch (createPlugin(file)) {
    case KSim::PluginLoader::EmptyLibName:
      KMessageBox::error(0, i18n("KSim was unable to load the plugin %1"
            " due to the X-KSIM-LIBRARY property being empty in the"
            " plugins desktop file").arg(file.readName()));
      return false;
      break;
    case KSim::PluginLoader::LibNotFound:
      KMessageBox::error(0, i18n("KSim was unable to load the plugin %1"
            " due to not being able to find the plugin, check that the plugin"
            " is installed and is in your $KDEDIR/lib path").arg(file.readName()));
      return false;
      break;
    case KSim::PluginLoader::UnSymbols:
      KMessageBox::error(0, i18n("<qt>An error occurred while trying \n"
            "to load the plugin '%1'. \nThis could be caused by the"
            " following:<ul>\n<li>The plugin doesn't have the %2"
            " macro</li>\n<li>The plugin has been damaged or has"
            " some unresolved symbols</li>\n</ul> \nLast"
            " error message that occurred: \n%3</qt>")
            .arg(d->lib.prepend("ksim_")).arg("KSIM_INIT_PLUGIN")
            .arg(d->error));
      return false;
      break;
    default:
      break;
  }

  return true;
}

bool KSim::PluginLoader::unloadPlugin(const QCString &name)
{
  if (name.isEmpty())
    return false;

  // see if our plugin is loaded
  KSim::Plugin plugin = find(name);
  if (plugin.isNull())
    return false;

  // try to unload the library
  kdDebug(2003) << "Unloading plugin " << plugin.libName() << endl;
  KLibLoader::self()->unloadLibrary(plugin.libName());
  d->pluginList.remove(plugin);
  return true;
}

void KSim::PluginLoader::unloadAllPlugins()
{
  kdDebug(2003) << "Unloading all plugins" << endl;

  // Go through the plugin list and unload each plugin;
  KSim::PluginList::ConstIterator it;
  for (it = d->pluginList.begin(); it != d->pluginList.end(); ++it) {
    KLibLoader::self()->unloadLibrary((*it).libName());
  }

  // Clear the plugin list;
  d->pluginList.clear();
  KLibLoader::cleanUp();
}

bool KSim::PluginLoader::isLoaded(const KSim::Plugin &info) const
{
  return isLoaded(info.libName());
}

bool KSim::PluginLoader::isLoaded(const QCString &library) const
{
  if (library.isEmpty())
    return false;

  return !find(library).isNull();
}

KSim::PluginInfo KSim::PluginLoader::findPluginInfo(const QString &name,
   SearchType type) const
{
  QString location;

  switch (type) {
    case Name: {
      QStringList files = KGlobal::dirs()->findAllResources("data", "ksim/monitors/*.desktop");
      QStringList::ConstIterator it;
      for (it = files.begin(); it != files.end(); ++it) {
        KDesktopFile file((*it));
        if (file.readName() == name) {
          location = (*it);
          break;
        }
      }
      break;
    }
    case LibName: {
      QStringList files = KGlobal::dirs()->findAllResources("data", "ksim/monitors/*.desktop");
      QStringList::ConstIterator it;
      for (it = files.begin(); it != files.end(); ++it) {
        KDesktopFile file((*it));
        if (file.readEntry("X-KSIM-LIBRARY") == name) {
          location = (*it);
          break;
        }
      }
      break;
    }
    case DesktopFile: {
      if (!KDesktopFile::isDesktopFile(name))
      return KSim::PluginInfo();

      location = name;
      break;
    }
  }

  KDesktopFile file(location);
  KSim::PluginInfo info;
  info.m_name = file.readName();
  info.m_libName = file.readEntry("X-KSIM-LIBRARY").local8Bit();
  info.m_location = location;
  return info;
}

KSim::Plugin &KSim::PluginLoader::find(const QCString &libName)
{
  if (libName.isEmpty())
    return KSim::Plugin::null;

  QCString library(libName);
  if (libName.find(Private::ksimString) == -1)
    library.prepend(Private::ksimString);

  KSim::PluginList::Iterator it;
  for (it = d->pluginList.begin(); it != d->pluginList.end(); ++it) {
    if ((*it).libName() == library)
      return (*it);
  }

  return KSim::Plugin::null;
}

const KSim::Plugin &KSim::PluginLoader::find(const QCString &libName) const
{
  if (libName.isEmpty())
    return KSim::Plugin::null;

  QCString library(libName);
  if (libName.find(Private::ksimString) == -1)
    library.prepend(Private::ksimString);

  KSim::PluginList::ConstIterator it;
  for (it = d->pluginList.begin(); it != d->pluginList.end(); ++it) {
    if ((*it).libName() == library)
      return (*it);
  }

  return KSim::Plugin::null;
}

KSim::Plugin &KSim::PluginLoader::find(const KSim::PluginInfo &info)
{
  return find(info.libName());
}

const KSim::Plugin &KSim::PluginLoader::find(const KSim::PluginInfo &info) const
{
  return find(info.libName());
}

const KSim::PluginList &KSim::PluginLoader::pluginList() const
{
  return d->pluginList;
}

KSim::PluginList &KSim::PluginLoader::pluginList()
{
  return d->pluginList;
}

const KSim::Plugin &KSim::PluginLoader::plugin() const
{
  return (d->lastLoaded ? d->pluginList.last() : KSim::Plugin::null);
}

KSim::Plugin &KSim::PluginLoader::plugin()
{
  return (d->lastLoaded ? d->pluginList.last() : KSim::Plugin::null);
}

KSim::PluginLoader::PluginLoader() : QObject(0, "PluginLoader")
{
  d = new KSim::PluginLoader::Private;
  d->lastLoaded = false;
}

void KSim::PluginLoader::cleanup()
{
  if (!m_instance)
    return;

  delete m_instance;
  m_instance = 0;
}

KSim::PluginLoader::ErrorCode KSim::PluginLoader::createPlugin(const KDesktopFile &file)
{
  d->error = QString::null;
  QCString pluginName(file.readEntry("X-KSIM-LIBRARY").local8Bit());
  if (pluginName.isEmpty())
    return EmptyLibName;

  QCString libName(Private::ksimString + pluginName);
  KLibrary *library = KLibLoader::self()->library(libName);
  if (!library)
    return LibNotFound;

  QCString symbol("init_plugin");
  if (Private::PluginPtr *create = (Private::PluginPtr *)(library->symbol(symbol))) {
    d->pluginList.append(KSim::Plugin(create(pluginName), file));
    d->lib = QString::null;
    d->lastLoaded = true;
  }
  else {
    d->error = KLibLoader::self()->lastErrorMessage().isEmpty() ?
       i18n("Unable to get last error message") :
       KLibLoader::self()->lastErrorMessage();

    KLibLoader::self()->unloadLibrary(libName);
    d->lib = pluginName;
    d->lastLoaded = false;
    return UnSymbols;
  }

  emit pluginLoaded(d->pluginList.last());
  return LibLoaded;
}
