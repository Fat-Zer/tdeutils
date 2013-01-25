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

#ifndef _HEX_EDITOR_WIDGET_H_
#define _HEX_EDITOR_WIDGET_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 


#include <tqvariant.h>
#include <tqptrdict.h> 
#include <kapplication.h>

#include "hexbuffer.h"
#include "hexviewwidget.h"
#include "progress.h"

class CGotoDialog;
class CFindDialog;
class CReplaceDialog;
class CInsertDialog;
class CFilterDialog;
class COptionDialog;
class CPrinterDialog;
class CStringDialog;
class CCharTableDialog;
class CFileInfoDialog;
class CExportDialog;
class CConverterDialog;
class CFindNavigatorDialog;
class CReplacePromptDialog;
namespace TDEIO { class Job; }


class CHexEditorWidget : public TQWidget
{
  Q_OBJECT
  

  public:
    enum EProgressMode
    {
      pg_read = 0,
      pg_write,
      pg_insert,
      pg_print,
      pg_encode,
      pg_strings,
      pg_export,
      pg_statistic,
      pg_MAX
    };

  public:
    CHexEditorWidget( TQWidget *parent = 0, const char *name = 0 );
    ~CHexEditorWidget( void );

    void initialize( void );
    void writeConfiguration( TDEConfig &config );
    void readConfiguration( TDEConfig &config );

    bool isOpen( const TQString &url, uint &offset );
    bool modified( void );
    
    inline int defaultTextWidth( void );
    inline CHexViewWidget *view( void );
    inline SDisplayLayout &layout( void );
    inline SDisplayLine &line( void );
    inline SDisplayInputMode &inputMode( void );
    inline SDisplayMisc::EOpenFile openFile( void );
    inline bool discardRecentFiles( void );
    inline bool gotoStartupOffset( void );
    inline bool gotoReloadOffset( void );

  signals:
    void errorLoadFile( const TQString &url );
    void setProgress( int percent );
    void setProgress( int curPage, int maxPage );
    void enableProgressText( bool state );
    void setProgressText( const TQString &msg );
    void operationChanged( bool state );
    void removeRecentFiles( void );

  public slots:
    void setHexadecimalMode( void );
    void setDecimalMode( void );
    void setOctalMode( void );
    void setTextMode( void );
    void setBinaryMode( void );
    void open( const TQString &url, bool reloadWhenChanged, uint offset );
    void newFile( void );
    void newFile( const TQByteArray &data );
    void stepFile( bool next );
    void open( void );
    void insertFile( void );
    void stop( void );
    bool close( void );
    bool closeAll( void );
    bool backup( void );
    bool save( void );
    bool saveAs( void );
    void reload( void );
    void print( void );
    void exportDialog( void );
    void encode( CConversion::EMode mode );
    void undo( void );
    void redo( void );
    void toggleWriteProtection( void );
    void defaultWriteProtection( void );
    void toggleResizeLock( void );
    void setResizeLock( bool state );
    void toggleOffsetColumnVisibility( void );
    void toggleTextColumnVisibility( void );
    void toggleOffsetAsDecimal( void );
    void toggleDataUppercase( void );
    void toggleOffsetUppercase( void );
    void toggleInsertMode( void );
    void benchmark( void );
    void copy( void );
    void copyText( void );
    void paste( void );
    void pasteNewFile( void );
    void cut( void );
    void selectAll( void );
    void unselect( void );
    void addBookmark( void );
    void removeBookmark( void );
    void removeAllBookmark( void );
    void replaceBookmark( void );
    void gotoBookmark( int position );
    void gotoNextBookmark( void );
    void gotoPrevBookmark( void );
    void gotoOffset( void );
    void find( void );
    void findAgain( void );
    void findNext( void );
    void findPrevious( void );
    void findData( SSearchControl &sc, uint mode, bool navigator );
    void replace( void );
    void insertPattern( void );
    void encoding( void );
    void strings( void );
    void recordView( void );
    void filter( void );
    void chart( void );
    void converter( void );
    void statistics( void );
    void options( void );
    void favorites( void );

  protected slots:
    void fontChanged( void );
    void paletteChanged( void );
    void layoutChanged( const SDisplayLayout &layout );
    void inputModeChanged( const SDisplayInputMode &input );
    void setLineSize(const SDisplayLine &line );
    void setLayout( const SDisplayLayout &layout );
    void setCursor( const SDisplayCursor &cursor );
    void setColor( const SDisplayColor &color );
    void setFont( const SDisplayFont &font );
    void setMisc( const SDisplayMisc &misc );

    void printPostscript( CHexPrinter & );
    void exportText( const SExportText & );
    void exportHtml( const SExportHtml & );
    void exportCArray( const SExportCArray &ex );

    void findNavigator( SSearchControl &sc );
    void replaceData( SSearchControl &sc, uint mode );
    void replacePrompt( SSearchControl &sc );
    void replaceResult( SSearchControl &sc );
    void collectStrings( void );
    void collectStatistics( SStatisticControl &sc );

  protected:
    void resizeEvent( TQResizeEvent *e );

  private:
    bool selectDocument( const TQString &url, bool reloadWhenChanged );
    bool querySave( void );
    int  readURL( const KURL &url, bool insert );
    void writeURL( TQString &url );
    bool readFile( const TQString &diskPath, const TQString &url, bool insert );
    bool writeFile( const TQString &diskPath );
    void saveWorkingDirectory( const TQString &url );

    bool confirmPrintPageNumber( CHexPrinter &printer );

    CHexBuffer *documentItem( const TQString &url );
    CHexBuffer *documentItem( const TQString &url, bool next );
    bool createBuffer( void );
    void removeBuffer( void );

    bool askWrap( bool fwd, const TQString &header );
    bool canFind( bool showError );
    void hideReplacePrompt( void );
    bool modifiedByAlien( const TQString &url );
    void enableInputLock( bool inputLock );

    int  prepareProgressData( EProgressMode mode );
    static int progressReceiver( void *clientData, SProgressData &pd );
    int  progressParse( const SProgressData &pd );
    bool busy( bool showWarning );

  private:
    TQString  mWorkDir; // Remembers last directroy used by file dialogs 
 
    uint mUntitledCount;
    TQPtrList<CHexBuffer> mDocumentList;
    SDisplayState mDisplayState;

    CProgress     mProgressData;
    EProgressMode mProgressMode;
    bool          mProgressBusy;
    bool          mProgressStop;

    CHexViewWidget   *mHexView;
    CGotoDialog      *mGotoDialog;
    CFindDialog      *mFindDialog;
    CReplaceDialog   *mReplaceDialog;
    CInsertDialog    *mInsertDialog;
    CFilterDialog    *mFilterDialog;
    COptionDialog    *mOptionDialog;
    CStringDialog    *mStringDialog;
    CCharTableDialog *mCharTableDialog;
    CFileInfoDialog  *mFileInfoDialog;
    CExportDialog    *mExportDialog;
    CConverterDialog *mConverterDialog;
    CFindNavigatorDialog *mFindNavigatorDialog;
    CReplacePromptDialog *mReplacePromptDialog;
};

inline int CHexEditorWidget::defaultTextWidth( void )
{
  return( mHexView->defaultWidth() );
}

inline CHexViewWidget *CHexEditorWidget::view( void )
{
  return( mHexView );
}

inline SDisplayLayout &CHexEditorWidget::layout( void )
{
  return( mDisplayState.layout );
}

inline SDisplayLine &CHexEditorWidget::line( void )
{
  return( mDisplayState.line );
}

inline SDisplayInputMode &CHexEditorWidget::inputMode( void )
{
  return( mDisplayState.input );
}

inline SDisplayMisc::EOpenFile CHexEditorWidget::openFile( void )
{
  return( mDisplayState.misc.openFile );
}

inline bool CHexEditorWidget::discardRecentFiles( void )
{
  return( mDisplayState.misc.discardRecent );
}

inline bool CHexEditorWidget::gotoStartupOffset( void )
{
  return( mDisplayState.misc.gotoOnStartup );
}

inline bool CHexEditorWidget::gotoReloadOffset( void )
{
  return( mDisplayState.misc.gotoOnReload );
}



#endif



