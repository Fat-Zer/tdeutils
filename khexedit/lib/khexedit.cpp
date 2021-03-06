/***************************************************************************
                          khexedit.cpp  -  description
                             -------------------
    begin                : Die Mai 13 2003
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


//#include <kdebug.h>

// c specific
#include <stdlib.h>
//#include <limits.h>
// c++ specific
//#include <limits>
// qt specific
#include <tqstyle.h>
#include <tqpainter.h>
#include <tqtimer.h>
#include <tqcursor.h>
#include <tqapplication.h>
// kde specific
#ifndef TQT_ONLY
#include <tdeglobalsettings.h>
#endif
// lib specific
#include "kdatabuffer.h"
#include "koffsetcolumn.h"
#include "kvaluecolumn.h"
#include "kcharcolumn.h"
#include "kbordercolumn.h"
#include "kbuffercursor.h"
#include "kbufferlayout.h"
#include "kbufferranges.h"
#include "controller/ktabcontroller.h"
#include "controller/knavigator.h"
#include "controller/kvalueeditor.h"
#include "controller/kchareditor.h"
#include "kbufferdrag.h"
#include "kcursor.h"
#include "kbytecodec.h"
#include "kcharcodec.h"
#include "kwordbufferservice.h"
#include "khexedit.h"

using namespace KHE;

// zooming is done in steps of font size points
static const int DefaultZoomStep = 1;
static const int DefaultStartOffset = 0;//5;
static const int DefaultFirstLineOffset = 0;
static const int DefaultNoOfBytesPerLine =  16;
static const KHexEdit::KResizeStyle DefaultResizeStyle = KHexEdit::FullSizeUsage;
static const KHexEdit::KEncoding DefaultEncoding = KHexEdit::LocalEncoding;
static const int DefaultScrollTimerPeriod = 100;
static const int InsertCursorWidth = 2;



KHexEdit::KHexEdit( KDataBuffer *Buffer, TQWidget *Parent, const char *Name, WFlags Flags )
 : KColumnsView( Parent, Name, Flags ),
   DataBuffer( Buffer ),
   BufferLayout( new TDEBufferLayout(DefaultNoOfBytesPerLine,DefaultStartOffset,0) ),
   BufferCursor( new TDEBufferCursor(BufferLayout) ),
   BufferRanges( new TDEBufferRanges(BufferLayout) ),
   CursorBlinkTimer( new TQTimer(this) ),
   ScrollTimer( new TQTimer(this) ),
   DragStartTimer( new TQTimer(this) ),
   TrippleClickTimer( new TQTimer(this) ),
   CursorPixmaps( new KCursor() ),
   Codec( 0 ),
   ClipboardMode( TQClipboard::Clipboard ),
   ResizeStyle( DefaultResizeStyle ),
   Encoding( MaxEncodingId ), // forces update
   ReadOnly( false ),
//    Modified( false ),
   OverWriteOnly( false ),
   OverWrite( true ),
   MousePressed( false ),
   InDoubleClick( false ),
   InDnD( false ),
   DragStartPossible( false ),
     CursorPaused( false ),
   BlinkCursorVisible( false ),
   InZooming( false ),
   d( 0 )
{
  // initalize layout
  if( DataBuffer )
    BufferLayout->setLength( DataBuffer->size() );
  BufferLayout->setNoOfLinesPerPage( noOfLinesPerPage() );

  // creating the columns in the needed order
  OffsetColumn =       new KOffsetColumn( this, DefaultFirstLineOffset, DefaultNoOfBytesPerLine, KOffsetFormat::Hexadecimal );
  FirstBorderColumn =  new KBorderColumn( this, false );
  ValueColumn =        new KValueColumn( this, DataBuffer, BufferLayout, BufferRanges );
  SecondBorderColumn = new KBorderColumn( this, true );
  CharColumn =         new KCharColumn( this, DataBuffer, BufferLayout, BufferRanges );

  // select the active column
  ActiveColumn = &charColumn();
  InactiveColumn = &valueColumn();

  // set encoding
  Codec = KCharCodec::createCodec( (KHE::KEncoding)DefaultEncoding );
  valueColumn().setCodec( Codec );
  charColumn().setCodec( Codec );
  Encoding = DefaultEncoding;

  TabController = new KTabController( this, 0 );
  Navigator = new KNavigator( this, TabController );
  ValueEditor = new KValueEditor( ValueColumn, BufferCursor, this, Navigator );
  CharEditor = new KCharEditor( CharColumn, BufferCursor, this, Navigator );

  Controller = Navigator;

#ifdef TQT_ONLY
  TQFont FixedFont( "fixed", 10 );
  FixedFont.setFixedPitch( true );
  setFont( FixedFont );
#else
  setFont( TDEGlobalSettings::fixedFont() );
#endif

  // get the full control
  viewport()->setFocusProxy( this );
  viewport()->setFocusPolicy( TQ_WheelFocus );

  viewport()->installEventFilter( this );
  installEventFilter( this );

  connect( CursorBlinkTimer, TQT_SIGNAL(timeout()), this, TQT_SLOT(blinkCursor()) );
  connect( ScrollTimer,      TQT_SIGNAL(timeout()), this, TQT_SLOT(autoScrollTimerDone()) );
  connect( DragStartTimer,   TQT_SIGNAL(timeout()), this, TQT_SLOT(startDrag()) );

  viewport()->setAcceptDrops( true );
}


KHexEdit::~KHexEdit()
{
  delete TabController;
  delete Navigator;
  delete ValueEditor;
  delete CharEditor;
}


int KHexEdit::noOfBytesPerLine()               const { return BufferLayout->noOfBytesPerLine(); }
int KHexEdit::firstLineOffset()                const { return OffsetColumn->firstLineOffset(); }
int KHexEdit::startOffset()                    const { return BufferLayout->startOffset(); }
KHexEdit::KResizeStyle KHexEdit::resizeStyle() const { return ResizeStyle; }
KHexEdit::KCoding KHexEdit::coding()           const { return (KHexEdit::KCoding)valueColumn().coding(); }
KPixelX KHexEdit::byteSpacingWidth()           const { return valueColumn().byteSpacingWidth(); }
int KHexEdit::noOfGroupedBytes()               const { return valueColumn().noOfGroupedBytes(); }
KPixelX KHexEdit::groupSpacingWidth()          const { return valueColumn().groupSpacingWidth(); }
KPixelX KHexEdit::binaryGapWidth()             const { return valueColumn().binaryGapWidth(); }
bool KHexEdit::isOverwriteMode()               const { return OverWrite; }
bool KHexEdit::isOverwriteOnly()               const { return OverWriteOnly; }
bool KHexEdit::isReadOnly()                    const { return ReadOnly; }
bool KHexEdit::isModified()                    const { return DataBuffer->isModified(); }
bool KHexEdit::tabChangesFocus()               const { return TabController->tabChangesFocus(); }
bool KHexEdit::showUnprintable()               const { return charColumn().showUnprintable(); }
TQChar KHexEdit::substituteChar()               const { return charColumn().substituteChar(); }
TQChar KHexEdit::undefinedChar()                const { return charColumn().undefinedChar(); }
KHexEdit::KEncoding KHexEdit::encoding()       const { return (KHexEdit::KEncoding)Encoding; }
const TQString &KHexEdit::encodingName()        const { return Codec->name(); }

KSection KHexEdit::selection()                 const { return BufferRanges->selection(); }
int KHexEdit::cursorPosition()                 const { return BufferCursor->index(); }
bool KHexEdit::isCursorBehind()                const { return BufferCursor->isBehind(); }
KHexEdit::TDEBufferColumnId KHexEdit::cursorColumn() const
{ return static_cast<KHE::KValueColumn *>( ActiveColumn ) == &valueColumn()? ValueColumnId : CharColumnId; }

void KHexEdit::setOverwriteOnly( bool OO )    { OverWriteOnly = OO; if( OverWriteOnly ) setOverwriteMode( true ); }
void KHexEdit::setModified( bool M )          { DataBuffer->setModified(M); }
void KHexEdit::setTabChangesFocus( bool TCF ) { TabController->setTabChangesFocus(TCF); }
void KHexEdit::setFirstLineOffset( int FLO )  { OffsetColumn->setFirstLineOffset( FLO ); }

bool KHexEdit::offsetColumnVisible() const { return OffsetColumn->isVisible(); }
int KHexEdit::visibleBufferColumns() const
{ return (valueColumn().isVisible() ? ValueColumnId : 0) | (charColumn().isVisible() ? CharColumnId : 0); }


void KHexEdit::setOverwriteMode( bool OM )
{
  if( (OverWriteOnly && !OM) || (OverWrite == OM) )
    return;

  OverWrite = OM;

  // affected:
  // cursor shape
  bool ChangeCursor = !( CursorPaused || ValueEditor->isInEditMode() );
  if( ChangeCursor )
    pauseCursor();

  BufferCursor->setAppendPosEnabled( !OverWrite );

  if( ChangeCursor )
    unpauseCursor();

  emit cutAvailable( !OverWrite && BufferRanges->hasSelection() );
}


void KHexEdit::setDataBuffer( KDataBuffer *B )
{
  //pauseCursor();
  ValueEditor->reset();
  CursorPaused = true;

  DataBuffer = B;
  valueColumn().set( DataBuffer );
  charColumn().set( DataBuffer);

  // affected:
  // length -> no of lines -> width
  BufferLayout->setLength( DataBuffer->size() );
  adjustLayoutToSize();

  // ensure that the widget is readonly if the buffer is
  if( DataBuffer->isReadOnly() )
    setReadOnly( true );

  updateView();
  BufferCursor->gotoStart();
  ensureCursorVisible();
  unpauseCursor();
}


void KHexEdit::setStartOffset( int SO )
{
  if( !BufferLayout->setStartOffset(SO) )
    return;

  pauseCursor();
  // affects:
  // the no of lines -> width
  adjustLayoutToSize();

  updateView();

  BufferCursor->updateCoord();
  ensureCursorVisible();
  unpauseCursor();
}


void KHexEdit::setReadOnly( bool RO )
{
  // don't set editor readwrite if databuffer is readonly
  ReadOnly = (DataBuffer && DataBuffer->isReadOnly()) ? true : RO;

  Controller = ReadOnly ? (KController*)Navigator :
      cursorColumn() == CharColumnId ? (KController*)CharEditor : (KController*)ValueEditor;
}


void KHexEdit::setBufferSpacing( KPixelX ByteSpacing, int NoOfGroupedBytes, KPixelX GroupSpacing )
{
  if( !valueColumn().setSpacing(ByteSpacing,NoOfGroupedBytes,GroupSpacing) )
    return;

  updateViewByWidth();
}


void KHexEdit::setCoding( KCoding C )
{
  uint OldCodingWidth = valueColumn().byteCodec()->encodingWidth();

  if( !valueColumn().setCoding((KHE::KCoding)C) )
    return;

  uint NewCodingWidth = valueColumn().byteCodec()->encodingWidth();
  ValueEditor->ByteBuffer.setLength( NewCodingWidth ); //hack for now

  // no change in the width?
  if( NewCodingWidth == OldCodingWidth )
    updateColumn( valueColumn() );
  else
    updateViewByWidth();
}


void KHexEdit::setResizeStyle( KResizeStyle NewStyle )
{
  if( ResizeStyle == NewStyle )
    return;

  ResizeStyle = NewStyle;

  updateViewByWidth();
}


void KHexEdit::setNoOfBytesPerLine( int NoBpL )
{
  // if the number is explicitly set we expect a wish for no automatic resize
  ResizeStyle = NoResize;

  if( !BufferLayout->setNoOfBytesPerLine(NoBpL) )
    return;
  updateViewByWidth();
}


void KHexEdit::setByteSpacingWidth( int/*KPixelX*/ BSW )
{
  if( !valueColumn().setByteSpacingWidth(BSW) )
    return;
  updateViewByWidth();
}

void KHexEdit::setNoOfGroupedBytes( int NoGB )
{
  if( !valueColumn().setNoOfGroupedBytes(NoGB) )
    return;
  updateViewByWidth();
}


void KHexEdit::setGroupSpacingWidth( int/*KPixelX*/ GSW )
{
  if( !valueColumn().setGroupSpacingWidth(GSW) )
    return;
  updateViewByWidth();
}


void KHexEdit::setBinaryGapWidth( int/*KPixelX*/ BGW )
{
  if( !valueColumn().setBinaryGapWidth(BGW) )
    return;
  updateViewByWidth();
}


void KHexEdit::setSubstituteChar( TQChar SC )
{
  if( !charColumn().setSubstituteChar(SC) )
    return;
  pauseCursor();
  updateColumn( charColumn() );
  unpauseCursor();
}

void KHexEdit::setUndefinedChar( TQChar UC )
{
  if( !charColumn().setUndefinedChar(UC) )
    return;
  pauseCursor();
  updateColumn( charColumn() );
  unpauseCursor();
}

void KHexEdit::setShowUnprintable( bool SU )
{
  if( !charColumn().setShowUnprintable(SU) )
    return;
  pauseCursor();
  updateColumn( charColumn() );
  unpauseCursor();
}


void KHexEdit::setEncoding( KEncoding C )
{
  if( Encoding == C )
    return;

  KCharCodec *NC = KCharCodec::createCodec( (KHE::KEncoding)C );
  if( NC == 0 )
    return;

  valueColumn().setCodec( NC );
  charColumn().setCodec( NC );

  delete Codec;
  Codec = NC;
  Encoding = C;

  pauseCursor();
  updateColumn( valueColumn() );
  updateColumn( charColumn() );
  unpauseCursor();
}

// TODO: join with function above!
void KHexEdit::setEncoding( const TQString& EncodingName )
{
  if( EncodingName == Codec->name() )
    return;

  KCharCodec *NC = KCharCodec::createCodec( EncodingName );
  if( NC == 0 )
    return;

  valueColumn().setCodec( NC );
  charColumn().setCodec( NC );

  delete Codec;
  Codec = NC;
  Encoding = LocalEncoding; // TODO: add encoding no to every known codec

  pauseCursor();
  updateColumn( valueColumn() );
  updateColumn( charColumn() );
  unpauseCursor();
}


void KHexEdit::fontChange( const TQFont &OldFont )
{
  TQScrollView::fontChange( OldFont );

  if( !InZooming )
    DefaultFontSize = font().pointSize();

  // get new values
  TQFontMetrics FM( fontMetrics() );
  KPixelX DigitWidth = FM.maxWidth();
  KPixelY DigitBaseLine = FM.ascent();

  setLineHeight( FM.height() );

  // update all dependant structures
  BufferLayout->setNoOfLinesPerPage( noOfLinesPerPage() );

  OffsetColumn->setMetrics( DigitWidth, DigitBaseLine );
  valueColumn().setMetrics( DigitWidth, DigitBaseLine );
  charColumn().setMetrics( DigitWidth, DigitBaseLine );

  updateViewByWidth();
}


void KHexEdit::updateViewByWidth()
{
  pauseCursor();

  adjustToLayoutNoOfBytesPerLine();
  adjustLayoutToSize();

  updateView();

  BufferCursor->updateCoord();
  ensureCursorVisible();

  unpauseCursor();
}


void KHexEdit::zoomIn()         { zoomIn( DefaultZoomStep ); }
void KHexEdit::zoomOut()        { zoomOut( DefaultZoomStep ); }

void KHexEdit::zoomIn( int PointInc )
{
  InZooming = true;
  TQFont F( font() );
  F.setPointSize( TQFontInfo(F).pointSize() + PointInc );
  setFont( F );
  InZooming = false;
}

void KHexEdit::zoomOut( int PointDec )
{
  InZooming = true;
  TQFont F( font() );
  F.setPointSize( TQMAX( 1, TQFontInfo(F).pointSize() - PointDec ) );
  setFont( F );
  InZooming = false;
}


void KHexEdit::zoomTo( int PointSize )
{
  InZooming = true;
  TQFont F( font() );
  F.setPointSize( PointSize );
  setFont( F );
  InZooming = false;
}


void KHexEdit::unZoom()
{
  zoomTo( DefaultFontSize );
}


void KHexEdit::adjustLayoutToSize()
{
  // check whether there is a change with the numbers of fitting bytes per line
  if( ResizeStyle != NoResize )
  {
    int FittingBytesPerLine = fittingBytesPerLine( size() );

//     std::cout<<"FitBpL"<<FittingBytesPerLine<<std::endl;

    // changes?
    if( BufferLayout->setNoOfBytesPerLine(FittingBytesPerLine) )
      adjustToLayoutNoOfBytesPerLine();
  }

  setNoOfLines( BufferLayout->noOfLines() );
}


void KHexEdit::adjustToLayoutNoOfBytesPerLine()
{
  OffsetColumn->setDelta( BufferLayout->noOfBytesPerLine() );
  valueColumn().resetXBuffer();
  charColumn().resetXBuffer();

  updateWidths();
}


void KHexEdit::setNoOfLines( int NewNoOfLines )
{
  KColumnsView::setNoOfLines( NewNoOfLines>1?NewNoOfLines:1 );
}


void KHexEdit::toggleOffsetColumn( bool Visible )
{
  bool OCVisible = OffsetColumn->isVisible();
  // no change?
  if( OCVisible == Visible )
    return;

  OffsetColumn->setVisible( Visible );
  FirstBorderColumn->setVisible( Visible );

  updateViewByWidth();
}


TQSize KHexEdit::sizeHint() const
{
  return TQSize( totalWidth(), totalHeight() );
}


TQSize KHexEdit::minimumSizeHint() const
{
  // TODO: better minimal width (visibility!)
  return TQSize( OffsetColumn->visibleWidth()+FirstBorderColumn->visibleWidth()+SecondBorderColumn->visibleWidth()+valueColumn().byteWidth()+charColumn().byteWidth(),
                lineHeight() + noOfLines()>1? style().pixelMetric(TQStyle::PM_ScrollBarExtent):0 );
}


void KHexEdit::resizeEvent( TQResizeEvent *ResizeEvent )
{
  if( ResizeStyle != NoResize )
  {
    int FittingBytesPerLine = fittingBytesPerLine( ResizeEvent->size() );

    // changes?
    if( BufferLayout->setNoOfBytesPerLine(FittingBytesPerLine) )
    {
      setNoOfLines( BufferLayout->noOfLines() );
      updateViewByWidth();
    }
  }

  TQScrollView::resizeEvent( ResizeEvent );

  BufferLayout->setNoOfLinesPerPage( noOfLinesPerPage() ); // TODO: doesn't work with the new size!!!
}


int KHexEdit::fittingBytesPerLine( const TQSize &NewSize ) const
{
  KPixelX ReservedWidth = OffsetColumn->visibleWidth() + FirstBorderColumn->visibleWidth() + SecondBorderColumn->visibleWidth();

  // abstract framewidth as well as offset and border columns width
  int UsedbyFrameWidth = 2 * frameWidth();
  KPixelX FullWidth = NewSize.width() - UsedbyFrameWidth - ReservedWidth;

//  // no width left for resizeable columns? TODO: put this in resizeEvent
//  if( FullWidth < 0 )
//    return;

  KPixelY FullHeight = NewSize.height() - UsedbyFrameWidth;

  // check influence of dis-/appearing of the vertical scrollbar
  bool VerticalScrollbarIsVisible = verticalScrollBar()->isVisible();
  KPixelX ScrollbarExtent = style().pixelMetric( TQStyle::PM_ScrollBarExtent );//verticalScrollBar()->width();

  KPixelX AvailableWidth = FullWidth;
  if( VerticalScrollbarIsVisible )
    AvailableWidth -= ScrollbarExtent;

  enum KMatchTrial { FirstRun, RerunWithScrollbarOn, TestWithoutScrollbar };
  KMatchTrial MatchRun = FirstRun;

  // prepare needed values
  KPixelX DigitWidth = valueColumn().digitWidth();
  KPixelX TextByteWidth = charColumn().isVisible() ? DigitWidth : 0;
  KPixelX HexByteWidth = valueColumn().isVisible() ? valueColumn().byteWidth() : 0;
  KPixelX ByteSpacingWidth = valueColumn().isVisible() ? valueColumn().byteSpacingWidth() : 0;
  KPixelX GroupSpacingWidth;
  int NoOfGroupedBytes = valueColumn().noOfGroupedBytes();
  // no grouping?
  if( NoOfGroupedBytes == 0 )
  {
    // faking grouping by 1
    NoOfGroupedBytes = 1;
    GroupSpacingWidth = 0;
  }
  else
    GroupSpacingWidth = valueColumn().isVisible() ? valueColumn().groupSpacingWidth() : 0;

  KPixelX HexByteGroupWidth =  NoOfGroupedBytes * HexByteWidth + (NoOfGroupedBytes-1)*ByteSpacingWidth;
  KPixelX TextByteGroupWidth = NoOfGroupedBytes * TextByteWidth;
  KPixelX TotalGroupWidth = HexByteGroupWidth + GroupSpacingWidth + TextByteGroupWidth;

  int FittingBytesPerLine;
  int WithScrollbarFittingBytesPerLine = 0;
  for(;;)
  {
//    std::cout << "matchWidth: " << FullWidth
//              << " (v:" << visibleWidth()
//              << ", f:" << frameWidth()
//              << ", A:" << AvailableWidth
//              << ", S:" << ScrollbarExtent
//              << ", R:" << ReservedWidth << ")" << std::endl;

    // calculate fitting groups per line
    int FittingGroupsPerLine = (AvailableWidth+GroupSpacingWidth) // fake spacing after last group
                               / TotalGroupWidth;

    // calculate the fitting bytes per line by groups
    FittingBytesPerLine = NoOfGroupedBytes * FittingGroupsPerLine;

    // not only full groups?
    if( ResizeStyle == FullSizeUsage && NoOfGroupedBytes > 1 )
    {
      if( FittingGroupsPerLine > 0 )
        AvailableWidth -= FittingGroupsPerLine*TotalGroupWidth; // includes additional spacing after last group

//        std::cout << "Left: " << AvailableWidth << "("<<HexByteWidth<<", "<<TextByteWidth<<")" << std::endl;

      if( AvailableWidth > 0 )
        FittingBytesPerLine += (AvailableWidth+ByteSpacingWidth) / (HexByteWidth+ByteSpacingWidth+TextByteWidth);

      // is there not even the space for a single byte?
      if( FittingBytesPerLine == 0 )
      {
        // ensure at least one byte per line
        FittingBytesPerLine = 1;
        // and
        break;
      }
    }
    // is there not the space for a single group?
    else if( FittingBytesPerLine == 0 )
    {
      // ensures at least one group
      FittingBytesPerLine = NoOfGroupedBytes;
      break;
    }

//    std::cout << "meantime: " << FittingGroupsPerLine << " (T:" << TotalGroupWidth
//              << ", h:" << HexByteGroupWidth
//              << ", t:" << TextByteGroupWidth
//              << ", s:" << GroupSpacingWidth << ") " <<FittingBytesPerLine<< std::endl;

    int NewNoOfLines = (BufferLayout->length()+BufferLayout->startOffset()+FittingBytesPerLine-1)
                       / FittingBytesPerLine;
    KPixelY NewHeight =  NewNoOfLines * LineHeight;

    if( VerticalScrollbarIsVisible )
    {
      if( MatchRun == TestWithoutScrollbar )
      {
        // did the test without the scrollbar fail, don't the data fit into the view?
        if( NewHeight>FullHeight )
          // reset to old calculated value
          FittingBytesPerLine =  WithScrollbarFittingBytesPerLine;
        break;
      }

      // a chance for to perhaps fit in height?
      if( FittingBytesPerLine <= BufferLayout->noOfBytesPerLine() )
      {
        // remember this trial's result and calc number of bytes with vertical scrollbar on
        WithScrollbarFittingBytesPerLine = FittingBytesPerLine;
        AvailableWidth = FullWidth;
        MatchRun = TestWithoutScrollbar;
//          std::cout << "tested without scrollbar..." << std::endl;
        continue;
      }
    }
    else
    {
      // doesn't it fit into the height anymore?
      if( NewHeight>FullHeight && MatchRun==FirstRun )
      {
        // need for a scrollbar has risen... ->less width, new calculation
        AvailableWidth = FullWidth - ScrollbarExtent;
        MatchRun = RerunWithScrollbarOn;
//          std::cout << "rerun with scrollbar on..." << std::endl;
        continue;
      }
    }

    break;
  }

  return FittingBytesPerLine;
}


bool KHexEdit::selectWord( /*unsigned TODO:change all unneeded signed into unsigned!*/ int Index )
{
  if( Index >= 0 && Index < BufferLayout->length()  )
  {
    KWordBufferService WBS( DataBuffer, Codec );
    KSection WordSection = WBS.wordSection( Index );
    if( WordSection.isValid() )
    {
      pauseCursor();

      BufferRanges->setFirstWordSelection( WordSection );
      BufferCursor->gotoIndex( WordSection.end()+1 );
      repaintChanged();

      unpauseCursor();
      return true;
    }
  }
  return false;
}

void KHexEdit::select( KSection Section )
{
  if( !Section.isValid() )
    return;

  Section.restrictTo( KSection(0,BufferLayout->length()-1) );

  pauseCursor();

  BufferRanges->setSelection( Section );
  BufferCursor->gotoIndex( Section.end()+1 );
  repaintChanged();

  unpauseCursor();

  if( !OverWrite ) emit cutAvailable( BufferRanges->hasSelection() );
  emit copyAvailable( BufferRanges->hasSelection() );
  emit selectionChanged( Section.start(), Section.end() );
}

void KHexEdit::selectAll( bool Select )
{
  KSection Selection;

  pauseCursor( true );

  if( !Select )
    BufferRanges->removeSelection();
  else
  {
    Selection.set( 0, BufferLayout->length()-1 );
    BufferRanges->setSelection( Selection );
    BufferCursor->gotoEnd();
  }

  repaintChanged();

  unpauseCursor();

  if( !OverWrite ) emit cutAvailable( BufferRanges->hasSelection() );
  emit copyAvailable( BufferRanges->hasSelection() );
  emit selectionChanged( Selection.start(), Selection.end() );
  viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
}


bool KHexEdit::hasSelectedData() const
{
  return BufferRanges->hasSelection();
}


TQByteArray KHexEdit::selectedData() const
{
  if( !BufferRanges->hasSelection() )
    return TQByteArray();

  KSection Selection = BufferRanges->selection();
  TQByteArray SD( Selection.width() );
  DataBuffer->copyTo( SD.data(), Selection.start(), Selection.width() );
  return SD;
}


TDEBufferDrag *KHexEdit::dragObject( TQWidget *Parent ) const
{
  if( !BufferRanges->hasSelection() )
    return 0;

  const KOffsetColumn *OC;
  const KValueColumn *HC;
  const KCharColumn *TC;
  KCoordRange Range;

  if( static_cast<KHE::KCharColumn *>( ActiveColumn ) == &charColumn() )
  {
    OC = 0;
    HC = 0;
    TC = 0;
  }
  else
  {
    OC = OffsetColumn->isVisible() ? OffsetColumn : 0;
    HC = valueColumn().isVisible() ? &valueColumn() : 0;
    TC = charColumn().isVisible() ? &charColumn() : 0;
    KSection S = BufferRanges->selection();
    Range.set( BufferLayout->coordOfIndex(S.start()),BufferLayout->coordOfIndex(S.end()) );
  }

  return new TDEBufferDrag( selectedData(), Range, OC, HC, TC,
                          charColumn().substituteChar(), charColumn().undefinedChar(),
                          Codec->name(), Parent );
}


void KHexEdit::cut()
{
  if( isReadOnly() || OverWrite )
    return;

  TDEBufferDrag *Drag = dragObject();
  if( !Drag )
    return;

  TQApplication::clipboard()->setData( Drag, ClipboardMode );

  removeSelectedData();
}


void KHexEdit::copy()
{
  TDEBufferDrag *Drag = dragObject();
  if( !Drag )
    return;

  TQApplication::clipboard()->setData( Drag, ClipboardMode );
}


void KHexEdit::paste()
{
  if( isReadOnly() )
    return;

  TQMimeSource *Source = TQApplication::clipboard()->data( ClipboardMode );
  pasteFromSource( Source );
}


void KHexEdit::pasteFromSource( TQMimeSource *Source )
{
  if( !Source || !TDEBufferDrag::canDecode(Source) )
    return;

  TQByteArray Data;
  if( !TDEBufferDrag::decode(Source,Data) )
    return;

  if( !Data.isEmpty() )
    insert( Data );
}


void KHexEdit::insert( const TQByteArray &D )
{
  pauseCursor( true );

  KSection ChangedRange;

  if( OverWrite )
  {
    if( BufferRanges->hasSelection() )
    {
      // replacing the selection:
      // we restrict the replacement to the minimum length of selection and input
      ChangedRange = BufferRanges->selection();
      ChangedRange.restrictEndTo( ChangedRange.start()+D.size()-1 );
      int W = DataBuffer->replace( ChangedRange, D.data(), ChangedRange.width() );
      BufferCursor->gotoCIndex( ChangedRange.start()+W );
      BufferRanges->removeSelection();
    }
    else
    {
      if( !BufferCursor->isBehind() )
      {
        // replacing the normal data, at least until the end
        ChangedRange.setByWidth( BufferCursor->realIndex(), D.size() );
        ChangedRange.restrictEndTo( BufferLayout->length()-1 );
        if( ChangedRange.isValid() )
        {
          int W = DataBuffer->replace( ChangedRange, D.data(), ChangedRange.width() );
          BufferCursor->gotoNextByte( W );
        }
      }
    }
  }
  else
  {
    if( BufferRanges->hasSelection() )
    {
      // replacing the selection
      KSection Selection = BufferRanges->selection();
      int OldLastIndex = BufferLayout->length() - 1;
      int W = DataBuffer->replace( Selection, D.data(), D.size() );
      updateLength();
      BufferCursor->gotoIndex( Selection.start() + W );
      if( W > 0 )
      {
        if( Selection.width() == (int)D.size() )
          ChangedRange = Selection;
        else
        {
          int NewLastIndex = DataBuffer->size() - 1;
          ChangedRange.set( Selection.start(), NewLastIndex>OldLastIndex?NewLastIndex:OldLastIndex );
        }
      }
      BufferRanges->removeSelection();
    }
    else
    {
      bool Appending = BufferCursor->atAppendPos();
      int OldIndex = BufferCursor->realIndex();
      int W = DataBuffer->insert( OldIndex, D.data(), D.size() );
      updateLength();
      // worked?
      if( W > 0 )
      {
        if( Appending )
          BufferCursor->gotoEnd();
        else
          BufferCursor->gotoNextByte( W );
        ChangedRange.set( OldIndex, DataBuffer->size()-1 );
      }
    }
  }

  bool Changed = ChangedRange.isValid();
  if( Changed )
  {
    BufferRanges->addChangedRange( ChangedRange );
    repaintChanged();
  }
  ensureCursorVisible();

  unpauseCursor();

  if( Changed ) emit bufferChanged( ChangedRange.start(), ChangedRange.end() );
  KSection Selection = BufferRanges->selection();
  emit selectionChanged( Selection.start(), Selection.end() );
}


void KHexEdit::removeSelectedData()
{
  // Can't we do this?
  if( isReadOnly() || OverWrite || ValueEditor->isInEditMode() )
    return;

  pauseCursor();

  KSection Selection = BufferRanges->selection();

  BufferRanges->removeFurtherSelections();

  KSection ChangedRange = removeData( Selection );
  BufferRanges->removeSelection();

  repaintChanged();

  BufferCursor->gotoCIndex( Selection.start() );

  ensureCursorVisible();
//     clearUndoRedo();
  viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );

  unpauseCursor();

  if( ChangedRange.isValid() ) emit bufferChanged( ChangedRange.start(), ChangedRange.end() );
  emit selectionChanged( -1, -1 );
}


KSection KHexEdit::removeData( KSection Indizes )
{
//   if( undoEnabled )
//   {
//     checkUndoRedoInfo( UndoRedoInfo::RemoveSelected );
//     if( !undoRedoInfo.valid() )
//     {
//       doc->selectionStart( selNum, undoRedoInfo.id, undoRedoInfo.index );
//       undoRedoInfo.d->text = TQString();
//     }
//     readFormats( c1, c2, undoRedoInfo.d->text, TRUE );
//   }

  KSection ChangedRange( Indizes.start(), BufferLayout->length()-1 );
  // do it!
  DataBuffer->remove( Indizes );
  updateLength();
  BufferRanges->addChangedRange( ChangedRange );

  return ChangedRange;
}


void KHexEdit::updateLength()
{
  BufferLayout->setLength( DataBuffer->size() );
  setNoOfLines( BufferLayout->noOfLines() );
}


void KHexEdit::clipboardChanged()
{
  // don't listen to selection changes
  disconnect( TQApplication::clipboard(), TQT_SIGNAL(selectionChanged()), this, 0 );
  selectAll( false );
}


void KHexEdit::setCursorPosition( int Index, bool Behind )
{
  pauseCursor( true );

  BufferCursor->gotoCIndex( Index );
  if( Behind )
    BufferCursor->stepBehind();

  BufferRanges->removeSelection();
  bool RangesModifed = BufferRanges->isModified();
  if( RangesModifed )
  {
    repaintChanged();

    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );

  }
  ensureCursorVisible();
  unpauseCursor();

  if( RangesModifed )
  {
    if( !OverWrite ) emit cutAvailable( BufferRanges->hasSelection() );
    emit copyAvailable( BufferRanges->hasSelection() );
    emit selectionChanged( -1, -1 );
  }
}


void KHexEdit::showBufferColumns( int CCs )
{
  int Columns = visibleBufferColumns();

  // no changes or no column selected?
  if( CCs == Columns || !(CCs&( ValueColumnId | CharColumnId )) )
    return;

  valueColumn().setVisible( ValueColumnId & CCs );
  charColumn().setVisible( CharColumnId & CCs );
  SecondBorderColumn->setVisible( CCs == (ValueColumnId|CharColumnId) );

  // active column not visible anymore?
  if( !activeColumn().isVisible() )
  {
    TDEBufferColumn *H = ActiveColumn;
    ActiveColumn = InactiveColumn;
    InactiveColumn = H;
    Controller = ReadOnly ? (KController*)Navigator :
      cursorColumn() == CharColumnId ? (KController*)CharEditor : (KController*)ValueEditor;
  }

  updateViewByWidth();
}


void KHexEdit::setCursorColumn( TDEBufferColumnId CC )
{
  // no changes or not visible?
  if( CC == cursorColumn()
      || (CC == ValueColumnId && !valueColumn().isVisible())
      || (CC == CharColumnId && !charColumn().isVisible()) )
    return;

  pauseCursor( true );

  if( CC == ValueColumnId )
  {
    ActiveColumn = &valueColumn();
    InactiveColumn = &charColumn();
  }
  else
  {
    ActiveColumn = &charColumn();
    InactiveColumn = &valueColumn();
  }
  Controller = ReadOnly ? (KController*)Navigator :
    cursorColumn() == CharColumnId ? (KController*)CharEditor : (KController*)ValueEditor;

  ensureCursorVisible();
  unpauseCursor();
}


void KHexEdit::placeCursor( const TQPoint &Point )
{
  resetInputContext();

  // switch active column if needed
  if( charColumn().isVisible() && Point.x() >= charColumn().x() )
  {
    ActiveColumn = &charColumn();
    InactiveColumn = &valueColumn();
  }
  else
  {
    ActiveColumn = &valueColumn();
    InactiveColumn = &charColumn();
  }
  Controller = ReadOnly ? (KController*)Navigator :
      cursorColumn() == CharColumnId ? (KController*)CharEditor : (KController*)ValueEditor;

  // get coord of click and whether this click was closer to the end of the pos
  TDEBufferCoord C( activeColumn().magPosOfX(Point.x()), lineAt(Point.y()) );

  BufferCursor->gotoCCoord( C );
}


int KHexEdit::indexByPoint( const TQPoint &Point ) const
{
  const TDEBufferColumn *C;
  if( charColumn().isVisible() && Point.x() >= charColumn().x() )
    C = &charColumn();
  else
    C = &valueColumn();

  TDEBufferCoord Coord( C->posOfX(Point.x()), lineAt(Point.y()) );

  return BufferLayout->indexAtCCoord( Coord );
}


void KHexEdit::showEvent( TQShowEvent *e )
{
    KColumnsView::showEvent( e );
    BufferLayout->setNoOfLinesPerPage( noOfLinesPerPage() );
}


bool KHexEdit::eventFilter( TQObject *O, TQEvent *E )
{
  if( TQT_BASE_OBJECT(O) == TQT_BASE_OBJECT(this) || TQT_BASE_OBJECT(O) == TQT_BASE_OBJECT(viewport()) )
  {
    if( E->type() == TQEvent::FocusIn )
    {
      startCursor();
    }
    else if( E->type() == TQEvent::FocusOut )
    {
      stopCursor();
    }
  }

//   if( O == this && E->type() == TQEvent::PaletteChange )
//   {
//     TQColor old( viewport()->colorGroup().color(TQColorGroup::Text) );
//
//     if( old != colorGroup().color(TQColorGroup::Text) )
//     {
//       TQColor c( colorGroup().color(TQColorGroup::Text) );
//       doc->setMinimumWidth( -1 );
//       doc->setDefaultFormat( doc->formatCollection()->defaultFormat()->font(), c );
//       lastFormatted = doc->firstParagraph();
//       formatMore();
//       repaintChanged();
//     }
//   }

  return TQScrollView::eventFilter( O, E );
}


void KHexEdit::blinkCursor()
{
  // skip the cursor drawing?
  if( CursorPaused || ValueEditor->isInEditMode() )
    return;

  // switch the cursor state
  paintActiveCursor( !BlinkCursorVisible );
}


void KHexEdit::startCursor()
{
  CursorPaused = false;

  updateCursor();

  CursorBlinkTimer->start( TQApplication::cursorFlashTime()/2 );
}


void KHexEdit::unpauseCursor()
{
  CursorPaused = false;

  if( CursorBlinkTimer->isActive() )
    updateCursor();
}


void KHexEdit::updateCursor()
{
  createCursorPixmaps();

  paintActiveCursor( true );
  paintInactiveCursor( true );
}


void KHexEdit::stopCursor()
{
  CursorBlinkTimer->stop();

  pauseCursor();
}


void KHexEdit::pauseCursor( bool LeaveEdit )
{
  paintActiveCursor( false );
  paintInactiveCursor( false );

  if( LeaveEdit )
    ValueEditor->InEditMode = false;
  CursorPaused = true;
}


void KHexEdit::createCursorPixmaps()
{
  // create CursorPixmaps
  CursorPixmaps->setSize( activeColumn().byteWidth(), LineHeight );

  int Index = BufferCursor->validIndex();

  TQPainter Paint;
  Paint.begin( const_cast<TQPixmap*>(&CursorPixmaps->offPixmap()), this );
  activeColumn().paintByte( &Paint, Index );
  Paint.end();

  Paint.begin( const_cast<TQPixmap*>(&CursorPixmaps->onPixmap()), this );
  activeColumn().paintCursor( &Paint, Index );
  Paint.end();

  // calculat the shape
  KPixelX CursorX;
  KPixelX CursorW;
  if( BufferCursor->isBehind() )
  {
    CursorX = TQMAX( 0, CursorPixmaps->onPixmap().width()-InsertCursorWidth );
    CursorW = InsertCursorWidth;
  }
  else
  {
    CursorX = 0;
    CursorW = OverWrite ? -1 : InsertCursorWidth;
  }
  CursorPixmaps->setShape( CursorX, CursorW );
}


void KHexEdit::pointPainterToCursor( TQPainter &Painter, const TDEBufferColumn &Column ) const
{
  int x = Column.xOfPos( BufferCursor->pos() ) - contentsX();
  int y = LineHeight * BufferCursor->line() - contentsY();

  Painter.begin( viewport() );
  Painter.translate( x, y );
}


void KHexEdit::paintActiveCursor( bool CursorOn )
{
  // any reason to skip the cursor drawing?
  if( !isUpdatesEnabled() || !viewport()->isUpdatesEnabled()
      || (CursorOn && !hasFocus() && !viewport()->hasFocus() && !InDnD ) )
    return;

  TQPainter Painter;
  pointPainterToCursor( Painter, activeColumn() );

  // paint edited byte?
  if( ValueEditor->isInEditMode() )
  {
    int Index = BufferCursor->index();

    if( CursorOn )
      valueColumn().paintEditedByte( &Painter, ValueEditor->EditValue, ValueEditor->ByteBuffer );
    else
      valueColumn().paintByte( &Painter, Index );
  }
  else
  {

    Painter.drawPixmap( CursorPixmaps->cursorX(), 0,
                        CursorOn?CursorPixmaps->onPixmap():CursorPixmaps->offPixmap(),
                        CursorPixmaps->cursorX(),0,CursorPixmaps->cursorW(),-1 );
    // store state
    BlinkCursorVisible = CursorOn;
  }
}


void KHexEdit::paintInactiveCursor( bool CursorOn )
{
  // any reason to skip the cursor drawing?
  if( !isUpdatesEnabled()
      || !viewport()->isUpdatesEnabled()
      || !inactiveColumn().isVisible()
      || (CursorOn && !hasFocus() && !viewport()->hasFocus() && !InDnD)  )
    return;

  int Index = BufferCursor->validIndex();

  TQPainter Painter;
  pointPainterToCursor( Painter, inactiveColumn() );
  if( CursorOn )
  {
    TDEBufferColumn::KFrameStyle Style =
      BufferCursor->isBehind() ? TDEBufferColumn::Right :
      (OverWrite||ValueEditor->isInEditMode()) ? TDEBufferColumn::Frame :
      TDEBufferColumn::Left;
    inactiveColumn().paintFramedByte( &Painter, Index, Style );
  }
  else
    inactiveColumn().paintByte( &Painter, Index );
}


void KHexEdit::drawContents( TQPainter *P, int cx, int cy, int cw, int ch )
{
  KColumnsView::drawContents( P, cx, cy, cw, ch );
  // TODO: update non blinking cursors. Should this perhaps be done in the buffercolumn?
  // Then it needs to know about inactive, insideByte and the like... well...
  // perhaps subclassing the buffer columns even more, to KCharColumn and KValueColumn?

  if( !CursorPaused && visibleLines(KPixelYs(cy,ch,false)).includes(BufferCursor->line()) )
  {
    paintActiveCursor( true );
    paintInactiveCursor( true );
  }
}

void KHexEdit::updateColumn( KColumn &Column )
{
  //kdDebug(1501) << "updateColumn\n";
  if( Column.isVisible() )
    updateContents( Column.x(), 0, Column.width(), totalHeight() );
}


void KHexEdit::keyPressEvent( TQKeyEvent *KeyEvent )
{
  if( !Controller->handleKeyPress( KeyEvent ) )
    KeyEvent->ignore();
}


void KHexEdit::repaintChanged()
{
  if( !isUpdatesEnabled() || !viewport()->isUpdatesEnabled() || !BufferRanges->isModified() )
    return;

  // TODO: we do this only to let the scrollview handle new or removed lines. overlaps with repaintRange
  resizeContents( totalWidth(), totalHeight() );

  KPixelXs Xs( contentsX(), visibleWidth(), true );

  // collect affected buffer columns
  TQPtrList<TDEBufferColumn> RepaintColumns;

  TDEBufferColumn *C = ValueColumn;
  while( true )
  {
    if( C->isVisible() && C->overlaps(Xs) )
    {
      RepaintColumns.append( C );
      C->preparePainting( Xs );
    }

    if( C == CharColumn )
      break;
    C = CharColumn;
  }

  // any colums to paint?
  if( RepaintColumns.count() > 0 )
  {
    KPixelYs Ys( contentsY(), visibleHeight(), true );

    // calculate affected lines/indizes
    KSection FullPositions( 0, BufferLayout->noOfBytesPerLine()-1 );
    KCoordRange VisibleRange( FullPositions, visibleLines(Ys) );

    KCoordRange ChangedRange;
    // as there might be multiple selections on this line redo until no more is changed
    while( hasChanged(VisibleRange,&ChangedRange) )
    {
//       std::cout << "  changed->"<<FirstChangedIndex<<","<<LastChangedIndex<<std::endl;

      // only one line?
      if( ChangedRange.start().line() == ChangedRange.end().line() )
        for( TDEBufferColumn *C=RepaintColumns.first(); C; C=RepaintColumns.next() )
          paintLine( C, ChangedRange.start().line(),
                     KSection(ChangedRange.start().pos(),ChangedRange.end().pos()) );
      //
      else
      {
        // first line
        for( TDEBufferColumn *C=RepaintColumns.first(); C; C=RepaintColumns.next() )
          paintLine( C, ChangedRange.start().line(),
                     KSection(ChangedRange.start().pos(),FullPositions.end()) );

        // at least one full line?
        for( int l = ChangedRange.start().line()+1; l < ChangedRange.end().line(); ++l )
          for( TDEBufferColumn *C=RepaintColumns.first(); C; C=RepaintColumns.next() )
            paintLine( C, l, FullPositions );

        // last line
        for( TDEBufferColumn *C=RepaintColumns.first(); C; C=RepaintColumns.next() )
          paintLine( C, ChangedRange.end().line(),
                     KSection(FullPositions.start(),ChangedRange.end().pos()) );
      }

      // continue the search at the overnext index
      VisibleRange.setStart( ChangedRange.end()+2 );
      if( !VisibleRange.isValid() )
        break;
    }
  }


  // Paint possible removed bytes at the end of the last line
  // Paint new/removed trailing lines
//   drawContents( P, cx, cy, cw, ch );
  // Paint empty rects
//  paintEmptyArea( P, cx, cy, cw, ch );
//   BufferLayout->noOfLines()

  BufferRanges->resetChangedRanges();
}


void KHexEdit::paintLine( TDEBufferColumn *C, int Line, KSection Positions )
{
  Positions.restrictTo( C->visiblePositions() );

  // nothing to paint?
  if( !Positions.isValid() )
    return;
//   std::cout << "  paintLine->"<<Line<< ":"<<FirstPos<<","<<LastPos<<std::endl;

  // calculating pixel values
  KPixelXs XPixels = C->wideXPixelsOfPos( Positions );

  KPixelY cy = Line * LineHeight;

  // to avoid flickers we first paint to the linebuffer
  TQPainter Paint;
  Paint.begin( &LineBuffer, this );

  Paint.translate( C->x(), 0 );
  C->paintPositions( &Paint, Line, Positions );
  Paint.translate( -C->x(), 0 );

  if( HorizontalGrid && XPixels.start() < TotalWidth )
    Paint.drawLine( XPixels.start(), LineHeight-1, XPixels.width(), LineHeight-1 );  // TODO: use a additional TotalHeight?

  Paint.end();
  // copy to screen
  bitBlt( viewport(), XPixels.start() - contentsX(), cy - contentsY(),
          &LineBuffer, XPixels.start(), 0, XPixels.width(), LineHeight );
}


bool KHexEdit::hasChanged( const KCoordRange &VisibleRange, KCoordRange *ChangedRange ) const
{
  if( !BufferRanges->overlapsChanges(VisibleRange,ChangedRange) )
    return false;

  ChangedRange->restrictTo( VisibleRange );
  return true;
}


void KHexEdit::ensureCursorVisible()
{
//   // Not visible or the user is draging the window, so don't position to caret yet
//   if ( !isVisible() || isHorizontalSliderPressed() || isVerticalSliderPressed() )
//   {
//     d->ensureCursorVisibleInShowEvent = true;
//     return;
//   }

  KPixelX x = activeColumn().xOfPos( BufferCursor->pos() )+ activeColumn().byteWidth()/2;
  KPixelY y = LineHeight * BufferCursor->line() + LineHeight/2;
  int xMargin = activeColumn().byteWidth()/2 + 1;
  int yMargin = LineHeight/2 + 1;
  ensureVisible( x, y, xMargin, yMargin );
}



void KHexEdit::contentsMousePressEvent( TQMouseEvent *e )
{
//   clearUndoRedo();
  pauseCursor( true );

  // care about a left button press?
  if( e->button() == Qt::LeftButton )
  {
    MousePressed = true;

    // select whole line?
    if( TrippleClickTimer->isActive()
        && (e->globalPos()-DoubleClickPoint).manhattanLength() < TQApplication::startDragDistance() )
    {
      BufferRanges->setSelectionStart( BufferLayout->indexAtLineStart(DoubleClickLine) );
      BufferCursor->gotoLineEnd();
      BufferRanges->setSelectionEnd( BufferCursor->realIndex() );
      repaintChanged();

      unpauseCursor();
      return;
    }

    TQPoint MousePoint = e->pos();
    placeCursor( MousePoint );
    ensureCursorVisible();

    // start of a drag perhaps?
    if( BufferRanges->selectionIncludes(BufferCursor->index()) )
    {
      DragStartPossible = true;
      DragStartTimer->start( TQApplication::startDragTime(), true );
      DragStartPoint = MousePoint;

      unpauseCursor();
      return;
    }

    int RealIndex = BufferCursor->realIndex();
    if( BufferRanges->selectionStarted() )
    {
      if( e->state() & ShiftButton )
        BufferRanges->setSelectionEnd( RealIndex );
      else
      {
        BufferRanges->removeSelection();
        BufferRanges->setSelectionStart( RealIndex );
      }
    }
    else // start of a new selection possible
    {
      BufferRanges->setSelectionStart( RealIndex );

      if( !isReadOnly() && (e->state()&ShiftButton) ) // TODO: why only for readwrite?
        BufferRanges->setSelectionEnd( RealIndex );
    }

    BufferRanges->removeFurtherSelections();
  }
  else if( e->button() == Qt::MidButton )
    BufferRanges->removeSelection();

  if( BufferRanges->isModified() )
  {
    repaintChanged();
    viewport()->setCursor( isReadOnly() ? arrowCursor : ibeamCursor );
  }

  unpauseCursor();
}


void KHexEdit::contentsMouseMoveEvent( TQMouseEvent *e )
{
  if( MousePressed )
  {
    if( DragStartPossible )
    {
      DragStartTimer->stop();
      // moved enough for a drag?
      if( (e->pos()-DragStartPoint).manhattanLength() > TQApplication::startDragDistance() )
        startDrag();
      if( !isReadOnly() )
        viewport()->setCursor( ibeamCursor );
      return;
    }
    // selecting
    TQPoint MousePoint = e->pos();
    handleMouseMove( MousePoint );
  }
  else if( !isReadOnly() )
  {
    // visual feedback for possible dragging
    bool InSelection = BufferRanges->hasSelection() && BufferRanges->selectionIncludes( indexByPoint(e->pos()) );
    viewport()->setCursor( InSelection?arrowCursor:ibeamCursor );
  }
}


void KHexEdit::contentsMouseReleaseEvent( TQMouseEvent *e )
{
  // this is not the release of a doubleclick so we need to process it?
  if( !InDoubleClick )
  {
    int Line = lineAt( e->pos().y() );
    int Pos = activeColumn().posOfX( e->pos().x() ); // TODO: can we be sure here about the active column?
    int Index = BufferLayout->indexAtCCoord( TDEBufferCoord(Pos,Line) ); // TODO: can this be another index than the one of the cursor???
    emit clicked( Index );
  }

  if( MousePressed )
  {
    MousePressed = false;

    if( ScrollTimer->isActive() )
      ScrollTimer->stop();

    // was only click inside selection, nothing dragged?
    if( DragStartPossible )
    {
      selectAll( false );
      DragStartTimer->stop();
      DragStartPossible = false;

      unpauseCursor();
    }
    // was end of selection operation?
    else if( BufferRanges->hasSelection() )
    {
      if( TQApplication::clipboard()->supportsSelection() )
      {
        ClipboardMode = TQClipboard::Selection;
        disconnect( TQApplication::clipboard(), TQT_SIGNAL(selectionChanged()), this, 0);

        copy();

        connect( TQApplication::clipboard(), TQT_SIGNAL(selectionChanged()), this, TQT_SLOT(clipboardChanged()) );
        ClipboardMode = TQClipboard::Clipboard;
      }
    }
  }
  // middle mouse button paste?
  else if( e->button() == Qt::MidButton && !isReadOnly() )
  {
    pauseCursor();

    placeCursor( e->pos() );

    // replace no selection?
    if( BufferRanges->hasSelection() && !BufferRanges->selectionIncludes(BufferCursor->index()) )
      BufferRanges->removeSelection();

    ClipboardMode = TQClipboard::Selection;
    paste();
    ClipboardMode = TQClipboard::Clipboard;

    // ensure selection changes to be drawn TODO: create a insert/pasteAtCursor that leaves out drawing
    repaintChanged();

    ensureCursorVisible();
    unpauseCursor();
  }

  InDoubleClick = false;

  if( BufferRanges->selectionJustStarted() )
    BufferRanges->removeSelection();

  emit cursorPositionChanged( BufferCursor->index() );

  if( !OverWrite ) emit cutAvailable( BufferRanges->hasSelection() );
  emit copyAvailable( BufferRanges->hasSelection() );
  KSection Selection = BufferRanges->selection();
  emit selectionChanged( Selection.start(), Selection.end() );
}


// gets called after press and release instead of a plain press event (?)
void KHexEdit::contentsMouseDoubleClickEvent( TQMouseEvent *e )
{
  // we are only interested in LMB doubleclicks
  if( e->button() != Qt::LeftButton )
  {
    e->ignore();
    return;
  }

  DoubleClickLine = BufferCursor->line();

  int Index = BufferCursor->validIndex();

  if( ActiveColumn == &charColumn() )
  {
    selectWord( Index );

    // as we already have a doubleclick maybe it is a tripple click
    TrippleClickTimer->start( tqApp->doubleClickInterval(), true );
    DoubleClickPoint = e->globalPos();
  }
//  else
//    ValueEditor->goInsideByte(); TODO: make this possible again

  InDoubleClick = true; //
  MousePressed = true;

  emit doubleClicked( Index );
}


void KHexEdit::autoScrollTimerDone()
{
  if( MousePressed )
    handleMouseMove( viewportToContents(viewport()->mapFromGlobal( TQCursor::pos() )) );
}


void KHexEdit::handleMouseMove( const TQPoint& Point ) // handles the move of the mouse with pressed buttons
{
  // no scrolltimer and outside of viewport?
  if( !ScrollTimer->isActive() && Point.y() < contentsY() || Point.y() > contentsY() + visibleHeight() )
    ScrollTimer->start( DefaultScrollTimerPeriod, false );
  // scrolltimer but inside of viewport?
  else if( ScrollTimer->isActive() && Point.y() >= contentsY() && Point.y() <= contentsY() + visibleHeight() )
    ScrollTimer->stop();

  pauseCursor();

  placeCursor( Point );
  ensureCursorVisible();

  // do wordwise selection?
  if( InDoubleClick && BufferRanges->hasFirstWordSelection() ) 
  {
    int NewIndex = BufferCursor->realIndex();
    KSection FirstWordSelection = BufferRanges->firstWordSelection();
    KWordBufferService WBS( DataBuffer, Codec );
    // are we before the selection?
    if( NewIndex < FirstWordSelection.start() )
    {
      BufferRanges->ensureWordSelectionForward( false );
      NewIndex = WBS.indexOfLeftWordSelect( NewIndex );
    }
    // or behind?
    else if( NewIndex > FirstWordSelection.end() )
    {
      BufferRanges->ensureWordSelectionForward( true );
      NewIndex = WBS.indexOfRightWordSelect( NewIndex );
    }
    // or inside?
    else
    {
      BufferRanges->ensureWordSelectionForward( true );
      NewIndex = FirstWordSelection.end()+1;
    }

    BufferCursor->gotoIndex( NewIndex );
  }

  if( BufferRanges->selectionStarted() )
    BufferRanges->setSelectionEnd( BufferCursor->realIndex() );

  repaintChanged();

  unpauseCursor();
}


void KHexEdit::startDrag()
{
  // reset states
  MousePressed = false;
  InDoubleClick = false;
  DragStartPossible = false;

  // create data
  TQDragObject *Drag = dragObject( viewport() );
  if( !Drag )
    return;

  // will we only copy the data?
  if( isReadOnly() || OverWrite )
    Drag->dragCopy();
  // or is this left to the user and he choose to move?
  else if( Drag->drag() )
    // Not inside this widget itself?
    if( TQDragObject::target() != this && TQDragObject::target() != viewport() )
      removeSelectedData();
}


void KHexEdit::contentsDragEnterEvent( TQDragEnterEvent *e )
{
  // interesting for this widget?
  if( isReadOnly() || !TDEBufferDrag::canDecode(e) )
  {
    e->ignore();
    return;
  }

  e->acceptAction();
  InDnD = true;
}


void KHexEdit::contentsDragMoveEvent( TQDragMoveEvent *e )
{
  // is this content still interesting for us? 
  if( isReadOnly() || !TDEBufferDrag::canDecode(e) )
  {
    e->ignore();
    return;
  }

  // let text cursor follow mouse
  pauseCursor( true );
  placeCursor( e->pos() );
  unpauseCursor();

  e->acceptAction();
}


void KHexEdit::contentsDragLeaveEvent( TQDragLeaveEvent * )
{
  // bye... and thanks for all the cursor movement...
  InDnD = false;
}



void KHexEdit::contentsDropEvent( TQDropEvent *e )
{
  // after drag enter and move check one more time
  if( isReadOnly() )
    return;

  // leave state
  InDnD = false;
  e->acceptAction();

  if( !TDEBufferDrag::canDecode(e) ) //TODO: why do we acept the action still?
    return;

  // is this an internal dnd?
  if( e->source() == this || e->source() == viewport() )
    handleInternalDrag( e );
  else
  {
   //BufferRanges->removeSelection();
    pasteFromSource( e );
  }
}


void KHexEdit::handleInternalDrag( TQDropEvent *e )
{
  KSection ChangedRange;

  // stop ui
  pauseCursor();

  // get drag origin
  KSection Selection = BufferRanges->selection();
  int InsertIndex = BufferCursor->realIndex();

  // is this a move?
  if( e->action() == TQDropEvent::Move )
  {
    // ignore the copy hold in the event but only move 
    int NewIndex = DataBuffer->move( InsertIndex, Selection );
    if( NewIndex != Selection.start() )
    {
      BufferCursor->gotoCIndex( NewIndex+Selection.width() );
      ChangedRange.set( TQMIN(InsertIndex,Selection.start()), TQMAX(InsertIndex,Selection.end()) );
    }
  }
  // is a copy
  else
  {
    // get data
    TQByteArray Data;
    if( TDEBufferDrag::decode(e,Data) && !Data.isEmpty() )
    {
      if( OverWrite )
      {
        if( !BufferCursor->isBehind() )
        {
          ChangedRange.setByWidth( InsertIndex, Data.size() );
          ChangedRange.restrictEndTo( BufferLayout->length()-1 );
          if( ChangedRange.isValid() )
          {
            int NoOfReplaced = DataBuffer->replace( ChangedRange, Data.data(), ChangedRange.width() );
            BufferCursor->gotoNextByte( NoOfReplaced );
          }
        }
      }
      else
      {
        int NoOfInserted = DataBuffer->insert( InsertIndex, Data.data(), Data.size() );
        updateLength();
        if( NoOfInserted > 0 )
        {
          BufferCursor->gotoCIndex( InsertIndex + NoOfInserted );
          ChangedRange.set( InsertIndex, DataBuffer->size()-1 );
        }
      }
    }
  }
  BufferRanges->addChangedRange( ChangedRange );
  BufferRanges->removeSelection();

  repaintChanged();
  ensureCursorVisible();

  // open ui
  unpauseCursor();

  // emit appropriate signals.
  emit selectionChanged( -1, -1 );
  if( ChangedRange.isValid() ) emit bufferChanged( ChangedRange.start(), ChangedRange.end() );
  emit cursorPositionChanged( BufferCursor->index() );
}


void KHexEdit::contentsWheelEvent( TQWheelEvent *e )
{
  if( isReadOnly() )
  {
    if( e->state() & ControlButton )
    {
      if( e->delta() > 0 )
        zoomOut();
      else if( e->delta() < 0 )
        zoomIn();
      return;
    }
  }
  TQScrollView::contentsWheelEvent( e );
}


#if 0
void KHexEdit::contentsContextMenuEvent( TQContextMenuEvent *e )
{
//   clearUndoRedo();
  MousePressed = false;

  e->accept();

  TQPopupMenu *PopupMenu = createPopupMenu( e->pos() );
  if( !PopupMenu )
    PopupMenu = createPopupMenu();
  if( !PopupMenu )
    return;
  int r = PopupMenu->exec( e->globalPos() );
  delete PopupMenu;

  if ( r == d->id[ IdClear ] )
    clear();
  else if ( r == d->id[ IdSelectAll ] )
  {
    selectAll();
    // if the clipboard support selections, put the newly selected text into the clipboard
    if( TQApplication::clipboard()->supportsSelection() )
    {
      ClipboardMode = TQClipboard::Selection;
      disconnect( TQApplication::clipboard(), TQT_SIGNAL(selectionChanged()), this, 0);

      copy();

      connect( TQApplication::clipboard(), TQT_SIGNAL(selectionChanged()), this, TQT_SLOT(clipboardChanged()) );
      ClipboardMode = TQClipboard::Clipboard;
    }
  }
  else if( r == d->id[IdUndo] )
    undo();
  else if( r == d->id[IdRedo] )
    redo();
  else if( r == d->id[IdCut] )
    cut();
  else if( r == d->id[IdCopy] )
    copy();
  else if( r == d->id[IdPaste] )
    paste();
}
#endif

#include "khexedit.moc"
