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

#include <qtooltip.h>
#include <qlayout.h>
#include <qtimer.h>
#include <qregexp.h>
#include <qtextstream.h>
#include <qdatetime.h>

#include <kdebug.h>
#include <klocale.h>
#include <kapplication.h>
#include <kglobal.h>

#include "ksimsysinfo.h"
#include "ksimsysinfo.moc"
#include "systeminfo.h"
#include <label.h>
#include <ksimconfig.h>
#include <progress.h>
#include <themetypes.h>

#include <time.h>
#include <string.h>

KSim::Sysinfo::Sysinfo(KSim::Config *config,
   QWidget *parent, const char *name, WFlags fl)
   : DCOPObject("sysinfo"), QWidget(parent, name, fl)
{
  m_config = config;
  m_layout = new QVBoxLayout(this);

  m_timeLabel = 0L;
  m_dateLabel = 0L;
  m_uptimeLabel = 0L;
  m_memLabel = 0L;
  m_swapLabel = 0L;
  //m_procsLabel = 0L;

  m_timer = new QTimer(this);
  connect(m_timer, SIGNAL(timeout()), SLOT(clockUptimeUpdate()));

  m_sysTimer = new QTimer(this);
  connect(m_sysTimer, SIGNAL(timeout()), SLOT(sysUpdate()));

  createView();
}

KSim::Sysinfo::~Sysinfo()
{
  // remember to cleanup
  System::cleanup();
}

void KSim::Sysinfo::clockUptimeUpdate()
{
  QString time;
  static bool updateDate = true;

  if (m_timeLabel) {
    QTime now = QTime::currentTime();
    time = KGlobal::locale()->formatTime(now, true);
    if ( now == QTime(0, 0) )
      updateDate = true;

    m_timeLabel->setText(time);
  }

  // only update the date when necessary
  if (m_dateLabel) {
    if (updateDate) {
      m_dateLabel->setText(KGlobal::locale()->formatDate(QDate::currentDate()));
      updateDate = false;
    }
  }
  else {
    updateDate = true;
  }

  if (m_uptimeLabel) {
    QString uptime = m_config->uptimeFormat();
    ++m_totalUptime;
    long uptimeDays, uptimeHours, uptimeMins, uptimeSecs;

    uptimeHours = m_totalUptime / 3600;
    uptimeDays = uptimeHours / 24;
    uptimeMins = (m_totalUptime - uptimeHours * 3600) / 60;
    uptimeSecs = m_totalUptime % 60;

    QString days;
    QString hours;
    QString minutes;
    QString seconds;

    // found days so we have to modify hours
    if (uptime.find(QRegExp("%d" ), 0) >= 0)
      uptimeHours -= (uptimeDays * 24);

    days.sprintf("%02li", uptimeDays);
    hours.sprintf("%02li", uptimeHours);
    minutes.sprintf("%02li", uptimeMins);
    seconds.sprintf("%02li", uptimeSecs);

    uptime.replace(QRegExp("%d"), days);
    uptime.replace(QRegExp("%h"), hours);
    uptime.replace(QRegExp("%m"), minutes);
    uptime.replace(QRegExp("%s"), seconds);
    m_uptimeLabel->setText(uptime);
  }
}

void KSim::Sysinfo::sysUpdate()
{
  const System &system = System::self();
  m_totalUptime = system.uptime();

  if (m_memLabel) {
    QString memory = m_config->memoryFormat();
    unsigned long total = system.totalRam();
    unsigned long free = system.freeRam();
  unsigned long shared = system.sharedRam();
  unsigned long buffer = system.bufferRam();
  unsigned long cache = system.cacheRam();
  unsigned long used = system.usedRam();
  unsigned long allFree = free + buffer + cache;
  bool all = memory.find( "%F" ) != -1;

  memory.replace(QRegExp("%s"), QString::number(System::bytesToMegs(shared)));
  memory.replace(QRegExp("%b"), QString::number(System::bytesToMegs(buffer)));
  memory.replace(QRegExp("%c"), QString::number(System::bytesToMegs(cache)));
  memory.replace(QRegExp("%u"), QString::number(System::bytesToMegs(used)));
  memory.replace(QRegExp("%t"), QString::number(System::bytesToMegs(total)));
  memory.replace(QRegExp("%f"), QString::number(System::bytesToMegs(free)));
  memory.replace(QRegExp("%F"), QString::number(System::bytesToMegs(allFree)));
    kdDebug(2003) << memory << endl;
    m_memLabel->setText("Memory");
    QToolTip::add(m_memLabel, memory );

    if ( !all )
      m_memLabel->setValue( System::bytesToMegs( total ) - System::bytesToMegs( free ) );
    else
      m_memLabel->setValue( System::bytesToMegs( total ) - System::bytesToMegs( allFree ) );
  }

  if (m_swapLabel) {
    QString swap = m_config->swapFormat();
    unsigned long total = system.totalSwap();
    unsigned long free = system.freeSwap();
    unsigned long swapUsed = system.usedSwap();
    swap.replace(QRegExp("%u"), QString::number(System::bytesToMegs(swapUsed)));
    swap.replace(QRegExp("%t"), QString::number(System::bytesToMegs(total)));
    swap.replace(QRegExp("%f"), QString::number(System::bytesToMegs(free)));
    m_swapLabel->setText("Swap");
    QToolTip::add(m_swapLabel, swap);
    m_swapLabel->setValue(System::bytesToMegs(total) - System::bytesToMegs(free));
  }

//  if (m_config->showProcs()) {
//    m_procsLabel->setText(i18n("Procs: %1").arg(sysInfo.procs));
//  }
}

void KSim::Sysinfo::createView()
{
  stopTimers();
  const System &system = System::self();
  int timeLocation = 0;
  int dateLocation = 1;
  int uptimeLocation = 2;
  int memLocation = 3;
  int swapLocation = 4;
  // int procsLocation = 5;
  int offset = 0;

  if (m_config->showTime()) {
    if (!m_timeLabel) {
      m_timeLabel = new KSim::Label(this);
      m_layout->insertWidget(timeLocation - offset, m_timeLabel);
    }
    QToolTip::add(m_timeLabel, i18n("Current system time"));
    m_timeLabel->show();
  }
  else {
    offset++;
    delete m_timeLabel;
    m_timeLabel = 0L;
  }

  if (m_config->showDate()) {
    if (!m_dateLabel) {
        m_dateLabel = new KSim::Label(this);
        m_layout->insertWidget(dateLocation - offset, m_dateLabel);
    }
    QToolTip::add(m_dateLabel, i18n("Current system date"));
    m_dateLabel->show();
  }
  else {
    offset++;
    delete m_dateLabel;
    m_dateLabel = 0L;
  }

  kdDebug(2003) << m_dateLabel << endl;

  if (m_config->showUptime()) {
    if (!m_uptimeLabel) {
      m_uptimeLabel = new KSim::Label(KSim::Types::Uptime, this);
      m_layout->insertWidget(uptimeLocation - offset, m_uptimeLabel);
    }
    QToolTip::add(m_uptimeLabel, i18n("System uptime"));
    m_uptimeLabel->show();
  }
  else {
    offset++;
    delete m_uptimeLabel;
    m_uptimeLabel = 0L;
  }

  if (m_config->showMemory()) {
    if (!m_memLabel) {
      m_memLabel = new KSim::Progress(System::bytesToMegs(system.totalRam()),
          KSim::Types::Mem, this);
      m_layout->insertWidget(memLocation - offset, m_memLabel);
    }
    
    m_memLabel->show();
  }
  else {
    offset++;
    delete m_memLabel;
    m_memLabel = 0L;
  }

  if (m_config->showSwap()) {
    if (!m_swapLabel) {
      m_swapLabel = new KSim::Progress(System::bytesToMegs(system.totalSwap()),
          KSim::Types::Swap, this);
      m_layout->insertWidget(swapLocation - offset, m_swapLabel);
    }
    m_swapLabel->show();
  }
  else {
    offset++;
    delete m_swapLabel;
    m_swapLabel = 0L;
   }

  /*if (m_config->showProcs()) {
    if (!m_procsLabel) {
      m_procsLabel = new KSimLabel(this);
      m_layout->insertWidget(procsLocation, m_procsLabel);
    }
    m_procsLabel->show();
  }
  else {
    delete m_procsLabel;
    m_procsLabel = 0L;
  }*/

//  m_layout->invalidate();
  updateGeometry();
  adjustSize();

  startTimers();
  sysUpdate();
  clockUptimeUpdate();
}

void KSim::Sysinfo::stopTimers()
{
  m_timer->stop();
  m_sysTimer->stop();
}

void KSim::Sysinfo::startTimers()
{
  // Cellecting meminfo on fbsd seems to be expensive :(
  // so better update the memory display every 5 seconds rather than 1
  // until i can find a better solution (if there is one)
  if (!m_sysTimer->isActive() && !m_timer->isActive()) {
    m_sysTimer->start(5000);
    m_timer->start(1000);
  }
}

QString KSim::Sysinfo::uptime() const
{
  if (m_uptimeLabel)
    return m_uptimeLabel->text();

  return i18n("Uptime display disabled");
}

QString KSim::Sysinfo::memInfo() const
{
  if (m_memLabel)
    return m_memLabel->text();

  return i18n("Memory display disabled");
}

QString KSim::Sysinfo::swapInfo() const
{
  if (m_swapLabel)
    return m_swapLabel->text();

  return i18n("Swap display disabled");
}
