/*
 *  Copyright (c) 2002-2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifdef TQT_ONLY
  #include "compat.h"
#else
  #include "regexpwidget.moc"
#endif

#include "regexpwidget.h"
#include <iostream>
#include <tqpainter.h>
#include <tqcursor.h>
#include "concwidget.h"
#include "dragaccepter.h"

const int RegExpWidget::pw = 1;
const int RegExpWidget::bdSize = 5;
const int RegExpWidget::space = 5;

RegExpWidget::RegExpWidget(RegExpEditorWindow* editorWindow, TQWidget *parent,
                           const char *name)
  : TQWidget(parent, name ? name : "RegExpWidget", WNoMousePropagation ),
    _editorWindow( editorWindow ),  _isSelected( false ), _isToplevel( false )
{
}


void RegExpWidget::addNewChild(DragAccepter *, RegExpWidget *)
{
  qFatal("This widget should not expect any tqchildren");
}

void RegExpWidget::addNewConcChild(DragAccepter *, ConcWidget *)
{
  qFatal("This widget should not expect any tqchildren");
}

void RegExpWidget::setConcChild(ConcWidget *)
{
  qFatal("This widget should not expect any tqchildren");
}

bool RegExpWidget::updateSelection(bool parentSelected)
{
  bool selected = ( parentSelected ||
                    _editorWindow->selectionOverlap( mapToGlobal( TQPoint(0,0) ), size() ) )
    && !_isToplevel;

  if ( _isSelected != selected ) {
    // Selection state changed
    _isSelected = selected;
    tqrepaint();
    return true;
  }
  return false;
}

void RegExpWidget::drawPossibleSelection( TQPainter& painter, TQSize mySize )
{
  if ( _isSelected ) {
    painter.fillRect( 0, 0, mySize.width(), mySize.height(), TQBrush( gray ) );
  }
}

bool RegExpWidget::isSelected() const
{
  return _isSelected;
}

bool RegExpWidget::hasSelection() const
{
  return _isSelected;
}

void RegExpWidget::clearSelection()
{
  _isSelected = false;
}

void RegExpWidget::applyRegExpToSelection( RegExpType )
{
  qFatal("This method should not be called for this widget");
}

void RegExpWidget::deleteSelection()
{
  qFatal("This method should be overridden if needed!");
}

RegExp* RegExpWidget::selection() const
{
  return regExp();
}

int RegExpWidget::edit()
{
  qFatal("This method should be overridden if needed!");
  return 0; // Compiler shut up
}

void RegExpWidget::mousePressEvent ( TQMouseEvent* event )
{
  if ( _editorWindow->isPasteing() || _editorWindow->isInserting() )
    return;

  if ( event->button() == Qt::LeftButton ) {
    if ( ! _editorWindow->pointSelected( TQCursor::pos() ) ) {
      _editorWindow->clearSelection( true );
      if ( dynamic_cast<DragAccepter*>(this) == 0 && dynamic_cast<ConcWidget*>(this) == 0 ) {
        selectWidget( true );
      }
    }

    TQMouseEvent ev( event->type(), mapTo(_editorWindow, event->pos()),
                    event->button(), event->state());
    TQApplication::sendEvent( _editorWindow, &ev );
  }
  else if ( event->button() == Qt::RightButton ) {
    _editorWindow->showRMBMenu( true );
  }

  // currently (TQt3.0) it seems like qt do not accept that the accept flag is set,
  // and thus sends the event to the parent - given that the following line is in.
  // It doesn't make any change to leave it out.
  // 25 Oct. 2001 19:03 -- Jesper K. Pedersen
  //  TQWidget::mousePressEvent( event );
}

void RegExpWidget::mouseReleaseEvent( TQMouseEvent* )
{
  if ( _editorWindow->isInserting() && acceptWidgetInsert( _editorWindow->insertType() ) ) {
    if ( !_editorWindow->hasSelection() )
      _isSelected = true;

    _editorWindow->applyRegExpToSelection( _editorWindow->insertType() );
    _editorWindow->clearSelection( true );
    _editorWindow->updateContent( this );
    _editorWindow->slotEndActions();
    _editorWindow->updateCursorUnderPoint();
  }
}

TQRect RegExpWidget::selectionRect() const
{
  return TQRect( mapToGlobal( TQPoint(0,0) ), size() );
}


void RegExpWidget::enterEvent( TQEvent * )
{
  updateCursorShape();
}

void RegExpWidget::updateCursorShape()
{
  TQCursor cursor;

  if ( _editorWindow->isPasteing() ) {
    if ( acceptWidgetPaste() )
      cursor =  CrossCursor;
    else
      cursor =  ForbiddenCursor;
  }
  else if ( _editorWindow->isInserting() ) {
    if ( acceptWidgetInsert( _editorWindow->insertType() ) )
      cursor = CrossCursor;
    else
      cursor = ForbiddenCursor;
  }
  else
    cursor = arrowCursor;

  setCursor( cursor );
}

void RegExpWidget::updateCursorRecursively()
{
  updateCursorShape();
}


bool RegExpWidget::acceptWidgetPaste() const
{
  return false;
}

bool RegExpWidget::acceptWidgetInsert( RegExpType tp ) const
{
  return WidgetFactory::isContainer( tp );
}

RegExpWidget* RegExpWidget::widgetUnderPoint( TQPoint globalPos, bool )
{
  if ( TQRect(mapToGlobal( TQPoint(0,0) ), size() ).contains( globalPos ) )
    return this;
  else
    return 0;
}

void RegExpWidget::selectWidget( bool sel)
{
  _isSelected = sel;
  update();
}

void RegExpWidget::updateAll()
{
  update();
}


