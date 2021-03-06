/***************************************************************************
                          kbuffercolumn.h  -  description
                             -------------------
    begin                : Mit Mai 14 2003
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


#ifndef KHE_KBUFFERCOLUMN_H
#define KHE_KBUFFERCOLUMN_H

// lib specific
#include "khe.h"
#include "kdatabuffer.h"
#include "khechar.h"
#include "kcolumn.h"
#include "kbufferlayout.h"
#include "ksection.h"

class TQPainter;
class TQColor;

namespace KHE
{

// class KHexEdit;
class TDEBufferRanges;
class KCharCodec;

const int NoByteFound = -1;

/** base class of all buffer column displayers
  * holds all information about the vertical layout of a buffer column
  * knows how to paint the data and the editing things (focus, cursor, selection)
  * but does not offer
  *
  *@author Friedrich W. H. Kossebauint TDEBufferColumn::posOfX( KPixelX PX, bool *ToTheRightFlag ) const
  */
class TDEBufferColumn : public KColumn
{
  public:
    enum KFrameStyle { Frame, Left, Right };
  public:
    TDEBufferColumn( KColumnsView/*KHexEdit*/ *HE, KDataBuffer *B, TDEBufferLayout *L, TDEBufferRanges *R );
    virtual ~TDEBufferColumn();


  public: // KColumn-API
    virtual void paintFirstLine( TQPainter *P, KPixelXs Xs, int FirstLine );
    virtual void paintNextLine( TQPainter *P );

  public:
    void preparePainting( KPixelXs Xs );

  public:
    void paintLine( TQPainter *P, int Line );
    void paintPositions( TQPainter *P, int Line, KSection Positions );
    /** paints a cursor based on the type of the byte.
      * @param Index Index of the byte to paint the cursor for. If -1 a space is used as char.
      */
    void paintCursor( TQPainter *P, int Index );
    /** paints the byte with background.
      * @param Index Index of the byte to paint. If -1 only the background is painted.
      */
    void paintByte( TQPainter *P, int Index );
    /** paints the byte with background and a frame around.
      * @param Index Index of the byte to paint the frame for. If -1 a space is used as char.
      * @param Style the style of the framing
      */
    void paintFramedByte( TQPainter *P, int Index, KFrameStyle Style );


  public: // modification access
    /** sets the spacing in the hex column
      * @param ByteSpacingW spacing between the bytes in pixels
      * @param NewNoOfGroupedBytes numbers of grouped bytes, 0 means no grouping
      * @param GroupSpacingW spacing between the groups in pixels
      * returns true if there was a change
      */
    bool setSpacing( KPixelX ByteSpacingW, int NewNoOfGroupedBytes = 0, KPixelX GroupSpacingW = 0 );
    /** sets the spacing between the bytes in the hex column
      * @param ByteSpacingW spacing between the bytes in pixels
      * returns true if there was a change
      */
    bool setByteSpacingWidth( KPixelX ByteSpacingW );
    /** sets the number of grouped bytes in the hex column
      * @param NewNoOfGroupedBytes numbers of grouped bytes, 0 means no grouping
      * returns true if there was a change
      */
    bool setNoOfGroupedBytes( int NewNoOfGroupedBytes );
    /** sets the spacing between the groups of bytes in the hex column
      * @param GroupSpacingW spacing between the groups in pixels
      * returns true if there was a change
      */
    bool setGroupSpacingWidth( KPixelX GroupSpacingW );
    /** sets width of digits and recalculates depend sizes
      * returns true if there was a change
      */
    bool setDigitWidth( KPixelX DW );
    /** sets the metrics of the used font
      * @param NewDigitWidth the new width of a digit
      * @param NewDigitBaseLine the new baseline of the digits
      */
    void setMetrics( KPixelX NewDigitWidth, KPixelY NewDigitBaseLine );
    /** */
    void set( KDataBuffer *B );
    /** creates new buffer for x-values; to be called on any change of NoOfBytesPerLine or metrics */
    void resetXBuffer();
    /** sets the codec to be used by the char column. */
    void setCodec( KCharCodec *C );

  public: // functional logic
    /** returns byte positions covered by pixels with absolute x-coord x */
    KSection posOfX( KPixelX x, KPixelX w ) const;
    /** returns byte pos at pixel with absolute x-coord x */
    int posOfX( KPixelX x ) const;
    /** returns byte pos at pixel with absolute x-coord x, and sets the flag to true if we are closer to the right */
    int magPosOfX( KPixelX PX ) const;
    /** returns absolute x-coord of byte at position Pos */
    KPixelX xOfPos( int Pos ) const;
    /** returns right absolute x-coord of byte at position Pos */
    KPixelX rightXOfPos( int Pos ) const;
    /** returns byte pos at pixel with relative x-coord x */
    int posOfRelX( KPixelX x ) const;
    /** returns byte positions covered by pixels with relative x-coord x */
    KSection posOfRelX( KPixelX x, KPixelX w ) const;
    /** returns relative x-coord of byte at position Pos */
    KPixelX relXOfPos( int Pos ) const;
    /** returns right relative x-coord of byte at position Pos */
    KPixelX relRightXOfPos( int Pos ) const;
    /** returns the positions that overlap with the absolute x-coords */
    KSection visiblePositions( KPixelX x, KPixelX w ) const;
    /** returns the */
    KPixelXs wideXPixelsOfPos( KSection Positions ) const;
    /** */
    KPixelXs relWideXPixelsOfPos( KSection Positions ) const;

  public: // value access
    KPixelX byteWidth()                      const;
    KPixelX digitWidth()                     const;
    KPixelX groupSpacingWidth()              const;
    KPixelX byteSpacingWidth()               const;
    int noOfGroupedBytes()                   const;

    int firstPos() const;
    int lastPos()  const;
    KSection visiblePositions() const;
    const TDEBufferLayout *layout() const;
    KCharCodec* codec() const;

  protected:
    /** */
    void recalcX();
    void recalcVerticalGridX();


  protected: // API to be refined
    /** default implementation simply prints the byte as ASCII */
    virtual void drawByte( TQPainter *P, char Byte, KHEChar B, const TQColor &Color ) const;
    /** default implementation sets byte width to one digit width */
    virtual void recalcByteWidth();


  protected:
    void paintGrid( TQPainter *P, KSection Range );
    void paintPlain( TQPainter *P, KSection Positions, int Index );
    void paintSelection( TQPainter *P, KSection Positions, int Index, int Flag );
    void paintMarking( TQPainter *P, KSection Positions, int Index, int Flag );
    void paintRange( TQPainter *P, const TQColor &Color, KSection Positions, int Flag );

    bool isSelected( KSection Range, KSection *Selection, unsigned int *Flag ) const;
    bool isMarked( KSection Range, KSection *Marking, unsigned int *Flag ) const;


  protected:
    /** pointer to the buffer */
    KDataBuffer *Buffer;
    /** pointer to the layout */
    const TDEBufferLayout *Layout;
    /** pointer to the ranges */
    TDEBufferRanges *Ranges;
    /** */
    KCharCodec *Codec;

    /** */
    KPixelX DigitWidth;
    /** */
    KPixelY DigitBaseLine;
    /** */
    KPixelX VerticalGridX;
    /** */
    bool VerticalGrid;

  protected:  // individual data
    /** total width of byte display in pixel */
    KPixelX ByteWidth;
    /** width of inserting cursor in pixel */
    KPixelX CursorWidth;
    /** size of the line margin */
    KPixelX ByteSpacingWidth;
    /** width of spacing in pixel */
    KPixelX GroupSpacingWidth;

    /** number of grouped bytes */
    int NoOfGroupedBytes;

    /** pointer to array with buffered positions (relative to column position)
      * a spacing gets assigned to the left byte -> ...c|c|c |c|c...
      */
    KPixelX *PosX;
    KPixelX *PosRightX;
    /** index of right position */
    int LastPos;


  protected: // buffering drawing data
    KSection PaintPositions;
    int PaintLine;
    KPixelX PaintX;
    KPixelX PaintW;
    int SpacingTrigger;
};


inline KPixelX TDEBufferColumn::byteWidth()         const { return ByteWidth; }
inline KPixelX TDEBufferColumn::digitWidth()        const { return DigitWidth; }
inline KPixelX TDEBufferColumn::byteSpacingWidth()  const { return ByteSpacingWidth; }
inline KPixelX TDEBufferColumn::groupSpacingWidth() const { return GroupSpacingWidth; }

inline int TDEBufferColumn::noOfGroupedBytes()      const { return NoOfGroupedBytes; }

inline int TDEBufferColumn::firstPos() const { return PaintPositions.start(); }
inline int TDEBufferColumn::lastPos()  const { return PaintPositions.end(); }
inline KSection TDEBufferColumn::visiblePositions() const { return PaintPositions; }

inline const TDEBufferLayout *TDEBufferColumn::layout() const { return Layout; }


inline void TDEBufferColumn::setCodec( KCharCodec *C ) { Codec = C; }
inline KCharCodec* TDEBufferColumn::codec() const { return Codec; }

}

#endif
