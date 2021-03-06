/***************************************************************************
                          khepart.cpp  -  description
                             -------------------
    begin                : Don Jun 19 2003
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


// kde specific
#include <tdelocale.h>
//#include <kinstance.h>
#include <tdeaction.h>
#include <kstdaction.h>
//#include <tdeglobalsettings.h>
// app specific
#include "khexedit.h"
#include "kcharcodec.h"
#include "khepartfactory.h"
#include "khebrowserextension.h"
#include "khepart.h"

using namespace KHE;

static const char RCFileName[] = "khexedit2partui.rc";

KHexEditPart::KHexEditPart( TQWidget *ParentWidget, const char *WidgetName,
                            TQObject *Parent, const char *Name,
                            bool BrowserViewWanted )
 : KParts::ReadOnlyPart( Parent, Name )
{
  setInstance( KHexEditPartFactory::instance() );

  HexEdit = new KHexEdit( &Wrapping, ParentWidget, WidgetName );
  HexEdit->setNoOfBytesPerLine( 16 );
  HexEdit->setBufferSpacing( 3, 4, 10 );
  HexEdit->setShowUnprintable( false );

  // notify the part that this is our internal widget
  setWidget( HexEdit );

  setupActions( BrowserViewWanted );

  if( CopyAction )
  {
    connect( HexEdit, TQT_SIGNAL(copyAvailable(bool)), CopyAction,TQT_SLOT(setEnabled(bool)) );
    connect( HexEdit, TQT_SIGNAL(selectionChanged()),  this,      TQT_SLOT(slotSelectionChanged()) );
    CopyAction->setEnabled( false );
  }

  // plugin to browsers
  if( BrowserViewWanted )
    new KHexEditBrowserExtension( this );
}


KHexEditPart::~KHexEditPart()
{
}

/*
void KHexEditPart::setupTools( bool BrowserViewWanted )
{
  if( !BrowserViewWanted ) new TDEClipboardTool( this );
  
  new KZoomToolet( this );
  new TDESelectToolet( this );
  new KHEValueCodingToolet( this );
  new KHECharEncodingToolet( this );
  new KHEResizeStyleToolet( this );
  new KHEColumnToggleToolet( this );
}
*/
void KHexEditPart::setupActions( bool BrowserViewWanted )
{
  TDEActionCollection *AC = actionCollection();
  // create our actions
  CopyAction = BrowserViewWanted ? 0 : KStdAction::copy( TQT_TQOBJECT(HexEdit), TQT_SLOT(copy()), AC );

  KStdAction::selectAll( this, TQT_SLOT(slotSelectAll()), AC );
  KStdAction::deselect(  this, TQT_SLOT(slotUnselect()),  AC );

  // value encoding
  CodingAction = new TDESelectAction( i18n("&Value Coding"), 0, AC, "view_valuecoding" );
  TQStringList List;
  List.append( i18n("&Hexadecimal") );
  List.append( i18n("&Decimal")     );
  List.append( i18n("&Octal")       );
  List.append( i18n("&Binary")      );
  CodingAction->setItems( List );
  connect( CodingAction, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotSetCoding(int)) );

  // document encoding
  EncodingAction = new TDESelectAction( i18n("&Char Encoding"), 0, AC, "view_charencoding" );
  EncodingAction->setItems( KCharCodec::codecNames() );
  connect( EncodingAction, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotSetEncoding(int)) );

  ShowUnprintableAction = new TDEToggleAction( i18n("Show &Unprintable Chars (<32)"), 0, this, TQT_SLOT(slotSetShowUnprintable()), actionCollection(), "view_showunprintable" );

  KStdAction::zoomIn(  TQT_TQOBJECT(HexEdit), TQT_SLOT(zoomIn()),   actionCollection() );
  KStdAction::zoomOut( TQT_TQOBJECT(HexEdit), TQT_SLOT(zoomOut()),  actionCollection() );

  // resize style
  ResizeStyleAction = new TDESelectAction( i18n("&Resize Style"), 0, AC, "resizestyle" );
  List.clear();
  List.append( i18n("&No Resize") );
  List.append( i18n("&Lock Groups") );
  List.append( i18n("&Full Size Usage") );
  ResizeStyleAction->setItems( List );
  connect( ResizeStyleAction, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotSetResizeStyle(int)) );

  ShowOffsetColumnAction = new TDEToggleAction( i18n("&Line Offset"), Key_F11, this, TQT_SLOT(slotToggleOffsetColumn()), AC, "view_lineoffset" );

  // show buffer columns
  ToggleColumnsAction = new TDESelectAction( i18n("&Columns"), 0, AC, "togglecolumns" );
  List.clear();
  List.append( i18n("&Values Column") );
  List.append( i18n("&Chars Column") );
  List.append( i18n("&Both Columns") );
  ToggleColumnsAction->setItems( List );
  connect( ToggleColumnsAction, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotToggleValueCharColumns(int)) );

  fitActionSettings();

  // set our XML-UI resource file
  setXMLFile( RCFileName );
}


void KHexEditPart::fitActionSettings()
{
  ShowOffsetColumnAction->setChecked( HexEdit->offsetColumnVisible() );
  ShowUnprintableAction->setChecked( HexEdit->showUnprintable() );

  CodingAction->setCurrentItem( (int)HexEdit->coding() );
  EncodingAction->setCurrentItem( KCharCodec::codecNames().findIndex(HexEdit->encodingName()) );

  ResizeStyleAction->setCurrentItem( (int)HexEdit->resizeStyle() );

  ToggleColumnsAction->setCurrentItem( (int)HexEdit->visibleBufferColumns()-1 );
}


bool KHexEditPart::openFile()
{
  Wrapping.open( m_file );
  HexEdit->setDataBuffer( &Wrapping );
  HexEdit->setCursorPosition( 0 );
  HexEdit->selectAll( false );

  return true;
}



void KHexEditPart::slotSelectionChanged()
{
  bool State = HexEdit->hasSelectedData();
  CopyAction->setEnabled( State );
}


void KHexEditPart::slotSelectAll()
{
  HexEdit->selectAll( true );
}


void KHexEditPart::slotUnselect()
{
  HexEdit->selectAll( false );
}


void KHexEditPart::slotSetCoding( int Coding )
{
  HexEdit->setCoding( (KHexEdit::KCoding)Coding );
}

void KHexEditPart::slotSetShowUnprintable()
{
  HexEdit->setShowUnprintable( ShowUnprintableAction->isChecked() );
}

void KHexEditPart::slotToggleOffsetColumn()
{
  HexEdit->toggleOffsetColumn( ShowOffsetColumnAction->isChecked() );
}

void KHexEditPart::slotSetResizeStyle( int ResizeStyle )
{
  HexEdit->setResizeStyle( (KHexEdit::KResizeStyle)ResizeStyle );
}

void KHexEditPart::slotSetEncoding( int Encoding )
{
  HexEdit->setEncoding( KCharCodec::codecNames()[Encoding] );
}

void KHexEditPart::slotToggleValueCharColumns( int VisibleColumns)
{
  HexEdit->showBufferColumns( VisibleColumns+1 );
}

#include "khepart.moc"
