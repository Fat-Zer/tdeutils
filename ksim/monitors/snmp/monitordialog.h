/* This file is part of the KDE project
   Copyright (C) 2003 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef MONITORDIALOG_H
#define MONITORDIALOG_H

#include "monitordialogbase.h"
#include "monitorconfig.h"

namespace KSim
{

namespace Snmp
{

class MonitorDialog : public MonitorDialogBase
{
    Q_OBJECT
public:
    MonitorDialog( const HostConfigMap &hosts, QWidget *parent, const char *name = 0 );
    MonitorDialog( const MonitorConfig &monitor, const HostConfigMap &hosts, QWidget *parent, const char *name = 0 );

    MonitorConfig monitorConfig() const;

protected:
    virtual void checkValues();
    virtual void testObject();
    virtual void browse();

private:
    void init( const MonitorConfig &monitor = MonitorConfig() );

    HostConfig currentHost() const;

    HostConfigMap m_hosts;
};

}

}

#endif // MONITORDIALOG_H
/* vim: et sw=4 ts=4
 */
