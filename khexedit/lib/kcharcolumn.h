/***************************************************************************
                          kcharcolumn.h  -  description
                             -------------------
    begin                : Mit Sep 3 2003
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


#ifndef KHE_KCHARCOLUMN_H
#define KHE_KCHARCOLUMN_H

// qt specific
#include <tqstring.h>
// lib specific
#include "kbuffercolumn.h"

class TQPainter;
class TQColor;


namespace KHE
{


/** buffer column that interprets the bytes as chars
  *
  *@author Friedrich W. H. Kossebau
  */
class KCharColumn : public KBufferColumn
{
  public:
    KCharColumn( KColumnsView *CV, KDataBuffer *B, KBufferLayout *L, KBufferRanges *R );
    virtual ~KCharColumn();


  public: // modification access
    /** sets whether "unprintable" chars (>32) should be displayed in the char column
      * with their corresponding character.
      * @param SU
      * returns true if there was a change
      */
    bool setShowUnprintable( bool SU = true );
    /** sets the substitute character for "unprintable" chars
      * returns true if there was a change
      */
    bool setSubstituteChar( TQChar SC );
    /** sets the undefined character for "undefined" chars
      * returns true if there was a change
      */
    bool setUndefinedChar( TQChar UC );


  public: // value access
    /** returns true if "unprintable" chars (>32) are displayed in the char column
      * with their corresponding character, default is false
      */
    bool showUnprintable() const;
    /** returns the actually used substitute character for "unprintable" chars, default is '.' */
    TQChar substituteChar() const;
    /** returns the actually used undefined character for "undefined" chars, default is '?' */
    TQChar undefinedChar() const;


  protected: // KBufferColumn API
    virtual void drawByte( TQPainter *P, char Byte, KHEChar B, const TQColor &Color ) const;

  protected:
    /** */
    bool ShowUnprintable;
    /** */
    TQChar SubstituteChar;
    /** */
    TQChar UndefinedChar;
};


inline bool KCharColumn::showUnprintable()  const { return ShowUnprintable; }
inline TQChar KCharColumn::substituteChar()  const { return SubstituteChar; }
inline TQChar KCharColumn::undefinedChar()   const { return UndefinedChar; }

inline bool KCharColumn::setSubstituteChar( TQChar SC )
{
  if( SubstituteChar == SC )
    return false;
  SubstituteChar = SC;
  return true;
}

inline bool KCharColumn::setUndefinedChar( TQChar UC )
{
  if( UndefinedChar == UC )
    return false;
  UndefinedChar = UC;
  return true;
}

inline bool KCharColumn::setShowUnprintable( bool SU )
{
  if( ShowUnprintable == SU )
    return false;
  ShowUnprintable = SU;
  return true;
}

}

#endif
