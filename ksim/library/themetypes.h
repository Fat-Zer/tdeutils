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

#ifndef THEMETYPES_H
#define THEMETYPES_H

namespace KSim
{
  namespace Types
  {
    /**
     * available types for the frames
     */
    enum FrameType
    {
      TopFrame = 0,
      BottomFrame,
      LeftFrame,
      RightFrame
    };

    /**
     * available theme types
     */
    enum ThemeType
    {
      None = -1,
      Apm,
      Cal,
      Clock,
      Fs,
      Host,
      Mail,
      Mem,
      Swap,
      Timer,
      Uptime,
      Net,
      Inet
    };

    /**
     * @return the ThemeType enum as a QString
     */
    inline QString typeToString(int type, bool incSlash = true)
    {
      if (type == Types::None)
        return QString::null;

      // This array MUST be in the same order
      // as the ThemeType enum
      const char *typeNames[] = {
        "apm", "cal", "clock",
        "fs", "host", "mail",
        "mem", "swap", "timer",
        "uptime", "net", "inet", 0
      };

      QString returnString;
      returnString.setLatin1(typeNames[type]);
      return incSlash ? returnString + QString::fromLatin1("/") : returnString;
    }
  }
}
#endif
