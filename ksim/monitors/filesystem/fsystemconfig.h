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

#ifndef FSYSTEMCONFIG_H
#define FSYSTEMCONFIG_H

#include <pluginmodule.h>
#include "filesystemstats.h"

class KIntSpinBox;
class TDEListView;
class TQGridLayout;
class TQCheckBox;
class TQLabel;
class TQListBoxItem;

class FsystemConfig : public KSim::PluginPage
{
  Q_OBJECT
  
  public:
    FsystemConfig(KSim::PluginObject *parent, const char *name);
    ~FsystemConfig();

    virtual void saveConfig();
    virtual void readConfig();

  protected:
    void showEvent(TQShowEvent *);

  private:
    void getStats();
    TQString splitString(const TQString &string) const;

    TQCheckBox *m_showPercentage;
    TQCheckBox *m_splitNames;
    TQLabel *m_intervalLabel;
    KIntSpinBox *m_updateTimer;
    TDEListView *m_availableMounts;
    TQGridLayout *m_mainLayout;
    FilesystemStats::List m_entries;
};
#endif // FILESYSTEM_H
