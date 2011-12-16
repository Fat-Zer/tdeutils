/***************************************************************************
                          kbuffercolumn.h  -  description
                             -------------------
    begin                : Mit Jun 26 2003
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

 
#ifndef KHE_KCURSOR_H
#define KHE_KCURSOR_H


#include <tqpixmap.h>

#include "kadds.h"

namespace KHE
{

/**
  *@author Friedrich W. H. Kossebau
  */
class KCursor
{
  public:
    KCursor();
    virtual ~KCursor();

  public:
    /** sets size of the full cursor */
    void setSize( KPixelX Width, KPixelY Height );
    /** sets the tqshape of the cursor to be drawn */
    void setShape( KPixelX X, KPixelX W );

  public: // access
    const TQPixmap &onPixmap() const;
    const TQPixmap &offPixmap() const;
    KPixelX cursorX() const;
    KPixelX cursorW() const;


  protected:
    TQPixmap OnPixmap;
    TQPixmap OffPixmap;

    KPixelX CursorX;
    KPixelX CursorW;
};


inline const TQPixmap &KCursor::onPixmap()  const { return OnPixmap; }
inline const TQPixmap &KCursor::offPixmap() const { return OffPixmap; }

inline KPixelX KCursor::cursorX() const { return CursorX; }
inline KPixelX KCursor::cursorW() const { return CursorW; }

}

#endif
