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
#include "dragaccepter.h"
#include "concwidget.h"

#include <tqpainter.h>

DragAccepter::DragAccepter(RegExpEditorWindow* editorWindow, RegExpWidget *parent,
                           const char *name)
  : RegExpWidget(editorWindow, parent, name == 0 ? "dragaccepter" : name ),
    _drawLine( false )
{
  setAcceptDrops(TRUE);
}

TQSize DragAccepter::sizeHint() const
{
  return TQSize(10,10);
}

void DragAccepter::paintEvent(TQPaintEvent *e)
{
  TQPainter painter(this);
  drawPossibleSelection( painter, size() );

  if ( _drawLine )
    painter.drawLine( 0, height()/2, width(), height()/2 );

  RegExpWidget::paintEvent(e);
}

void DragAccepter::mousePressEvent ( TQMouseEvent* event )
{
  if ( event->button() == Qt::RightButton ) {
    _editorWindow->showRMBMenu( _editorWindow->hasSelection() );
  }
  else {
    RegExpWidget::mousePressEvent( event );
  }
}

void DragAccepter::mouseReleaseEvent( TQMouseEvent* event )
{
  if ( _editorWindow->isPasteing() && event->button() == Qt::LeftButton ) {
    RegExp* regexp = _editorWindow->pasteData();

    RegExpWidget *newElm = WidgetFactory::createWidget( regexp, _editorWindow, 0 );
    if ( newElm ) {
      ConcWidget* elm;
      if (! (elm = dynamic_cast<ConcWidget*>( newElm ) ) ) {
        elm = new ConcWidget( _editorWindow, newElm, 0 );
      }

      Q_ASSERT( elm );

      RegExpWidget *w = dynamic_cast<RegExpWidget*>(parent());
      if (w)
        w->addNewConcChild(this, elm);
      _editorWindow->updateContent( this );
      _editorWindow->clearSelection( true );
    }
  }
  else if ( _editorWindow->isInserting() && event->button() == Qt::LeftButton  ) {
    if ( WidgetFactory::isContainer( _editorWindow->insertType() ) &&
         _editorWindow->pointSelected( mapToGlobal( event->pos() ) ) ) {
      RegExpWidget::mouseReleaseEvent( event );
    }
    else {
      RegExpWidget *child = WidgetFactory::createWidget( _editorWindow,
                                                         dynamic_cast<TQWidget*>(parent()),
                                                         _editorWindow->insertType() );
      if ( child ) {
        RegExpWidget *w = dynamic_cast<RegExpWidget*>(parent());
        if (w)
          w->addNewChild(this, child);
        _editorWindow->updateContent( child );
        child->setFocus();
        _editorWindow->clearSelection( true );
      }
    }
  }
  _editorWindow->slotEndActions();
}


void DragAccepter::dragEnterEvent(TQDragEnterEvent *event)
{
  bool selfDrag = (  event->source() && event->source()->topLevelWidget() == topLevelWidget() && _isSelected );
  event->accept(RegExpWidgetDrag::canDecode( event ) && !selfDrag );
}

void DragAccepter::dropEvent(TQDropEvent *event)
{
  // The widget will be reparent afterward or part of it will, so no need to give
  // it a parent here.
  RegExpWidget *newElm = RegExpWidgetDrag::decode( event, _editorWindow, 0 );
  ConcWidget* elm;
  if ( !(elm = dynamic_cast<ConcWidget*>( newElm ) ) ) {
    elm = new ConcWidget( _editorWindow, newElm, 0 );
  }

  Q_ASSERT( elm );

  RegExpWidget *rew = dynamic_cast<RegExpWidget*>(parent());
  if (rew)
    rew->addNewConcChild(this, elm);
  TQWidget *w = dynamic_cast<TQWidget*>(parent());
  if (w)
    w->update();
  _editorWindow->updateContent( this );

  bool selfDrag = (  event->source() && event->source()->topLevelWidget() == topLevelWidget() );
  if ( ! selfDrag )
    _editorWindow->clearSelection( true );
  else {
    // selection should not be cleared here, since we might want to delete it.
  }
  event->accept();
}

RegExp* DragAccepter::regExp() const
{
	return 0;
}
