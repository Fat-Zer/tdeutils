/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999-2000 Espen Sand, espensa@online.no
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

// kate: space-indent on; indent-width 2; replace-tabs on;

#include <tqptrlist.h>
#include <tqsignalmapper.h>
#include <tqtimer.h>
#include <tqtooltip.h>

#include <kaction.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstatusbar.h>
#include <kpopupmenu.h>
#include <kstdaction.h>
#include <ktoolbarbutton.h>
#include <kdebug.h>
#include <kurl.h>
#include <kurldrag.h>

#include "dialog.h"
#include "draglabel.h"
#include "hexerror.h"
#include "toplevel.h"


TQPtrList<KHexEdit> KHexEdit::mWindowList;

KHexEdit::KHexEdit( void )
  : KMainWindow(0, "toplevel"), mStartupOffset(0),
  mIsModified(0)
{
  mWindowList.append( this );

  //
  // Create main widget and register it.
  //
  mManager = new CHexManagerWidget( this, "manager" );
  setCentralWidget( mManager );

  //
  // I use an eventfilter to catch all drag/drop events for the
  // area outside the editor window. It will only accept KURLDrag.
  // (The editor window will catch KURLDrag, TQTextDrag and CHexDrag)
  //
  installEventFilter( this );
  setAcceptDrops(true);

  //
  // Prepare menus and status bar
  //
  mAction.bookmarkMapper = new TQSignalMapper(TQT_TQOBJECT(this));
  connect(mAction.bookmarkMapper, TQT_SIGNAL(mapped(int)), TQT_TQOBJECT(editor()), TQT_SLOT(gotoBookmark(int)));
  setupActions();
  setupStatusBar();

  connect( hexView(), TQT_SIGNAL( cursorChanged( SCursorState & ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( cursorChanged( SCursorState & ) ) );
  connect( hexView(), TQT_SIGNAL( editMode( CHexBuffer::EEditMode ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( editMode( CHexBuffer::EEditMode ) ) );
  connect( hexView(), TQT_SIGNAL( encodingChanged( const SEncodeState &)),
	   TQT_TQOBJECT(this), TQT_SLOT( encodingChanged( const SEncodeState & )) );
  connect( hexView(), TQT_SIGNAL( textWidth( uint ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( textWidth( uint ) ) );
  connect( hexView(), TQT_SIGNAL( fileState( SFileState & ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( fileState( SFileState & ) ) );
  connect( hexView(), TQT_SIGNAL( layoutChanged( const SDisplayLayout & ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( layoutChanged( const SDisplayLayout & ) ) );
  connect( hexView(), TQT_SIGNAL( inputModeChanged( const SDisplayInputMode & ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( inputModeChanged( const SDisplayInputMode & ) ) );
  connect( hexView(), TQT_SIGNAL( bookmarkChanged( TQPtrList<SCursorOffset> &)),
	   TQT_TQOBJECT(this), TQT_SLOT( bookmarkChanged( TQPtrList<SCursorOffset> & ) ) );
  connect( hexView(), TQT_SIGNAL( fileName( const TQString &, bool ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( fileActive( const TQString &, bool ) ) );
  connect( hexView(), TQT_SIGNAL( fileRename( const TQString &, const TQString & )),
	   TQT_TQOBJECT(this), TQT_SLOT( fileRename( const TQString &, const TQString & ) ) );
  connect( hexView(), TQT_SIGNAL( fileClosed( const TQString & ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( fileClosed( const TQString & ) ) );
  connect( TQT_TQOBJECT(editor()), TQT_SIGNAL( errorLoadFile( const TQString & ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( removeRecentFile( const TQString & ) ) );
  connect( TQT_TQOBJECT(editor()), TQT_SIGNAL( operationChanged( bool ) ),
	   TQT_TQOBJECT(this), TQT_SLOT( operationChanged( bool ) ) );
  connect( TQT_TQOBJECT(editor()), TQT_SIGNAL( removeRecentFiles() ),
	   TQT_TQOBJECT(this), TQT_SLOT( removeRecentFiles() ) );
  connect( mManager, TQT_SIGNAL( conversionClosed() ),
	   TQT_TQOBJECT(this), TQT_SLOT(conversionClosed()) );
  connect( mManager, TQT_SIGNAL( searchBarClosed() ),
	   TQT_TQOBJECT(this), TQT_SLOT(searchBarClosed()) );

  //
  // Read configuration from file and set the default editor size.
  // Open files if this is the first toplevel window.
  //
  readConfiguration();
  initialize( mWindowList.count() == 1 ? true : false );
  setAutoSaveSettings();
}


KHexEdit::~KHexEdit( void )
{
  delete mAction.bookmarkMapper;
  delete mManager;
}


void KHexEdit::setupActions( void )
{
  KStdAction::openNew( TQT_TQOBJECT(editor()), TQT_SLOT(newFile()), actionCollection() );
  KStdAction::open( TQT_TQOBJECT(editor()), TQT_SLOT(open()), actionCollection() );
  KStdAction::keyBindings(guiFactory(), TQT_SLOT(configureShortcuts()), 
actionCollection());
   
  mAction.insert     = new KAction( i18n("&Insert..."), CTRL+Key_I,
    TQT_TQOBJECT(editor()), TQT_SLOT(insertFile()), actionCollection(), "insert_file" );
  mAction.openRecent = KStdAction::openRecent( TQT_TQOBJECT(this), TQT_SLOT( slotFileOpenRecent( const KURL& ) ), actionCollection() );
  mAction.save       = KStdAction::save( TQT_TQOBJECT(editor()), TQT_SLOT(save()), actionCollection() );
  mAction.saveAs     = KStdAction::saveAs( TQT_TQOBJECT(editor()), TQT_SLOT(saveAs()), actionCollection() );
  mAction.revert     = KStdAction::revert( TQT_TQOBJECT(editor()), TQT_SLOT(reload()), actionCollection() );
  //mAction.revert     = KStdAction::revert( TQT_TQOBJECT(this), TQT_SLOT(resizeTest()), actionCollection() );

  mAction.close      = KStdAction::close( TQT_TQOBJECT(editor()), TQT_SLOT(close()), actionCollection() );
  mAction.print      = KStdAction::print( TQT_TQOBJECT(editor()), TQT_SLOT(print()), actionCollection() );
  mAction.exportData = new KAction( i18n("E&xport..."), 0,
    TQT_TQOBJECT(editor()), TQT_SLOT(exportDialog()), actionCollection(), "export" );
  mAction.cancel     = new KAction( i18n("&Cancel Operation"),
    "stop", 0, TQT_TQOBJECT(editor()), TQT_SLOT(stop()), actionCollection(), "cancel" );
  mAction.readOnly   = new KToggleAction( i18n("&Read Only"),
    0, TQT_TQOBJECT(editor()), TQT_SLOT(toggleWriteProtection()),actionCollection(), "read_only" );
  mAction.resizeLock = new KToggleAction( i18n("&Allow Resize"),
   0, TQT_TQOBJECT(editor()), TQT_SLOT(toggleResizeLock()),actionCollection(), "resize_lock" );
  mAction.newWindow  = new KAction( i18n("N&ew Window"),
    0, TQT_TQOBJECT(this), TQT_SLOT(newWindow()),actionCollection(), "new_window" );
  mAction.closeWindow= new KAction( i18n("Close &Window"),
    0, TQT_TQOBJECT(this), TQT_SLOT(closeWindow()),actionCollection(), "close_window" );
  mAction.quit       = KStdAction::quit( TQT_TQOBJECT(this), TQT_SLOT(closeProgram()), actionCollection() );

  mAction.undo       = KStdAction::undo(      TQT_TQOBJECT(editor()), TQT_SLOT(undo()), actionCollection() );
  mAction.redo       = KStdAction::redo(      TQT_TQOBJECT(editor()), TQT_SLOT(redo()), actionCollection() );
  mAction.cut        = KStdAction::cut(       TQT_TQOBJECT(editor()), TQT_SLOT(cut()), actionCollection() );
  mAction.copy       = KStdAction::copy(      TQT_TQOBJECT(editor()), TQT_SLOT(copy()), actionCollection() );
  mAction.paste      = KStdAction::paste(     TQT_TQOBJECT(editor()), TQT_SLOT(paste()), actionCollection() );
  mAction.selectAll  = KStdAction::selectAll( TQT_TQOBJECT(editor()), TQT_SLOT(selectAll()),actionCollection() );
  mAction.unselect   = KStdAction::deselect(  TQT_TQOBJECT(editor()), TQT_SLOT(unselect()), actionCollection());
  mAction.find     = KStdAction::find(      TQT_TQOBJECT(editor()), TQT_SLOT(find()), actionCollection() );
  mAction.findNext   = KStdAction::findNext(  TQT_TQOBJECT(editor()), TQT_SLOT(findNext()), actionCollection() );
  mAction.findPrev   = KStdAction::findPrev(  TQT_TQOBJECT(editor()),TQT_SLOT(findPrevious()),actionCollection() );
  mAction.replace  = KStdAction::replace(   TQT_TQOBJECT(editor()), TQT_SLOT(replace()), actionCollection() );
  mAction.gotoOffset = new KAction( i18n("&Goto Offset..."), CTRL+Key_G,
    TQT_TQOBJECT(editor()), TQT_SLOT(gotoOffset()),actionCollection(), "goto_offset" );
  mAction.insertPattern = new KAction( i18n("&Insert Pattern..."), CTRL+Key_Insert,
    TQT_TQOBJECT(editor()), TQT_SLOT(insertPattern()), actionCollection(), "insert_pattern" );

  mAction.copyAsText = new KAction( i18n("Copy as &Text"), 0,
    TQT_TQOBJECT(editor()), TQT_SLOT(copyText()), actionCollection(), "copy_as_text" );
  mAction.pasteToNewFile = new KAction( i18n("Paste into New &File"), 0,
    TQT_TQOBJECT(editor()), TQT_SLOT(pasteNewFile()), actionCollection(), "paste_into_new_file" );
  mAction.pasteToNewWindow = new KAction( i18n("Paste into New &Window"), 0,
    TQT_TQOBJECT(this), TQT_SLOT(pasteNewWindow()), actionCollection(), "paste_into_new_window" );

  mAction.hexadecimal = new KRadioAction( i18n("&Hexadecimal"),
    0, TQT_TQOBJECT(this), TQT_SLOT(setDisplayMode()), actionCollection(), "mode_hex" );
  mAction.decimal     = new KRadioAction( i18n("&Decimal"),
    0, TQT_TQOBJECT(this), TQT_SLOT(setDisplayMode()), actionCollection(), "mode_dec" );
  mAction.octal       = new KRadioAction( i18n("&Octal"),
    0, TQT_TQOBJECT(this), TQT_SLOT(setDisplayMode()), actionCollection(), "mode_oct" );
  mAction.binary      = new KRadioAction( i18n("&Binary"),
    0, TQT_TQOBJECT(this), TQT_SLOT(setDisplayMode()), actionCollection(), "mode_bin" );
  mAction.textOnly    = new KRadioAction( i18n("&Text"),
    0, TQT_TQOBJECT(this), TQT_SLOT(setDisplayMode()), actionCollection(), "mode_text" );
  mAction.hexadecimal->setExclusiveGroup( "displayMode" );
  mAction.decimal->setExclusiveGroup( "displayMode" );
  mAction.octal->setExclusiveGroup( "displayMode" );
  mAction.binary->setExclusiveGroup( "displayMode" );
  mAction.textOnly->setExclusiveGroup( "displayMode" );

  mAction.showOffsetColumn = new KToggleAction( i18n("Show O&ffset Column"),
    0, TQT_TQOBJECT(editor()), TQT_SLOT(toggleOffsetColumnVisibility()),actionCollection(), "show_offset_column" );
  mAction.showTextColumn = new KToggleAction( i18n("Show Te&xt Field"),
    0, TQT_TQOBJECT(editor()), TQT_SLOT(toggleTextColumnVisibility()),actionCollection(), "show_text_field" );
  mAction.offsetAsDecimal = new KToggleAction( i18n("Off&set as Decimal"),
    0, TQT_TQOBJECT(editor()), TQT_SLOT(toggleOffsetAsDecimal()),actionCollection(), "offset_as_decimal" );
  mAction.dataUppercase = new KToggleAction( i18n("&Upper Case (Data)"),
    0, TQT_TQOBJECT(editor()), TQT_SLOT(toggleDataUppercase()),actionCollection(), "upper_case_data" );
  mAction.offsetUppercase = new KToggleAction( i18n("Upper &Case (Offset)"),
    0, TQT_TQOBJECT(editor()), TQT_SLOT(toggleOffsetUppercase()),actionCollection(), "upper_case_offset" );

  mAction.defaultEncoding = new KRadioAction( i18n("&Default encoding", "&Default"),
    0, TQT_TQOBJECT(this), TQT_SLOT( setEncoding()), actionCollection(), "enc_default" );
  mAction.usAsciiEncoding = new KRadioAction( i18n("US-&ASCII (7 bit)"),
    0, TQT_TQOBJECT(this), TQT_SLOT( setEncoding()), actionCollection(), "enc_ascii");
  mAction.ebcdicEncoding  = new KRadioAction( i18n("&EBCDIC"),
    0, TQT_TQOBJECT(this), TQT_SLOT( setEncoding()), actionCollection(), "enc_ebcdic" );
//   mAction.customEncoding  = new KAction( i18n("&Custom..."),
//     0, TQT_TQOBJECT(editor()), TQT_SLOT( encoding()), actionCollection(), "enc_custom" );
  mAction.defaultEncoding->setExclusiveGroup( "encodingMode" );
  mAction.usAsciiEncoding->setExclusiveGroup( "encodingMode" );
  mAction.ebcdicEncoding->setExclusiveGroup( "encodingMode" );

  mAction.strings = new KAction( i18n("&Extract Strings..."), 0,
    TQT_TQOBJECT(editor()), TQT_SLOT(strings()), actionCollection(), "extract_strings" );
//   mAction.recordViewer = new KAction( i18n("&Record Viewer"), 0,
//     TQT_TQOBJECT(editor()), TQT_SLOT(recordView()), actionCollection(), "record_viewer" );
  mAction.filter = new KAction( i18n("&Binary Filter..."), 0,
    TQT_TQOBJECT(editor()), TQT_SLOT(filter()), actionCollection(), "binary_filter" );
  mAction.characterTable = new KAction( i18n("&Character Table"), 0,
    TQT_TQOBJECT(editor()), TQT_SLOT(chart()), actionCollection(), "char_table" );
  mAction.converter = new KAction( i18n("C&onverter"), 0,
    TQT_TQOBJECT(editor()), TQT_SLOT(converter()), actionCollection(), "converter" );
  mAction.statistics = new KAction( i18n("&Statistics"), 0,
    TQT_TQOBJECT(editor()), TQT_SLOT(statistics()), actionCollection(), "statistics" );

  mAction.addBookmark = KStdAction::addBookmark( TQT_TQOBJECT(editor()),
    TQT_SLOT(addBookmark()), actionCollection() );
  mAction.replaceBookmark = new KAction( i18n("&Replace Bookmark"), CTRL+Key_E,
    TQT_TQOBJECT(editor()), TQT_SLOT(replaceBookmark()), actionCollection(), "replace_bookmark");
  mAction.removeBookmark = new KAction( i18n("R&emove Bookmark"), CTRL+Key_U,
    TQT_TQOBJECT(editor()), TQT_SLOT(removeBookmark()), actionCollection(), "remove_bookmark" );
  mAction.removeAllBookmark = new KAction( i18n("Re&move All"), 0,
    TQT_TQOBJECT(editor()), TQT_SLOT(removeAllBookmark()), actionCollection(), "remove_all_bookmarks" );
  mAction.nextBookmark = new KAction( i18n("Goto &Next Bookmark"),
				      ALT+Key_Down, TQT_TQOBJECT(editor()),
				      TQT_SLOT(gotoNextBookmark()), actionCollection(), "next_bookmark" );
  mAction.prevBookmark = new KAction( i18n("Goto &Previous Bookmark"),
				      ALT+Key_Up, TQT_TQOBJECT(editor()),
				      TQT_SLOT(gotoPrevBookmark()), actionCollection(), "prev_bookmark" );

  createStandardStatusBarAction();
  setStandardToolBarMenuEnabled(true);
  mAction.showFullPath  = new KToggleAction( i18n("Show F&ull Path"),
    0, TQT_TQOBJECT(this), TQT_SLOT(showFullPath()), actionCollection(), "show_full_path" );

  mAction.tabHide = new KRadioAction( i18n("&Hide"),
    0, TQT_TQOBJECT(this), TQT_SLOT(showDocumentTabs()), actionCollection(), "doctab_hide" );
  mAction.tabShowAboveEditor = new KRadioAction( i18n("&Above Editor"),
    0, TQT_TQOBJECT(this), TQT_SLOT(showDocumentTabs()), actionCollection(), "doctab_above" );
  mAction.tabShowBelowEditor = new KRadioAction( i18n("&Below Editor"),
    0, TQT_TQOBJECT(this), TQT_SLOT(showDocumentTabs()), actionCollection(), "doctab_below" );
  mAction.tabHide->setExclusiveGroup( "editorTab" );
  mAction.tabShowAboveEditor->setExclusiveGroup( "editorTab" );
  mAction.tabShowBelowEditor->setExclusiveGroup( "editorTab" );

  mAction.conversionHide  = new KRadioAction( i18n("&Hide"),
    0, TQT_TQOBJECT(this), TQT_SLOT(showConversionField()), actionCollection(), "conversion_field_hide");
  mAction.conversionFloat = new KRadioAction( i18n("&Floating"),
    0, TQT_TQOBJECT(this), TQT_SLOT(showConversionField()), actionCollection(), "conversion_field_float");
  mAction.conversionEmbed = new KRadioAction( i18n("&Embed in Main Window"),
    0, TQT_TQOBJECT(this), TQT_SLOT(showConversionField()), actionCollection(), "conversion_field_embed");
  mAction.conversionHide->setExclusiveGroup( "conversionField" );
  mAction.conversionFloat->setExclusiveGroup( "conversionField" );
  mAction.conversionEmbed->setExclusiveGroup( "conversionField" );

  mAction.searchHide = new KRadioAction( i18n("&Hide"),
    0, TQT_TQOBJECT(this), TQT_SLOT(showSearchBar()), actionCollection(), "searchbar_hide" );
  mAction.searchShowAboveEditor = new KRadioAction( i18n("&Above Editor"),
    Key_F5, TQT_TQOBJECT(this), TQT_SLOT(showSearchBar()), actionCollection(), "searchbar_above" );
  mAction.searchShowBelowEditor = new KRadioAction( i18n("&Below Editor"),
    Key_F6, TQT_TQOBJECT(this), TQT_SLOT(showSearchBar()), actionCollection(), "searchbar_below" );
  mAction.searchHide->setExclusiveGroup( "searchBar" );
  mAction.searchShowAboveEditor->setExclusiveGroup( "searchBar" );
  mAction.searchShowBelowEditor->setExclusiveGroup( "searchBar" );

  KStdAction::saveOptions(TQT_TQOBJECT(this), TQT_SLOT(writeConfiguration()), actionCollection());
  KStdAction::preferences(TQT_TQOBJECT(editor()) ,TQT_SLOT(options()),actionCollection() );
//   mAction.favorites = new KAction( i18n("P&rofiles..."), 0,
//     TQT_TQOBJECT(editor()), TQT_SLOT(favorites()), actionCollection(), "favorites" );

  KStdAction::help( TQT_TQOBJECT(this), TQT_SLOT(appHelpActivated()), actionCollection() );

  mDragLabel = new CDragLabel(this);
  mDragLabel->setPixmap( UserIcon( "hexdrag" ) );
  mDragLabel->setDragMask( UserIcon( "hexmask" ) );
  mDragLabel->setEnabled( false ); // Enabled once we open a document
  TQToolTip::add( mDragLabel, i18n("Drag document") );
 (void) new KWidgetAction(mDragLabel, i18n("Drag Document"), 0, TQT_TQOBJECT(editor()), TQT_SLOT(options()), actionCollection(), "drag_document");

  createGUI("khexeditui.rc", false);

  TQPopupMenu *popup = (TQPopupMenu *)factory()->container("editor_popup", this);
  hexView()->setPopupMenu( popup );

  int id = 100;
  toolBar(0)->insertButton( MainBarIcon("lock"), id, TQT_SIGNAL(clicked()),
    TQT_TQOBJECT(editor()), TQT_SLOT(toggleWriteProtection()), true,
    i18n("Toggle write protection") );
  toolBar(0)->alignItemRight( id );
  mWriteProtectButton = toolBar(0)->getButton(id);

  TQPopupMenu *documentMenu = (TQPopupMenu *)factory()->container("documents", this);
  hexView()->setDocumentMenu(documentMenu);
  connect(documentMenu, TQT_SIGNAL(activated(int)), TQT_SLOT(documentMenuCB(int)));

}


void KHexEdit::setupStatusBar( void )
{
  CStatusBarProgress *progess = new CStatusBarProgress( statusBar() );
  statusBar()->addWidget( progess, 10 );
  connect( progess, TQT_SIGNAL(pressed()), TQT_TQOBJECT(editor()), TQT_SLOT(stop()) );
  connect( TQT_TQOBJECT(editor()), TQT_SIGNAL( setProgress( int ) ),
	   progess, TQT_SLOT( setValue( int ) ) );
  connect( TQT_TQOBJECT(editor()), TQT_SIGNAL( setProgress( int, int ) ),
	   progess, TQT_SLOT( setValue( int, int ) ) );
  connect( TQT_TQOBJECT(editor()), TQT_SIGNAL( enableProgressText( bool ) ),
	   progess, TQT_SLOT( setTextEnabled( bool ) ) );
  connect( TQT_TQOBJECT(editor()), TQT_SIGNAL( setProgressText( const TQString & ) ),
	   progess, TQT_SLOT( setText( const TQString & ) ) );

  statusBar()->insertFixedItem( i18n("Selection: 0000:0000 0000:0000"),
				status_Selection );
  statusBar()->insertFixedItem( i18n("M"), status_Modified );
  statusBar()->insertFixedItem( i18n("OVR"), status_Ovr );
  statusBar()->insertFixedItem( i18n("Size: FFFFFFFFFF"), status_Size );
  statusBar()->insertFixedItem( i18n("Offset: FFFFFFFFFF-F"), status_Offset );
  statusBar()->insertFixedItem( i18n("FFF"), status_Layout );
  statusBar()->insertFixedItem( i18n("RW"), status_WriteProtect );

  statusBar()->setItemAlignment( status_Selection, AlignLeft|AlignVCenter );
  statusBar()->setItemAlignment( status_Size, AlignLeft|AlignVCenter );
  statusBar()->setItemAlignment( status_Offset, AlignLeft|AlignVCenter );

  //
  // Some good default strings we can use after the fields have got
  // their fixed width.
  //
  statusBar()->changeItem( i18n("Offset:"), status_Offset );
  statusBar()->changeItem( i18n("Size:"), status_Size );
  statusBar()->changeItem( "", status_Modified );
  statusBar()->changeItem( "", status_Selection );
  connect( statusBar(),TQT_SIGNAL(pressed(int)),this,TQT_SLOT(statusBarPressed(int)) );
}


void KHexEdit::removeRecentFiles( void )
{
  mAction.openRecent->clearURLList();
}


void KHexEdit::initialize( bool openFiles )
{
  mUndoState = CHexBuffer::UndoOk; // Reset in function below
  setUndoState( 0 );

  mSelectionOffset = 0;
  mSelectionSize   = 1;
  mSelectionAsHexadecimal = true;
  setSelectionState( 0, 0 );

  operationChanged( false );

  editor()->initialize();

  mAction.showFullPath->setChecked( mShowFullPath );
  showFullPath();

  if ( openFiles && editor()->openFile() != SDisplayMisc::none )
  {
    // Code modified from tdelibs/tdeui/kactionclasses.cpp KRecentFilesAction::loadEntries
      
    KConfig* config = kapp->config();

    const TQString oldGroup = config->group();
    
    const TQString groupname = "RecentFiles";
    config->setGroup( groupname );
    
        // read file list
    unsigned int maxItems = mAction.openRecent->maxItems();
    if ( editor()->openFile() == SDisplayMisc::mostRecent )
        maxItems = 1;
    for( unsigned int i = 1 ; i <= maxItems ; i++ )
    {
        const TQString key = TQString( "File%1" ).tqarg( i );
        const TQString value = config->readPathEntry( key );
    
        if (!value.isEmpty())
        {
            mStartupFileList.append( value );
            mStartupOffsetList.append( "0" ); // ### TODO find a way to still have kept offsets
        }
    }

    config->setGroup( oldGroup );

  }  
  
  //
  // Open file(s) after editor has been displayed.
  // Main reason: If anything goes wrong and an error dialog box
  // is displayed, then the editor will not be visible until the error
  // is confirmed and the (modal) dialog is closed.
  //
  TQTimer::singleShot( 100, TQT_TQOBJECT(this), TQT_SLOT(delayedStartupOpen()) );
}


void KHexEdit::delayedStartupOpen( void )
{
  open( mStartupFileList, mStartupOffsetList );
  mStartupFileList.clear();
  mStartupOffsetList.clear();
}


void KHexEdit::statusBarPressed( int id )
{
  if( id == status_WriteProtect )
  {
    editor()->toggleWriteProtection();
  }
  else if( id == status_Ovr )
  {
    editor()->toggleInsertMode();
  }
  else if( id == status_Offset )
  {
    editor()->toggleOffsetAsDecimal();
  }
  else if( id == status_Selection )
  {
    if( mSelectionSize > 0 )
    {
      mSelectionAsHexadecimal = mSelectionAsHexadecimal == true ? false : true;
      setSelectionText( mSelectionOffset, mSelectionSize );
    }
  }

}



void KHexEdit::open( TQStringList &fileList, TQStringList &offsetList )
{
  //
  // We load in reverse to allow the first document in
  // the list to become the active one.
  //

  uint offset;
  for( uint i = fileList.count(); i>0; i-- )
  {
    if( i <= offsetList.count() )
    {
      offset = (*offsetList.at(i-1)).toUInt(0,16);
    }
    else
    {
      offset = 0;
    }
    editor()->open( *fileList.at(i-1), true, offset );
  }
}

void KHexEdit::addRecentFile( const TQString &fileName )
{
  if( fileName.isEmpty() == true )
  {
    return;
  }

  if( fileName.contains( i18n( "Untitled" ), false ) )
  {
    return;
  }

  const KURL url( KURL::fromPathOrURL( fileName ) );
  kdDebug(1501) << k_funcinfo << " adding recent " << fileName << " => " << url.prettyURL() << endl;
  mAction.openRecent->addURL( url );

}


void KHexEdit::removeRecentFile( const TQString &fileName )
{
  if( fileName.isEmpty() == true )
  {
    return;
  }

  if( fileName.contains( i18n( "Untitled" ), false ) )
  {
    return;
  }

  const KURL url( KURL::fromPathOrURL( fileName ) );
  mAction.openRecent->removeURL( url );

}


void KHexEdit::renameRecentFile(const TQString &curName, const TQString &newName)
{
  if( curName.contains( i18n( "Untitled" ), false ) )
  {
    addRecentFile( newName );
  }
  else
  {
      mAction.openRecent->removeURL( KURL::fromPathOrURL( curName ) );
      mAction.openRecent->addURL( KURL::fromPathOrURL( newName ) );
  }
}



void KHexEdit::slotFileOpenRecent( const KURL& url )
{
  if ( url.isLocalFile() )
  {
    editor()->open( url.path(), false, 0 );
  }
  else
  {
    // ### TODO: support network transparency
    KMessageBox::error( this, i18n("Non local recent file: %1").tqarg( url.prettyURL() ) );
  }
}


KHexEdit *KHexEdit::newWindow( void )
{
  KHexEdit *hexEdit = new KHexEdit;
  if( hexEdit == 0 )
  {
    TQString msg = i18n( "Can not create new window.\n" );
    msg += hexError( Err_NoMemory );
    KMessageBox::error( topLevelWidget(), msg );
    return(0);
  }
  hexEdit->show();
  return(hexEdit);
}



void KHexEdit::pasteNewWindow( void )
{
  KHexEdit *app = newWindow();
  if( app != 0 )
  {
    app->editor()->pasteNewFile();
  }
}



void KHexEdit::closeWindow( void )
{
  close();
}


void KHexEdit::closeProgram( void )
{
  if( mWindowList.count() > 1 )
  {
    for( KHexEdit *w = mWindowList.first(); w != 0; w = mWindowList.next() )
    {
      if( w->editor()->modified() == true )
      {
	TQString msg = i18n(""
	  "There are windows with unsaved modified documents. "
	  "If you quit now, these modifications will be lost.");
	int reply = KMessageBox::warningContinueCancel( topLevelWidget(), msg, TQString(), KStdGuiItem::quit() );
	if( reply == KMessageBox::Continue )
	{
	  break;
	}
	else
	{
	  return;
	}
      }
    }


    //
    // Ths will write current configuration to disk.
    //
    queryExit();

    //
    // Make sure every widget is destroyed. The reason why I do this
    // (besides it is the clean way to do it) is to make sure any
    // configuration updates in the various destructors are triggered.
    //
    mWindowList.setAutoDelete(true);
    while( mWindowList.first() )
    {
      mWindowList.removeRef( mWindowList.first() );
    }

    kapp->quit();
  }
  else
  {
    closeWindow();
  }

}


bool KHexEdit::queryClose( void )
{
  bool success = editor()->closeAll();
  if( success == true )
  {
    mWindowList.removeRef( this );
  }

  return( success );
}


//
// We will always return true here.
//
bool KHexEdit::queryExit( void )
{
  writeConfiguration();
  return( true );
}

void KHexEdit::readConfiguration( void )
{
  KConfig &config = *kapp->config();
  readConfiguration( config );
}


void KHexEdit::writeConfiguration( void )
{
  KConfig &config = *kapp->config();
  writeConfiguration( config );
}


void KHexEdit::writeConfiguration( KConfig &config )
{
    saveMainWindowSettings(& config, "General Options");
  config.setGroup("General Options" );
  config.writeEntry("ShowFullPath",  mShowFullPath );
  config.writeEntry("ConversionPosition", (int)mManager->conversionPosition());
  config.writeEntry("TabBarPosition", (int)mManager->tabBarPosition());
  config.writeEntry("SearchBarPosition", (int)mManager->searchBarPosition());

  if ( editor()->discardRecentFiles() )
      mAction.openRecent->clearURLList();
  mAction.openRecent->saveEntries( &config );
  
  converter()->writeConfiguration( config );
  editor()->writeConfiguration( config );

  config.sync();
}


void KHexEdit::readConfiguration( KConfig &config )
{
    applyMainWindowSettings(& config, "General Options");

    config.setGroup("General Options");
  mShowFullPath   = config.readBoolEntry( "ShowFullPath", true );
  int position    = config.readNumEntry( "ConversionPosition",
					 CHexManagerWidget::Embed );

  mAction.conversionHide->blockSignals(true);
  mAction.conversionFloat->blockSignals(true);
  mAction.conversionEmbed->blockSignals(true);
  mAction.conversionHide->setChecked(
    position == CHexManagerWidget::Hide );
  mAction.conversionFloat->setChecked(
    position == CHexManagerWidget::Float );
  mAction.conversionEmbed->setChecked(
    position == CHexManagerWidget::Embed );
  mAction.conversionHide->blockSignals(false);
  mAction.conversionFloat->blockSignals(false);
  mAction.conversionEmbed->blockSignals(false);
  showConversionField();

  position = config.readNumEntry( "TabBarPosition", CHexManagerWidget::Hide );
  mAction.tabHide->blockSignals(true);
  mAction.tabShowAboveEditor->blockSignals(true);
  mAction.tabShowBelowEditor->blockSignals(true);
  mAction.tabHide->setChecked(
    position==CHexManagerWidget::Hide );
  mAction.tabShowAboveEditor->setChecked(
    position==CHexManagerWidget::AboveEditor );
  mAction.tabShowBelowEditor->setChecked(
    position==CHexManagerWidget::BelowEditor );
  mAction.tabHide->blockSignals(false);
  mAction.tabShowAboveEditor->blockSignals(false);
  mAction.tabShowBelowEditor->blockSignals(false);
  showDocumentTabs();

  position = config.readNumEntry( "SearchBarPosition",
				  CHexManagerWidget::BelowEditor );
  mAction.searchHide->blockSignals(true);
  mAction.searchShowAboveEditor->blockSignals(true);
  mAction.searchShowBelowEditor->blockSignals(true);
  mAction.searchHide->setChecked(
    position==CHexManagerWidget::Hide );
  mAction.searchShowAboveEditor->setChecked(
    position==CHexManagerWidget::AboveEditor );
  mAction.searchShowBelowEditor->setChecked(
    position==CHexManagerWidget::BelowEditor );
  mAction.searchHide->blockSignals(false);
  mAction.searchShowAboveEditor->blockSignals(false);
  mAction.searchShowBelowEditor->blockSignals(false);
  showSearchBar();

  mAction.openRecent->loadEntries( &config );

  converter()->readConfiguration( config );
  editor()->readConfiguration( config );
}



void KHexEdit::operationChanged( bool state )
{
  mAction.cancel->setEnabled( state );
}



void KHexEdit::cursorChanged( SCursorState &state )
{
  TQString offset( i18n("Offset:") );

  if( state.valid == false )
  {
    statusBar()->changeItem( offset, status_Offset );
  }
  else
  {
    SDisplayLayout &tqlayout = editor()->tqlayout();

    if( tqlayout.offsetMode == SDisplayLayout::hexadecimal )
    {
      if( tqlayout.offsetUpperCase == false )
      {
	offset += TQString().sprintf( " %04x:%04x-%u", state.offset>>16,
				     state.offset&0x0000FFFF, state.cell );
      }
      else
      {
	offset += TQString().sprintf( " %04X:%04X-%u", state.offset>>16,
				     state.offset&0x0000FFFF, state.cell );
      }
    }
    else
    {
      offset += TQString().sprintf( " %010u-%u", state.offset, state.cell );
    }
    statusBar()->changeItem( offset, status_Offset );
  }

  setUndoState( state.undoState );
  setSelectionState( state.selectionOffset, state.selectionSize );
}




void KHexEdit::fileState( SFileState &state )
{
  if( state.valid == true )
  {
    statusBar()->changeItem( i18n("Size: %1").tqarg( state.size ), status_Size);
    statusBar()->changeItem( state.modified ? "!" : "", status_Modified);

    if( mIsModified != state.modified )
    {
      mIsModified = state.modified;
      setupCaption( hexView()->url() );
    }

  }
  else
  {
    statusBar()->changeItem( "", status_Size );
    statusBar()->changeItem( "", status_Modified );
  }
}



void KHexEdit::layoutChanged( const SDisplayLayout &tqlayout )
{
  KRadioAction *radioAction;

  if( tqlayout.primaryMode == SDisplayLayout::hexadecimal )
  {
    radioAction = mAction.hexadecimal;
  }
  else if( tqlayout.primaryMode == SDisplayLayout::decimal )
  {
    radioAction = mAction.decimal;
  }
  else if( tqlayout.primaryMode == SDisplayLayout::octal )
  {
    radioAction = mAction.octal;
  }
  else if( tqlayout.primaryMode == SDisplayLayout::binary )
  {
    radioAction = mAction.binary;
  }
  else
  {
    radioAction = mAction.textOnly;
  }
  radioAction->blockSignals(true);
  radioAction->setChecked( true );
  radioAction->blockSignals(false);

  mAction.showOffsetColumn->blockSignals(true);
  mAction.showTextColumn->blockSignals(true);
  mAction.offsetAsDecimal->blockSignals(true);
  mAction.dataUppercase->blockSignals(true);
  mAction.offsetUppercase->blockSignals(true);

  mAction.showOffsetColumn->setChecked( tqlayout.offsetVisible );
  mAction.showTextColumn->setEnabled(
    tqlayout.primaryMode != SDisplayLayout::textOnly );
  mAction.showTextColumn->setChecked(
    tqlayout.secondaryMode != SDisplayLayout::hide );
  mAction.offsetAsDecimal->setChecked(
    tqlayout.offsetMode != SDisplayLayout::hexadecimal);
  mAction.dataUppercase->setChecked( tqlayout.primaryUpperCase );
  mAction.offsetUppercase->setChecked( tqlayout.offsetUpperCase );

  mAction.showOffsetColumn->blockSignals(false);
  mAction.showTextColumn->blockSignals(false);
  mAction.offsetAsDecimal->blockSignals(false);
  mAction.dataUppercase->blockSignals(false);
  mAction.offsetUppercase->blockSignals(false);

  if( tqlayout.primaryMode == SDisplayLayout::hexadecimal )
  {
    statusBar()->changeItem( i18n("Hex"), status_Layout );
  }
  else if( tqlayout.primaryMode == SDisplayLayout::decimal )
  {
    statusBar()->changeItem( i18n("Dec"), status_Layout );
  }
  else if( tqlayout.primaryMode == SDisplayLayout::octal )
  {
    statusBar()->changeItem( i18n("Oct"), status_Layout );
  }
  else if( tqlayout.primaryMode == SDisplayLayout::binary )
  {
    statusBar()->changeItem( i18n("Bin"), status_Layout );
  }
  else if( tqlayout.primaryMode == SDisplayLayout::textOnly )
  {
    statusBar()->changeItem( i18n("Txt"), status_Layout );
  }
}


void KHexEdit::inputModeChanged( const SDisplayInputMode &mode )
{
  mAction.readOnly->blockSignals(true);
  mAction.resizeLock->blockSignals(true);
  mAction.readOnly->setChecked( mode.readOnly );
  mAction.resizeLock->setChecked( mode.allowResize );
  mAction.readOnly->blockSignals(false);
  mAction.resizeLock->blockSignals(false);

  statusBar()->changeItem( mode.readOnly == true ?
			   i18n("R") : i18n("RW"), status_WriteProtect );

  if( mode.readOnly == false )
  {
    mWriteProtectButton->setIcon("edit");
  }
  else
  {
    mWriteProtectButton->setIcon("lock");
  }
}


void KHexEdit::bookmarkChanged( TQPtrList<SCursorOffset> &list )
{
  unplugActionList("bookmark_list");
  mAction.bookmarkList.setAutoDelete(true);
  mAction.bookmarkList.clear();

  if( list.count() == 0 )
  {
    return;
  }

  mAction.bookmarkList.append(new KActionSeparator());

  TQString text, offset;
  int i=0;

  for( SCursorOffset *p=list.first(); p!=0; p=list.next(), i++ )
  {
    offset.sprintf("%04X:%04X", p->offset>>16, p->offset&0x0000FFFF );
    text = i18n("Offset: %1").tqarg(offset);
    KAction *action = new KAction(text, 0, mAction.bookmarkMapper, TQT_SLOT(map()), TQT_TQOBJECT(this), text.latin1());
    int key = acceleratorNumKey( i );
    if( key > 0 )
    {
      action->setShortcut(KShortcut(ALT+key));
    }
    mAction.bookmarkMapper->setMapping(action, i);
    mAction.bookmarkList.append(action);
  }
  plugActionList("bookmark_list", mAction.bookmarkList);
}


void KHexEdit::setupCaption( const TQString &url )
{
  KURL u(url);
  if( mShowFullPath == true )
  {
    setCaption( u.prettyURL(), hexView()->modified() );
  }
  else
  {
    setCaption( u.fileName(), hexView()->modified() );
  }
}


void KHexEdit::fileActive( const TQString &url, bool onDisk )
{
  setupCaption( url );
  mDragLabel->setUrl( url );
  mDragLabel->setEnabled( onDisk );

  mAction.revert->setEnabled( onDisk );

  if( url.isNull() == false )
  {
    addDocument( url );
    addRecentFile( url );
    setTickedDocument( url );
  }
}


void KHexEdit::fileRename( const TQString &curName, const TQString &newName )
{
  if( newName.isNull() == true )
  {
    return;
  }

  renameDocument( curName, newName );
  renameRecentFile( curName, newName );
  setupCaption( newName );
  //setCaption( newName );
}


void KHexEdit::fileClosed( const TQString &url )
{
  removeDocument( url );
}


void KHexEdit::editMode( CHexBuffer::EEditMode editMode )
{
  statusBar()->changeItem( editMode == CHexBuffer::EditReplace ?
			   i18n("OVR") : i18n("INS"), status_Ovr );
}


void KHexEdit::encodingChanged( const SEncodeState &encodeState )
{
  KRadioAction *radioAction;

  if( encodeState.mode == CConversion::cnvDefault )
  {
    radioAction = mAction.defaultEncoding;
  }
  else if( encodeState.mode == CConversion::cnvUsAscii )
  {
    radioAction = mAction.usAsciiEncoding;
  }
  else if( encodeState.mode == CConversion::cnvEbcdic )
  {
    radioAction = mAction.ebcdicEncoding;
  }
  else
  {
    return;
  }

  radioAction->blockSignals(true);
  radioAction->setChecked( true );
  radioAction->blockSignals(false);

  if( mSelectionSize == 0 )
  {
    statusBar()->changeItem( i18n("Encoding: %1").tqarg(encodeState.name),
			     status_Selection );
  }
}


void KHexEdit::textWidth( uint )
{
  resize( mManager->preferredWidth(), height() );
}


void KHexEdit::setDisplayMode( void )
{
  if( mAction.hexadecimal->isChecked() == true )
  {
    editor()->setHexadecimalMode();
  }
  else if( mAction.decimal->isChecked() == true )
  {
    editor()->setDecimalMode();
  }
  else if( mAction.octal->isChecked() == true )
  {
    editor()->setOctalMode();
  }
  else if( mAction.binary->isChecked() == true )
  {
    editor()->setBinaryMode();
  }
  else if( mAction.textOnly->isChecked() == true )
  {
    editor()->setTextMode();
  }
}


void KHexEdit::showFullPath( void )
{
  mShowFullPath = mAction.showFullPath->isChecked();
  setupCaption( hexView()->url() );
}


void KHexEdit::showDocumentTabs( void )
{
  if( mAction.tabHide->isChecked() == true )
  {
    mManager->setTabBarPosition( CHexManagerWidget::HideItem );
  }
  else if( mAction.tabShowAboveEditor->isChecked() == true )
  {
    mManager->setTabBarPosition( CHexManagerWidget::AboveEditor );
  }
  else if( mAction.tabShowBelowEditor->isChecked() == true )
  {
    mManager->setTabBarPosition( CHexManagerWidget::BelowEditor );
  }
}


void KHexEdit::showConversionField( void )
{
  if( mAction.conversionHide->isChecked() == true )
  {
    mManager->setConversionVisibility( CHexManagerWidget::Hide );
  }
  else if( mAction.conversionFloat->isChecked() == true )
  {
    mManager->setConversionVisibility( CHexManagerWidget::Float );
  }
  else if( mAction.conversionEmbed->isChecked() == true )
  {
    mManager->setConversionVisibility( CHexManagerWidget::Embed );
    if( width() < mManager->preferredWidth() )
    {
      resize( mManager->preferredWidth(), height() );
    }
  }
}


void KHexEdit::showSearchBar( void )
{
  if( mAction.searchHide->isChecked() == true )
  {
    mManager->setSearchBarPosition( CHexManagerWidget::HideItem );
  }
  else if( mAction.searchShowAboveEditor->isChecked() == true )
  {
    mManager->setSearchBarPosition( CHexManagerWidget::AboveEditor );
  }
  else if( mAction.searchShowBelowEditor->isChecked() == true )
  {
    mManager->setSearchBarPosition( CHexManagerWidget::BelowEditor );
  }
}


void KHexEdit::setEncoding( void )
{
  if( mAction.defaultEncoding->isChecked() == true )
  {
    editor()->encode( CConversion::cnvDefault );
  }
  else if( mAction.usAsciiEncoding->isChecked() == true )
  {
    editor()->encode( CConversion::cnvUsAscii );
  }
  else if( mAction.ebcdicEncoding->isChecked() == true )
  {
    editor()->encode( CConversion::cnvEbcdic );
  }
}


void KHexEdit::setUndoState( uint undoState )
{
  if( mUndoState != undoState )
  {
    mUndoState = undoState;
    mAction.undo->setEnabled( mUndoState & CHexBuffer::UndoOk );
    mAction.redo->setEnabled( mUndoState & CHexBuffer::RedoOk );
  }
}


void KHexEdit::setSelectionState( uint selectionOffset, uint selectionSize )
{
  if( mSelectionSize != selectionSize )
  {
    mSelectionOffset = selectionOffset;
    mSelectionSize   = selectionSize;

    bool valid = mSelectionSize > 0 ? true : false;
    mAction.copy->setEnabled( valid );
    mAction.cut->setEnabled( valid );
    mAction.copyAsText->setEnabled( valid );

    setSelectionText( selectionOffset, selectionSize );
  }
}


void KHexEdit::setSelectionText( uint selectionOffset, uint selectionSize )
{
  if( selectionSize > 0 )
  {
    TQString selection = i18n("Selection:");
    if( mSelectionAsHexadecimal == true )
    {
      selection += TQString().sprintf( " %04x:%04x %04x:%04x",
        selectionOffset>>16, selectionOffset&0x0000FFFF,
        selectionSize>>16, selectionSize&0x0000FFFF );
    }
    else
    {
      selection += TQString().sprintf( " %08u %08u",
        selectionOffset, selectionSize);
    }
    statusBar()->changeItem( selection, status_Selection );
  }
  else
  {
    statusBar()->changeItem(
      i18n("Encoding: %1").tqarg(hexView()->encoding().name), status_Selection);
  }
}


void KHexEdit::documentMenuCB( int index )
{
  if( (uint)index < mDocumentList.count() )
  {
    editor()->open( mDocumentList[index], false, 0 );
  }
}


void KHexEdit::addDocument( const TQString &fileName )
{
  uint documentCount = mDocumentList.count();
  for( uint i=0; i < documentCount; i++ )
  {
    if( fileName == mDocumentList[i] )
    {
      return;
    }
  }

  mDocumentList.append( fileName );

  TQPopupMenu *documentMenu = (TQPopupMenu *)factory()->container("documents", this);
  documentMenu->insertItem( fileName, documentCount, documentCount );
  int key = acceleratorNumKey( documentCount );
  if( key > 0 )
  {
    documentMenu->setAccel( CTRL+key, documentCount );
  }
}


void KHexEdit::removeDocument( const TQString &fileName )
{
  TQStringList::Iterator it;
  for( it = mDocumentList.begin(); it != mDocumentList.end(); it++ )
  {
    if( *it == fileName )
    {
      TQPopupMenu *documentMenu = (TQPopupMenu *)factory()->container("documents", this);

      documentMenu->removeItemAt( mDocumentList.findIndex(*it) );
      mDocumentList.remove( it );

      for( uint i=0; i < mDocumentList.count(); i++ )
      {
	documentMenu->setId( i, i );
	int key = acceleratorNumKey( i );
	if( key > 0 )
	{
	  documentMenu->setAccel( CTRL+key, i );
	}
      }
      return;

    }
  }
}


void KHexEdit::renameDocument( const TQString &curName, const TQString &newName )
{
  TQStringList::Iterator it;
  for( it = mDocumentList.begin(); it != mDocumentList.end(); it++ )
  {
    if( *it == curName )
    {
      TQPopupMenu *documentMenu = (TQPopupMenu *)factory()->container("documents", this);
      documentMenu->changeItem( newName, mDocumentList.findIndex(*it) );
      mDocumentList.insert( it, newName );
      mDocumentList.remove( it );
      return;
    }
  }
}


void KHexEdit::setTickedDocument( const TQString &fileName )
{
  uint documentCount = mDocumentList.count();
  TQPopupMenu *documentMenu = (TQPopupMenu *)factory()->container("documents", this);
  for( uint i=0; i < documentCount; i++ )
  {
    documentMenu->setItemChecked( i, fileName == mDocumentList[i] );
  }
}


void KHexEdit::conversionClosed( void )
{
  mAction.conversionHide->setChecked(true);
}


void KHexEdit::searchBarClosed( void )
{
  mAction.searchHide->setChecked(true);
}


bool KHexEdit::eventFilter( TQObject *o, TQEvent *event )
{
  if( event->type() == TQEvent::DragEnter )
  {
    TQDragEnterEvent *e = (TQDragEnterEvent*)event;
    if( KURLDrag::canDecode(e) )
    {
      e->accept();
      hexView()->setDropHighlight( true );
    }
    return( true );
  }
  else if( event->type() == TQEvent::Drop )
  {
    TQMimeSource &m = *(TQDropEvent*)event;
    hexView()->setDropHighlight( false );

    KURL::List list;
    if( KURLDrag::decode( &m, list ) == true )
    {
      TQStringList offset;
      TQStringList urlList = list.toStringList();
      open( urlList, offset );
      return( true );
    }
    return( true );
  }
  else if( event->type() == TQEvent::DragLeave )
  {
    hexView()->setDropHighlight( false );
    return( true );
  }

  return KMainWindow::eventFilter( o, event );
}


int KHexEdit::acceleratorNumKey( uint index )
{
  static int keys[9] =
  {
    Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9
  };
  return( index >= 9 ? -1 : keys[index] );
}


#include <sys/time.h>
#include <unistd.h>

//
// This function is used to test twin performance
//
void KHexEdit::resizeTest()
{
  struct timeval t1, t2;
  gettimeofday( &t1, 0 );

  int loop=400;

  for( int i=0; i<loop; i+=4 )
  {
    resize( i+400, i+400 );
    //kapp->processOneEvent();
  }

  gettimeofday( &t2, 0 );
  uint last = (t2.tv_sec-t1.tv_sec) * 1000000 + (t2.tv_usec - t1.tv_usec);

  kdDebug(1501) << "Duration: " << (float)last/ 1000000.0 <<  endl;
  kdDebug(1501) << "Duration/loop: " << (float)last/ (1000000.0*(float)loop) << endl;
}
#include "toplevel.moc"
