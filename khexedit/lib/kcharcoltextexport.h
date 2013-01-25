/***************************************************************************
                          kcharcoltextexport.h  -  description
                             -------------------
    begin                : Wed Sep 3 2003
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


#ifndef KHE_KCHARCOLTEXTEXPORT_H
#define KHE_KCHARCOLTEXTEXPORT_H

// lib specific
#include "khe.h"
#include "kbuffercoltextexport.h"


namespace KHE
{

class KCharColumn;
class KCharCodec;


class KCharColTextExport : public TDEBufferColTextExport
{
  public:
    KCharColTextExport( const KCharColumn* BF, const char *D, KCoordRange CR, const TQString &CodecName );
    virtual ~KCharColTextExport();

  protected: //API
    virtual void print( TQString &T ) const;

  protected:
    KCharCodec *CharCodec;
    TQChar SubstituteChar;
    TQChar UndefinedChar;
};

}

#endif
