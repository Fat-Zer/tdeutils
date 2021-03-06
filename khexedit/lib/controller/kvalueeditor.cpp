/***************************************************************************
                          kvalueeditor.cpp  -  description
                             -------------------
    begin                : Sa Dez 4 2004
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
#include <tqevent.h>
// lib specific
#include "kvaluecolumn.h"
#include "kbufferranges.h"
#include "kbuffercursor.h"
#include "khexedit.h"
#include "kvalueeditor.h"


using namespace KHE;

KValueEditor::KValueEditor( KValueColumn *VC, TDEBufferCursor *BC, KHexEdit* HE, KController *P )
  : KEditor( BC, HE, P ),
  ValueColumn( VC ),
  InEditMode( false ),
  EditModeByInsert( false )
{
}


bool KValueEditor::handleKeyPress( TQKeyEvent *KeyEvent )
{
  bool KeyUsed = true;

  // TODO: for now we don't touch it if there are selections
  if( !HexEdit->BufferRanges->hasSelection() )
  {
    //
    switch( KeyEvent->key() )
    {
      case TQt::Key_Plus:
        doValueEditAction( IncValue );
        break;
      case TQt::Key_Minus:
        doValueEditAction( DecValue );
        break;
      case TQt::Key_Space:
        if( !InEditMode )
        {
          KeyUsed = false;
          break;
        }
      case TQt::Key_Enter:
      case TQt::Key_Return:
        doValueEditAction( InEditMode?LeaveValue:EnterValue );
        break;
      case TQt::Key_Escape:
        if( InEditMode )
          doValueEditAction( CancelValue );
        else
          KeyUsed = false;
        break;
      case TQt::Key_Backspace:
        if( InEditMode )
          doValueEditAction( ValueBackspace );
        else
          KeyUsed = false;
        break;
      default:
        // is plain char?
        if( KeyEvent->text().length() > 0
            && ( !(KeyEvent->state()&( TQt::ControlButton | TQt::AltButton | TQt::MetaButton )) ) )
        {
          TQChar C = TQString(KeyEvent->text())[0];
          // no usable char?
          if( !C.isLetterOrNumber() )
          {
            KeyUsed = false;
            break;
          }
          int Input = C.latin1();

          if( InEditMode )
            doValueEditAction( ValueAppend, Input );
          else
          {
            unsigned char InputValue = 0;
            const KByteCodec *ByteCodec = ValueColumn->byteCodec();
            // valid digit?
            if( ByteCodec->appendDigit(&InputValue,Input) )
            {
              if( HexEdit->OverWrite )
                doValueEditAction( ValueEdit, InputValue );
              else
              {
                int Index = BufferCursor->realIndex();
                if( HexEdit->DataBuffer->insert(Index,(char*)&InputValue,1) > 0 )
                {
                  HexEdit->pauseCursor();
                  HexEdit->updateLength();

                  InEditMode = true;
                  EditModeByInsert = true;
                  OldValue = EditValue = InputValue;
                  ByteCodec->encode( ByteBuffer, 0, EditValue );

                  BufferCursor->gotoRealIndex();
                  KSection ChangedRange( Index,HexEdit->DataBuffer->size()-1 );
                  HexEdit->BufferRanges->addChangedRange( ChangedRange );
                  HexEdit->repaintChanged();
                  HexEdit->ensureCursorVisible();
                  HexEdit->unpauseCursor();
                  HexEdit->updateCursor();
                  emit HexEdit->bufferChanged( ChangedRange.start(), ChangedRange.end() );
                }
              }
            }
          }
        }
        else
          KeyUsed = false;
    }
  }
  else
    KeyUsed = false;

  return KeyUsed ? true : KEditor::handleKeyPress(KeyEvent);
}


void KValueEditor::doValueEditAction( KValueEditAction Action, int Input )
{
  // we are not yet in edit mode?
  if( !InEditMode )
  {
    int ValidIndex = BufferCursor->validIndex();
    // no valid cursor position?
    if( ValidIndex == -1 || (!HexEdit->OverWrite && Input == -1) || BufferCursor->isBehind() )
      return;

    InEditMode = true;
    EditModeByInsert = false; // default, to be overwritten if so

    // save old value
    OldValue = EditValue = (unsigned char)HexEdit->DataBuffer->datum(ValidIndex);
  }

  const KByteCodec *ByteCodec = ValueColumn->byteCodec();
  // 
  unsigned char NewValue = EditValue;
  bool StayInEditMode = true;
  bool MoveToNext = false;

  switch( Action )
  {
    case ValueEdit:
      NewValue = Input;
      EditValue = NewValue^255; // force update
      EditModeByInsert = true;
      break;
    case ValueBackspace:
      if( NewValue > 0 )
        ByteCodec->removeLastDigit( &NewValue );
      break;
    case EnterValue:
      EditValue ^= 255; // force update
      break;
    case IncValue:
      if( NewValue < 255 )
        ++NewValue;
      break;
    case DecValue:
      if( NewValue > 0 )
        --NewValue;
      break;
    case ValueAppend:
      if( ByteCodec->appendDigit(&NewValue,Input) )
        if( EditModeByInsert && NewValue >= ByteCodec->digitsFilledLimit() )
        {
          StayInEditMode = false;
          MoveToNext = true;
        }
      break;
    case LeaveValue:
      StayInEditMode = false;
      MoveToNext = EditModeByInsert;
      break;
    case CancelValue:
      NewValue = OldValue;
      StayInEditMode = false;
      break;
  }

  bool Changed = (NewValue != EditValue);
  int Index = BufferCursor->index();
  if( Changed )
  {
    // sync value
    EditValue = NewValue;
    ByteCodec->encode( ByteBuffer, 0, EditValue );

    HexEdit->DataBuffer->replace( Index, 1, (char*)&EditValue, 1 );
  }

  HexEdit->updateCursor();

  if( !StayInEditMode )
  {
    HexEdit->pauseCursor();
    InEditMode = false;
    if( MoveToNext )
      BufferCursor->gotoNextByte();
    HexEdit->unpauseCursor();
  }

  if( Changed )
    if( Action != EnterValue ) emit HexEdit->bufferChanged( Index, Index );
}
