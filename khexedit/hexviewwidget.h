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

#ifndef _HEX_VIEW_WIDGET_H_
#define _HEX_VIEW_WIDGET_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 


#include <tqframe.h>
#include <tqpixmap.h> 
#include <tqpopupmenu.h> 
#include <tqscrollbar.h> 

#include <kapplication.h>
#include <kcmenumngr.h>

#include "hexclipboard.h"
#include "hexbuffer.h"

class CScrollBar : public TQScrollBar
{
  Q_OBJECT
  
 
  public:
    CScrollBar( Qt::Orientation o, TQWidget *parent, const char *name = 0 )
      : TQScrollBar( o, parent, name )
    {
    } 
  
  signals:
    void hidden( void );
  
  protected:
    virtual void hideEvent( TQHideEvent * )
    {
      emit hidden();
    }
};
   

class CDragManager : public TQWidget
{
  Q_OBJECT
  
    
  public:
    enum EDragActivateMode
    {
      Movement = 0,
      Timer = 1
    };

  public:
    CDragManager( void );
    void setActivateMode( EDragActivateMode mode );
    void setup( int x, int y );
    bool start( TQMouseEvent *e );
    bool clear( void );

  protected:
    virtual void timerEvent( TQTimerEvent *e );

  private:
    void removeTimer( void );
    void setupTimer( void );

  signals:
    void startDrag( bool asText );

  private:
    EDragActivateMode mActivateMode;
    bool   mPending;
    int    mTimerId;
    TQPoint mOrigin;
};



class CHexViewWidget : public TQFrame
{
  Q_OBJECT
  

  public:
    CHexViewWidget( TQWidget *parent, const char *name, CHexBuffer *hexBuffer );
    ~CHexViewWidget( void );
    inline bool widgetValid( void );

    int  readFile( TQFile &file, const TQString &url, CProgress &p );
    int  insertFile( TQFile &file, CProgress &p );
    int  writeFile( TQFile &file, CProgress &p );
    int  newFile( const TQString &url );
    void closeFile( void );
    void initFile( void );
    void setBuffer( CHexBuffer *hexBuffer );
    void updateView( bool redraw, bool fixCursor );
    
    void selectAll( void );
    void unselect( void );
    void unmark( void );
    uint numPage( CHexPrinter &printer );
    int  print( CHexPrinter &printer, CProgress &p );
    int  exportText( const SExportText &ex, CProgress &p );
    int  exportHtml( const SExportHtml &ex, CProgress &p );
    int  exportCArray( const SExportCArray &ex, CProgress &p );

    void copy( void );
    void copyText( int columnSegment=CHexBuffer::VisibleColumn );
    void paste( void );
    void cut( void );
    void undo( void );
    void redo( void );
    void addBookmark( int position );
    void removeBookmark( bool all );
    void replaceBookmark( void );
    void gotoBookmark( uint position );
    void gotoNextBookmark( bool next );
    void benchmark( void );

    virtual void setPalette( const TQPalette & );
    void setInputMode( SDisplayInputMode &mode );
    void setLayout( SDisplayLayout &layout );
    void setCursor( const SDisplayCursor &cursor, bool updateDisplay );
    void setColor( const SDisplayColor &color, bool updateDisplay );
    void setFont( const SDisplayFontInfo &fontInfo, bool updateDisplay );
    void setMisc( SDisplayMisc &misc );
    void setInsertMode( bool insertMode );
    int  setEncoding( CConversion::EMode mode, CProgress &p );
    void reportEncoding( void );

    int  findFirst( SSearchControl &sc );
    int  findNext( SSearchControl &sc );
    int  findWrap( SSearchControl &sc );
    int  replaceAll( SSearchControl &sc, bool init );
    int  replaceMarked( SSearchControl &sc );
    int  collectStrings( CStringCollectControl &sc );
    int  collectStatistic( SStatisticControl &sc, CProgress &p );


    inline void setPopupMenu( TQPopupMenu *popupMenu );
    inline void setDocumentMenu( TQPopupMenu *popupMenu );
    inline bool insertMode( void );
    inline int  scrollBarWidth( void );
    inline int  dataWidth( void );
    inline int  defaultWidth( void );
    inline uint offset( void );
    inline uint bookmarkCount( void );
    inline bool modified( void );
    inline void setModified( bool modified );
    inline const TQDateTime &diskModifyTime( void );
    inline bool losslessEncoding( CConversion::EMode mode );
    inline const SEncodeState &encoding( void );
    inline bool documentPresent( void );
    inline bool urlValid( void );
    inline TQString &url( void );
    inline void setUrl( TQString &url );
    inline const CHexBuffer *hexBuffer( void );

  public slots:
    void filter( SFilterControl &fc );
    void insert( SInsertData &id );
    void insert( const TQByteArray &buf );
    void append( const TQByteArray &buf );
    void valueOnCursor( TQByteArray &buf, uint size );
    void paletteChanged( void );
    void fontChanged( void );
    void gotoOffset( uint offset, uint bit, bool fromCursor, bool forward );
    void gotoOffset( uint offset );
    void setMark( uint offset, uint size, bool moveCursor );
    void setDropHighlight( bool mode );

  protected:
    virtual void drawFrame( TQPainter *p );
    virtual void paintEvent( TQPaintEvent *e );
    virtual void resizeEvent( TQResizeEvent *e );
    virtual void keyPressEvent( TQKeyEvent *e );
    virtual void keyReleaseEvent( TQKeyEvent *e );
    virtual void mousePressEvent( TQMouseEvent *e );
    virtual void mouseMoveEvent( TQMouseEvent *e );
    virtual void wheelEvent( TQWheelEvent * );
    virtual void mouseReleaseEvent( TQMouseEvent *e );
    virtual void dragEnterEvent( TQDragEnterEvent *e );
    virtual void dragLeaveEvent( TQDragLeaveEvent *e );
    virtual void dragMoveEvent( TQDragMoveEvent *e );
    virtual void dropEvent( TQDropEvent *e );
    virtual void showEvent( TQShowEvent * );
    virtual void timerEvent( TQTimerEvent *e );
    virtual void focusInEvent( TQFocusEvent *e );
    virtual void focusOutEvent( TQFocusEvent *e );

  protected slots:
    void changeXPos( int pos );
    void changeYPos( int pos );
    void clipboardChanged( void );

  signals:
    void cursorChanged( SCursorState &state );
    void fileState( SFileState &state );
    void dataChanged( void );
    void layoutChanged( const SDisplayLayout &layout );
    void inputModeChanged( const SDisplayInputMode &mode );
    void bookmarkChanged( TQPtrList<SCursorOffset> &list );
    void editMode( CHexBuffer::EEditMode editMode );
    void encodingChanged( const SEncodeState &state );
    void textWidth( uint width );
    void fileName( const TQString &url, bool onDisk );
    void fileRename( const TQString &curName, const TQString &newName );
    void fileClosed( const TQString &url );

    void pleaseOpenNewFile( void );
    void pleaseStepFile( bool next );
    void pleaseOpenFile(const TQString &url,bool reloadWhenChanged,uint offset);

  private:
    void setSelection( uint offset, bool init );
    void setCursorPosition( int x, int y, bool init, bool cellLevel );
    void updateCursor( SCursorConfig &cc, bool always = false,
		       bool touchSelection = true );
    void setEditMode( CHexBuffer::EEditMode mode );
    
    void paintFrame( void );
    void updateFrameSize( void );
 
    void redrawInterval( uint start, uint stop );
    void redrawLines( uint docLine, int numLine );
    void redrawFromOffset( uint offset, bool finishWindow );
    void paintText( const TQRect &r, bool expand );
    void paintCursor( int cursorMode );
    

    void toggleEditor( void );
    void cursorStep( SCursorConfig &cc, uint stepSize );
    void cursorLeft( SCursorConfig &cc );
    void cursorRight( SCursorConfig &cc );
    void cursorHome( SCursorConfig &cc );
    void cursorEnd( SCursorConfig &cc );
    void cursorUp( SCursorConfig &cc );
    void cursorDown( SCursorConfig &cc );
    void cursorPageUp( SCursorConfig &cc );
    void cursorPageDown( SCursorConfig &cc );
    void cursorInsert( SCursorConfig &cc );
    void cursorDelete( SCursorConfig &cc );
    void cursorBackspace( SCursorConfig &cc );
    void cursorInput( TQChar c );

    int  bookmarkMenu( const TQString &title );

    static inline bool shiftButtonState( void );

    inline void setupCursorTimer( void );
    inline int  startX( void );
    inline int  startY( void );
    inline void setStartX( int val );
    inline void setStartY( int val );
    inline void updateWindow( bool completeRedraw, bool touchSelection );
    inline void updateWindow( uint line );
    inline void updateWindow( uint fromOffset, bool finishWindow );
    inline void updateWindow( void );
    inline void setTextBufferSize( void );
    inline void autoCopy( void );

  private slots:
    void startDrag( bool asText );

  private:
    CScrollBar *mVertScroll;
    CScrollBar *mHorzScroll;
    TQWidget    *mCorner;
  
    CHexBuffer *mHexBuffer;
    TQPixmap    mTextBuffer;
    SDisplayLayout   mLayout; 
    SDisplayCursor   mCursor;
    SDisplayColor    mColor;
    SDisplayFontInfo mFontInfo;
    SDisplayMisc     mMisc;

    TQPopupMenu *mDocumentMenu;

    int  mScrollBarSize;
    CHexBuffer::EEditMode mEditMode;
    bool mShowCursor;
    bool mDropHighlight;
    
    int  mCursorTimerId;

    CDragManager  *mDragManager;
    CHexClipboard mClipConvert;
};


inline bool CHexViewWidget::shiftButtonState( void )
{
  return kapp->keyboardModifiers() & TDEApplication::ShiftModifier;
}



inline bool CHexViewWidget::widgetValid( void )
{
  if( mVertScroll == 0 || mHorzScroll == 0 || mHexBuffer == 0 )
  {
    return( false );
  }
  else
  {
    return( true );

  }
}
    
inline void CHexViewWidget::setupCursorTimer( void )
{
  if( mCursorTimerId != 0 )
  {
    killTimer( mCursorTimerId ); 
    mCursorTimerId = 0;
  }
  
  if( hasFocus() == true )
  {
    if( mCursor.alwaysVisible == false )
    {
      mCursorTimerId = startTimer( mCursor.interval );
    }
    mShowCursor = true;
    mHexBuffer->setDisableCursor( false );
  }
  else
  {
    if( mCursor.alwaysVisible == false )
    {
      if( mCursor.focusMode == SDisplayCursor::ignore )
      {
	mCursorTimerId = startTimer( mCursor.interval );
      }
    }
    if( mCursor.focusMode != SDisplayCursor::hide )
    {
      mShowCursor = true;
    }
    else
    {
      mShowCursor = false;
      mHexBuffer->setDisableCursor( true );
    }
  }

  mHexBuffer->setShowCursor( mShowCursor );
}

inline void CHexViewWidget::setPopupMenu( TQPopupMenu *popupMenu )
{
  KContextMenuManager::insert( this, popupMenu );
}


inline void CHexViewWidget::setDocumentMenu( TQPopupMenu *popupMenu )
{
  mDocumentMenu = popupMenu;
}


inline int  CHexViewWidget::startX( void )
{
  return( mHexBuffer->startX() );
} 

inline int  CHexViewWidget::startY( void )
{
  return( mHexBuffer->startY() );
}

inline void CHexViewWidget::setStartX( int val )
{
  mHexBuffer->setStartX( val );
}

inline void CHexViewWidget::setStartY( int val )
{
  mHexBuffer->setStartY( val );
}


inline void CHexViewWidget::updateWindow( bool completeRedraw, 
					  bool touchSelection )
{
  if( completeRedraw == true )
  {
    SCursorConfig cc;
    updateCursor( cc, true, touchSelection );
    updateView( true, false );
  }
  else
  {
    SCursorConfig cc;
    updateCursor( cc, false, touchSelection );
    redrawFromOffset( mHexBuffer->cursorOffset(), true );
    updateView( false, false );
  }

  emit fileState( mHexBuffer->fileState() );
}



inline void CHexViewWidget::updateWindow( uint line )
{
  SCursorConfig cc;
  updateCursor( cc, false, true );
  if( line == mHexBuffer->cursorLine() )
  {
    redrawLines( line, 1 );
  }
  else if( line < mHexBuffer->cursorLine() ) 
  {
    redrawLines( line, mHexBuffer->cursorLine() - line );
  }
  else
  {
    redrawLines( line, line - mHexBuffer->cursorLine() );
  }
  emit fileState( mHexBuffer->fileState() );
}


inline void CHexViewWidget::updateWindow( uint fromOffset, bool finishWindow )
{
  SCursorConfig cc;
  updateCursor( cc, true, true );
  updateView( false, false );
  redrawFromOffset( fromOffset, finishWindow );

  emit fileState( mHexBuffer->fileState() );
}


inline void CHexViewWidget::updateWindow( void )
{
  setTextBufferSize();
  mHexBuffer->cursorUp(0); // Makes sure cursor is visible

  SCursorConfig cc;
  cc.emulateControlButton( true );
  updateCursor( cc, true, false  );
  updateView( true, false );

  emit fileState( mHexBuffer->fileState() );
}


inline void CHexViewWidget::setTextBufferSize( void )
{
  int w = width();
  int h = mHexBuffer->lineHeight();

  if( w != mTextBuffer.width() || h != mTextBuffer.height() )
  {
    mTextBuffer.resize( w, h );
  }
}


inline void CHexViewWidget::autoCopy( void )
{
  if( mMisc.autoCopyToClipboard == true )
  {
    copy();
  }
}


inline bool CHexViewWidget::insertMode( void )
{
  return mEditMode == CHexBuffer::EditInsert;
}


inline int CHexViewWidget::scrollBarWidth( void )
{
  return( mScrollBarSize );
}

inline int CHexViewWidget::dataWidth( void )
{
  return( mHexBuffer->lineWidth() );
}

inline int CHexViewWidget::defaultWidth( void )
{
  return( dataWidth() + scrollBarWidth() + frameWidth()*2 );
}


inline uint CHexViewWidget::offset( void )
{
  return( mHexBuffer->cursorOffset() );
}

inline uint CHexViewWidget::bookmarkCount( void )
{
  const TQPtrList<SCursorOffset> &list = mHexBuffer->bookmarkList();
  return( list.count() );
}

inline bool CHexViewWidget::modified( void )
{
  return( mHexBuffer->modified() );
}

inline void CHexViewWidget::setModified( bool modified )
{
  mHexBuffer->setModified( modified );
  emit fileState( mHexBuffer->fileState() );
}

inline const TQDateTime &CHexViewWidget::diskModifyTime( void )
{
  return( mHexBuffer->diskModifyTime() );
}

inline bool CHexViewWidget::losslessEncoding( CConversion::EMode mode )
{
  return( mHexBuffer->losslessEncoding(mode) );
}

inline const SEncodeState &CHexViewWidget::encoding( void )
{
  return( mHexBuffer->encoding() );
}

inline bool CHexViewWidget::documentPresent( void )
{
  return( mHexBuffer->documentPresent() );
}

inline bool CHexViewWidget::urlValid( void )
{
  return( mHexBuffer->hasFileName() );
}

inline TQString &CHexViewWidget::url( void )
{
  return( mHexBuffer->url() );
}

inline void CHexViewWidget::setUrl( TQString &url )
{
  if( mHexBuffer->url() != url )
  {
    emit fileRename( mHexBuffer->url(), url ); 
    mHexBuffer->setUrl( url );
  }
}

inline const CHexBuffer *CHexViewWidget::hexBuffer( void )
{
  return( mHexBuffer );
}


#endif




