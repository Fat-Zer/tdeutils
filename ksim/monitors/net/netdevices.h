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

#ifndef NETDEVICES_H
#define NETDEVICES_H

#include <chart.h>
#include <led.h>

class NetData
{
  public:
    NetData() : in(0), out(0) {}
    NetData &operator=( const NetData & rhs )
    {
      in = rhs.in;
      out = rhs.out;

      return *this;
    }

    unsigned long in;
    unsigned long out;
};

class Network
{
  public:
    typedef QValueList<Network> List;

    Network()
    {
      m_chart = 0;
      m_led = 0;
      m_label = 0;
      m_popup = 0;

      m_max = 0;
    }

    Network( const QString & name,
       const QString & format,
       bool timer,
       bool commands,
       const QString & cCommand,
       const QString & dCommand )
       : m_name( name ),
       m_format( format ),
       m_timer( timer ),
       m_commands( commands ),
       m_cCommand( cCommand ),
       m_dCommand( dCommand )
    {
      m_chart = 0;
      m_led = 0;
      m_label = 0;
      m_popup = 0;

      m_max = 0;
    }

    bool operator==( const Network & rhs ) const
    {
      return m_name == rhs.m_name &&
         m_format == rhs.m_format &&
         m_timer == rhs.m_timer &&
         m_commands == rhs.m_commands &&
         m_cCommand == rhs.m_cCommand &&
         m_dCommand == rhs.m_dCommand;
    }

    bool operator!=( const Network & rhs ) const
    {
      return !operator==(rhs);
    }

    bool operator<( const Network & rhs ) const
    {
      return m_name < rhs.m_name;
    }

    bool operator>( const Network & rhs ) const
    {
      return m_name > rhs.m_name;
    }

    bool operator<=( const Network & rhs ) const
    {
      return !( *this > rhs );
    }

    void cleanup()
    {
      delete m_chart;
      delete m_label;
      delete m_led;
      delete m_popup;

      m_chart = 0;
      m_label = 0;
      m_led = 0;
      m_popup = 0;
    }

    void setData( const NetData & data )
    {
      m_old = m_data;
      m_data = data;
    }

    const NetData & data() const
    {
      return m_data;
    }

    const NetData & oldData() const
    {
      return m_old;
    }

    void setMaxValue( int max )
    {
      m_max = max;
    }

    int maxValue() const
    {
      return m_max;
    }

    const QString & name() const
    {
      return m_name;
    }

    bool showTimer() const
    {
      return m_timer;
    }

    const QString & format() const
    {
      return m_format;
    }

    bool commandsEnabled() const
    {
      return m_commands;
    }

    const QString & connectCommand() const
    {
      return m_cCommand;
    }

    const QString & disconnectCommand() const
    {
      return m_dCommand;
    }

    void setDisplay( KSim::Chart * chart, KSim::LedLabel * led,
       KSim::Label * label, QPopupMenu * popup )
    {
      m_chart = chart;
      m_led = led;
      m_label = label;
      m_popup = popup;
    }

    KSim::Chart * chart()
    {
      return m_chart;
    }

    KSim::LedLabel * led()
    {
      return m_led;
    }

    KSim::Label * label()
    {
      return m_label;
    }

  private:
    NetData m_data;
    NetData m_old;
    QString m_name;
    QString m_format;
    bool m_timer;
    bool m_commands;
    QString m_cCommand;
    QString m_dCommand;

    KSim::Chart * m_chart;
    KSim::LedLabel * m_led;
    KSim::Label * m_label;
    QPopupMenu * m_popup;

    int m_max;
};
#endif
