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

#include "fsystemconfig.h"
#include "fsystemconfig.moc"

#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <klistview.h>
#include <kapplication.h>
#include <knuminput.h>
#include <kdebug.h>

#include <tqcheckbox.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqlabel.h>
#include <tqwhatsthis.h>

class FSysViewItem : public TQCheckListItem
{
  public:
    FSysViewItem(TQListView *parent, const TQString &text1,
       const TQString &text2, const TQString &text3)
        : TQCheckListItem(parent, text1, CheckBox)
    {
      setText(1, text2);
      setText(2, text3);
    }
};

FsystemConfig::FsystemConfig(KSim::PluginObject *parent, const char *name)
   : KSim::PluginPage(parent, name)
{
  m_mainLayout = new TQGridLayout(this);
  m_mainLayout->setSpacing(6);

  m_availableMounts = new KListView(this);
  m_availableMounts->addColumn(i18n("Mounted Partition"));
  m_availableMounts->addColumn(i18n("Device"));
  m_availableMounts->addColumn(i18n("Type"));
  m_mainLayout->addMultiCellWidget(m_availableMounts, 0, 0, 0, 3);

  m_showPercentage = new TQCheckBox(this);
  m_showPercentage->setText(i18n("Show percentage"));
  m_mainLayout->addMultiCellWidget(m_showPercentage, 1, 1, 0, 3);

  m_splitNames = new TQCheckBox(this);
  m_splitNames->setText(i18n("Display short mount point names"));
  TQWhatsThis::add(m_splitNames, i18n("This option shortens the text"
     " to shrink down a mount point. E.G: a mount point /home/myuser"
     " would become myuser."));
  m_mainLayout->addMultiCellWidget(m_splitNames, 2, 2, 0, 3);

  m_intervalLabel = new TQLabel(this);
  m_intervalLabel->setText( i18n("Update interval:"));
  m_intervalLabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed,
     TQSizePolicy::Fixed));
  m_mainLayout->addMultiCellWidget(m_intervalLabel, 3, 3, 0, 0);

  m_updateTimer = new KIntSpinBox(this);
  m_updateTimer->setValue(60);
  TQToolTip::add(m_updateTimer, i18n("0 means no update"));
  m_mainLayout->addMultiCellWidget(m_updateTimer, 3, 3, 1, 1);

  TQLabel *intervalLabel = new TQLabel(this);
  intervalLabel->setText(i18n("seconds"));
  intervalLabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed,
     TQSizePolicy::Fixed));
  m_mainLayout->addMultiCellWidget(intervalLabel, 3, 3, 2, 2);

  m_entries = FilesystemStats::readEntries();
  getStats();
}

FsystemConfig::~FsystemConfig()
{
}

void FsystemConfig::readConfig()
{
  config()->setGroup("Fsystem");
  m_showPercentage->setChecked(config()->readBoolEntry("ShowPercentage", true));
  m_updateTimer->setValue(config()->readNumEntry("updateValue", 60));
  m_splitNames->setChecked(config()->readBoolEntry("ShortenEntries", false));

  if (!m_availableMounts->childCount())
    return;

  TQStringList list = config()->readListEntry("mountEntries");
  for (TQListViewItemIterator it(m_availableMounts); it.current(); ++it) {
    TQString string = it.current()->text(0) + ":" + splitString(it.current()->text(0));
    static_cast<TQCheckListItem *>(it.current())->setOn(list.contains(string) > 0);
  }
}

void FsystemConfig::saveConfig()
{
  config()->setGroup("Fsystem");
  config()->writeEntry("ShowPercentage", m_showPercentage->isChecked());
  config()->writeEntry("updateValue", m_updateTimer->value());
  config()->writeEntry("ShortenEntries", m_splitNames->isChecked());

  TQStringList list;
  for (TQListViewItemIterator it(m_availableMounts); it.current(); ++it) {
    if (static_cast<TQCheckListItem *>(it.current())->isOn())
      list.append(it.current()->text(0) + ":" + splitString(it.current()->text(0)));
  }

  config()->writeEntry("mountEntries", list);
}

void FsystemConfig::showEvent(TQShowEvent *)
{
  // FIXME: Maybe this is the slow method of doing this?
  // Eitherway, i need to find a way to only update the list
  // if the amount of mounted partitions has changed
  FilesystemStats::List entries = FilesystemStats::readEntries();
  if ( entries.count() == m_entries.count() )
    return;

  m_entries = entries;

  // Update the entries to take into account
  // any mounted/unmounted filesystems
  m_availableMounts->clear();
  getStats();
}

void FsystemConfig::getStats()
{
  int total = 0;
  int free = 0;

  FilesystemStats::List::ConstIterator it;
  for ( it = m_entries.begin(); it != m_entries.end(); ++it )
  {
    if ( !FilesystemStats::readStats( ( *it ).dir, total, free ) )
      continue;

    if ( !m_availableMounts->findItem( ( *it ).dir, 0 ) )
    {
      (void) new FSysViewItem( m_availableMounts, ( *it ).dir,
         ( *it ).fsname, ( *it ).type );
    }
  }

  if (!m_availableMounts->childCount())
    return;

  config()->setGroup("Fsystem");
  TQStringList list = config()->readListEntry("mountEntries");
  for (TQListViewItemIterator it(m_availableMounts); it.current(); ++it) {
    TQString string = it.current()->text(0) + ":" + splitString(it.current()->text(0));
    static_cast<TQCheckListItem *>(it.current())->setOn(list.contains(string) > 0);
  }
}

TQString FsystemConfig::splitString(const TQString &string) const
{
  if (string == "/" || !m_splitNames->isChecked())
    return string;

  int location = string.findRev("/");
  TQString newString(string);
  return newString.remove(0, location + 1);
}
