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

#ifndef KSIMCPU_H
#define KSIMCPU_H

#include <pluginmodule.h>
#include <chart.h>
#include <progress.h>

#include <stdio.h>

class QPushButton;
class QTextStream;
class QVBoxLayout;
class QHBoxLayout;
class QCheckBox;
class QLabel;
class QGroupBox;
class QTimer;
class QListViewItem;
class KListView;

class CpuPlugin : public KSim::PluginObject
{
  public:
    CpuPlugin(const char *name);
    ~CpuPlugin();

    virtual KSim::PluginView *createView(const char *);
    virtual KSim::PluginPage *createConfigPage(const char *);

    virtual void showAbout();
};

class CpuView : public KSim::PluginView
{
  Q_OBJECT
  public:
    CpuView(KSim::PluginObject *parent, const char *name);
    ~CpuView();

    virtual void reparseConfig();

  private slots:
    void updateView();

  private:
    class CpuData
    {
      public:
        CpuData() { user = nice = sys = idle = 0; }

        QString name;
        unsigned long user;
        unsigned long nice;
        unsigned long sys;
        unsigned long idle;

        CpuData &operator-=(const CpuData &rhs)
        {
          if (name != rhs.name)
            return *this;

          user -= rhs.user;
          nice -= rhs.nice;
          sys -= rhs.sys;
          idle -= rhs.idle;
          return *this;
        }
    };

    class Cpu
    {
      public:
        Cpu()
        {
          m_num = 0;
          m_chart = 0;
          m_label = 0;
        }

        Cpu(const QString &name,
           const QString &format,
           int number)
           : m_name(name),
           m_format(format),
           m_num(number)
        {
          m_chart = 0;
          m_label = 0;
        }

        ~Cpu()
        {
        }

        bool operator==(const Cpu &rhs) const
        {
          return m_name == rhs.m_name &&
             m_format == rhs.m_format;
        }

        bool operator!=(const Cpu &rhs) const
        {
          return !operator==(rhs);
        }

        void cleanup()
        {
          delete m_chart;
          delete m_label;
        }

        void setData(const CpuData &data)
        {
          m_old = m_data;
          m_data = data;
        }

        const CpuData &oldData() const
        {
          return m_old;
        }

        const QString &format() const
        {
          return m_format;
        }

        int number() const
        {
          return m_num;
        }

        void setDisplay(KSim::Chart *chart, KSim::Progress *label)
        {
          m_chart = chart;
          m_label = label;
          m_label->setText(m_name);
        }

        KSim::Chart *chart()
        {
          return m_chart;
        }

        KSim::Progress *label()
        {
          return m_label;
        }

      private:
        CpuData m_data;
        CpuData m_old;
        QString m_name;
        QString m_format;
        KSim::Chart *m_chart;
        KSim::Progress *m_label;
        int m_num;
    };

    typedef QValueList<Cpu> CpuList;
    void updateCpu(CpuData &cpu, int cpuNumber);
    CpuList createList();

    void addDisplay();
    void cleanup(CpuList &);

    KSim::Chart *addChart();
    KSim::Progress *addLabel();

    QTimer *m_timer;
    QVBoxLayout *m_mainLayout;
    bool m_firstTime;
    CpuList m_cpus;

#ifdef __linux__
    FILE *m_procFile;
    QTextStream *m_procStream;
#endif
};

class CpuConfig : public KSim::PluginPage
{
  Q_OBJECT
  public:
    CpuConfig(KSim::PluginObject *parent, const char *name);
    ~CpuConfig();

    virtual void saveConfig();
    virtual void readConfig();

  private slots:
    void modify( QListViewItem * );
    void modify();

  private:
    uint addCpus();

    KListView *m_listView;
    QGroupBox *m_legendBox;
    QLabel *m_totalNiceLabel;
    QLabel *m_totalLabel;
    QLabel *m_sysLabel;
    QLabel *m_userLabel;
    QLabel *m_niceLabel;
    QVBoxLayout *m_legendLayout;

    QPushButton * m_modify;
};
#endif
