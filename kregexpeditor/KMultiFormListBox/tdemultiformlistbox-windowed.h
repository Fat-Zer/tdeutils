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
#ifndef __tdemultiformlistboxwindowed
#define __tdemultiformlistboxwindowed

#ifdef TQT_ONLY
  #include "compat.h"
  #include <tqlistbox.h>
#else
  #include <tdelistbox.h>
#endif

#include "tdemultiformlistboxfactory.h"
#include "tdemultiformlistbox.h"
class WindowListboxItem;


/**
   This class implements the windowed look for a @ref KMultiFormListBox

   @internal
*/
class KMultiFormListBoxWindowed  :public TQWidget, KMultiFormListBoxShower {

Q_OBJECT
  

friend class KMultiFormListBox;

private:
  KMultiFormListBoxWindowed(KMultiFormListBoxFactory *factory, TQWidget *parent,
														bool showUpDownButtons, bool showHelpButton, TQString addButtonText,
														const char *name);

  KMultiFormListBoxEntryList elements();
  const KMultiFormListBoxEntryList elements() const;
  void append(KMultiFormListBoxEntry *);
  WindowListboxItem *selected();
	TQWidget* qWidget() { return this; }

	TQVBoxLayout* _layout;
  KMultiFormListBoxFactory* _factory;
	TQPtrList<TQPushButton> _buttonList;
  TDEListBox* _listbox;
	virtual void delElement(TQWidget *); // Deletes the given element
	virtual void delAnElement();
  void addElement();


public slots:
  void addNewElement();

signals:
	void showHelp();

private slots:
  void slotEditSelected();
  void slotEditSelected(TQListBoxItem *item);
  void slotCopySelected();
	void slotMoveItemUp();
	void slotMoveItemDown();
  void slotDeleteEntry();
	void slotUpdateButtonState();

};


#endif /* tdemultiformlistboxwindowed */

