/***************************************************************************
                          khebrowserextension.cpp  -  description
                             -------------------
    begin                : Di Nov 16 2004
    copyright            : (C) 2004 by Friedrich W. H. Kossebau
    email                : Friedrich.W.H@Kossebau.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Library General Public           *
 *   License version 2 as published by the Free Software Foundation.       *
 *                                                                         *
 ***************************************************************************/


// qt specific
#include <tdeaction.h>
// app specific
#include "khexedit.h"
#include "khepart.h"
#include "khebrowserextension.h"

using namespace KHE;


KHexEditBrowserExtension::KHexEditBrowserExtension( KHexEditPart *P )
  : KParts::BrowserExtension( P, "khexeditpartbrowserextension" ),
  HexEditPart( P )
{
  connect( HexEditPart->HexEdit, TQT_SIGNAL( selectionChanged() ), this, TQT_SLOT( slotSelectionChanged() ) );
}

void KHexEditBrowserExtension::copy()
{
  HexEditPart->HexEdit->copy();
}


void KHexEditBrowserExtension::slotSelectionChanged()
{
  emit enableAction( "copy", HexEditPart->HexEdit->hasSelectedData() );
}


void KHexEditBrowserExtension::saveState( TQDataStream &stream )
{
  KParts::BrowserExtension::saveState( stream );

  KHexEdit *HexEdit = HexEditPart->HexEdit;

  stream << (int)HexEdit->offsetColumnVisible() << HexEdit->visibleBufferColumns()
      << (int)HexEdit->resizeStyle() << (int)HexEdit->coding() 
      << HexEdit->encodingName() << (int)HexEdit->showUnprintable()
      << HexEdit->contentsX() << HexEdit->contentsY()
      << HexEdit->cursorPosition() << (int)HexEdit->isCursorBehind()
      << HexEdit->cursorColumn();
}


void KHexEditBrowserExtension::restoreState( TQDataStream &stream )
{
  KParts::BrowserExtension::restoreState( stream );

  int OffsetColumnVisible;
  int VisibleBufferColumns;
  int ResizeStyle;
  int Coding;
  TQString EncodingName;
  int ShowUnprintable;
  int x, y;
  int Position;
  int CursorBehind;
  int CursorColumn;

  stream >> OffsetColumnVisible >> VisibleBufferColumns >> ResizeStyle >> Coding >> EncodingName >> ShowUnprintable 
         >> x >> y >> Position >> CursorBehind >> CursorColumn;

  KHexEdit *HexEdit = HexEditPart->HexEdit;

  HexEdit->toggleOffsetColumn( OffsetColumnVisible );
  HexEdit->showBufferColumns( VisibleBufferColumns );
  HexEdit->setResizeStyle( (KHexEdit::KResizeStyle)ResizeStyle );
  HexEdit->setCoding( (KHexEdit::KCoding)Coding );
  HexEdit->setEncoding( EncodingName );
  HexEdit->setShowUnprintable( ShowUnprintable );
  HexEdit->setContentsPos( x, y );
  HexEdit->setCursorPosition( Position, CursorBehind );
  HexEdit->setCursorColumn( (KHexEdit::TDEBufferColumnId)CursorColumn );

  HexEditPart->fitActionSettings();
}

#include "khebrowserextension.moc"
