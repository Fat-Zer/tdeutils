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

#include "ksimcpu.h"
#include "ksimcpu.moc"

#include <qtextstream.h>
#include <qregexp.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qfile.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qglobal.h>
#include <qpushbutton.h>

#include <klistview.h>
#include <kdebug.h>
#include <kaboutapplication.h>
#include <kaboutdata.h>
#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <kapplication.h>
#include <kinputdialog.h>

#include <themetypes.h>

#ifdef Q_OS_BSD4
#include <sys/param.h>
#if defined(__DragonFly__)
#include <kinfo.h>
#include <err.h>
#include <sys/time.h>
#include <sys/resource.h>

#elif defined Q_OS_FREEBSD
#if __FreeBSD_version < 500101
#include <sys/dkstat.h>
#else
#include <sys/resource.h>
#endif
#else
#include <sys/dkstat.h>
#endif
#include <sys/sysctl.h>
#include <string.h>
#include <kvm.h>
#ifdef Q_OS_NETBSD
#include <sys/sched.h>
#endif
#endif

#define CPU_NAME(cpu) "Cpu" + QString::number(cpu) + "_options"
#define CPU_SPEED 1000

KSIM_INIT_PLUGIN(CpuPlugin)

CpuPlugin::CpuPlugin(const char *name)
   : KSim::PluginObject(name)
{
  setConfigFileName(instanceName());
}

CpuPlugin::~CpuPlugin()
{
}

KSim::PluginView *CpuPlugin::createView(const char *className)
{
  return new CpuView(this, className);
}

KSim::PluginPage *CpuPlugin::createConfigPage(const char *className)
{
  return new CpuConfig(this, className);
}

void CpuPlugin::showAbout()
{
  QString version = kapp->aboutData()->version();

  KAboutData aboutData(instanceName(),
     I18N_NOOP("KSim CPU Plugin"), version.latin1(),
     I18N_NOOP("A cpu monitor plugin for KSim"),
     KAboutData::License_GPL, "(C) 2001 Robbie Ward");

  aboutData.addAuthor("Robbie Ward", I18N_NOOP("Author"),
     "linuxphreak@gmx.co.uk");

  KAboutApplication(&aboutData).exec();
}

CpuView::CpuView(KSim::PluginObject *parent, const char *name)
   : KSim::PluginView(parent, name)
{
#ifdef Q_OS_LINUX
  m_procStream = 0L;
  if ((m_procFile = fopen("/proc/stat", "r")))
    m_procStream = new QTextStream(m_procFile, IO_ReadOnly);
#endif

  m_mainLayout = new QVBoxLayout(this);
  QSpacerItem *item = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
  m_mainLayout->addItem(item);
  
  m_firstTime = true;

  m_cpus = createList();
  addDisplay();

  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), SLOT(updateView()));
  m_timer->start(CPU_SPEED);
  updateView();
}

CpuView::~CpuView()
{
#ifdef Q_OS_LINUX
  delete m_procStream;

  if (m_procFile)
    fclose(m_procFile);
#endif

  cleanup(m_cpus);
}

void CpuView::reparseConfig()
{
  CpuList cpus = createList();
  if (m_cpus != cpus) {
    if (m_timer->isActive())
      m_timer->stop();

    cleanup(m_cpus);
    m_cpus = cpus;
    addDisplay();
    m_timer->start(CPU_SPEED);
  }
}

void CpuView::updateView()
{
  if (m_cpus.isEmpty())
    return;

  CpuList::Iterator it;
  for (it = m_cpus.begin(); it != m_cpus.end(); ++it) {
    Cpu &current = (*it);
    CpuData cpuData;
    updateCpu(cpuData, current.number());

    QString text = current.format();
    current.setData(cpuData);
    cpuData -= current.oldData();
    int cpuDiff = 0;
    int total = cpuData.sys + cpuData.user + cpuData.nice + cpuData.idle;

//    kdDebug(2003) << "name = " << cpuData.name << endl;
//    kdDebug(2003) << "user = " << cpuData.user << endl;
//    kdDebug(2003) << "nice = " << cpuData.nice << endl;
//    kdDebug(2003) << "sys = " << cpuData.sys << endl;
//    kdDebug(2003) << "idle = " << cpuData.idle << endl;
    
    if (!m_firstTime) {
      if (text.find("%T") != -1)
        cpuDiff = cpuData.sys + cpuData.user + cpuData.nice;
      else if (text.find("%t") != -1)
        cpuDiff = cpuData.sys + cpuData.user;
      else if (text.find("%s") != -1)
        cpuDiff = cpuData.sys;
      else if (text.find("%u") != -1)
        cpuDiff = cpuData.user;
      else if (text.find("%n") != -1)
        cpuDiff = cpuData.nice;

      cpuDiff *= 100;
	  
	  if ( total > 0 )
		  cpuDiff /= total;

      if (cpuDiff > 100)
        cpuDiff = 100;
    }

    current.chart()->setText(i18n("%1%").arg(cpuDiff));
    current.chart()->setValue(cpuDiff, 0);
    current.label()->setValue(cpuDiff);
  }

  m_firstTime = false;
}

void CpuView::updateCpu(CpuData &cpu, int cpuNumber)
{
#ifdef Q_OS_LINUX
  if (!m_procStream)
    return;

  bool cpuFound = false;
  QString output;
  QString parser;
  QString cpuString;
  cpuString.setNum(cpuNumber).prepend("cpu");

  // Parse the proc file
  while (!m_procStream->atEnd()) {
    parser = m_procStream->readLine();
    // remove all the entries apart from the line containing 'cpuString'
    if (!cpuFound && parser.find(QRegExp(cpuString)) != -1) {
      output = parser;
      cpuFound = true;
    }
  }

  QStringList cpuList = QStringList::split(' ', output);

  if (!cpuList.isEmpty()) {
    cpu.name = cpuList[0].stripWhiteSpace();
    cpu.user = cpuList[1].toULong();
    cpu.nice = cpuList[2].toULong();
    cpu.sys = cpuList[3].toULong();
    cpu.idle = cpuList[4].toULong();
  }

  fseek(m_procFile, 0L, SEEK_SET);
#endif

#ifdef __DragonFly__
  struct kinfo_cputime cp_time;
  if (kinfo_get_sched_cputime(&cp_time))
    err(1, "kinfo_get_sched_cputime");
  cpu.user = cp_time.cp_user;
  cpu.nice = cp_time.cp_nice;
  cpu.sys = cp_time.cp_sys;
  cpu.idle = cp_time.cp_idle;
#elif defined(Q_OS_FREEBSD)
#warning "add support for SMP on FreeBSD"
  Q_UNUSED(cpuNumber);
  static int name2oid[2] = { 0, 3 };
  static int oidCpuTime[CTL_MAXNAME + 2];
  static size_t oidCpuTimeLen = sizeof(oidCpuTime);
  long cpuTime[CPUSTATES];
  size_t cpuTimeLen = sizeof(cpuTime);
  static char name[] = "kern.cp_time";
  static int initialized = 0;

  if (!initialized) {
    if (sysctl(name2oid, 2, oidCpuTime, &oidCpuTimeLen,
       name, strlen(name)) < 0)
      return;

    oidCpuTimeLen /= sizeof(int);
    initialized = 1;
  }

  if (sysctl(oidCpuTime, oidCpuTimeLen,
     cpuTime, &cpuTimeLen, 0, 0) < 0)
    return;

  cpu.user = cpuTime[CP_USER];
  cpu.nice = cpuTime[CP_NICE];
  cpu.sys = cpuTime[CP_SYS];
  cpu.idle = cpuTime[CP_IDLE];
#endif

#if defined(Q_OS_NETBSD)
#define KERN_CPTIME KERN_CP_TIME
#endif
#if defined(Q_OS_OPENBSD) || defined(Q_OS_NETBSD)
//#warning "add support for SMP on OpenBSD and NetBSD"
  int name2oid[2] = { CTL_KERN, KERN_CPTIME };
  long cpuTime[CPUSTATES];
  size_t cpuTimeLen = sizeof(cpuTime);

  if (sysctl(name2oid, 2, &cpuTime, &cpuTimeLen,
     0, 0) < 0)
    return;

  cpu.user = cpuTime[CP_USER];
  cpu.nice = cpuTime[CP_NICE];
  cpu.sys = cpuTime[CP_SYS];
  cpu.idle = cpuTime[CP_IDLE];
#endif
}

CpuView::CpuList CpuView::createList()
{
  config()->setGroup("CpuPlugin");
  CpuList list;

  int number = 0;
  QStringList cpus = config()->readListEntry("Cpus");
  QStringList::ConstIterator it;
  for (it = cpus.begin(); it != cpus.end(); ++it) {
    list.append(Cpu((*it), config()->readEntry(CPU_NAME(number),
       "%T"), number));

    ++number;
  }

  return list;
}

void CpuView::addDisplay()
{
  CpuList::Iterator it;
  for (it = m_cpus.begin(); it != m_cpus.end(); ++it) {
    KSim::Progress *progress = addLabel();
    KSim::Chart *chart = addChart();
    //KSim::Progress *progress = addLabel();

    (*it).setDisplay(chart, progress);
  }
}

void CpuView::cleanup(CpuList &list)
{
  CpuList::Iterator it;
  for (it = list.begin(); it != list.end(); ++it)
    (*it).cleanup();

  list.clear();
}

KSim::Chart *CpuView::addChart()
{
  KSim::Chart *chart = new KSim::Chart(false, 0, this);
  chart->show();
  m_mainLayout->addWidget(chart);

  return chart;
}

KSim::Progress *CpuView::addLabel()
{
  KSim::Progress *progress = new KSim::Progress(100,
     KSim::Types::None, KSim::Progress::Panel, this);

  progress->show();
  m_mainLayout->addWidget(progress);

  return progress;
}

CpuConfig::CpuConfig(KSim::PluginObject *parent, const char *name)
   : KSim::PluginPage(parent, name)
{
  QVBoxLayout * mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( 6 );

  m_listView = new KListView(this);
  m_listView->addColumn(i18n("Available CPUs"));
  m_listView->addColumn(i18n("Chart Format"));
  m_listView->setAllColumnsShowFocus(true);
  m_listView->setSelectionMode(QListView::Single);
  connect( m_listView, SIGNAL( doubleClicked( QListViewItem * ) ),
     SLOT( modify( QListViewItem * ) ) );

  mainLayout->addWidget( m_listView );

  QHBoxLayout * layout = new QHBoxLayout;
  layout->setSpacing( 6 );

  QSpacerItem * spacer = new QSpacerItem( 20, 20,
     QSizePolicy::Expanding, QSizePolicy::Minimum );
  layout->addItem(spacer);

  m_modify = new QPushButton( this );
  m_modify->setText( i18n( "Modify..." ) );
  connect( m_modify, SIGNAL( clicked() ), SLOT( modify() ) );
  layout->addWidget( m_modify );
  mainLayout->addLayout( layout );

  m_legendBox = new QGroupBox(this);
  m_legendBox->setColumnLayout(0, Qt::Vertical);
  m_legendBox->setTitle(i18n("Chart Legend"));
  m_legendBox->layout()->setSpacing(0);
  m_legendBox->layout()->setMargin(0);

  m_legendLayout = new QVBoxLayout(m_legendBox->layout());
  m_legendLayout->setAlignment(Qt::AlignTop);
  m_legendLayout->setSpacing(6);
  m_legendLayout->setMargin(11);

  m_totalNiceLabel = new QLabel(i18n("%T - Total CPU time (sys + user + nice)"), m_legendBox);
  m_legendLayout->addWidget(m_totalNiceLabel);

  m_totalLabel = new QLabel(i18n("%t - Total CPU time (sys + user)"), m_legendBox);
  m_legendLayout->addWidget(m_totalLabel);

  m_sysLabel = new QLabel(i18n("%s - Total sys time"), m_legendBox);
  m_legendLayout->addWidget(m_sysLabel);

  m_userLabel = new QLabel(i18n("%u - Total user time"), m_legendBox);
  m_legendLayout->addWidget(m_userLabel);

  m_niceLabel = new QLabel(i18n("%n - Total nice time"), m_legendBox);
  m_legendLayout->addWidget(m_niceLabel);

  mainLayout->addWidget( m_legendBox );

  for (uint i = 0; i < addCpus(); ++i)
  {
    QCheckListItem *item = new QCheckListItem(m_listView, i18n("cpu %1").arg(i), QCheckListItem::CheckBox);
    item->setText(1, "%T");
  }
}

CpuConfig::~CpuConfig()
{
}

void CpuConfig::readConfig()
{
  config()->setGroup("CpuPlugin");
  QStringList enabledCpus(config()->readListEntry("Cpus"));

  int cpuNum = 0;
  QStringList::ConstIterator it;
  for (it = enabledCpus.begin(); it != enabledCpus.end(); ++it) {
    if (QCheckListItem *item =
          static_cast<QCheckListItem *>(m_listView->findItem((*it), 0))) {
      item->setOn(true);
      item->setText(1, config()->readEntry(CPU_NAME(cpuNum), "%T"));
    }
    ++cpuNum;
  }
}

void CpuConfig::saveConfig()
{
  config()->setGroup("CpuPlugin");

  int cpuNum = 0;
  QStringList enabledCpus;
  for (QListViewItemIterator it(m_listView); it.current(); ++it) {
    config()->writeEntry(CPU_NAME(cpuNum), it.current()->text(1));
    if (static_cast<QCheckListItem *>(it.current())->isOn())
      enabledCpus.append(it.current()->text(0));
    ++cpuNum;
  }

  config()->writeEntry("Cpus", enabledCpus);
}

uint CpuConfig::addCpus()
{
#ifdef Q_OS_LINUX
  QStringList output;
  QString parser;
  QFile file("/proc/stat");
  if (!file.open(IO_ReadOnly))
    return 0;

  // Parse the proc file
  QTextStream procStream(&file);
  while (!procStream.atEnd()) {
    parser = procStream.readLine();
    if (QRegExp("cpu").search(parser, 0) != -1
          && QRegExp("cpu0").search(parser, 0) == -1) {
      output.append(parser);
    }
  }

  return output.count();
#endif

#ifdef Q_OS_BSD4
  int mib[] = { CTL_HW, HW_NCPU }; // hw.ncpu
  uint cpu;
  size_t cpuLen = sizeof(cpu);
  if (sysctl(mib, 2, &cpu, &cpuLen, NULL, 0) < 0)
    return 0;

  return cpu;
#endif
}

void CpuConfig::modify( QListViewItem * item )
{
  if ( !item )
    return;

  bool ok = false;
  QString text = KInputDialog::getText( i18n( "Modify CPU Format" ), i18n( "Chart format:" ),
     item->text( 1 ), &ok, this );

  if ( ok )
    item->setText( 1, text );
}

void CpuConfig::modify()
{
  modify( m_listView->selectedItem() );
}
