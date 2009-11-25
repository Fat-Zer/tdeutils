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

#ifndef KSIMNET_H
#define KSIMNET_H

#include <pluginmodule.h>
#include "netdevices.h"
#include <stdio.h>

class QTimer;
class QVBoxLayout;
class QTextStream;

class NetPlugin : public KSim::PluginObject
{
  public:
    NetPlugin(const char *name);
    ~NetPlugin();

    virtual KSim::PluginView *createView(const char *);
    virtual KSim::PluginPage *createConfigPage(const char *);

    virtual void showAbout();
};

class NetView : public KSim::PluginView
{
  Q_OBJECT
  public:
    NetView(KSim::PluginObject *parent, const char *name);
    ~NetView();

    virtual void reparseConfig();

  private slots:
    void cleanup();
    void updateLights();
    void updateGraph();
    void addDisplay();
    void runConnectCommand(int);
    void runDisconnectCommand(int);

  protected:
    bool eventFilter(QObject *, QEvent *);

  private:
    Network::List createList() const;

    KSim::Chart *addChart();
    KSim::LedLabel *addLedLabel(const QString &device);
    KSim::Label *addLabel();
    QPopupMenu *addPopupMenu(const QString &device, int value);

    void netStatistics(const QString &device, NetData &data);
    bool isOnline(const QString &device);

    void showMenu(int i);

    bool m_firstTime;
    Network::List m_networkList;
    QTimer *m_netTimer;
    QTimer *m_lightTimer;
    QVBoxLayout *m_netLayout;
#ifdef __linux__
    FILE *m_procFile;
    QTextStream *m_procStream;
#endif
#ifdef __FreeBSD__
    char *m_buf;
    int m_allocSize;
#endif
};
#endif
