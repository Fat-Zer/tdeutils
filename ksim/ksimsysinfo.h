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

#ifndef SYSINFO_H
#define SYSINFO_H

#include <qwidget.h>
#include <dcopobject.h>

class QTimer;
class QVBoxLayout;
namespace KSim
{
  class Config;
  class Label;
  class Progress;

  class Sysinfo : public QWidget, virtual public DCOPObject
  {
    Q_OBJECT
    K_DCOP
    public:
      Sysinfo(KSim::Config *config, QWidget *parent=0,
         const char *name=0, WFlags fl=0);
      ~Sysinfo();

    k_dcop:
      QString uptime() const;
      QString memInfo() const;
      QString swapInfo() const;

    public slots:
      void clockUptimeUpdate();
      void sysUpdate();
      void createView();
      void stopTimers();
      void startTimers();


    private:
      KSim::Label *m_timeLabel;
      KSim::Label *m_dateLabel;
      KSim::Label *m_uptimeLabel;
      KSim::Progress *m_memLabel;
      KSim::Progress *m_swapLabel;
      //KSimLabel *m_procsLabel;
      QVBoxLayout *m_layout;
      QTimer *m_timer;
      QTimer *m_sysTimer;
      KSim::Config *m_config;
      long m_totalUptime;
  };
}
#endif
