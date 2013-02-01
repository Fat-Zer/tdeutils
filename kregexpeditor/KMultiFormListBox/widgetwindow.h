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
#ifndef __configwindow
#define __configwindow

#ifdef TQT_ONLY
  #include "compat.h"
  #include <tqlistbox.h>
#else
  #include <kdialogbase.h>
  #include <klistbox.h>
#endif

#include "tdemultiformlistbox-windowed.h"

/**
   @internal
*/
class WidgetWindow :public KDialogBase
{
  friend class KMultiFormListBoxWindowed;
  friend class WindowListboxItem;

  WidgetWindow(KMultiFormListBoxFactory *, TDEListBox *);
  WidgetWindow(KMultiFormListBoxFactory *, KMultiFormListBoxEntry *widget, TDEListBox *);
  void init(KMultiFormListBoxFactory *, TDEListBox *, KMultiFormListBoxEntry *widget = 0);
  void display();
  KMultiFormListBoxEntry *entry();
  WidgetWindow *clone();
  ~WidgetWindow();

protected slots:
  void slotOk();
  void slotCancel();

private:
  KMultiFormListBoxFactory *myFact;
  KMultiFormListBoxEntry *myWidget;
  TQByteArray _backup;
  TDEListBox *listbox;
  WindowListboxItem *myListboxItem;
  bool initialShow;
};

#endif // configwindow
