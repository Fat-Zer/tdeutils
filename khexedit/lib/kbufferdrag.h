/***************************************************************************
                          kbufferdrag.h  -  description
                             -------------------
    begin                : Mon Jul 07 2003
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


#ifndef KHE_KBUFFERDRAG_H
#define KHE_KBUFFERDRAG_H

// qt specific
#include <tqdragobject.h>
// lib specific
#include "khe.h"
#include "kcoordrange.h"
#include "kcoltextexport.h"

namespace KHE
{

// class KBorderColumn;
class KOffsetColumn;
class KValueColumn;
class KCharColumn;

typedef KColTextExport* KColTextExportPtr;
/**
  *@author Friedrich W. H. Kossebau
  */
class KBufferDrag : public QDragObject
{
    Q_OBJECT

  public:
    // TODO: make this call somewhat more generic
    KBufferDrag( const TQByteArray &, KCoordRange Range,
                 const KOffsetColumn *OC, const KValueColumn *HC, const KCharColumn *TC,
                 TQChar SC, TQChar UC, const TQString &CN,
                 TQWidget *Source = 0, const char *Name = 0 );
    ~KBufferDrag();

  public: // TQDragObject API
    virtual const char *format( int i ) const;
    virtual TQByteArray encodedData( const char* ) const;

  public:
    virtual void setData( const TQByteArray &);

  public:
    static bool canDecode( const TQMimeSource* Source );
    static bool decode( const TQMimeSource* Source, TQByteArray &Dest );

  protected:
    TQByteArray Data;
    KCoordRange CoordRange;
    /** collection of all the columns. All columns will be autodeleted. */
    KColTextExportPtr Columns[5];
    uint NoOfCol;
    TQChar SubstituteChar;
    TQChar UndefinedChar;
    const TQString &CodecName;
};

}

#endif
