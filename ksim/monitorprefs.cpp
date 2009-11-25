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

#include "monitorprefs.h"
#include "monitorprefs.moc"

#include <klocale.h>
#include <kglobal.h>
#include <kdesktopfile.h>
#include <kstandarddirs.h>
#include <kdebug.h>

#include <ksimconfig.h>
#include <pluginloader.h>

KSim::MonitorPrefs::MonitorPrefs(QWidget *parent, const char *name)
    : KListView(parent, name)
{
  addColumn(i18n("Monitor"));
  addColumn(i18n("Description"));
  addColumn(i18n("LMB Command"));

  setItemsRenameable(true);
  setRenameable(0, false);
  setRenameable(2, true);
  setDragEnabled(true);
  setAcceptDrops(true);
  setAllColumnsShowFocus(true);
  setSelectionMode(QListView::Single);

  // Scan for .desktop files and enter them into the KListView
  m_locatedFiles = KGlobal::dirs()->findAllResources("data", "ksim/monitors/*.desktop");
  m_locatedFiles.sort();
  QStringList::ConstIterator it;
  for (it = m_locatedFiles.begin(); it != m_locatedFiles.end(); ++it) {
    KDesktopFile file((*it), true);
    (void)(new QCheckListItem(this, file.readName(), QCheckListItem::CheckBox))->setText(1, file.readComment());
  }

  sort();
  setSorting(-1, false);
}

KSim::MonitorPrefs::~MonitorPrefs()
{
}

void KSim::MonitorPrefs::saveConfig(KSim::Config *config)
{
  QCheckListItem *item;
  QStringList::ConstIterator it;
  for (it = m_locatedFiles.begin(); it != m_locatedFiles.end(); ++it) {
    KSim::PluginInfo info = KSim::PluginLoader::self().findPluginInfo((*it));
    item = static_cast<QCheckListItem *>(findItem(info.name(), 0));
    config->setEnabledMonitor(info.libName(), item->isOn());
    config->setMonitorCommand(info.libName(), item->text(2));
    config->setMonitorLocation(info.libName(), itemIndex(item));
  }
}

void KSim::MonitorPrefs::readConfig(KSim::Config *config)
{
  int location;
  QCheckListItem *origItem;
  QStringList::ConstIterator it;
  for (it = m_locatedFiles.begin(); it != m_locatedFiles.end(); ++it) {
    KSim::PluginInfo info = KSim::PluginLoader::self().findPluginInfo((*it));
    location = config->monitorLocation(info.libName());
    origItem = static_cast<QCheckListItem *>(findItem(info.name(), 0));
    origItem->setOn(config->enabledMonitor(info.libName()));
    origItem->setText(2, config->monitorCommand(info.libName()));
    if (QListViewItem *item = itemAtIndex(location)) {
      if (location) {
        origItem->moveItem(item->itemAbove());
      }
      else {
        origItem->moveItem(firstChild());
        firstChild()->moveItem(origItem);
      }
    }
  }
}
