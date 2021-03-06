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
#ifndef LINEPARSER_H
#define LINEPARSER_H

/**
@author See README for the list of authors
*/

#include <tqstring.h>
#include <tqcolor.h>

class LineParser
{
  public:
    LineParser(const TQString& line = TQString());
    ~LineParser();

    void set(const TQString& line);

    int getInt(TQString w, int def = 0) const;
    TQColor getColor(TQString w, TQColor def = TQColor()) const;
    TQString getString(TQString w, TQString def = TQString()) const;
    bool getBoolean(TQString w, bool def = false) const;

    const TQString& meter() const { return m_meter; };

  private:
    TQString m_line;
    TQString m_meter;
};

#endif
