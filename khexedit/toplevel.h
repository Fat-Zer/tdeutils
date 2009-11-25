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

#include <qptrlist.h>
#include <kapplication.h>
#include <kstdaccel.h>
#include <kmainwindow.h>

#include "hexbuffer.h"
#include "hexmanagerwidget.h"
#include "statusbarprogress.h"

class QSignalMapper;
class KAction;
class KRecentFilesAction;
class KRadioAction;
class KSelectAction;
class KToggleAction;
class KToolBarButton;
class KURL;
class CDragLabel;


class KHexEdit : public KMainWindow
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
    KAction *insert;
    KRecentFilesAction *openRecent;
    KAction *save;
    KAction *saveAs;
    KAction *revert;
    KAction *close;
    KAction *print;
    KAction *exportData;
    KAction *cancel;
    KToggleAction *readOnly;
    KToggleAction *resizeLock;
    KAction *newWindow;
    KAction *closeWindow;
    KAction *quit;
    KAction *undo;
    KAction *redo;
    KAction *cut;
    KAction *copy;
    KAction *paste;
    KAction *selectAll;
    KAction *unselect;
    KAction *find;
    KAction *findNext;
    KAction *findPrev;
    KAction *replace;
    KAction *gotoOffset;
    KAction *insertPattern;
    KAction *copyAsText;
    KAction *pasteToNewFile;
    KAction *pasteToNewWindow;
    KRadioAction *hexadecimal;
    KRadioAction *decimal;
    KRadioAction *octal;
    KRadioAction *binary;
    KRadioAction *textOnly;
    KToggleAction *showOffsetColumn;
    KToggleAction *showTextColumn;
    KToggleAction *offsetAsDecimal;
    KToggleAction *dataUppercase;
    KToggleAction *offsetUppercase;
    KRadioAction *defaultEncoding;
    KRadioAction *usAsciiEncoding;
    KRadioAction *ebcdicEncoding;
//     KAction *customEncoding;

    KAction *strings;
//     KAction *recordViewer;
    KAction *filter;
    KAction *characterTable;
    KAction *converter;
    KAction *statistics;

    KAction *addBookmark;
    KAction *replaceBookmark;
    KAction *removeBookmark;
    KAction *removeAllBookmark;
    KAction *nextBookmark;
    KAction *prevBookmark;

    KToggleAction *showFullPath;
    KRadioAction *tabHide;
    KRadioAction *tabShowBelowEditor;
    KRadioAction *tabShowAboveEditor;
    KRadioAction *conversionHide;
    KRadioAction *conversionFloat;
    KRadioAction *conversionEmbed;
    KRadioAction *searchHide;
    KRadioAction *searchShowAboveEditor;
    KRadioAction *searchShowBelowEditor;

//     KAction *favorites;

    QPtrList< KAction > bookmarkList;
    QSignalMapper *bookmarkMapper;
  };

  public:
    KHexEdit( void );
    ~KHexEdit( void );

    inline void addStartupFile( const QString &fileName );
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
    void bookmarkChanged( QPtrList<SCursorOffset> &list );
    void removeRecentFile( const QString &fileName );
    void renameRecentFile( const QString &curName, const QString &newName );

    void setupCaption( const QString &url );
    void fileActive( const QString &url, bool onDisk );
    void fileRename( const QString &curName, const QString &newName );
    void fileClosed( const QString &url );
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
    void open( QStringList &fileList, QStringList &offsetList );
    void initialize( bool openFiles );
    void addRecentFile( const QString &fileName );
    bool closeAllWindow( void );
    void setUndoState( uint undoState );
    void setSelectionState( uint selectionOffset, uint selectionSize );
    void setSelectionText( uint selectionOffset, uint selectionSize );
    void addDocument( const QString &fileName );
    void removeDocument( const QString &fileName );
    void renameDocument( const QString &curName, const QString &newName );
    void setTickedDocument( const QString &fileName );

    void writeConfiguration( KConfig &config );
    void readConfiguration( KConfig &config );
    bool eventFilter( QObject *obj, QEvent *event );

    int acceleratorNumKey( uint index );
    inline CHexEditorWidget *editor( void );
    inline CHexToolWidget *converter( void );
    inline CHexViewWidget *hexView( void );

  private:
    static QPtrList<KHexEdit> mWindowList;
    CHexManagerWidget *mManager;
    QStringList mDocumentList;

    QStringList mStartupFileList;   ///< Files to automatically open on startup
    QStringList mStartupOffsetList; ///< Start offset for those files.
    uint mStartupOffset; ///< Value read from the command line

    SActionList mAction;

    CDragLabel     *mDragLabel;
    KToolBarButton *mWriteProtectButton;

    bool mIsModified;
    bool mShowFullPath;
    bool mSelectionAsHexadecimal;
    uint mSelectionOffset;
    uint mSelectionSize;
    uint mUndoState;
    int  mRecentFileId;
};


inline void KHexEdit::addStartupFile( const QString &fileName )
{
  mStartupFileList.prepend( fileName );
  mStartupOffsetList.prepend( QString("%1").arg(mStartupOffset,0,16) );
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


