/*
 * makecrc.cpp
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




#include "checkcrc.h"

int main(int, char**)
{
	unsigned long len, crc;
	checkcrc("./klaptop_acpi_helper", len, crc);
	printf("#ifndef _CRCRESULT_H_\n");
	printf("#define _CRCRESULT_H_\n");
	printf("static unsigned long file_len = 0x%lx;\n", len);
	printf("static unsigned long file_crc = 0x%lx;\n", crc);
	printf("#endif\n");
	return(0);
}
