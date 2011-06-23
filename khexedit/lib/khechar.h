/***************************************************************************
                          helper.h  -  description
                             -------------------
    begin                : Do Nov 25 2004
    copyright            : (C) 2004 by Friedrich W. H. Kossebau
    email                : Friedrich.W.H@Kossebau.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/

#ifndef KHE_KHECHAR_H
#define KHE_KHECHAR_H

// qt specific
#include <tqstring.h>

namespace KHE
{

class KHEChar : public TQChar
{
  public:
    KHEChar( TQChar C );
    KHEChar( TQChar C, bool U );
  public:
    bool isUndefined() const;
  protected:
    // the byte is not defined
    bool IsUndefined:1;
};

inline KHEChar::KHEChar( TQChar C ) : TQChar( C ), IsUndefined( false ) {}
inline KHEChar::KHEChar( TQChar C, bool U ) : TQChar( C ), IsUndefined( U ) {}
inline bool KHEChar::isUndefined() const { return IsUndefined; }

}

#endif
