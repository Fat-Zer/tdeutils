/***************************************************************************
                          helper.h  -  description
                             -------------------
    begin                : Fri Oct 03 2003
    copyright            : (C) 2003 by Friedrich W. H. Kossebau
    email                : Friedrich.W.H@Kossebau.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/

#ifndef KHEXEDIT_HELPER
#define KHEXEDIT_HELPER

// qt specific
#include <tqcolor.h>
// lib specific
#include <khechar.h>

// temporary solution until syntax highlighting is implemented
static inline TQColor colorForChar( const KHE::KHEChar Byte )
{
  return Byte.isUndefined() ? TQt::yellow : Byte.isPunct() ? TQt::red : Byte.isPrint() ? TQt::black : TQt::blue;
}

#endif
