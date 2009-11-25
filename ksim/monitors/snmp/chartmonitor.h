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
#ifndef CHARTMONITOR_H
#define CHARTMONITOR_H

#include "chart.h"
#include "monitor.h"

namespace KSim
{

namespace Snmp
{

class ChartMonitor : public KSim::Chart
{
    Q_OBJECT
public:
    ChartMonitor( const MonitorConfig &config, QWidget *parent, const char *name = 0 );

public slots:
    void setData( const Value &data );

private:
    Q_UINT64 m_lastValue;
    MonitorConfig m_config;
    bool m_firstSampleReceived;

    static Q_UINT64 convertToInt( const Value &data );
};

}

}

#endif // CHARTMONITOR_H
/* vim: et sw=4 ts=4
 */
