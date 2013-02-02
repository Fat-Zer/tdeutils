/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999-2000  Espen Sand, espensa@online.no
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

#ifndef _TOPLEVEL_H_
#define _TOPLEVEL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqptrlist.h>
#include <kapplication.h>
#include <tdestdaccel.h>
#include <tdemainwindow.h>

#include "hexbuffer.h"
#include "hexmanagerwidget.h"
#include "statusbarprogress.h"

class TQSignalMapper;
class TDEAction;
class TDERecentFilesAction;
class TDERadioAction;
class TDESelectAction;
class TDEToggleAction;
class TDEToolBarButton;
class KURL;
class CDragLabel;


class KHexEdit : public TDEMainWindow
{
  Q_OBJECT
  

  enum StatusBarId
  {
    status_WriteProtect = 0,
    status_Layout       = 1,
    status_Offset       = 2,
    status_Size         = 3,
    status_Ovr          = 4,
    status_Modified     = 5,
    status_Selection    = 6,
    status_Progress     = 7
  };

  struct SActionList
  {
    TDEAction *insert;
    TDERecentFilesAction *openRecent;
    TDEAction *save;
    TDEAction *saveAs;
    TDEAction *revert;
    TDEAction *close;
    TDEAction *print;
    TDEAction *exportData;
    TDEAction *cancel;
    TDEToggleAction *readOnly;
    TDEToggleAction *resizeLock;
    TDEAction *newWindow;
    TDEAction *closeWindow;
    TDEAction *quit;
    TDEAction *undo;
    TDEAction *redo;
    TDEAction *cut;
    TDEAction *copy;
    TDEAction *paste;
    TDEAction *selectAll;
    TDEAction *unselect;
    TDEAction *find;
    TDEAction *findNext;
    TDEAction *findPrev;
    TDEAction *replace;
    TDEAction *gotoOffset;
    TDEAction *insertPattern;
    TDEAction *copyAsText;
    TDEAction *pasteToNewFile;
    TDEAction *pasteToNewWindow;
    TDERadioAction *hexadecimal;
    TDERadioAction *decimal;
    TDERadioAction *octal;
    TDERadioAction *binary;
    TDERadioAction *textOnly;
    TDEToggleAction *showOffsetColumn;
    TDEToggleAction *showTextColumn;
    TDEToggleAction *offsetAsDecimal;
    TDEToggleAction *dataUppercase;
    TDEToggleAction *offsetUppercase;
    TDERadioAction *defaultEncoding;
    TDERadioAction *usAsciiEncoding;
    TDERadioAction *ebcdicEncoding;
//     TDEAction *customEncoding;

    TDEAction *strings;
//     TDEAction *recordViewer;
    TDEAction *filter;
    TDEAction *characterTable;
    TDEAction *converter;
    TDEAction *statistics;

    TDEAction *addBookmark;
    TDEAction *replaceBookmark;
    TDEAction *removeBookmark;
    TDEAction *removeAllBookmark;
    TDEAction *nextBookmark;
    TDEAction *prevBookmark;

    TDEToggleAction *showFullPath;
    TDERadioAction *tabHide;
    TDERadioAction *tabShowBelowEditor;
    TDERadioAction *tabShowAboveEditor;
    TDERadioAction *conversionHide;
    TDERadioAction *conversionFloat;
    TDERadioAction *conversionEmbed;
    TDERadioAction *searchHide;
    TDERadioAction *searchShowAboveEditor;
    TDERadioAction *searchShowBelowEditor;

//     TDEAction *favorites;

    TQPtrList< TDEAction > bookmarkList;
    TQSignalMapper *bookmarkMapper;
  };

  public:
    KHexEdit( void );
    ~KHexEdit( void );

    inline void addStartupFile( const TQString &fileName );
    inline void setStartupOffset( uint offset );

  public slots:
    KHexEdit *newWindow( void );
    void pasteNewWindow( void );
    void closeWindow( void );
    void closeProgram( void );
    void statusBarPressed( int id );

    void operationChanged( bool state );
    void cursorChanged( SCursorState &state );
    void fileState( SFileState &state );
    void layoutChanged( const SDisplayLayout &layout );
    void inputModeChanged( const SDisplayInputMode &mode );
    void bookmarkChanged( TQPtrList<SCursorOffset> &list );
    void removeRecentFile( const TQString &fileName );
    void renameRecentFile( const TQString &curName, const TQString &newName );

    void setupCaption( const TQString &url );
    void fileActive( const TQString &url, bool onDisk );
    void fileRename( const TQString &curName, const TQString &newName );
    void fileClosed( const TQString &url );
    void readConfiguration( void );
    void writeConfiguration( void );
    void editMode( CHexBuffer::EEditMode editMode );
    void encodingChanged( const SEncodeState &state );
    void textWidth( uint width );

    void setDisplayMode( void );
    void showFullPath( void );
    void showDocumentTabs( void );
    void showConversionField( void );
    void showSearchBar( void );

    void setEncoding( void );
    void documentMenuCB( int id );

    /**
     *  Slot for opening a file among the recently opened files.
     */
    void slotFileOpenRecent( const KURL & );

protected:
    virtual bool queryExit( void );
    virtual bool queryClose( void );

  private slots:
    void delayedStartupOpen( void );
    void removeRecentFiles( void );
    void conversionClosed( void );
    void searchBarClosed( void );
    void resizeTest();

  private:
    void setupActions( void );
    void setupStatusBar( void );
    void open( TQStringList &fileList, TQStringList &offsetList );
    void initialize( bool openFiles );
    void addRecentFile( const TQString &fileName );
    bool closeAllWindow( void );
    void setUndoState( uint undoState );
    void setSelectionState( uint selectionOffset, uint selectionSize );
    void setSelectionText( uint selectionOffset, uint selectionSize );
    void addDocument( const TQString &fileName );
    void removeDocument( const TQString &fileName );
    void renameDocument( const TQString &curName, const TQString &newName );
    void setTickedDocument( const TQString &fileName );

    void writeConfiguration( TDEConfig &config );
    void readConfiguration( TDEConfig &config );
    bool eventFilter( TQObject *obj, TQEvent *event );

    int acceleratorNumKey( uint index );
    inline CHexEditorWidget *editor( void );
    inline CHexToolWidget *converter( void );
    inline CHexViewWidget *hexView( void );

  private:
    static TQPtrList<KHexEdit> mWindowList;
    CHexManagerWidget *mManager;
    TQStringList mDocumentList;

    TQStringList mStartupFileList;   ///< Files to automatically open on startup
    TQStringList mStartupOffsetList; ///< Start offset for those files.
    uint mStartupOffset; ///< Value read from the command line

    SActionList mAction;

    CDragLabel     *mDragLabel;
    TDEToolBarButton *mWriteProtectButton;

    bool mIsModified;
    bool mShowFullPath;
    bool mSelectionAsHexadecimal;
    uint mSelectionOffset;
    uint mSelectionSize;
    uint mUndoState;
    int  mRecentFileId;
};


inline void KHexEdit::addStartupFile( const TQString &fileName )
{
  mStartupFileList.prepend( fileName );
  mStartupOffsetList.prepend( TQString("%1").arg(mStartupOffset,0,16) );
  mStartupOffset = 0;
}

inline void KHexEdit::setStartupOffset( uint offset )
{
  mStartupOffset = offset;
}

inline CHexEditorWidget *KHexEdit::editor( void )
{
  return( mManager->editor() );
}

inline CHexViewWidget *KHexEdit::hexView( void )
{
  return( mManager->editor()->view() );
}

inline CHexToolWidget *KHexEdit::converter( void )
{
  return( mManager->converter() );
}



#endif


