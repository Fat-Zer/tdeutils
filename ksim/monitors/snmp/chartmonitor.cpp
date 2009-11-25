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
#include "chartmonitor.h"

#include <kio/global.h>

using namespace KSim::Snmp;

ChartMonitor::ChartMonitor( const MonitorConfig &config, QWidget *parent, const char *name )
    : KSim::Chart( true /* displayMeter */, 0, QString::null, parent, name ),
      m_lastValue( 0 ), m_config( config ), m_firstSampleReceived( false )
{
    setTitle( m_config.name );
    disableAutomaticUpdates();
}

void ChartMonitor::setData( const Value &data )
{
    Q_UINT64 currentValue = convertToInt( data );

    if ( data.isCounterType() ) {
        int diff = currentValue - m_lastValue;

        if ( !m_firstSampleReceived ) {
            diff = 0;
            m_firstSampleReceived = true;
        }

        m_lastValue = currentValue;
        currentValue = diff;
    }

    // move the graph and add the new sample then
    updateDisplay();
    setValue( currentValue );
    if ( m_config.displayCurrentValueInline ) {
        Q_UINT64 bytesPerSec = currentValue / ( m_config.refreshInterval.seconds + m_config.refreshInterval.minutes * 60 );
        setText( KIO::convertSize( bytesPerSec ), 0 );
    }
}

Q_UINT64 ChartMonitor::convertToInt( const Value &data )
{
    switch ( data.type() ) {
        case Value::TimeTicks:
        case Value::Int: return data.toInt();
        case Value::Gauge:
        case Value::Counter:
        case Value::UInt: return data.toUInt();
        case Value::Counter64: return data.toCounter64();
        case Value::Double: // ### not sure what to do here...
        case Value::ByteArray:
        case Value::Oid: // ### could treat this as a sort of hyperlink... hmm
        case Value::IpAddress:
        case Value::NoSuchObject:
        case Value::NoSuchInstance:
        case Value::EndOfMIBView:
        case Value::Invalid:
        case Value::Null: return 0;
    }
    return 0;
}

#include "chartmonitor.moc"

/* vim: et sw=4 ts=4
 */
