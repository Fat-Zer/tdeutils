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

#ifndef _HEX_ERROR_H_
#define _HEX_ERROR_H_

enum EHexError
{
  Err_NoErr = 0,
  Err_Success = 0,
  Err_NoData = -10000, // Must be the first
  Err_NoMemory,
  Err_ListFull,
  Err_ReadFailed,
  Err_WriteFailed,
  Err_EmptyArgument,
  Err_IllegalArgument,
  Err_NullArgument,
  Err_WrapBuffer,
  Err_NoMatch,
  Err_NoSelection,
  Err_EmptyDocument,
  Err_NoActiveDocument,
  Err_NoMark,
  Err_WriteProtect,
  Err_NoResize,
  Err_Stop,
  Err_IllegalMode,
  Err_Busy,
  Err_IllegalRange,
  Err_OperationAborted,
  Err_OpenWriteFailed,
  Err_OpenReadFailed,
  Err_MAXERROR         // Must be the last
};

const TQString &hexError( int index );


#endif
