/*
 * checkcrc.h
 *
 * Copyright (c) 2003 Paul Campbell <paul@taniwha.com>
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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

#ifndef _CHECKCRC_H_
#define _CHECKCRC_H_



#include <zlib.h>
#include <stdio.h>

static void
checkcrc(const char *name, unsigned long &len_res, unsigned long &crc_res)
{
	unsigned long crc = ::crc32(0L, Z_NULL, 0);
	unsigned long len = 0;

	FILE *f = ::fopen(name, "r");
	if (f) {
		unsigned char buffer[1024];
		for (;;) {
			int l = ::fread(buffer, 1, sizeof(buffer), f);
			if (l <= 0)
				break;
			len += l;
		        crc = ::crc32(crc, buffer, l);
		}
		::fclose(f);
	}
	crc_res = crc;
	len_res = len;
}

#endif
