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
//---------------------
// ccp = Cut-Copy-Paste
//---------------------

#include "tdemultiformlistbox-multivisible.h"
#include "ccp.h"
#include <tqobjectlist.h>
#include <tqpopupmenu.h>
#ifdef TQT_ONLY
  #include "compat.h"
#else
  #include <klocale.h>
#endif

CCP::CCP(KMultiFormListBoxMultiVisible *ee_, KMultiFormListBoxEntry *eee_) : TQObject() {
  ee = ee_;
  eee = eee_;
  install(TQT_TQOBJECT(eee));
}

void CCP::install(TQObject *elm)
{
  elm->installEventFilter(this);
  const TQObjectList children = elm->childrenListObject();
  if (!children.isEmpty()) {
    TQObjectListIt it = TQObjectListIt(children);

    while (TQObject *child=it.current()) {
      if (child->inherits("KMultiFormListBoxMultiVisible")) {
        // Stop if the widget is an KMultiFormListBox, as this widget has its own cut/copy/paste
      }
      else {
        install(child);
      }
      ++it;
    }
  }
}

// This function post the Cut/Copy/Paste menu
bool CCP::eventFilter(TQObject *, TQEvent *event)
{
  if (event->type() != TQEvent::MouseButtonPress ||
      ((TQMouseEvent *) event)->button() != Qt::RightButton ||
      ((TQMouseEvent *) event)->state() != TQEvent::ControlButton) {
    return false;
  }

  TQPoint pos = ((TQMouseEvent *) event)->globalPos();

  TQPopupMenu *menu = new TQPopupMenu();
  menu->insertItem(i18n("Cut"),1);
  menu->insertItem(i18n("Copy"),2);
  menu->insertItem(i18n("Paste"),3);
  menu->insertItem(i18n("Insert Blank"),4);

  int res=menu->exec(pos);
  switch (res) {
  case 1: ee->cut(eee); break;
  case 2: ee->copy(eee); break;
  case 3: ee->paste(eee); break;
  case 4: ee->addElement(eee); break;
  }
  return true;
}

