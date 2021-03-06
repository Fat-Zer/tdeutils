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

#include "ksimpref.h"
#include "ksimpref.moc"

#include "generalprefs.h"
#include "monitorprefs.h"
#include "themeprefs.h"
#include <pluginloader.h>
#include <ksimconfig.h>
#include <pluginmodule.h>

#include <tdelocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <tdemessagebox.h>

#include <tqhbox.h>
#include <tqframe.h>

KSim::ConfigDialog::ConfigDialog(KSim::Config *config,
   TQWidget *parent, const char *name)
   : KDialogBase(TreeList, i18n("Configure"),
   Help | Ok | Apply | Close, Ok, parent, name, true, true)
{
  resize(466, 363);
  setShowIconsInTreeList(true);
  setRootIsDecorated(false);

  m_config = config;

  setFolderIcon(TQStringList(' ' + i18n("Plugins")),
     BarIcon("folder", TDEIcon::SizeSmall));
  setFolderIcon(TQStringList(' ' + i18n("Miscellaneous")),
     BarIcon("misc", TDEIcon::SizeSmall));

  TQFrame *monitors = addHBoxPage(' ' + i18n("Monitors"),
     i18n("Monitors Installed"), BarIcon("ksim", TDEIcon::SizeSmall));
  m_monPage = new KSim::MonitorPrefs(monitors);

  TQStringList list;
  list << ' ' + i18n("Miscellaneous") << ' ' + i18n("General");
  TQFrame *general = addHBoxPage(list, i18n("General Options"),
     SmallIcon("misc"));
  m_generalPage = new KSim::GeneralPrefs(general);
  list.clear();

  list << ' ' + i18n("Miscellaneous") << ' ' + i18n("Clock");
  TQFrame *clock = addHBoxPage(list, i18n("Clock Options"),
     TQPixmap(locate("data", "ksim/pics/clock.png")));
  m_clockPage = new KSim::ClockPrefs(clock);
  list.clear();

  list << ' ' + i18n("Miscellaneous") << ' ' + i18n("Uptime");
  TQFrame *uptime = addHBoxPage(list, i18n("Uptime Options"),
     TQPixmap(locate("data", "ksim/pics/uptime.png")));
  m_uptimePage = new KSim::UptimePrefs(uptime);
  list.clear();

  list << ' ' + i18n("Miscellaneous") << ' ' + i18n("Memory");
  TQFrame *memory = addHBoxPage(list, i18n("Memory Options"),
     SmallIcon("memory"));
  m_memoryPage = new KSim::MemoryPrefs(memory);
  list.clear();

  list << ' ' + i18n("Miscellaneous") << ' ' + i18n("Swap");
  TQFrame *swap = addHBoxPage(list, i18n("Swap Options"),
     SmallIcon("hdd_unmount"));
  m_swapPage = new KSim::SwapPrefs(swap);

  TQFrame *theme = addHBoxPage(' ' + i18n("Themes"),
     i18n("Theme Selector"), BarIcon("folder_image", TDEIcon::SizeSmall));
  m_themePage = new KSim::ThemePrefs(theme);

  connect(this, TQT_SIGNAL(applyClicked()), TQT_SLOT(savePrefs()));
  connect(this, TQT_SIGNAL(okClicked()), TQT_SLOT(closePrefs()));
  connect(this, TQT_SIGNAL(closeClicked()), TQT_SLOT(loadPluginConfig()));
  connect(parent, TQT_SIGNAL(reload()), TQT_SLOT(reload()));

  KSim::PluginList::ConstIterator it;
  const KSim::PluginList &pluginList = KSim::PluginLoader::self().pluginList();
  for (it = pluginList.begin(); it != pluginList.end(); ++it)
    createPage((*it));

  readConfig();
}

KSim::ConfigDialog::~ConfigDialog()
{
  ChangedPluginList::ConstIterator it;
  for (it = m_currentPlugins.begin(); it != m_currentPlugins.end(); ++it) {
    if ((*it).isEnabled())
      removePage((*it).libName());
  }
}

void KSim::ConfigDialog::reload()
{
  enableButtons();
}

void KSim::ConfigDialog::removePage(const TQCString &name)
{
  const KSim::Plugin &plugin = KSim::PluginLoader::self().find(name);
  if (plugin.isNull() || !plugin.configPage()) {
    KMessageBox::sorry(0, i18n("Failed to remove %1's config page due to the "
       "plugin not being loaded or the config page has not been created").arg(name.data()));
    return;
  }

  TQWidget *frame = plugin.configPage()->parentWidget();
  // reparent the widget if the parent is not null
  if (frame) {
    plugin.configPage()->hide();
    plugin.configPage()->reparent(0, TQPoint(0, 0), false);
    // delete the frame so it removes the page from the config dialog
    delete frame;
    kdDebug(2003) << "removing " << name << " from KSimPref" << endl;
  }
}

void KSim::ConfigDialog::createPage(const TQCString &name)
{
  const KSim::Plugin &plugin = KSim::PluginLoader::self().find(name);
  createPage(plugin);
}

void KSim::ConfigDialog::createPage(const KSim::Plugin &plugin)
{
  if (plugin.isNull() || !plugin.configPage()) {
    KMessageBox::sorry(0, i18n("Failed to add %1's config page due to the plugin"
       " not being loaded or the config page has not been created")
       .arg(plugin.libName().data()));
    return;
  }

  kdDebug(2003) << "adding " << plugin.libName() << " to KSimPref" << endl;

  TQStringList list;
  list << ' ' + i18n("Plugins") << ' ' + plugin.name();
  TQFrame *pluginFrame = addHBoxPage(list, i18n("%1 Options").arg(plugin.name()),
     plugin.icon());

  plugin.configPage()->reparent(pluginFrame, TQPoint(0, 0), true);
  plugin.configPage()->readConfig();

// TODO: implement this correctly one day
//  connect (info.configPage(), TQT_SIGNAL(pageChanged()),
//     this, TQT_SLOT(enableButtons()));
}

void KSim::ConfigDialog::savePrefs()
{
  saveConfig(true);
}

void KSim::ConfigDialog::saveConfig(bool reload)
{
  disableButtons();

  m_monPage->saveConfig(m_config);
  m_generalPage->saveConfig(m_config);
  m_clockPage->saveConfig(m_config);
  m_uptimePage->saveConfig(m_config);
  m_memoryPage->saveConfig(m_config);
  m_swapPage->saveConfig(m_config);
  m_themePage->saveConfig(m_config);

  ChangedPluginList changedPlugins;
  for (TQListViewItemIterator it(m_monPage); it.current(); ++it) {
    TQCheckListItem *item = static_cast<TQCheckListItem *>(it.current());
    KSim::PluginInfo info = KSim::PluginLoader::self().findPluginInfo(item->text(0),
       KSim::PluginLoader::Name);
    changedPlugins.append(ChangedPlugin(item->isOn(),
       info.libName(true), item->text(0), info.location(),
       findPlugin(item->text(0)).isEnabled()));

    KSim::PluginLoader::self().find(info).setEnabled(item->isOn());
  }

  m_currentPlugins = changedPlugins;
  emit reparse(reload, changedPlugins);
}

void KSim::ConfigDialog::readConfig()
{
  m_monPage->readConfig(m_config);
  m_generalPage->readConfig(m_config);
  m_clockPage->readConfig(m_config);
  m_uptimePage->readConfig(m_config);
  m_memoryPage->readConfig(m_config);
  m_swapPage->readConfig(m_config);
  m_themePage->readConfig(m_config);

  m_currentPlugins.clear();
  for (TQListViewItemIterator it(m_monPage); it.current(); ++it) {
    TQCheckListItem *item = static_cast<TQCheckListItem *>(it.current());
    KSim::PluginInfo info = KSim::PluginLoader::self().findPluginInfo(item->text(0),
       KSim::PluginLoader::Name);
    m_currentPlugins.append(ChangedPlugin(item->isOn(),
       info.libName(true), item->text(0), info.location()));
  }
}

void KSim::ConfigDialog::closePrefs()
{
  saveConfig(false);
  close();
}

void KSim::ConfigDialog::loadPluginConfig()
{
  const KSim::PluginList &list = KSim::PluginLoader::self().pluginList();
  KSim::PluginList::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    if ((*it).configPage())
      (*it).configPage()->readConfig();
  }
}

void KSim::ConfigDialog::enableButtons()
{
  enableButtonApply(true);
  enableButtonOK(true);
}

void KSim::ConfigDialog::disableButtons()
{
  enableButtonApply(false);
  enableButtonOK(false);
}

const KSim::ChangedPlugin &KSim::ConfigDialog::findPlugin(const TQString &name) const
{
  ChangedPluginList::ConstIterator it;
  for (it = m_currentPlugins.begin(); it != m_currentPlugins.end(); ++it) {
    if ((*it).name() == name)
      return (*it);
  }

  return *m_currentPlugins.end();
}
