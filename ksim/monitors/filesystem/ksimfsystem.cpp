/*  ksim - a system monitor for kde
 *
 *  Copyright (C) 2001  Robbie Ward <linuxphreak@gmx.co.uk>
 *
 *  $Id$
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

#include <config.h>

#include <tqlayout.h>
#include <tqvaluelist.h>
#include <tqtimer.h>
#include <tqfile.h>
#include <tqregexp.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <tdeaboutapplication.h>
#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdeconfig.h>

#include "ksimfsystem.h"
#include "fsystemconfig.h"
#include "filesystemwidget.h"
#include "filesystemstats.h"
#include <themeloader.h>

KSIM_INIT_PLUGIN(PluginModule)

PluginModule::PluginModule(const char *name)
           : KSim::PluginObject(name)
{
  setConfigFileName(instanceName());
}

PluginModule::~PluginModule()
{
}

KSim::PluginView *PluginModule::createView(const char *className)
{
  return new Fsystem(this, className);
}

KSim::PluginPage *PluginModule::createConfigPage(const char *className)
{
  return new FsystemConfig(this, className);
}

void PluginModule::showAbout()
{
  TQString version = kapp->aboutData()->version();

  TDEAboutData aboutData(instanceName(),
     I18N_NOOP("KSim FileSystem Plugin"), version.latin1(),
     I18N_NOOP("A filesystem plugin for KSim"),
     TDEAboutData::License_GPL, "(C) 2001 Robbie Ward");

  aboutData.addAuthor("Robbie Ward", I18N_NOOP("Author"),
     "linuxphreak@gmx.co.uk");
  aboutData.addAuthor("Jason Katz-Brown", I18N_NOOP("Some Fixes"),
     "jason@katzbrown.com");
  aboutData.addAuthor("Heitham Omar", I18N_NOOP("FreeBSD ports"),
     "super_ice@ntlworld.com");

  TDEAboutApplication(&aboutData).exec();
}

Fsystem::Fsystem(KSim::PluginObject *parent, const char *name)
   : DCOPObject("fsystem"),
   KSim::PluginView(parent, name)
{
  config()->setGroup("Fsystem");
  TQVBoxLayout *vbLayout = new TQVBoxLayout(this);
  vbLayout->setAutoAdd(true);

  TQSpacerItem *item = new TQSpacerItem(0, 0, TQSizePolicy::Expanding, TQSizePolicy::Expanding);
  vbLayout->addItem(item);

  m_mountEntries = makeList(config()->readListEntry("mountEntries"));
  m_showPercentage = config()->readBoolEntry("ShowPercentage", true);

  m_widget = new FilesystemWidget(this, "FilesystemWidget");

  createFreeInfo();

  m_updateTimer = new TQTimer(this);
  connect(m_updateTimer, TQT_SIGNAL(timeout()), TQT_SLOT(updateFS()));
  m_updateTimer->start(config()->readNumEntry("updateValue", 60) * 1000);
}

Fsystem::~Fsystem()
{
}

void Fsystem::reparseConfig()
{
  config()->setGroup("Fsystem");

  m_showPercentage = config()->readBoolEntry("ShowPercentage", true);
  MountEntryList mountEntries = makeList(config()->readListEntry("mountEntries"));

  if (m_mountEntries != mountEntries) {
    m_widget->clear();
    m_mountEntries = mountEntries;
    createFreeInfo();
  }

  updateFS();
}

int Fsystem::totalFreeSpace() const
{
  int totalSize, totalFree;
  int totalFreeSpace = 0;
  MountEntryList::ConstIterator it;
  for (it = m_mountEntries.begin(); it != m_mountEntries.end(); ++it) {
    if ( FilesystemStats::readStats( ( *it ).first, totalSize, totalFree ) )
      totalFreeSpace += totalFree;
  }

  return totalFreeSpace;
}

void Fsystem::createFreeInfo()
{
  int total, free;
  int i = 0;
  MountEntryList::ConstIterator it;
  for (it = m_mountEntries.begin(); it != m_mountEntries.end(); ++it) {
    // returns the total space and free space from *it
    // (the current mounted partition)
    if ( !FilesystemStats::readStats( ( *it ).first, total, free ) )
      continue;

    int percent = 0;
	if( total != 0)
		percent = ((total - free) * 100) / total;
    m_widget->append(total, (*it).first);
    m_widget->setValue(i, total - free);
    if (m_showPercentage)
      m_widget->setText(i, ((*it).second.isEmpty() ? (*it).first : (*it).second)
         + " - " + TQString::number(percent) + "%");
    else
      m_widget->setText(i, ((*it).second.isEmpty() ? (*it).first : (*it).second));
    i++;
  }
}

void Fsystem::updateFS()
{
  int total, free;
  int i = 0;
  MountEntryList::ConstIterator it;
  for (it = m_mountEntries.begin(); it != m_mountEntries.end(); ++it) {
    // returns the total space and free space from *it
    // (the current mounted partition)
    if ( !FilesystemStats::readStats( ( *it ).first, total, free ) )
      continue;

    int percent = 0;
	if( total != 0 )
		percent = ((total - free) * 100) / total;
    m_widget->setValue(i, total - free);
    if (m_showPercentage)
      m_widget->setText(i, ((*it).second.isEmpty() ? (*it).first : (*it).second)
         + " - " + TQString::number(percent) + "%");
    else
      m_widget->setText(i, ((*it).second.isEmpty() ? (*it).first : (*it).second));
    i++;
  }
}

Fsystem::MountEntryList Fsystem::makeList(const TQStringList &list) const
{
  MountEntryList newList;
  TQStringList splitList;
  TQStringList::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    splitList = TQStringList::split(":", (*it));
    newList.append(tqMakePair(splitList[0], splitList[1]));
  }

  return newList;
}

#include "ksimfsystem.moc"
