/*
 * Copyright (C) 2003-2004 Adam Geitgey <adam@rootnode.org>
 * Copyright (C) 2003 Hans Karlsson <karlsson.h@home.se>
 * Copyright (c) 2005 Ryan Nickell <p0z3r@earthlink.net>
 * Copyright (c) 2005 Petri Damsten <damu@iki.fi>
 *
 * This file is part of SuperKaramba.
 *
 *  SuperKaramba is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  SuperKaramba is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SuperKaramba; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/
#include "lineparser.h"
#include <tqregexp.h>

LineParser::LineParser(const TQString& line)
{
  set(line);
}

LineParser::~LineParser()
{
}

void LineParser::set(const TQString& line)
{
  TQRegExp rx("^\\s*(\\S+)");
  m_line = line;

  rx.search(m_line);
  m_meter = rx.cap(1).upper();
}

int LineParser::getInt(TQString w, int def) const
{
  TQRegExp rx( "\\W+" + w +"=([-]?\\d+)", false );
  if (rx.search(m_line) != -1)
    return rx.cap(1).toInt();
  else
    return def;
}

TQColor LineParser::getColor(TQString w, TQColor def) const
{
  TQRegExp rx( "\\W+" + w + "=([-]?\\d+),([-]?\\d+),([-]?\\d+)", false );
  if (rx.search(m_line) != -1)
    return TQColor(rx.cap(1).toInt(), rx.cap(2).toInt(), rx.cap(3).toInt());
  else
    return def;
}

TQString LineParser::getString(TQString w, TQString def) const
{
  TQString result;
  TQRegExp rx( "\\W+" + w + "=\"([^\"]*)\"", false );

  bool found = (rx.search(m_line)==-1)?false:true;
  if (rx.cap(1).isEmpty())
  {
    rx = TQRegExp(w + "=(\\S+)", false);
    found = (rx.search(m_line)==-1)?false:true;
    result = rx.cap(1);
  }
  else
  {
    result = rx.cap(1);
  }
  if(found)
    return result;
  else
    return def;
}

bool LineParser::getBoolean(TQString w, bool def) const
{
  TQString boolean = getString(w, "-").lower();
  if(boolean == "-")
    return def;
  else if (boolean == "true") // true / false
    return true;
  else if (boolean == "1") // 1 / 0
    return true;
  else if (boolean == "on") // on / off
    return true;
  return false;
}
