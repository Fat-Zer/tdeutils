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

#include "labelmonitor.h"

using namespace KSim::Snmp;

LabelMonitor::LabelMonitor( const MonitorConfig &config, TQWidget *tqparent, const char *name )
    : KSim::Label( tqparent, name ), m_config( config )
{
}

void LabelMonitor::setData( const Value &data )
{
     TQString dataString = data.toString( m_config.refreshInterval.minutes == 0 ? Value::TimeTicksWithSeconds : 0 );

     if ( m_config.useCustomFormatString ) {
         TQString text = m_config.customFormatString;
         text.tqreplace( "%n", m_config.name );
         text.tqreplace( "%s", dataString );
         setText( text );
     } else
         setText( m_config.name + ": " + dataString );
}

#include "labelmonitor.moc"

/* vim: et sw=4 ts=4
 */
