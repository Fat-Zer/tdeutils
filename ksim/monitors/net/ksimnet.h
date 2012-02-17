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

class TQTimer;
class TQVBoxLayout;
class TQTextStream;

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
    bool eventFilter(TQObject *, TQEvent *);

  private:
    Network::List createList() const;

    KSim::Chart *addChart();
    KSim::LedLabel *addLedLabel(const TQString &device);
    KSim::Label *addLabel();
    TQPopupMenu *addPopupMenu(const TQString &device, int value);

    void netStatistics(const TQString &device, NetData &data);
    bool isOnline(const TQString &device);

    void showMenu(int i);

    bool m_firstTime;
    Network::List m_networkList;
    TQTimer *m_netTimer;
    TQTimer *m_lightTimer;
    TQVBoxLayout *m_netLayout;
#ifdef __linux__
    FILE *m_procFile;
    TQTextStream *m_procStream;
#endif
#ifdef __FreeBSD__
    char *m_buf;
    int m_allocSize;
#endif
};
#endif
