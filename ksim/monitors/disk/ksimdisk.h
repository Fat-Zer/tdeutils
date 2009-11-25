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

#ifndef KSIMDISK_H
#define KSIMDISK_H

#include <qvaluelist.h>
#include <pluginmodule.h>
#include <stdio.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qvaluevector.h>

class QTextStream;
class QTimer;
class KListView;
class QVBoxLayout;
class QVButtonGroup;
class QRadioButton;
class QPushButton;
namespace KSim
{
  class Chart;
  class Progress;
}

class DiskPlugin : public KSim::PluginObject
{
  public:
    DiskPlugin(const char *name);
    ~DiskPlugin();

    virtual KSim::PluginView *createView(const char *);
    virtual KSim::PluginPage *createConfigPage(const char *);

    virtual void showAbout();
};

class DiskView : public KSim::PluginView
{
  Q_OBJECT
  public:
    DiskView(KSim::PluginObject *parent, const char *name);
    ~DiskView();

    virtual void reparseConfig();

  private slots:
    void updateDisplay();

  private:
    class DiskData
    {
      public:
        DiskData()
        {
          major = minor = readIO = readBlocks =
          writeIO = writeBlocks = 0;
        }

        DiskData &operator+=(const DiskData &rhs)
        {
          total += rhs.total;
          readIO += rhs.readIO;
          readBlocks += rhs.readBlocks;
          writeIO += rhs.writeIO;
          writeBlocks += rhs.writeBlocks;
          return *this;
        }

        DiskData &operator-=(const DiskData &rhs)
        {
          total -= rhs.total;
          readIO -= rhs.readIO;
          readBlocks -= rhs.readBlocks;
          writeIO -= rhs.writeIO;
          writeBlocks -= rhs.writeBlocks;
          return *this;
        }

        QString name;
        int major;
        int minor;
        unsigned long total;
        unsigned long readIO;
        unsigned long readBlocks;
        unsigned long writeIO;
        unsigned long writeBlocks;
    };

    typedef QValueList<DiskData> DiskList;
    typedef QPair<KSim::Chart *, KSim::Progress *> DiskPair;

    void updateData(DiskList &disks);
    QString diskName( int, int ) const;
    DiskPair *addDisk();
    DiskData findDiskData(const DiskList& diskList, QString diskName);

    void init();
    void cleanup();

    QValueVector<QPair<DiskData, DiskData> > m_data;
    QTimer *m_timer;
    bool m_bLinux24;
    FILE *m_procFile;
    QTextStream *m_procStream;
    QVBoxLayout *m_layout;
    QPtrList<DiskPair> m_diskList;
    int m_firstTime;
    bool m_useSeperatly;
    QStringList m_list;
    bool m_addAll;
};

class DiskConfig : public KSim::PluginPage
{
  Q_OBJECT
  public:
    DiskConfig(KSim::PluginObject *parent, const char *name);
    ~DiskConfig();

    virtual void saveConfig();
    virtual void readConfig();

  private slots:
    void addItem();
    void removeItem();

  private:
    QVBoxLayout *m_layout;
    KListView *m_listview;
    QPushButton *m_add;
    QPushButton *m_remove;
    QVButtonGroup *m_buttonBox;
    QRadioButton *m_totalButton;
    QRadioButton *m_bothButton;
};
#endif
