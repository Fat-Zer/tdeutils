/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _HEX_CLIPBOARD_H_
#define _HEX_CLIPBOARD_H_

#include <tqstring.h> 

class CHexClipboard
{
  public:
    CHexClipboard( void );
    ~CHexClipboard( void );

    bool encode( TQByteArray &dst, TQByteArray &src );
    bool decode( TQByteArray &dst, TQString &src );

  private:
    bool plainDecode( TQByteArray &dst, TQString &src );
};


#endif
