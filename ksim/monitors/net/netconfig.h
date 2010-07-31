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

#ifndef NETCONFIG_H
#define NETCONFIG_H

#include <pluginmodule.h>
#include "netdevices.h"

class NetDialog;
class QPopupMenu;
class QPushButton;
class KListView;
class QHBoxLayout;
class QListViewItem;

class NetConfig : public KSim::PluginPage
{
  Q_OBJECT
  public:
    NetConfig(KSim::PluginObject *parent, const char *name);
    virtual ~NetConfig();

    virtual void saveConfig();
    virtual void readConfig();

  private slots:
    void menu(KListView *, TQListViewItem *, const TQPoint &);
    void modifyItem(TQListViewItem *);
    void removeItem(TQListViewItem *);
    void removeCurrent();
    void modifyCurrent();
    void showNetDialog();
    void getStats();

  private:
    // Couldnt think of a better name for this :)
    const TQString &boolToString(bool) const;

    TQHBoxLayout *layout;
    TQPushButton *insertButton;
    TQPushButton *removeButton;
    TQPushButton *modifyButton;
    KListView *usingBox;
    NetDialog *netDialog;
    Network::List m_networkList;
    TQString m_yes;
    TQString m_no;
    TQPopupMenu *aboutMenu;
};
#endif
