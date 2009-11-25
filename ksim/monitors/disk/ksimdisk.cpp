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

#include "ksimdisk.h"
#include "ksimdisk.moc"

#include <qtextstream.h>
#include <qregexp.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qradiobutton.h>
#include <qvbuttongroup.h>
#include <qpushbutton.h>

#include <kdebug.h>
#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kapplication.h>
#include <klistview.h>
#include <kinputdialog.h>
#include <kconfig.h>

#include <chart.h>
#include <progress.h>
#include <themetypes.h>

#if defined(__DragonFly__)
#include <sys/time.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <devstat.h>
#include <stdlib.h>
#elif defined(Q_OS_FREEBSD)
#include <sys/param.h>
#if __FreeBSD_version < 500101
#include <sys/dkstat.h>
#else
#include <sys/resource.h>
#endif
#include <devstat.h>
#include <stdlib.h>
#endif

#ifdef Q_OS_LINUX
#include <linux/major.h>
#endif

#include <iostream>

#define DISK_SPEED 1000

KSIM_INIT_PLUGIN(DiskPlugin)

DiskPlugin::DiskPlugin(const char *name)
   : KSim::PluginObject(name)
{
  setConfigFileName(instanceName());
}

DiskPlugin::~DiskPlugin()
{
}

KSim::PluginView *DiskPlugin::createView(const char *className)
{
  return new DiskView(this, className);
}

KSim::PluginPage *DiskPlugin::createConfigPage(const char *className)
{
  return new DiskConfig(this, className);
}

void DiskPlugin::showAbout()
{
  QString version = kapp->aboutData()->version();

  KAboutData aboutData(instanceName(),
     I18N_NOOP("KSim Disk Plugin"), version.latin1(),
     I18N_NOOP("A disk monitor plugin for KSim"),
     KAboutData::License_GPL, "(C) 2001 Robbie Ward");

  aboutData.addAuthor("Robbie Ward", I18N_NOOP("Author"),
     "linuxphreak@gmx.co.uk");

  KAboutApplication(&aboutData).exec();
}

DiskView::DiskView(KSim::PluginObject *parent, const char *name)
   : KSim::PluginView(parent, name)
{
#ifdef Q_OS_LINUX
  m_bLinux24 = true;
  m_procStream = 0L;
  if ((m_procFile = fopen("/proc/stat", "r")))
    m_procStream = new QTextStream(m_procFile, IO_ReadOnly);
#endif

  config()->setGroup("DiskPlugin");
  m_list = config()->readListEntry("Disks", QStringList() << "complete");
  m_useSeperatly = config()->readBoolEntry("UseSeperatly", true);

  m_firstTime = 1;
  m_addAll = false;
  m_layout = new QVBoxLayout(this);
  QSpacerItem *item = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_layout->addItem(item);

  init();

  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), SLOT(updateDisplay()));
  m_timer->start(DISK_SPEED);
  updateDisplay();
}

DiskView::~DiskView()
{
#ifdef Q_OS_LINUX
  delete m_procStream;

  if (m_procFile)
    fclose(m_procFile);
#endif
}

void DiskView::reparseConfig()
{
  config()->setGroup("DiskPlugin");
  QStringList list = config()->readListEntry("Disks", QStringList() << "complete");
  m_useSeperatly = config()->readBoolEntry("UseSeperatly", true);

  if (list != m_list) {
    m_list = list;
    m_timer->stop();
    cleanup();

    QPtrListIterator<DiskPair> it(m_diskList);
    for (; it.current(); ++it) {
      delete it.current()->first;
      delete it.current()->second;
    }

    m_diskList.clear();
    init();
    m_timer->start(DISK_SPEED);
    updateDisplay();
  }
}

DiskView::DiskData DiskView::findDiskData(const DiskList& diskList, QString diskName)
{
  if (diskName == "complete")
     diskName = i18n("All Disks");

  DiskView::DiskList::ConstIterator disk;
  for (disk = diskList.begin(); disk != diskList.end(); ++disk)
     if ((*disk).name == diskName)
        return *disk;

  // Not found
  DiskView::DiskData dummy;
  dummy.name = "["+diskName+"]";
  return dummy;  
}

// Kind of messy code, dont ya think?
void DiskView::updateDisplay()
{
  DiskList diskList;

  updateData(diskList);

  if (m_addAll)
  {
    DiskData all;
    all.name = i18n("All Disks");

    for (DiskList::ConstIterator disk = diskList.begin(); 
         disk != diskList.end(); ++disk)
    {
      all += (*disk);
    }

    diskList.prepend(all);
  }

  // merge all the disks into one
  QPtrListIterator<DiskPair> it(m_diskList);
  for (int i = 0; it.current(); ++it, ++i) {
    DiskData diskData = findDiskData(diskList, m_list[i]);
    m_data[i].second = m_data[i].first;
    m_data[i].first = diskData;
    diskData -= m_data[i].second;
    unsigned long diff = diskData.readBlocks + diskData.writeBlocks;
    if (m_firstTime)
      diff = diskData.readBlocks = diskData.writeBlocks = 0;

    if (m_useSeperatly) {
      it.current()->first->setValue(diskData.readBlocks, diskData.writeBlocks);
      it.current()->first->setText(i18n("in: %1k")
         .arg(KGlobal::locale()->formatNumber((float)diskData.readBlocks / 1024.0, 1)),
         i18n("out: %1k").arg(KGlobal::locale()->formatNumber((float)diskData.writeBlocks / 1024.0, 1)));
    }
    else {
      it.current()->first->setValue(diff, 0);
      it.current()->first->setText(i18n("%1k")
         .arg(KGlobal::locale()->formatNumber((float)diff / 1024.0, 1)));
    }

    it.current()->second->setMaxValue(it.current()->first->maxValue());
    it.current()->second->setText(diskData.name);
    it.current()->second->setValue(diff);
  }

  m_firstTime = 0;
}

void DiskView::updateData(DiskList &disks)
{
#ifdef Q_OS_LINUX
  if (!m_procStream)
     return;

  m_procStream->device()->reset();
  fseek(m_procFile, 0L, SEEK_SET);

  if (m_bLinux24)
  {
    // here we need a regexp to match something like:
    // (3,0):(108911,48080,1713380,60831,1102644)
    QRegExp regexp("\\([0-9]+,[0-9]+\\):\\([0-9]+,[0-9]+,[0-9]+,[0-9]+,[0-9]+\\)");
    QString content = m_procStream->read();
    if (content.find("disk_io") == -1)
    {
       m_bLinux24 = false;
       
       delete m_procStream;
       m_procStream = 0;
       fclose(m_procFile);
       
       if ((m_procFile = fopen("/proc/diskstats", "r")))
          m_procStream = new QTextStream(m_procFile, IO_ReadOnly);
       
       updateData(disks);
       return;
    }
    int idx = 0;
    while ((idx = regexp.search(content, idx)) != -1)
    {
      idx += regexp.matchedLength();
      QString diskStr = regexp.cap(0);
      diskStr.replace(':', ',');
      diskStr.replace(QRegExp("\\)?\\(?"), QString::null);

      QStringList list = QStringList::split(',', diskStr);
      if (list.count() < 7)
        continue;

      DiskData diskData;
      diskData.major = list[0].toInt();
      diskData.minor = list[1].toInt();
      diskData.name = diskName( diskData.major, diskData.minor );
      diskData.total = list[2].toULong();
      diskData.readIO = list[3].toULong();
      diskData.readBlocks = list[4].toULong();
      diskData.writeIO = list[5].toULong();
      diskData.writeBlocks = list[6].toULong();
      disks.append(diskData);
    }
  }
  else
  {
    // 3    0 hda 564142 160009 14123957 12369403 1052983 2801992 30905928 78981451 0 4531584 91518334
    // The 11 fields after the device name are defined as follows:
    // Field  1 -- # of reads issued
    //    This is the total number of reads completed successfully.
    // Field  2 -- # of reads merged, 
    //    Reads and writes which are adjacent to each other may be merged for
    //    efficiency.  Thus two 4K reads may become one 8K read before it is
    //    ultimately handed to the disk, and so it will be counted (and queued)
    //    as only one I/O.  This field lets you know how often this was done.
    // Field  3 -- # of sectors read
    //    This is the total number of sectors read successfully.
    // Field  4 -- # of milliseconds spent reading
    //    This is the total number of milliseconds spent by all reads (as
    //    measured from __make_request() to end_that_request_last()).
    // Field  5 -- # of writes completed
    //    This is the total number of writes completed successfully.
    // Field  6 -- # of writes merged
    //    See field 2
    // Field  7 -- # of sectors written
    //    This is the total number of sectors written successfully.
    // Field  8 -- # of milliseconds spent writing
    //    This is the total number of milliseconds spent by all writes (as
    //    measured from __make_request() to end_that_request_last()).
    // Field  9 -- # of I/Os currently in progress
    //    The only field that should go to zero. Incremented as requests are
    //    given to appropriate request_queue_t and decremented as they finish.
    // Field 10 -- # of milliseconds spent doing I/Os
    //    This field is increases so long as field 9 is nonzero.
    // Field 11 -- weighted # of milliseconds spent doing I/Os
    //    This field is incremented at each I/O start, I/O completion, I/O
    //    merge, or read of these stats by the number of I/Os in progress
    //    (field 9) times the number of milliseconds spent doing I/O since the
    //    last update of this field.  This can provide an easy measure of both
    //    I/O completion time and the backlog that may be accumulating.
    QString content = m_procStream->read();
    QStringList lines = QStringList::split('\n', content);

    for(QStringList::ConstIterator it = lines.begin();
        it != lines.end(); ++it)
    {
      QString diskStr = (*it).simplifyWhiteSpace();
      QStringList list = QStringList::split(' ', diskStr);
      if (list.count() < 14)
        continue;

      DiskData diskData;
      diskData.major = list[0].toInt();
      diskData.minor = list[1].toInt();
      diskData.name = list[2];
      diskData.readIO = 0;
      diskData.readBlocks = list[3+2].toULong();
      diskData.writeIO = 0;
      diskData.writeBlocks = list[7+2].toULong();
      diskData.total = diskData.readBlocks + diskData.writeBlocks;
      disks.append(diskData);
    }
  }

#endif

#ifdef Q_OS_FREEBSD
#if defined(__DragonFly__) || __FreeBSD_version < 500107
#define	devstat_getdevs(fd, stats)	getdevs(stats)
#define	devstat_selectdevs		selectdevs
#define	bytes_read(dev)			(dev).bytes_read
#define	bytes_written(dev)		(dev).bytes_written
#else
#define	bytes_read(dev)			(dev).bytes[DEVSTAT_READ]
#define	bytes_written(dev)		(dev).bytes[DEVSTAT_WRITE]
#endif

  statinfo diskStats;
  bzero(&diskStats, sizeof(diskStats));
  diskStats.dinfo = (struct devinfo *)malloc(sizeof(struct devinfo));
  bzero(diskStats.dinfo, sizeof(struct devinfo));
  int deviceAmount;
  int selected;
  int selections;
  long generation;
  device_selection *deviceSelect = 0;

  if (devstat_getdevs(NULL, &diskStats) < 0)
    return;

  deviceAmount = diskStats.dinfo->numdevs;
  if (devstat_selectdevs(&deviceSelect, &selected, &selections,
     &generation, diskStats.dinfo->generation,
     diskStats.dinfo->devices, deviceAmount,
     0, 0, 0, 0, DS_SELECT_ONLY, 10, 1) < 0)
    return;

  unsigned long readBlocks = 0, writeBlocks = 0, blockSize;
  for (int i = 0; i < deviceAmount; ++i) {
    int disk;
    devstat device;
    disk = deviceSelect[i].position;
    device = diskStats.dinfo->devices[disk];
    blockSize = (device.block_size <= 0 ? 512 : device.block_size);
    readBlocks = bytes_read(device) / blockSize;
    writeBlocks = bytes_written(device) / blockSize;

    DiskData diskData;
    diskData.name = device.device_name
       + QString::number(device.unit_number);
    diskData.major = device.device_number;
    diskData.minor = 0;
    diskData.total = readBlocks + writeBlocks;
    diskData.readIO = 0;
    diskData.readBlocks = readBlocks;
    diskData.writeIO = 0;
    diskData.writeBlocks = writeBlocks;
    disks.append(diskData);
  }

  free(deviceSelect);
  free(diskStats.dinfo);
#endif
}

QString DiskView::diskName( int major, int minor ) const
{
#ifdef Q_OS_LINUX
  QString returnValue;
  switch ( major )
  {
    case IDE0_MAJOR:
      returnValue.prepend(QString::fromLatin1("hda"));
      break;
    case IDE1_MAJOR:
      returnValue.prepend(QString::fromLatin1("hdc"));
      break;
    case IDE3_MAJOR:
      returnValue.prepend(QString::fromLatin1("hde"));
      break;
    case SCSI_DISK0_MAJOR:
      returnValue.prepend(QString::fromLatin1("sda"));
      break;
    case SCSI_GENERIC_MAJOR:
      returnValue.prepend(QString::fromLatin1("sg0"));
      break;
  }

  returnValue.at(2) = returnValue.at(2).latin1() + minor;
  return returnValue;
#else
  Q_UNUSED(major);
  Q_UNUSED(minor);
  return 0;
#endif
}

DiskView::DiskPair *DiskView::addDisk()
{
  KSim::Chart *chart = new KSim::Chart(false, 0, this);
  chart->show();
  m_layout->addWidget(chart);

  KSim::Progress *progress = new KSim::Progress(0, KSim::Types::None,
     KSim::Progress::Panel, this);
  progress->show();
  m_layout->addWidget(progress);

  return new DiskPair(chart, progress);
}

void DiskView::init()
{
  m_data.resize(m_list.size());

  QStringList::ConstIterator it;
  for (it = m_list.begin(); it != m_list.end(); ++it) {
    if ((*it) == "complete")
      m_addAll = true;

    m_diskList.append(addDisk());
  }
}

void DiskView::cleanup()
{
  m_data.clear();
  m_addAll = false;
}

DiskConfig::DiskConfig(KSim::PluginObject *parent, const char *name)
   : KSim::PluginPage(parent, name)
{
  m_layout = new QVBoxLayout(this);
  m_layout->setSpacing(6);

  m_listview = new KListView(this);
  m_listview->addColumn(i18n("Disks"));
  m_layout->addWidget(m_listview);

  QHBoxLayout *layout = new QHBoxLayout;
  layout->setSpacing(6);

  QSpacerItem *spacer = new QSpacerItem(20, 20,
     QSizePolicy::Expanding, QSizePolicy::Minimum);
  layout->addItem(spacer);

  m_add = new QPushButton(this);
  m_add->setText(i18n("Add..."));
  connect(m_add, SIGNAL(clicked()), SLOT(addItem()));
  layout->addWidget(m_add);

  m_remove = new QPushButton(this);
  m_remove->setText(i18n("Remove"));
  connect(m_remove, SIGNAL(clicked()), SLOT(removeItem()));
  layout->addWidget(m_remove);
  m_layout->addLayout(layout);

  m_buttonBox = new QVButtonGroup(i18n("Disk Styles"), this);
  m_layout->addWidget(m_buttonBox);

  m_totalButton = new QRadioButton(m_buttonBox);
  m_totalButton->setText(i18n("Display the read and write data as one"));
  m_bothButton = new QRadioButton(m_buttonBox);
  m_bothButton->setText(i18n("Display the read and write data"
     "\nseparately as in/out data"));

  QSpacerItem *vSpacer = new QSpacerItem(20, 20,
     QSizePolicy::Minimum, QSizePolicy::Expanding);
  m_layout->addItem(vSpacer);
}

DiskConfig::~DiskConfig()
{
}

void DiskConfig::readConfig()
{
  config()->setGroup("DiskPlugin");
  m_buttonBox->setButton(config()->readBoolEntry("UseSeperatly", true));
  QStringList list = config()->readListEntry("Disks");

  QStringList::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    QString text = ((*it) == "complete" ? i18n("All Disks") : (*it));
    if (!m_listview->findItem(text, 0))
      new QListViewItem(m_listview, text);
  }
}

void DiskConfig::saveConfig()
{
  QStringList list;
  for (QListViewItemIterator it(m_listview); it.current(); ++it) {
    if (it.current()->text(0) == i18n("All Disks"))
      list.append("complete");
    else
      list.append(it.current()->text(0));
  }

  config()->setGroup("DiskPlugin");
  config()->writeEntry("UseSeperatly", (bool)m_buttonBox->id(m_buttonBox->selected()));
  config()->writeEntry("Disks", list);
}

void DiskConfig::addItem()
{
  bool ok = false;
  QString text = KInputDialog::getText(i18n("Add Disk Device"), i18n("Disk name:"),
     QString::null, &ok, this);
  
  if (text.startsWith("/dev/"))
    text = text.mid(5);

  if (ok)
    new QListViewItem(m_listview, text);
}

void DiskConfig::removeItem()
{
  if (!m_listview->selectedItem())
    return;

  QListViewItem *item = m_listview->selectedItem();
  delete item;
}
