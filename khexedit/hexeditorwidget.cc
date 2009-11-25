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


#include <iostream>
#include <unistd.h>

#undef Unsorted

#include <kfiledialog.h>
#include <kglobalsettings.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include "chartabledialog.h"
#include "converterdialog.h"
#include "dialog.h"
#include "exportdialog.h"
#include "fileinfodialog.h"
#include "hexeditorwidget.h"
#include "hexerror.h"
#include "optiondialog.h"
#include "printdialogpage.h"
#include "stringdialog.h"




CHexEditorWidget::CHexEditorWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  CHexBuffer *hexBuffer = new CHexBuffer;
  if( hexBuffer == 0 ) { return; }

  mDocumentList.setAutoDelete( true );
  mDocumentList.append( hexBuffer );
  mUntitledCount = 1;

  mHexView = new CHexViewWidget( this, "hexview", hexBuffer );
  if( mHexView == 0 || mHexView->widgetValid() == false ) { return; }

  connect( mHexView, SIGNAL(pleaseOpenNewFile()), SLOT( newFile()) );
  connect( mHexView, SIGNAL(pleaseOpenFile( const QString&, bool, uint )),
	   SLOT( open(const QString&, bool, uint)) );
  connect( mHexView, SIGNAL(pleaseStepFile(bool)), SLOT( stepFile(bool)) );
  connect( kapp, SIGNAL( kdisplayFontChanged() ), SLOT( fontChanged() ) );
  connect( kapp, SIGNAL( kdisplayPaletteChanged() ),SLOT( paletteChanged()) );
  connect( mHexView, SIGNAL( layoutChanged( const SDisplayLayout & ) ),
	   SLOT( layoutChanged( const SDisplayLayout & ) ) );
  connect( mHexView, SIGNAL( inputModeChanged( const SDisplayInputMode & ) ),
	   this, SLOT( inputModeChanged( const SDisplayInputMode & ) ) );

  mHexView->setFocus();
  setBackgroundColor( palette().active().base() );

  mProgressBusy    = false;
  mGotoDialog      = 0;
  mFindDialog      = 0;
  mReplaceDialog   = 0;
  mInsertDialog    = 0;
  mFilterDialog    = 0;
  mOptionDialog    = 0;
  mStringDialog    = 0;
  mCharTableDialog = 0;
  mFileInfoDialog  = 0;
  mExportDialog    = 0;
  mConverterDialog = 0;
  mFindNavigatorDialog   = 0;
  mReplacePromptDialog   = 0;
}

CHexEditorWidget::~CHexEditorWidget( void )
{
  delete mHexView;
  delete mGotoDialog;
  delete mFindDialog;
  delete mReplaceDialog;
  delete mInsertDialog;
  delete mFilterDialog;
  delete mOptionDialog;
  delete mStringDialog;
  delete mCharTableDialog;
  delete mFileInfoDialog;
  delete mExportDialog;
  delete mConverterDialog;
  delete mFindNavigatorDialog;
  delete mReplacePromptDialog;
}


void CHexEditorWidget::initialize( void )
{
  setColor( mDisplayState.color );
  setFont( mDisplayState.font );
  mHexView->setMisc( mDisplayState.misc );
  mHexView->setCursor( mDisplayState.cursor, false );
  mHexView->setLayout( mDisplayState.layout );
  mHexView->setInputMode( mDisplayState.input );
  mHexView->setInsertMode( mDisplayState.misc.insertMode );
}


void CHexEditorWidget::writeConfiguration( KConfig &config )
{
  SDisplayLayout &layout = mDisplayState.layout;
  config.setGroup( "Display Options" );
  config.writeEntry( "PrimaryMode",          layout.primaryModeString() );
  config.writeEntry( "SecondaryMode",        layout.secondaryModeString() );
  config.writeEntry( "OffsetMode",           layout.offsetModeString() );
  config.writeEntry( "OffsetVisible",        layout.offsetVisible );
  config.writeEntry( "PrimaryUpperCase",     layout.primaryUpperCase );
  config.writeEntry( "OffsetUpperCase",      layout.offsetUpperCase );
  config.writeEntry( "LineSize",             layout.lineSize );
  config.writeEntry( "ColumnSize",           layout.columnSize );
  config.writeEntry( "LockLine",             layout.lockLine );
  config.writeEntry( "LockColumn",           layout.lockColumn );
  config.writeEntry( "ColumnCharSpace",      layout.columnCharSpace );
  config.writeEntry( "ColumnSpacing",        layout.columnSpacing );
  config.writeEntry( "SeparatorMarginWidth", layout.separatorMarginWidth );
  config.writeEntry( "EdgeMarginWidth",      layout.edgeMarginWidth );
  config.writeEntry( "LeftSeparatorWidth",   layout.leftSeparatorWidth );
  config.writeEntry( "RightSeparatorWidth",  layout.rightSeparatorWidth );
  config.writeEntry( "GridMode",             layout.gridModeString() );

  SDisplayLine &line = mDisplayState.line;
  config.setGroup( "Line Size" );
  config.writeEntry( "Hexadecimal",line.lineSize[SDisplayLine::hexadecimal] );
  config.writeEntry( "Decimal",    line.lineSize[SDisplayLine::decimal] );
  config.writeEntry( "Octal",      line.lineSize[SDisplayLine::octal] );
  config.writeEntry( "Binary",     line.lineSize[SDisplayLine::binary] );
  config.writeEntry( "TextOnly",   line.lineSize[SDisplayLine::textOnly] );
  config.setGroup( "Column Size" );
  config.writeEntry( "Hexadecimal",line.columnSize[SDisplayLine::hexadecimal]);
  config.writeEntry( "Decimal",    line.columnSize[SDisplayLine::decimal] );
  config.writeEntry( "Octal",      line.columnSize[SDisplayLine::octal] );
  config.writeEntry( "Binary",     line.columnSize[SDisplayLine::binary] );
  config.writeEntry( "TextOnly",   line.columnSize[SDisplayLine::textOnly] );

  SDisplayColor &color = mDisplayState.color;
  config.setGroup( "Display Colors" );
  config.writeEntry( "UseSystemColor",       color.useSystemColor );
  config.writeEntry( "TextBg",               color.textBg );
  config.writeEntry( "SecondTextBg",         color.secondTextBg );
  config.writeEntry( "OffsetBg",             color.offsetBg );
  config.writeEntry( "InactiveBg",           color.inactiveBg );
  config.writeEntry( "PrimaryFg1",           color.primaryFg[0] );
  config.writeEntry( "PrimaryFg2",           color.primaryFg[1] );
  config.writeEntry( "OffsetFg",             color.offsetFg );
  config.writeEntry( "SecondaryFg",          color.secondaryFg );
  config.writeEntry( "MarkBg",               color.markBg );
  config.writeEntry( "MarkFg",               color.markFg );
  config.writeEntry( "LeftSeparatorFg",      color.leftSeparatorFg );
  config.writeEntry( "RightSeparatorFg",     color.leftSeparatorFg );
  config.writeEntry( "NonPrintFg",           color.nonPrintFg );
  config.writeEntry( "CursorBg",             color.cursorBg );
  config.writeEntry( "CursorFg",             color.cursorFg );
  config.writeEntry( "GridFg",               color.gridFg );
  config.writeEntry( "BookmarkBg",           color.bookmarkBg );
  config.writeEntry( "BookmarkFg",           color.bookmarkFg );

  SDisplayFont &font = mDisplayState.font;
  config.setGroup( "Display Font" );
  config.writeEntry( "UseSystemFont",        font.useSystemFont );
  config.writeEntry( "LocalFont",            font.localFont );
  config.writeEntry( "NonPrintChar",         font.nonPrintChar.unicode() );

  SDisplayCursor &cursor = mDisplayState.cursor;
  config.setGroup( "Display Cursor" );
  config.writeEntry( "NeverBlink",           cursor.alwaysVisible );
  config.writeEntry( "AlwaysBlockShape",     cursor.alwaysBlockShape );
  config.writeEntry( "ThickInsertShape",     cursor.thickInsertShape );
  config.writeEntry( "FocusMode",            cursor.focusModeString() );
  config.writeEntry( "Interval",             cursor.interval );

  SDisplayMisc &misc = mDisplayState.misc;
  config.setGroup( "Display Misc" );
  config.writeEntry( "UndoLevel",            misc.undoLevel );
  config.writeEntry( "OpenFile",             misc.openFileString() );
  config.writeEntry( "InputSound",           misc.inputSound );
  config.writeEntry( "FatalSound",           misc.fatalSound );
  config.writeEntry( "AutoCopyToClipboard",  misc.autoCopyToClipboard );
  config.writeEntry( "InsertMode",           misc.insertMode );
  config.writeEntry( "WriteProtect",         misc.writeProtect );
  config.writeEntry( "ConfirmWrap",          misc.confirmWrap );
  config.writeEntry( "CursorJump",           misc.cursorJump );
  config.writeEntry( "MakeBackup",           misc.makeBackup );
  config.writeEntry( "ConfirmThreshold",     misc.confirmThreshold );
  config.writeEntry( "ThresholdValue",       misc.thresholdValue );
  config.writeEntry( "DiscardRecent",        misc.discardRecent );
  config.writeEntry( "GotoOnStartup",        misc.gotoOnStartup );
  config.writeEntry( "GotoOnReload",         misc.gotoOnReload );
  config.writeEntry( "ShowBookmarkInOffsetColumn", misc.bookmarkOffsetColumn );
  config.writeEntry( "ShowBookmarkInEditor", misc.bookmarkEditor );

  if( mExportDialog != 0 )
  {
    mExportDialog->writeConfiguration();
  }
}

void CHexEditorWidget::readConfiguration( KConfig &config )
{
  SDisplayLayout &layout = mDisplayState.layout;
  config.setGroup( "Display Options" );
  layout.setPrimaryMode( config.readEntry("PrimaryMode") );
  layout.setSecondaryMode( config.readEntry("SecondaryMode") );
  layout.setOffsetMode( config.readEntry("OffsetMode") );
  layout.offsetVisible = config.readBoolEntry(
    "OffsetVisible", layout.offsetVisible );
  layout.primaryUpperCase = config.readBoolEntry(
    "PrimaryUpperCase", layout.primaryUpperCase );
  layout.offsetUpperCase = config.readBoolEntry(
    "OffsetUpperCase", layout.offsetUpperCase );
  layout.lineSize = config.readNumEntry(
    "LineSize", layout.lineSize );
  layout.columnSize = config.readNumEntry(
    "ColumnSize", layout.columnSize );
  layout.lockLine = config.readBoolEntry(
    "LockLine", layout.lockLine );
  layout.lockColumn = config.readBoolEntry(
    "LockColumn", layout.lockColumn );
  layout.columnCharSpace = config.readBoolEntry(
    "ColumnCharSpace", layout.columnCharSpace );
  layout.columnSpacing = config.readNumEntry(
    "ColumnSpacing", layout.columnSpacing );
  layout.separatorMarginWidth = config.readNumEntry(
    "SeparatorMarginWidth", layout.separatorMarginWidth );
  layout.edgeMarginWidth = config.readNumEntry(
    "EdgeMarginWidth", layout.edgeMarginWidth );
  layout.leftSeparatorWidth = config.readNumEntry(
    "LeftSeparatorWidth", layout.leftSeparatorWidth );
  layout.rightSeparatorWidth = config.readNumEntry(
    "RightSeparatorWidth", layout.rightSeparatorWidth );
  layout.setGridMode( config.readEntry("GridMode") );

  SDisplayLine &line = mDisplayState.line;
  config.setGroup( "Line Size" );
  line.lineSize[SDisplayLine::hexadecimal] = config.readUnsignedNumEntry(
    "Hexadecimal", line.lineSize[SDisplayLine::hexadecimal] );
  line.lineSize[SDisplayLine::decimal] = config.readUnsignedNumEntry(
    "Decimal", line.lineSize[SDisplayLine::decimal] );
  line.lineSize[SDisplayLine::octal] = config.readUnsignedNumEntry(
    "Octal", line.lineSize[SDisplayLine::octal] );
  line.lineSize[SDisplayLine::binary] = config.readUnsignedNumEntry(
    "Binary", line.lineSize[SDisplayLine::binary] );
  line.lineSize[SDisplayLine::textOnly] = config.readUnsignedNumEntry(
    "TextOnly", line.lineSize[SDisplayLine::textOnly] );
  config.setGroup( "Column Size" );
  line.columnSize[SDisplayLine::hexadecimal] = config.readUnsignedNumEntry(
    "Hexadecimal", line.columnSize[SDisplayLine::hexadecimal] );
  line.columnSize[SDisplayLine::decimal] = config.readUnsignedNumEntry(
    "Decimal", line.columnSize[SDisplayLine::decimal] );
  line.columnSize[SDisplayLine::octal] = config.readUnsignedNumEntry(
    "Octal", line.columnSize[SDisplayLine::octal] );
  line.columnSize[SDisplayLine::binary] = config.readUnsignedNumEntry(
    "Binary", line.columnSize[SDisplayLine::binary] );
  line.columnSize[SDisplayLine::textOnly] = config.readUnsignedNumEntry(
    "TextOnly", line.columnSize[SDisplayLine::textOnly] );

  SDisplayFont &font = mDisplayState.font;
  config.setGroup( "Display Font" );
  font.useSystemFont = config.readBoolEntry(
    "UseSystemFont", true );
  font.localFont = config.readFontEntry(
    "LocalFont", &font.localFont );
  font.nonPrintChar = config.readNumEntry(
    "NonPrintChar", font.nonPrintChar );

  SDisplayColor &color = mDisplayState.color;
  config.setGroup( "Display Colors" );
  color.useSystemColor = config.readBoolEntry(
    "UseSystemColor", color.useSystemColor );
  color.textBg = config.readColorEntry(
    "TextBg", &color.textBg );
  color.secondTextBg = config.readColorEntry(
    "SecondTextBg", &color.secondTextBg );
  color.offsetBg = config.readColorEntry(
    "OffsetBg", &color.offsetBg );
  color.inactiveBg = config.readColorEntry(
    "InactiveBg", &color.inactiveBg );
  color.primaryFg[0] = config.readColorEntry(
    "PrimaryFg1", &color.primaryFg[0] );
  color.primaryFg[1] = config.readColorEntry(
    "PrimaryFg2", &color.primaryFg[1] );
  color.offsetFg = config.readColorEntry(
    "OffsetFg", &color.offsetFg );
  color.secondaryFg = config.readColorEntry(
    "SecondaryFg", &color.secondaryFg );
  color.markBg = config.readColorEntry(
    "MarkBg", &color.markBg );
  color.markFg = config.readColorEntry(
    "MarkFg", &color.markFg );
  color.cursorBg = config.readColorEntry(
    "CursorBg", &color.cursorBg );
  color.cursorFg = config.readColorEntry(
    "CursorFg", &color.cursorFg );
  color.leftSeparatorFg = config.readColorEntry(
    "LeftSeparatorFg", &color.leftSeparatorFg );
  color.rightSeparatorFg = config.readColorEntry(
    "RightSeparatorFg", &color.rightSeparatorFg );
  color.nonPrintFg = config.readColorEntry(
    "NonPrintFg", &color.nonPrintFg );
  color.gridFg = config.readColorEntry(
    "GridFg", &color.gridFg );
  color.bookmarkBg = config.readColorEntry(
    "BookmarkBg", &color.bookmarkBg );
  color.bookmarkFg = config.readColorEntry(
    "BookmarkFg", &color.bookmarkFg );

  SDisplayCursor &cursor = mDisplayState.cursor;
  config.setGroup( "Display Cursor" );
  cursor.alwaysVisible = config.readBoolEntry(
    "NeverBlink", cursor.alwaysVisible );
  cursor.alwaysBlockShape = config.readBoolEntry(
    "AlwaysBlockShape", cursor.alwaysBlockShape );
  cursor.thickInsertShape = config.readBoolEntry(
    "ThickInsertShape", cursor.thickInsertShape );
  cursor.setFocusMode( config.readEntry("FocusMode") );
  cursor.interval = config.readNumEntry(
    "Interval", cursor.interval );

  SDisplayMisc &misc = mDisplayState.misc;
  config.setGroup( "Display Misc" );
  misc.undoLevel = config.readNumEntry(
    "UndoLevel", misc.undoLevel );
  misc.setOpenFile( config.readEntry("OpenFile") );
  misc.inputSound = config.readBoolEntry(
    "InputSound", misc.inputSound );
  misc.fatalSound = config.readBoolEntry(
    "FatalSound", misc.fatalSound );
  misc.autoCopyToClipboard = config.readBoolEntry(
    "AutoCopyToClipboard", misc.autoCopyToClipboard );
  misc.insertMode = config.readBoolEntry(
    "InsertMode", misc.insertMode );
  misc.writeProtect = config.readBoolEntry(
    "WriteProtect", misc.writeProtect );
  misc.confirmWrap = config.readBoolEntry(
    "ConfirmWrap", misc.confirmWrap );
  misc.cursorJump = config.readBoolEntry(
    "CursorJump", misc.cursorJump );
  misc.makeBackup = config.readBoolEntry(
    "MakeBackup", misc.makeBackup );
  misc.confirmThreshold = config.readBoolEntry(
    "ConfirmThreshold", misc.confirmThreshold );
  misc.thresholdValue = config.readNumEntry(
    "ThresholdValue", misc.thresholdValue );
  misc.discardRecent = config.readBoolEntry(
    "DiscardRecent", misc.discardRecent );
  misc.gotoOnStartup = config.readBoolEntry(
    "GotoOnStartup", misc.gotoOnStartup );
  misc.gotoOnReload = config.readBoolEntry(
    "GotoOnReload", misc.gotoOnReload );
  misc.bookmarkOffsetColumn = config.readBoolEntry(
    "ShowBookmarkInOffsetColumn", misc.bookmarkOffsetColumn );
  misc.bookmarkEditor = config.readBoolEntry(
    "ShowBookmarkInEditor", misc.bookmarkEditor );
}



void CHexEditorWidget::resizeEvent( QResizeEvent *e )
{
  mHexView->resize( e->size().width(), e->size().height() );
}


void CHexEditorWidget::fontChanged( void )
{
  if( mDisplayState.font.useSystemFont == true )
  {
    setFont( mDisplayState.font );
  }
}

void CHexEditorWidget::paletteChanged( void )
{
  setColor( mDisplayState.color );
}

void CHexEditorWidget::layoutChanged( const SDisplayLayout &/*layout*/ )
{
  //mDisplayState.layout = layout;
}

void CHexEditorWidget::inputModeChanged( const SDisplayInputMode &input )
{
  mDisplayState.input = input;
}


void CHexEditorWidget::setLineSize(const SDisplayLine &line )
{
  mDisplayState.line = line;
}

void CHexEditorWidget::setLayout( const SDisplayLayout &layout )
{
  //
  // We only set the values that can be modified by the dialog
  //
  mDisplayState.layout.lockLine = layout.lockLine;
  mDisplayState.layout.lockColumn = layout.lockColumn;
  mDisplayState.layout.leftSeparatorWidth = layout.leftSeparatorWidth;
  mDisplayState.layout.rightSeparatorWidth = layout.rightSeparatorWidth;
  mDisplayState.layout.separatorMarginWidth = layout.separatorMarginWidth;
  mDisplayState.layout.edgeMarginWidth = layout.edgeMarginWidth;
  mDisplayState.layout.columnCharSpace = layout.columnCharSpace;
  mDisplayState.layout.columnSpacing = layout.columnSpacing;
  mDisplayState.layout.horzGridWidth = layout.horzGridWidth;
  mDisplayState.layout.vertGridWidth = layout.vertGridWidth;

  //
  // Select the line and column sizes we shall use now.
  //
  SDisplayLayout &l = mDisplayState.layout;
  l.lineSize   = mDisplayState.line.lineSize[ l.primaryMode ];
  l.columnSize = mDisplayState.line.columnSize[ l.primaryMode ];

  mHexView->setLayout( mDisplayState.layout );
}

void CHexEditorWidget::setCursor( const SDisplayCursor &cursor )
{
  mDisplayState.cursor = cursor;
  mHexView->setCursor( mDisplayState.cursor, true );
}

void CHexEditorWidget::setColor( const SDisplayColor &color )
{
  mDisplayState.color = color;

  //
  // The selection colors can not be chosen.
  //
  mDisplayState.color.selectBg = kapp->palette().active().highlight();
  mDisplayState.color.selectFg = kapp->palette().active().highlightedText();

  if( mDisplayState.color.useSystemColor == true )
  {
    SDisplayColor c = mDisplayState.color;
    c.textBg           = kapp->palette().active().base();
    c.secondTextBg     = kapp->palette().active().base();
    c.offsetBg         = kapp->palette().active().base();
    c.inactiveBg       = kapp->palette().active().base();
    c.primaryFg[0]     = kapp->palette().active().text();
    c.primaryFg[1]     = kapp->palette().active().text();
    c.nonPrintFg       = kapp->palette().active().text();
    c.offsetFg         = kapp->palette().active().text();
    c.secondaryFg      = kapp->palette().active().text();
    c.leftSeparatorFg  = kapp->palette().active().text();
    c.rightSeparatorFg = kapp->palette().active().text();
    c.cursorBg         = kapp->palette().active().text();
    c.cursorFg         = kapp->palette().active().base();
    c.gridFg           = kapp->palette().active().text();
    SDisplayColor defaultColor;
    c.bookmarkBg       = defaultColor.bookmarkBg;
    c.bookmarkFg       = defaultColor.bookmarkFg;

    mHexView->setColor( c, true );
  }
  else
  {
    mHexView->setColor( mDisplayState.color, true );
  }
}


void CHexEditorWidget::setFont( const SDisplayFont &font )
{
  mDisplayState.font = font;

  SDisplayFontInfo fontInfo;
  if( mDisplayState.font.useSystemFont == true )
  {
    fontInfo.font = KGlobalSettings::fixedFont();
  }
  else
  {
    fontInfo.font = mDisplayState.font.localFont;
  }
  fontInfo.nonPrintChar = mDisplayState.font.nonPrintChar;
  mHexView->setFont( fontInfo, true );
}

void CHexEditorWidget::setMisc( const SDisplayMisc &misc )
{
  mDisplayState.misc = misc;
  mHexView->setMisc( mDisplayState.misc );
}



void CHexEditorWidget::setHexadecimalMode( void )
{
  layout().primaryMode   = SDisplayLayout::hexadecimal;
  layout().lineSize      = line().lineSize[ SDisplayLine::hexadecimal ];
  layout().columnSize    = line().columnSize[ SDisplayLine::hexadecimal ];
  mHexView->setLayout( layout() );
}


void CHexEditorWidget::setDecimalMode( void )
{
  layout().primaryMode   = SDisplayLayout::decimal;
  layout().lineSize      = line().lineSize[ SDisplayLine::decimal ];
  layout().columnSize    = line().columnSize[ SDisplayLine::decimal ];
  mHexView->setLayout( layout() );
}


void CHexEditorWidget::setOctalMode( void )
{
  layout().primaryMode   = SDisplayLayout::octal;
  layout().lineSize      = line().lineSize[ SDisplayLine::octal ];
  layout().columnSize    = line().columnSize[ SDisplayLine::octal ];
  mHexView->setLayout( layout() );
}


void CHexEditorWidget::setBinaryMode( void )
{
  layout().primaryMode   = SDisplayLayout::binary;
  layout().lineSize      = line().lineSize[ SDisplayLine::binary ];
  layout().columnSize    = line().columnSize[ SDisplayLine::binary ];
  mHexView->setLayout( layout() );
}


void CHexEditorWidget::setTextMode( void )
{
  layout().primaryMode   = SDisplayLayout::textOnly;
  layout().lineSize      = line().lineSize[ SDisplayLine::textOnly ];
  layout().columnSize    = line().columnSize[ SDisplayLine::textOnly ];
  mHexView->setLayout( layout() );
}



void CHexEditorWidget::saveWorkingDirectory( const QString &url )
{
  if( url.isEmpty() == true )
  {
    return;
  }

  int index = url.findRev( '/' );
  if( index != -1 )
  {
    mWorkDir = url.left( index+1 );
  }
}



void CHexEditorWidget::newFile( void )
{
  if( busy( true ) == true )
  {
    return;
  }

  QString url = i18n("Untitled %1").arg( mUntitledCount );

  //
  // If the url is already present in the document list (should not happen),
  // then this document is "raised".
  //
  bool success = selectDocument( url, false );
  if( success == true )
  {
    return;
  }

  //
  // Prepare a new buffer we can load the document into
  //
  success = createBuffer();
  if( success == false )
  {
    return;
  }

  int errCode = mHexView->newFile( url );
  if( errCode != Err_Success )
  {
    QString msg = i18n("Unable to create new document.");
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Operation Failed") );
    return;
  }

  mUntitledCount += 1;
}




void CHexEditorWidget::newFile( const QByteArray &data )
{
  newFile();
  mHexView->append( data );
}




void CHexEditorWidget::open()
{
  KURL file = KFileDialog::getOpenURL( mWorkDir, "*" ,topLevelWidget() );

  if( file.isEmpty() )
    return;

  QString url = file.url();

  saveWorkingDirectory( url );

  if( busy( true ) )
    return;

  //
  // If the url is already present in the document list, then this
  // document is "raised".
  //
  if( selectDocument( url, true ) )
    return;

  //
  // Prepare a new buffer we can load the document into
  //
  if( !createBuffer() )
    return;

  int errCode = readURL( file, false );
  if( errCode != Err_Success )
  {
    emit errorLoadFile( url );
    removeBuffer();
  }
  else if( errCode == Err_Success )
  {
    defaultWriteProtection();
  }
}



void CHexEditorWidget::open( const QString &url, bool reloadWhenChanged,
			     uint offset )
{
  if( busy( true ) == true )
  {
    return;
  }

  //
  // If the url is already present in the document list, then this
  // document is "raised".
  //
  bool success = selectDocument( url, reloadWhenChanged );
  if( success == true )
  {
    return;
  }

  //
  // Prepare a new buffer we can load the document into
  //
  success = createBuffer();
  if( success == false )
  {
    return;
  }

  int errCode = readURL( url, false );
  if( errCode != Err_Success )
  {
    emit errorLoadFile( url );
    removeBuffer();
  }
  else if( errCode == Err_Success )
  {
    mHexView->gotoOffset( offset );
    defaultWriteProtection();
  }
}


void CHexEditorWidget::stepFile( bool next )
{
  //
  // The buffer list is is reverse when compared with the visible
  // document list in the menu so I toggle the flag.
  //
  next = next == true ? false : true;

  CHexBuffer *hexBuffer = documentItem( mHexView->url(), next );
  if( hexBuffer == 0 )
  {
    return;
  }

  mHexView->setBuffer( hexBuffer );
}





bool CHexEditorWidget::isOpen( const QString &url, uint &offset )
{
  CHexBuffer *hexBuffer = documentItem( url );
  if( hexBuffer == 0 )
  {
    return( false );
  }
  offset = hexBuffer->cursorOffset();
  return( true );
}


bool CHexEditorWidget::selectDocument( const QString &url,
				       bool reloadWhenChanged )
{
  CHexBuffer *hexBuffer = documentItem( url );
  if( hexBuffer == 0 )
  {
    return( false );
  }

  mHexView->setBuffer( hexBuffer );

  if( reloadWhenChanged == true && mHexView->urlValid() == true )
  {
    if( modifiedByAlien( mHexView->url() ) == true )
    {
      reload();
    }
  }

  return( true );
}


void CHexEditorWidget::insertFile( void )
{
  KFileDialog fdlg(mWorkDir, QString::null, topLevelWidget(), 0, TRUE);
  fdlg.setOperationMode( KFileDialog::Opening );
  fdlg.okButton()->setGuiItem( KStdGuiItem::insert() );
  fdlg.setCaption(i18n("Insert File"));

  if (!fdlg.exec()) return;
  KURL file = fdlg.selectedURL();

  if( file.isEmpty() )
    return;

  if( !file.isLocalFile() )
  {
    KMessageBox::sorry( this, i18n( "Only local files are currently supported." ) );
    return;
  }

  QString url = file.path();

  saveWorkingDirectory( url );

  if( busy( true ) )
    return;

  readURL( url, true );
}



void CHexEditorWidget::stop( void )
{
  mProgressStop = mProgressBusy;
}


bool CHexEditorWidget::close( void )
{
  if( busy( true ) == true )
  {
    return( false );
  }

  if( querySave() == false )
  {
    return( false );
  }

  removeBuffer();
  return( true );
}




bool CHexEditorWidget::closeAll( void )
{
  if( busy( true ) == true )
  {
    return( false );
  }

  while( mDocumentList.count() > 0 )
  {
    CHexBuffer *buf;
    for( buf = mDocumentList.first(); buf != 0; buf = mDocumentList.next() )
    {
      //
      // We select the buffer we are about to close because it will
      // the become visible, and because 'close()' works on the current
      // buffer.
      //
      mHexView->setBuffer( buf );
      if( close() == false )
      {
	return( false );
      }
    }

    //
    // The document list will always contain at least one element because
    // the hexview widget needs a hexbuffer to work. If this hexbuffer is
    // not valid, then all files have been closed.
    //
    if( mDocumentList.count() == 1 && mHexView->documentPresent() == false )
    {
      break;
    }
  }

  return( true );
}



bool CHexEditorWidget::querySave( void )
{
  if( mHexView->modified() == false )
  {
    return( true );
  }

  QString msg = i18n(""
    "The current document has been modified.\n"
    "Do you want to save it?" );
  int reply = KMessageBox::warningYesNoCancel( topLevelWidget(), msg, QString::null, KStdGuiItem::save(), KStdGuiItem::discard() );
  if( reply == KMessageBox::Yes )
  {
    return( save() );
  }
  else if( reply == KMessageBox::No )
  {
    return( true );
  }
  else
  {
    return( false );
  }
}


bool CHexEditorWidget::backup( void )
{
  if( mHexView->documentPresent() == false || mHexView->urlValid() == false )
  {
    return( false );
  }

  KURL kurl( mHexView->url() );
  if( kurl.isLocalFile() == false )
  {
    return( false );
  }

  const QString currentName = kurl.path();
  QString backupName = currentName + '~';

  int errCode = rename( currentName.latin1(), backupName.latin1() );
  if( errCode != 0 )
  {
    return( false );
  }


  return( true );
}



//
// Returns false if operation was canceled
//
bool CHexEditorWidget::save( void )
{
  if( mHexView->documentPresent() == false )
  {
    return( true );
  }

  if( mHexView->urlValid() == false )
  {
    return( saveAs() );
  }
  else
  {
    if( modifiedByAlien( mHexView->url() ) == true )
    {
      QString msg = i18n(""
       "Current document has been changed on disk.\n"
       "If you save now, those changes will be lost.\n"
       "Proceed?" );
      int reply = KMessageBox::warningYesNoCancel( topLevelWidget(), msg,
						   i18n("Save"), KStdGuiItem::save(), KStdGuiItem::discard() );
      if( reply == KMessageBox::No || reply == KMessageBox::Cancel )
      {
	return( reply == KMessageBox::No ? true : false );
      }
    }

    if( mDisplayState.misc.makeBackup == true )
    {
      backup();
    }
    writeURL( mHexView->url() );
  }

  return( true );
}


//
// Returns false if operation was canceled
//
bool CHexEditorWidget::saveAs( void )
{
  if( mHexView->documentPresent() == false )
  {
    return( true );
  }

  KURL url;
  while( 1 )
  {
    url = KFileDialog::getSaveURL( mHexView->url(), "*", this );
    if( url.isEmpty() == true )
    {
      return( false );
    }
    else
    {
      saveWorkingDirectory( url.url() );
    }

    if( url.isLocalFile() )
    {
      QString name( url.path() );
      QFileInfo info( name );

      if( info.exists() )
      {
	QString msg = i18n(""
          "A document with this name already exists.\n"
          "Do you want to overwrite it?" );
	int reply = KMessageBox::warningContinueCancel( topLevelWidget(), msg,
					       i18n("Save As"), i18n("Overwrite") );
	if( reply == KMessageBox::Continue )
	  break;
      }
      else
	break;
    }
  }

  QString symbolicName( url.url() );
//  KURL::decode( symbolicName );

  mHexView->setUrl( symbolicName );
  writeURL( mHexView->url() );
  return( true );
}



void CHexEditorWidget::reload( void )
{
  if( mHexView->documentPresent() == false )
  {
    return;
  }

  if( busy( true ) == true )
  {
    return;
  }

  if( mHexView->urlValid() == false )
  {
    QString msg = i18n( "The current document does not exist on the disk." );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Reload") );
    return;
  }

  if( mHexView->modified() == true )
  {
    QString msg;
    if( modifiedByAlien( mHexView->url() ) == true )
    {
      msg = i18n( "The current document has changed on the disk and "
                  "also contains unsaved modifications.\n"
                  "If you reload now, the modifications will be lost." );
    }
    else
    {
      msg = i18n( "The current document contains unsaved modifications.\n"
                   "If you reload now, the modifications will be lost." );
    }

    int reply = KMessageBox::warningContinueCancel(topLevelWidget(),msg,i18n("Reload"), i18n("&Reload"));
    if( reply != KMessageBox::Continue )
    {
      return;
    }
  }

  QString url( mHexView->url() );
  uint offset = mHexView->offset();

  //mHexView->closeFile();
  int errCode = readURL( url, false );
  if( errCode == Err_Success && gotoReloadOffset() == true )
  {
    mHexView->gotoOffset( offset );
  }

}


void CHexEditorWidget::print( void )
{
  CHexPrinter prt;

  prt.addDialogPage( new LayoutDialogPage() );

  prt.setPageSelection( KPrinter::SystemSide );
  prt.setFullPage( true ); // I use my own marings

  // FIXME: Make a better header for the printingdialog
  if (prt.setup(topLevelWidget(), i18n("Print Hex-Document")))
  {
     prt.setTopMarginMM( prt.option("kde-khexedit-topmarginmm").toInt() );
     prt.setBottomMarginMM( prt.option("kde-khexedit-bottommarginmm").toInt() );
     prt.setLeftMarginMM( prt.option("kde-khexedit-leftmarginmm").toInt() );
     prt.setRightMarginMM( prt.option("kde-khexedit-rightmarginmm").toInt() );

     prt.setPageHeader( (prt.option("kde-khexedit-headercheck") == "true"),
			  prt.option("kde-khexedit-headercombo0").toInt(),
			  prt.option("kde-khexedit-headercombo1").toInt(),
			  prt.option("kde-khexedit-headercombo2").toInt(),
			  prt.option("kde-khexedit-headercombo3").toInt() );

     prt.setPageFooter( (prt.option("kde-khexedit-footercheck") == "true"),
			  prt.option("kde-khexedit-footercombo0").toInt(),
			  prt.option("kde-khexedit-footercombo1").toInt(),
			  prt.option("kde-khexedit-footercombo2").toInt(),
			  prt.option("kde-khexedit-footercombo3").toInt() );

     printPostscript(prt);
  }

}




void CHexEditorWidget::printPostscript( CHexPrinter &printer )
{
  if( confirmPrintPageNumber( printer ) == false )
  {
    return;
  }

  //
  // No i18n() on this one!
  // Constants come from config.h
  //
  QString creator( PACKAGE );
  creator += " ";
  creator += VERSION;

  printer.setCreator( creator );

  int errCode = prepareProgressData( pg_print );
  if( errCode == Err_Success )
  {
    errCode = mHexView->print( printer, mProgressData );
  }

  QString msg = i18n("Could not print data.\n");
  msg += hexError( errCode );

  if( errCode != Err_Success )
  {
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Print") );
  }
}


bool CHexEditorWidget::confirmPrintPageNumber( CHexPrinter &printer )
{
  if( mDisplayState.misc.confirmThreshold == true )
  {
    uint numPage = mHexView->numPage( printer );
    printer.setMinMax( 1, numPage );
    uint numPageSelected = printer.pageList().count();

    if( numPageSelected > mDisplayState.misc.thresholdValue )
    {
      QString msg = i18n(
        "<qt>Print threshold exceeded.<br>"
        "You are about to print one page.<br>"
	"Proceed?</qt>",
        "<qt>Print threshold exceeded.<br>"
        "You are about to print %n pages.<br>"
	"Proceed?</qt>",
	numPageSelected );
      int reply = KMessageBox::warningYesNo( topLevelWidget(), msg,
					     i18n("Print"), KStdGuiItem::print(), KStdGuiItem::cancel() );
      if( reply != KMessageBox::Continue )
      {
	return( false );
      }
    }
  }

  return( true );
}


void CHexEditorWidget::exportDialog( void )
{
  if( mExportDialog == 0 )
  {
    mExportDialog = new CExportDialog( topLevelWidget(), 0, false );
    if( mExportDialog == 0 ) { return; }
    connect( mExportDialog, SIGNAL( exportText(const SExportText &)),
	     this, SLOT( exportText( const SExportText &)) );
    connect( mExportDialog, SIGNAL( exportHtml(const SExportHtml &)),
	     this, SLOT( exportHtml( const SExportHtml &)) );
    connect( mExportDialog, SIGNAL( exportCArray(const SExportCArray &)),
	     this, SLOT( exportCArray( const SExportCArray &)) );
  }
  mExportDialog->show();
}



void CHexEditorWidget::exportText( const SExportText &ex )
{
  int errCode = prepareProgressData( pg_export );
  if( errCode == Err_Success )
  {
    errCode = mHexView->exportText( ex, mProgressData );
  }

  if( errCode != Err_Success )
  {
    QString msg = i18n("Unable to export data.\n");
    msg += hexError( errCode );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Export") );
  }
}


void CHexEditorWidget::exportHtml( const SExportHtml &ex )
{
  int errCode = prepareProgressData( pg_export );
  if( errCode == Err_Success )
  {
    errCode = mHexView->exportHtml( ex, mProgressData );
  }

  if( errCode != Err_Success )
  {
    QString msg = i18n("Unable to export data.\n");
    msg += hexError( errCode );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Export") );
  }
}


void CHexEditorWidget::exportCArray( const SExportCArray &ex )
{
  int errCode = prepareProgressData( pg_export );
  if( errCode == Err_Success )
  {
    errCode = mHexView->exportCArray( ex, mProgressData );
  }

  if( errCode != Err_Success )
  {
    QString msg = i18n("Unable to export data.\n");
    msg += hexError( errCode );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Export") );
  }
}


void CHexEditorWidget::encode( CConversion::EMode mode )
{
  if( mHexView->losslessEncoding( mode ) == false )
  {
    QString msg = i18n(""
      "The encoding you have selected is not reversible.\n"
      "If you revert to the original encoding later, there is no "
      "guarantee that the data can be restored to the original state.");
    int reply = KMessageBox::warningContinueCancel( topLevelWidget(), msg,
					   i18n("Encode"), i18n("&Encode"));
    if( reply != KMessageBox::Continue )
    {
      // Restore correct menu setting
      mHexView->reportEncoding();
      return;
    }
  }

  int errCode = prepareProgressData( pg_encode );
  if( errCode == Err_Success )
  {
    errCode = mHexView->setEncoding( mode, mProgressData );
  }

  if( errCode != Err_Success )
  {
    QString msg = i18n("Could not encode data.\n");
    msg += hexError( errCode );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Encode") );
  }
}

void CHexEditorWidget::undo( void )
{
  mHexView->undo();
}

void CHexEditorWidget::redo( void )
{
  mHexView->redo();
}

void CHexEditorWidget::toggleWriteProtection( void )
{
  mDisplayState.input.readOnly = mDisplayState.input.readOnly == true ?
    false : true;
  mHexView->setInputMode( mDisplayState.input );
}

void CHexEditorWidget::defaultWriteProtection( void )
{
  mDisplayState.input.readOnly = mDisplayState.misc.writeProtect;
  mHexView->setInputMode( mDisplayState.input );
}

void CHexEditorWidget::toggleResizeLock( void )
{
  mDisplayState.input.allowResize = mDisplayState.input.allowResize == true ?
    false : true;
  mHexView->setInputMode( mDisplayState.input );
}

void CHexEditorWidget::setResizeLock( bool state )
{
  mDisplayState.input.allowResize = state;
  mHexView->setInputMode( mDisplayState.input );
}

void CHexEditorWidget::enableInputLock( bool inputLock )
{
  mDisplayState.input.inputLock = inputLock;
  mHexView->setInputMode( mDisplayState.input );
}

void CHexEditorWidget::toggleOffsetColumnVisibility( void )
{
  layout().offsetVisible = layout().offsetVisible == true ? false : true;
  mHexView->setLayout( layout() );
}

void CHexEditorWidget::toggleTextColumnVisibility( void )
{
  layout().secondaryMode = layout().secondaryMode == SDisplayLayout::hide ?
    SDisplayLayout::textOnly : SDisplayLayout::hide;
  mHexView->setLayout( layout() );
}

void CHexEditorWidget::toggleOffsetAsDecimal( void )
{
  layout().offsetMode = layout().offsetMode == SDisplayLayout::hexadecimal ?
    SDisplayLayout::decimal : SDisplayLayout::hexadecimal;
  mHexView->setLayout( layout() );
}

void CHexEditorWidget::toggleDataUppercase( void )
{
  layout().primaryUpperCase = layout().primaryUpperCase == true ? false : true;
  mHexView->setLayout( layout() );
}

void CHexEditorWidget::toggleOffsetUppercase( void )
{
  layout().offsetUpperCase = layout().offsetUpperCase == true ? false : true;
  mHexView->setLayout( layout() );
}

void CHexEditorWidget::toggleInsertMode( void )
{
  mHexView->setInsertMode( !mHexView->insertMode() );
}

void CHexEditorWidget::benchmark( void )
{
  mHexView->benchmark();
}

void CHexEditorWidget::copy( void )
{
  mHexView->copy();
}

void CHexEditorWidget::copyText( void )
{
  mHexView->copyText();
}

void CHexEditorWidget::paste( void )
{
  if( mHexView->documentPresent() == false )
  {
    pasteNewFile();
  }
  else
  {
    mHexView->paste();
  }
}

void CHexEditorWidget::pasteNewFile( void )
{
  newFile();
  mHexView->paste();
}

void CHexEditorWidget::cut( void )
{
  mHexView->cut();
}

void CHexEditorWidget::selectAll( void )
{
  mHexView->selectAll();
}

void CHexEditorWidget::unselect( void )
{
  mHexView->unselect();
}


void CHexEditorWidget::addBookmark( void )
{
  mHexView->addBookmark( -1 );
}


void CHexEditorWidget::removeBookmark( void )
{
  if( mHexView->bookmarkCount() > 0 )
  {
    mHexView->removeBookmark( false );
  }
}


void CHexEditorWidget::removeAllBookmark( void )
{
  if( mHexView->bookmarkCount() > 0 )
  {
    QString msg = i18n(""
      "Deleted bookmarks can not be restored.\n"
      "Proceed?" );
    int reply = KMessageBox::warningContinueCancel( topLevelWidget(), msg );
    if( reply != KMessageBox::Continue )
    {
      return;
    }
    mHexView->removeBookmark( true );
  }
}


void CHexEditorWidget::replaceBookmark( void )
{
  mHexView->replaceBookmark();
}


void CHexEditorWidget::gotoBookmark( int position )
{
  mHexView->gotoBookmark( (uint)position );
}

void CHexEditorWidget::gotoNextBookmark( void )
{
  mHexView->gotoNextBookmark(true);
}

void CHexEditorWidget::gotoPrevBookmark( void )
{
  mHexView->gotoNextBookmark(false);
}

void CHexEditorWidget::gotoOffset( void )
{
  if( mGotoDialog == 0 )
  {
    mGotoDialog = new CGotoDialog( topLevelWidget(), 0, false );
    if( mGotoDialog == 0 ) { return; }
    connect( mGotoDialog, SIGNAL(gotoOffset( uint, uint, bool, bool )),
	     mHexView, SLOT(gotoOffset( uint, uint, bool, bool )) );
  }
  mGotoDialog->show();
}


void CHexEditorWidget::find( void )
{
  if( mFindNavigatorDialog != 0 )
  {
    mFindNavigatorDialog->hide();
  }

  if( mFindDialog == 0 )
  {
    mFindDialog = new CFindDialog( topLevelWidget(), 0, false );
    if( mFindDialog == 0 ) { return; }
    connect( mFindDialog,
	     SIGNAL(findData(SSearchControl &, uint, bool)),
	     SLOT(findData(SSearchControl &, uint, bool)) );
  }
  mFindDialog->show();
}



void CHexEditorWidget::findData( SSearchControl &sc, uint mode, bool navigator)
{
  for( uint i=0; i < 2; i++ )
  {
    int errCode;
    if( mode == Find_First )
    {
      errCode = mHexView->findFirst( sc );
      if( errCode == Err_Success )
      {
	if( navigator == true )
	{
	  findNavigator( sc );
	}
	return;
      }
    }
    else if( mode == Find_Next )
    {
      errCode = mHexView->findNext( sc );
      if( errCode == Err_Success )
      {
	if( navigator == true )
	{
	  findNavigator( sc );
	}
	return;
      }
    }
    else
    {
      return;
    }

    if( errCode == Err_WrapBuffer && i == 0 )
    {
      bool reply = askWrap( sc.forward, i18n("Find") );
      if( reply == false )
      {
	return;
      }

      sc.fromCursor = false;
      mode = Find_First;
    }
  }

  if( mode == Find_First )
  {
    QString msg = i18n( "Search key not found in document." );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Find") );
  }
}



void CHexEditorWidget::findAgain( void )
{
  if( canFind( true ) == true )
  {
    mFindDialog->findAgain( CFindDialog::find_Again );
  }
}


void CHexEditorWidget::findNext( void )
{
  if( canFind( true ) == true )
  {
    mFindDialog->findAgain( CFindDialog::find_Next );
  }
}


void CHexEditorWidget::findPrevious( void )
{
  if( canFind( true ) == true )
  {
    mFindDialog->findAgain( CFindDialog::find_Previous );
  }
}


bool CHexEditorWidget::askWrap( bool fwd, const QString &header )
{
  if( mDisplayState.misc.confirmWrap == false )
  {
    return( true ); // Never ask the user
  }

  QString msg;
  if( fwd == true )
  {
    msg += i18n(""
      "End of document reached.\n"
      "Continue from the beginning?" );
  }
  else
  {
    msg += i18n(""
      "Beginning of document reached.\n"
      "Continue from the end?" );
  }

  int reply = KMessageBox::questionYesNo( topLevelWidget(), msg, header, KStdGuiItem::cont(), KStdGuiItem::cancel() );
  return( reply == KMessageBox::Yes ? true : false );
}


bool CHexEditorWidget::canFind( bool showError )
{
  if( mFindDialog == 0 || mFindDialog->isEmpty() == true )
  {
    if( showError == true )
    {
      QString msg = i18n(""
        "Your request can not be processed.\n"
        "No search pattern defined." );
      KMessageBox::sorry( topLevelWidget(), msg, i18n("Find") );
    }
    return( false );
  }

  return( true );
}



void CHexEditorWidget::findNavigator( SSearchControl &sc )
{
  if( canFind( false ) == false )
  {
    return;
  }

  if( mFindNavigatorDialog == 0 )
  {
    mFindNavigatorDialog = new CFindNavigatorDialog(topLevelWidget(),0,false);
    if( mFindNavigatorDialog == 0 ) { return; }
    connect( mFindNavigatorDialog,
	     SIGNAL(findData(SSearchControl &, uint, bool)),
	     SLOT(findData(SSearchControl &, uint, bool)) );
    connect( mFindNavigatorDialog, SIGNAL(makeKey(void)),
	     SLOT(find()) );
  }
  if( mFindNavigatorDialog->isVisible() == false )
  {
    mFindNavigatorDialog->defineData( sc );
  }
  mFindNavigatorDialog->show();
}



void CHexEditorWidget::replace( void )
{
  hideReplacePrompt();

  if( mReplaceDialog == 0 )
  {
    mReplaceDialog = new CReplaceDialog( topLevelWidget(), 0, false );
    if( mReplaceDialog == 0 ) { return; }
    connect( mReplaceDialog,
	     SIGNAL( replaceData( SSearchControl &, uint)),
	     SLOT( replaceData( SSearchControl &, uint)));
  }
  mReplaceDialog->show();
}



void CHexEditorWidget::replaceData( SSearchControl &sc, uint mode )
{
  while( 1 )
  {
    if( mode == Replace_All || mode == Replace_AllInit )
    {
      mHexView->replaceAll( sc, mode == Replace_AllInit ? true : false );
    }
    else if( mode == Replace_First )
    {
      int errCode = mHexView->findFirst( sc );
      if( errCode == Err_Success )
      {
	replacePrompt( sc );
	return;
      }
    }
    else if( mode == Replace_Next )
    {
      int errCode = mHexView->replaceMarked( sc );
      if( errCode != Err_Success )
      {
	// Perhaps a notification here ?
      }
      errCode = mHexView->findNext( sc );
      if( errCode == Err_Success )
      {
	replacePrompt( sc );
	return;
      }
    }
    else if( mode == Replace_Ignore )
    {
      int errCode = mHexView->findNext( sc );
      if( errCode == Err_Success )
      {
	replacePrompt( sc );
	return;
      }
    }
    else
    {
      break;
    }

    if( sc.wrapValid == false )
    {
      break;
    }

    bool reply = askWrap( sc.forward, i18n("Find and Replace") );
    if( reply == false )
    {
      break;
    }

    int errCode = mHexView->findWrap( sc );
    if( errCode != Err_Success )
    {
      break;
    }

    if( mode == Replace_All || mode == Replace_AllInit )
    {
      mode = Replace_All;
      continue;
    }

    replacePrompt( sc );
    return;
  }

  replaceResult( sc );
}

void CHexEditorWidget::replacePrompt( SSearchControl &sc )
{
  if( mReplacePromptDialog == 0 )
  {
    mReplacePromptDialog = new CReplacePromptDialog(topLevelWidget(), 0,false);
    if( mReplacePromptDialog == 0 ) { return; }
    connect( mReplacePromptDialog,
	     SIGNAL( replaceData( SSearchControl &, uint)),
	     SLOT( replaceData( SSearchControl &, uint)));
  }
  if( mReplacePromptDialog->isVisible() == false )
  {
    mReplacePromptDialog->defineData( sc );
  }
  mReplacePromptDialog->show();
}


void CHexEditorWidget::hideReplacePrompt( void )
{
  if( mReplacePromptDialog != 0 )
  {
    mReplacePromptDialog->hide();
  }
}



void CHexEditorWidget::replaceResult( SSearchControl &sc )
{
  hideReplacePrompt();

  if( sc.match == false )
  {
    QString msg;
    if( sc.inSelection == true )
    {
      msg += i18n( "Search key not found in selected area." );
    }
    else
    {
      msg += i18n( "Search key not found in document." );
    }
    KMessageBox::information( topLevelWidget(), msg, i18n("Find & Replace"));
  }
  else
  {
    const QString msg = i18n(
      "<qt>Operation complete.<br><br>One replacement was made.</qt>",
      "<qt>Operation complete.<br><br>%n replacements were made.</qt>", sc.numReplace );
    KMessageBox::information( topLevelWidget(), msg, i18n("Find & Replace"));
  }
}


void CHexEditorWidget::insertPattern( void )
{
  if( mInsertDialog == 0 )
  {
    mInsertDialog = new CInsertDialog( topLevelWidget(), 0, false );
    if( mInsertDialog == 0 ) { return; }
    connect( mInsertDialog, SIGNAL(execute( SInsertData & )),
	     mHexView, SLOT(insert( SInsertData & )) );
  }
  mInsertDialog->show();
}


void CHexEditorWidget::encoding( void )
{
  QString msg = i18n(""
    "Not available yet!\n"
    "Define your own encoding" );
  KMessageBox::sorry( topLevelWidget(), msg, i18n("Encoding") );
}


void CHexEditorWidget::strings( void )
{
  if( mStringDialog == 0 )
  {
    mStringDialog = new CStringDialog( topLevelWidget(), 0, false );
    if( mStringDialog == 0 ) { return; }
    connect( mStringDialog, SIGNAL(markText( uint, uint, bool )),
	     mHexView, SLOT(setMark( uint, uint, bool )) );
    connect( mStringDialog, SIGNAL(collect()), SLOT(collectStrings()) );
    connect( mHexView, SIGNAL(fileName( const QString &, bool ) ),
	     mStringDialog, SLOT( removeList() ) );
    connect( mHexView, SIGNAL(dataChanged()),
	     mStringDialog, SLOT(setDirty()) );
  }
  mStringDialog->show();
}


void CHexEditorWidget::collectStrings( void )
{
  int errCode = prepareProgressData( pg_strings );
  if( errCode == Err_Success )
  {
    mHexView->collectStrings( mStringDialog->stringData() );
    errCode = mStringDialog->updateList( mProgressData );
  }

  if( errCode != Err_Success && errCode != Err_Stop )
  {
    QString msg = i18n("Could not collect strings.\n");
    msg += hexError( errCode );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Collect Strings") );
  }

}



void CHexEditorWidget::recordView( void )
{
  QString msg = i18n(""
   "Not available yet!\n"
   "Define a record (structure) and fill it with data from the document." );
  KMessageBox::sorry( topLevelWidget(), msg, i18n("Record Viewer") );
}

void CHexEditorWidget::filter( void )
{
  if( mFilterDialog == 0 )
  {
    mFilterDialog = new CFilterDialog( topLevelWidget(), 0, false );
    if( mFilterDialog == 0 ) { return; }
    connect( mFilterDialog, SIGNAL(filterData( SFilterControl & )),
	     mHexView, SLOT(filter( SFilterControl & )) );
  }
  mFilterDialog->show();
}


void CHexEditorWidget::chart( void )
{
  if( mCharTableDialog == 0 )
  {
    mCharTableDialog = new CCharTableDialog( topLevelWidget(), 0, false );
    if( mCharTableDialog == 0 ) { return; }
    connect( mCharTableDialog, SIGNAL(assign( const QByteArray & )),
	     mHexView, SLOT(insert( const QByteArray & )) );
  }
  mCharTableDialog->show();
}


void CHexEditorWidget::converter( void )
{
  if( mConverterDialog == 0 )
  {
    mConverterDialog = new CConverterDialog( this, "converter", false );
    connect( mConverterDialog, SIGNAL(probeCursorValue(QByteArray &, uint)),
	     mHexView, SLOT(valueOnCursor(QByteArray &, uint)) );
  }
  mConverterDialog->show();
}


void CHexEditorWidget::statistics( void )
{
  if( mFileInfoDialog == 0 )
  {
    mFileInfoDialog = new CFileInfoDialog( topLevelWidget(), 0, false );
    if( mFileInfoDialog == 0 ) { return; }
    connect( mFileInfoDialog, SIGNAL(collectStatistic(SStatisticControl &)),
	     SLOT(collectStatistics(SStatisticControl &)));
    connect( mHexView, SIGNAL(dataChanged()),
	     mFileInfoDialog, SLOT(setDirty()) );
  }
  mFileInfoDialog->show();
}


void CHexEditorWidget::collectStatistics( SStatisticControl &sc )
{
  int errCode = prepareProgressData( pg_statistic );
  if( errCode == Err_Success )
  {
    errCode = mHexView->collectStatistic( sc, mProgressData );
    if( errCode == Err_Success )
    {
      mFileInfoDialog->setStatistics( sc );
    }
  }

  if( errCode != Err_Success && errCode != Err_Stop )
  {
    mFileInfoDialog->setStatistics(); // Default values

    QString msg = i18n("Could not collect document statistics.\n");
    msg += hexError( errCode );
    KMessageBox::sorry( topLevelWidget(), msg,
			i18n("Collect Document Statistics") );
  }

}


void CHexEditorWidget::options( void )
{
  if( mOptionDialog == 0 )
  {
    mOptionDialog = new COptionDialog( topLevelWidget(), 0, false );
    if( mOptionDialog == 0 ) { return; }

    connect( mOptionDialog, SIGNAL(lineSizeChoice(const SDisplayLine &)),
	     SLOT(setLineSize(const SDisplayLine &)) );
    connect( mOptionDialog, SIGNAL(layoutChoice(const SDisplayLayout &)),
	     SLOT(setLayout(const SDisplayLayout &)) );
    connect( mOptionDialog, SIGNAL(fontChoice(const SDisplayFont &)),
	     SLOT(setFont(const SDisplayFont &)) );
    connect( mOptionDialog, SIGNAL(colorChoice(const SDisplayColor &)),
	     SLOT(setColor(const SDisplayColor &)) );
    connect( mOptionDialog, SIGNAL(cursorChoice(const SDisplayCursor &)),
	     SLOT(setCursor(const SDisplayCursor &)) );
    connect( mOptionDialog, SIGNAL(miscChoice(const SDisplayMisc &)),
	     SLOT(setMisc(const SDisplayMisc &)) );
    connect( mOptionDialog, SIGNAL(removeRecentFiles()),
	     SIGNAL(removeRecentFiles()) );
  }
  if( mOptionDialog->isVisible() == false )
  {
    mOptionDialog->setState( mDisplayState );
  }
  mOptionDialog->show();
}



void CHexEditorWidget::favorites( void )
{
  QString msg = i18n(""
    "Not available yet!\n"
    "Save or retrive your favorite layout" );
  KMessageBox::sorry( topLevelWidget(), msg, i18n("Profiles") );
}


int CHexEditorWidget::readURL( const KURL &url, bool insert )
{
  //
  // 1) Make sure there is data in the url
  //
  if( url.isEmpty() )
    return( Err_EmptyArgument );

  //
  // 2) Verify that the url is valid URL string. If not, try to repair it.
  // This will work if the url contains a name of a file in the
  // current directory.
  //
  if( !url.isValid() )
  {
    QString msg = i18n("Malformed URL\n%1").arg( url.url() );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Read URL") );
    return( Err_IllegalArgument );
  }

  //
  // 3) Load the file.
  //
  QString tmpfile;
  if ( !KIO::NetAccess::download( url, tmpfile, this ) )
    return Err_ReadFailed;

  bool success = readFile( tmpfile, url.url(), insert );

  KIO::NetAccess::removeTempFile( tmpfile );

  return( success == true ? Err_Success : Err_ReadFailed );
}


void CHexEditorWidget::writeURL( QString &url )
{
  KURL kurl( url );
  if( kurl.isLocalFile() )
    writeFile( kurl.path() );
  else
  {
    bool modified = mHexView->modified();
    KTempFile tf;
    tf.setAutoDelete( true );
    writeFile( tf.name() );
    if( !KIO::NetAccess::upload(tf.name(),url,this) )
    {
      mHexView->setModified( modified );
      QString msg = i18n("Could not save remote file.");
      KMessageBox::sorry( topLevelWidget(), msg, i18n("Write Failure") );
    }
  }
}

bool CHexEditorWidget::modifiedByAlien( const QString &url )
{
  KURL kurl( url );
  if( kurl.isLocalFile() == false )
  {
    return( false );
  }

  QFileInfo fileInfo( kurl.path() );
  if( fileInfo.exists() == false )
  {
    return( false );
  }

  if( fileInfo.lastModified() == mHexView->diskModifyTime() )
  {
    return( false );
  }

  return( true );
}



bool CHexEditorWidget::readFile( const QString &diskPath, const QString &url,
				 bool insert )
{

  QFileInfo info( diskPath );
  if( info.exists() == false )
  {
    const QString msg = i18n("The specified file does not exist.\n%1").arg( diskPath );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Read") );
    return( false );
  }

  if( info.isDir() == true )
  {
    const QString msg = i18n("You have specified a folder.\n%1").arg( diskPath );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Read") );
    return( false );
  }

  if( info.isReadable() == false )
  {
    const QString msg = i18n("You do not have read permission to this file.\n%1").arg( diskPath );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Read") );
    return( false );
  }

  QFile file( diskPath );
  if( file.open( IO_ReadOnly | IO_Raw ) == false )
  {
    const QString msg = i18n("An error occurred while trying to open the file.\n%1").arg( diskPath );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Read") );
    return( false );
  }

  if( mHexView->documentPresent() == false )
  {
    //
    // Can not insert if there is no document present.
    //
    insert = false;
  }

  int errCode = prepareProgressData( insert == true ? pg_insert: pg_read );
  if( errCode == Err_Success )
  {
    if( insert == true )
    {
      errCode = mHexView->insertFile( file, mProgressData );
    }
    else
    {
      errCode = mHexView->readFile( file, url, mProgressData );
    }
  }

  if( errCode != Err_Success )
  {
    QString header = insert == true ? i18n("Insert") : i18n("Read");
    QString msg = i18n("Could not read file.\n");
    msg += hexError( errCode );
    KMessageBox::sorry( topLevelWidget(), msg, header );
  }

  file.close();
  return( errCode == Err_Success || errCode == Err_Busy ? true : false );
}




bool CHexEditorWidget::writeFile( const QString &diskPath )
{
  QFileInfo info( diskPath );
  if( info.exists() == true )
  {
    if( info.isDir() == true )
    {
      QString msg = i18n("You have specified a folder.");
      KMessageBox::sorry( topLevelWidget(), msg, i18n("Write Failure") );
      return( false );
    }

    if( info.isWritable() == false )
    {
      QString msg = i18n("You do not have write permission.");
      KMessageBox::sorry( topLevelWidget(), msg, i18n("Write Failure") );
      return( false );
    }
  }

  QFile file( diskPath );
  if( file.open( IO_WriteOnly | IO_Raw | IO_Truncate ) == false )
  {
    QString msg = i18n("An error occurred while trying to open the file.");
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Write Failure") );
    return( false );
  }

  int errCode = prepareProgressData( pg_write );
  if( errCode == Err_Success )
  {
    errCode = mHexView->writeFile( file, mProgressData );
  }
  if( errCode != Err_Success )
  {
    QString msg = i18n("Could not write data to disk.\n");
    msg += hexError( errCode );
    KMessageBox::sorry( topLevelWidget(), msg, i18n("Write Failure") );
  }

  file.close();
  return( true );
}


CHexBuffer *CHexEditorWidget::documentItem( const QString &url )
{
  QString symbolicName( url );
//  KURL::decode( symbolicName );

  for( CHexBuffer *hexBuffer = mDocumentList.first(); hexBuffer != 0;
       hexBuffer = mDocumentList.next() )
  {
    if( hexBuffer->url() == symbolicName )
    {
      return( hexBuffer );
    }
  }

  return( 0 );
}


CHexBuffer *CHexEditorWidget::documentItem( const QString &url, bool next )
{
  if( mDocumentList.count() <= 1 )
  {
    return( 0 );
  }

  QString symbolicName( url );
//  KURL::decode( symbolicName );

  if( next == true )
  {
    CHexBuffer *hexBuffer = mDocumentList.first();
    for( ; hexBuffer != 0; hexBuffer = mDocumentList.next() )
    {
      if( hexBuffer->url() == symbolicName )
      {
	hexBuffer = mDocumentList.next();
	return( hexBuffer == 0 ? mDocumentList.first() : hexBuffer );
      }
    }
  }
  else
  {
    CHexBuffer *hexBuffer = mDocumentList.last();
    for( ; hexBuffer != 0; hexBuffer = mDocumentList.prev() )
    {
      if( hexBuffer->url() == symbolicName )
      {
	hexBuffer = mDocumentList.prev();
	return( hexBuffer == 0 ? mDocumentList.last() : hexBuffer );
      }
    }
  }

  return( 0 );
}



bool CHexEditorWidget::createBuffer( void )
{
  if( mHexView->documentPresent() == false )
  {
    //
    // The document is not valid, i.e. the buffer contains no data
    // so we can use this one without destroying anything.
    //
    return( true );
  }

  CHexBuffer *hexBuffer = new CHexBuffer;
  if( hexBuffer == 0 )
  {
    QString msg = i18n( "Can not create text buffer.\n" );
    msg += hexError( Err_NoMemory );
    KMessageBox::error( topLevelWidget(), msg, i18n("Loading Failed" ) );
    return( false );
  }

  mDocumentList.append( hexBuffer );
  mHexView->setBuffer( hexBuffer );

  return( true );
}


void CHexEditorWidget::removeBuffer( void )
{
  mHexView->closeFile();

  if( mDocumentList.count() > 1 )
  {
    CHexBuffer *prev = 0;
    CHexBuffer *curr = mDocumentList.first();
    for( ; curr != 0; curr = mDocumentList.next() )
    {
      if( curr == mHexView->hexBuffer() )
      {
	CHexBuffer *ptr = prev != 0 ? prev : mDocumentList.next();
	if( ptr != 0 )
	{
	  mHexView->setBuffer( ptr );
	  mDocumentList.remove( curr );
	  break;
	}
      }
      prev = curr;
    }
  }

}


bool CHexEditorWidget::modified( void )
{
  for( CHexBuffer *hexBuffer = mDocumentList.first(); hexBuffer != 0;
       hexBuffer = mDocumentList.next() )
  {
    if( hexBuffer->modified() == true )
    {
      return( true );
    }
  }

  return( false );
}




int CHexEditorWidget::prepareProgressData( EProgressMode mode )
{
  if( mode >= pg_MAX )
  {
    return( Err_IllegalMode );
  }

  if( mProgressBusy == true )
  {
    return( Err_Busy );
  }


  mProgressMode = mode;
  mProgressBusy = true;
  mProgressStop = false;
  enableInputLock( true );

  static QString names[] =
  {
    i18n("Reading"),
    i18n("Writing"),
    i18n("Inserting"),
    i18n("Printing"),
    i18n("Encoding"),
    i18n("Collect strings"),
    i18n("Exporting"),
    i18n("Scanning"),
  };

  mProgressData.define( progressReceiver, this );
  emit setProgressText( QString(names[mode]) );
  emit operationChanged( true );

  return( Err_Success );
}


int CHexEditorWidget::progressReceiver( void *clientData, SProgressData &pd )
{
  if( clientData != 0 )
  {
    int errCode = ((CHexEditorWidget*)clientData)->progressParse( pd );
    return( errCode );
  }
  else
  {
    return( Err_Success );
  }
}


int CHexEditorWidget::progressParse( const SProgressData &pd )
{
  if( pd.valid() == 0 )
  {
    emit enableProgressText( false );
    emit setProgress( 0 );
    emit operationChanged( false );
    mProgressBusy = false;
    enableInputLock( false );
    kapp->processEvents();
    return( Err_Success );
  }
  else if( pd.useFraction == 1 )
  {
    emit enableProgressText( true );
    emit setProgress( (int)(100.0 * pd.fraction ) );
    kapp->processEvents();
  }
  else
  {
    emit enableProgressText( true );
    emit setProgress( pd.curPage, pd.maxPage );
    kapp->processEvents();
  }

  if( mProgressStop == false )
  {
    return( Err_Success );
  }

  QString msg, header;
  switch( mProgressMode )
  {
    case pg_read:
      header = i18n("Read");
      msg = i18n("Do you really want to cancel reading?");
    break;

    case pg_write:
      header = i18n("Write");
      msg  = i18n("Do you really want to cancel writing?\n"
                  "WARNING: Canceling can corrupt your data on disk");
    break;

    case pg_insert:
      header = i18n("Insert");
      msg = i18n("Do you really want to cancel inserting?");
    break;

    case pg_print:
      header = i18n("Print");
      msg = i18n("Do you really want to cancel printing?");
    break;

    case pg_encode:
      header = i18n("Encode");
      msg = i18n("Do you really want to cancel encoding?");
    break;

    case pg_strings:
      header = i18n("Collect strings");
      msg = i18n("Do you really want to cancel string scanning?");
    break;

    case pg_export:
      header = i18n("Export");
      msg = i18n("Do you really want to cancel exporting?");
    break;

    case pg_statistic:
      header = i18n("Collect document statistics");
      msg = i18n("Do you really want to cancel document scanning?");
    break;

    default:
      return( Err_Success );
    break;

  }

  int reply = KMessageBox::warningYesNo( topLevelWidget(), msg, header, KStdGuiItem::cancel(), KStdGuiItem::cont() );
  mProgressStop = false;
  return( reply == KMessageBox::Yes ? Err_Stop : Err_Success );
}



bool CHexEditorWidget::busy( bool showWarning )
{
  if( mProgressBusy == true && showWarning == true )
  {
    QString msg = i18n("Could not finish operation.\n");
    msg += hexError( Err_Busy );
    KMessageBox::sorry( topLevelWidget(), msg );
  }

  return( mProgressBusy );
}




#include "hexeditorwidget.moc"
