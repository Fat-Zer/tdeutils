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

#ifndef FSYSTEM_H
#define FSYSTEM_H

#include <pluginmodule.h>
#include "fsystemiface.h"

class KPopupMenu;
class FilesystemWidget;
class QTimer;

/**
 * This class is the filesystem monitor plugin
 *
 * @author Robbie Ward <linuxphreak@gmx.co.uk>
 * @version 0.1
 */
class PluginModule : public KSim::PluginObject
{
  public:
    PluginModule(const char *name);
    ~PluginModule();

    virtual KSim::PluginView *createView(const char *);
    virtual KSim::PluginPage *createConfigPage(const char *);

    virtual void showAbout();
};

class Fsystem : public KSim::PluginView, public FsystemIface
{
  Q_OBJECT
  public:
    /**
     * construtor of Fsystem, calls all init functions to create the plugin.
     */
    Fsystem(KSim::PluginObject *parent, const char *name);
    ~Fsystem();

    virtual void reparseConfig();
    int totalFreeSpace() const;

  public slots:
    void createFreeInfo();

  private slots:
    void updateFS();

  private:
    typedef QValueList<QPair<QString, QString> > MountEntryList;
    MountEntryList makeList(const QStringList &list) const;

    FilesystemWidget *m_widget;
    QTimer *m_updateTimer;
    MountEntryList m_mountEntries;
    bool m_showPercentage;
};
#endif
