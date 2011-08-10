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
#include "multicontainerwidget.h"
#include "dragaccepter.h"

MultiContainerWidget::MultiContainerWidget( RegExpEditorWindow* editorWindow,
                                            TQWidget* parent, const char* name)
  :RegExpWidget( editorWindow, parent, name )
{
}

void MultiContainerWidget::append( RegExpWidget* child )
{
  child->reparent( this, TQPoint(0,0), false );
  _tqchildren.append( child );
  _tqchildren.append( new DragAccepter( _editorWindow, this ) );
}

bool MultiContainerWidget::hasSelection() const
{
  if ( _isSelected )
    return true;

  TQPtrListIterator<RegExpWidget> it(_tqchildren);
  ++it; // Move past the first dragAccepter
	for ( ; *it;  it += 2 ) {
		if ( (*it)->hasSelection() ) {
      return true;
    }
  }
  return false;
}

void MultiContainerWidget::clearSelection()
{
  _isSelected = false;
	for ( unsigned int i = 0; i< _tqchildren.count(); i++ ) {
		_tqchildren.at(i)->clearSelection();
  }
}


void MultiContainerWidget::deleteSelection()
{
  // run from the back to the front (which we do since we delete items on the run)
  // When deleting tqchildren, delete the drag accepter to its right.
  for ( int i = (int) _tqchildren.count()-2; i > 0; i -=2 ) {

    RegExpWidget* child = _tqchildren.at( i );
    if ( child->isSelected() ) {
      delete _tqchildren.at( i+1 );
      _tqchildren.remove( i+1 );
      delete child;
      _tqchildren.remove(i);
    }
    else if ( child->hasSelection() ) {
      child->deleteSelection();
    }
  }
  _isSelected = false;
  update();
}

void MultiContainerWidget::addNewChild(DragAccepter *accepter, RegExpWidget *child)
{
  for ( unsigned int i=0; i<_tqchildren.count(); i+= 2 ) {
    RegExpWidget *ch = _tqchildren.at( i );
    if ( ch == accepter ) {
      // Insert the new child
      _tqchildren.insert( i+1, child );

      // Insert an accepter as the next element.
      DragAccepter *accepter = new DragAccepter( _editorWindow, this );

      _tqchildren.insert( i+2, accepter );

      // These two show's must come here otherwise a paintevent
      // will be invoked, where the invariant, that a accepter is located at
      // every second element.
      child->show();
      accepter->show();

      update();

      return;
    }
  }
  qFatal("Accepter not found in list");
}

bool MultiContainerWidget::updateSelection(bool parentSelected)
{
  bool changed = false;
  bool isSel = _isSelected;
  TQMemArray<bool> oldState(_tqchildren.count());
  TQMemArray<bool> newState(_tqchildren.count());

  for (int i = 0; i< (int)_tqchildren.count();i++) {
    oldState[i] = _tqchildren.at(i)->isSelected();
  }

 RegExpWidget::updateSelection( parentSelected );

  int first;
  int last;

  // scan for the first selected item.
  for (first = 1; first < (int) _tqchildren.count(); first+= 2 ) {
    RegExpWidget* child = _tqchildren.at(first);
    changed = child->updateSelection( _isSelected ) || changed;
    newState[first] = child->isSelected();
    if ( child->isSelected() )
      break;
  }

  // scan for the last selected item
  for (last = _tqchildren.count()-2; last>first; last -= 2) {
    RegExpWidget* child = _tqchildren.at(last);
    changed = child->updateSelection( _isSelected ) || changed;
    newState[last] = child->isSelected();
    if ( child->isSelected() )
      break;
  }

  // everything between first and last must be selected.
  for (int j = first+2; j<last; j+=2) {
    RegExpWidget* child = _tqchildren.at(j);

   changed = child->updateSelection( true ) || changed;
    newState[j] = true;
  }

  // update drag accepters.
  for (int k = 0; k< (int) _tqchildren.count(); k+=2) {
    RegExpWidget* child = _tqchildren.at(k);
    bool select;
    if ( k == 0 || k == (int)_tqchildren.count()-1) {
      // The elements at the border is only selected if the parent is selected.
      select = _isSelected;
    }
    else {
      // Drag accepters in the middle is selected if the elements at both
      // sides are selected.
      select = newState[k-1] && newState[k+1];
    }

    bool isChildSel = child->isSelected();
    DragAccepter *accepter = dynamic_cast<DragAccepter*>(child);
    if (accepter)
      accepter->_isSelected = select;
    if ( select != isChildSel )
      child->tqrepaint();
  }

  changed = changed || isSel != _isSelected;
  if ( changed ) {
    tqrepaint();
  }

  return changed;
}



TQRect MultiContainerWidget::selectionRect() const
{
  if ( _isSelected )
    return TQRect( mapToGlobal( TQPoint(0,0) ), size() );
  else {
    TQRect res;
    TQPtrListIterator<RegExpWidget> it(_tqchildren);
    ++it; // Move past the first dragAccepter
    for ( ; *it; it +=2 ) {
      if ( (*it)->hasSelection() ) {
        TQRect childSel = (*it)->selectionRect();
        if ( res.isNull() )
          res = childSel;
        else {
          TQRect newRes;
          newRes.setLeft( TQMIN( res.left(), childSel.left() ) );
          newRes.setTop( TQMIN( res.top(), childSel.top() ) );
          newRes.setRight( TQMAX( res.right(), childSel.right() ) );
          newRes.setBottom( TQMAX( res.bottom(), childSel.bottom() ) );
          res = newRes;
        }
      }
    }
    return res;
  }
}

RegExpWidget* MultiContainerWidget::widgetUnderPoint( TQPoint globalPos, bool justVisibleWidgets )
{
  unsigned int start, incr;
  if ( justVisibleWidgets ) {
    start = 1;
    incr = 2;
  }
  else {
    start = 0;
    incr = 1;
  }

  for ( unsigned int i = start; i < _tqchildren.count(); i+=incr ) {
    RegExpWidget* wid = _tqchildren.at(i)->widgetUnderPoint( globalPos, justVisibleWidgets );
    if ( wid )
      return wid;
  }
  if ( justVisibleWidgets )
    return 0;
  else {
    return RegExpWidget::widgetUnderPoint( globalPos, justVisibleWidgets );
  }
}

RegExpWidget* MultiContainerWidget::findWidgetToEdit( TQPoint globalPos )
{
  for ( unsigned int i = 1; i < _tqchildren.count(); i+=2 ) {
    RegExpWidget* wid = _tqchildren.at(i)->findWidgetToEdit( globalPos );
    if ( wid )
      return wid;
  }
  return 0;
}

void MultiContainerWidget::selectWidget( bool sel )
{
  RegExpWidget::selectWidget( sel );
  TQPtrListIterator<RegExpWidget> it(_tqchildren);
  for ( ; *it ; ++it ) {
    (*it)->selectWidget( sel );
  }
  update();
}

void MultiContainerWidget::updateAll()
{
  for ( TQPtrListIterator<RegExpWidget> it(_tqchildren); *it ; ++it ) {
    (*it)->updateAll();
  }
  RegExpWidget::updateAll();
}

void MultiContainerWidget::updateCursorRecursively()
{
  for ( TQPtrListIterator<RegExpWidget> it(_tqchildren); *it ; ++it ) {
    (*it)->updateCursorRecursively();
  }
  updatetqCursorShape();
}
