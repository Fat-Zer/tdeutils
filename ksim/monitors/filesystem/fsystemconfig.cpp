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

#include <qcheckbox.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qwhatsthis.h>

class FSysViewItem : public QCheckListItem
{
  public:
    FSysViewItem(QListView *parent, const QString &text1,
       const QString &text2, const QString &text3)
        : QCheckListItem(parent, text1, CheckBox)
    {
      setText(1, text2);
      setText(2, text3);
    }
};

FsystemConfig::FsystemConfig(KSim::PluginObject *parent, const char *name)
   : KSim::PluginPage(parent, name)
{
  m_mainLayout = new QGridLayout(this);
  m_mainLayout->setSpacing(6);

  m_availableMounts = new KListView(this);
  m_availableMounts->addColumn(i18n("Mounted Partition"));
  m_availableMounts->addColumn(i18n("Device"));
  m_availableMounts->addColumn(i18n("Type"));
  m_mainLayout->addMultiCellWidget(m_availableMounts, 0, 0, 0, 3);

  m_showPercentage = new QCheckBox(this);
  m_showPercentage->setText(i18n("Show percentage"));
  m_mainLayout->addMultiCellWidget(m_showPercentage, 1, 1, 0, 3);

  m_splitNames = new QCheckBox(this);
  m_splitNames->setText(i18n("Display short mount point names"));
  QWhatsThis::add(m_splitNames, i18n("This option shortens the text"
     " to shrink down a mount point. E.G: a mount point /home/myuser"
     " would become myuser."));
  m_mainLayout->addMultiCellWidget(m_splitNames, 2, 2, 0, 3);

  m_intervalLabel = new QLabel(this);
  m_intervalLabel->setText( i18n("Update interval:"));
  m_intervalLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
     QSizePolicy::Fixed));
  m_mainLayout->addMultiCellWidget(m_intervalLabel, 3, 3, 0, 0);

  m_updateTimer = new KIntSpinBox(this);
  m_updateTimer->setValue(60);
  QToolTip::add(m_updateTimer, i18n("0 means no update"));
  m_mainLayout->addMultiCellWidget(m_updateTimer, 3, 3, 1, 1);

  QLabel *intervalLabel = new QLabel(this);
  intervalLabel->setText(i18n("seconds"));
  intervalLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
     QSizePolicy::Fixed));
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

  QStringList list = config()->readListEntry("mountEntries");
  for (QListViewItemIterator it(m_availableMounts); it.current(); ++it) {
    QString string = it.current()->text(0) + ":" + splitString(it.current()->text(0));
    static_cast<QCheckListItem *>(it.current())->setOn(list.contains(string) > 0);
  }
}

void FsystemConfig::saveConfig()
{
  config()->setGroup("Fsystem");
  config()->writeEntry("ShowPercentage", m_showPercentage->isChecked());
  config()->writeEntry("updateValue", m_updateTimer->value());
  config()->writeEntry("ShortenEntries", m_splitNames->isChecked());

  QStringList list;
  for (QListViewItemIterator it(m_availableMounts); it.current(); ++it) {
    if (static_cast<QCheckListItem *>(it.current())->isOn())
      list.append(it.current()->text(0) + ":" + splitString(it.current()->text(0)));
  }

  config()->writeEntry("mountEntries", list);
}

void FsystemConfig::showEvent(QShowEvent *)
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
  QStringList list = config()->readListEntry("mountEntries");
  for (QListViewItemIterator it(m_availableMounts); it.current(); ++it) {
    QString string = it.current()->text(0) + ":" + splitString(it.current()->text(0));
    static_cast<QCheckListItem *>(it.current())->setOn(list.contains(string) > 0);
  }
}

QString FsystemConfig::splitString(const QString &string) const
{
  if (string == "/" || !m_splitNames->isChecked())
    return string;

  int location = string.findRev("/");
  QString newString(string);
  return newString.remove(0, location + 1);
}
