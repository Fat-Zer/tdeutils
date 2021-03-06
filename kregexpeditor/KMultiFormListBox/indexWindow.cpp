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
  #include "indexWindow.moc"
#endif

#include "indexWindow.h"
#include <iostream>
#include <tqlayout.h>

indexWindow::indexWindow() : TQWidget(0,"", WStyle_Customize | WType_Popup)
{
  lb = new TQListBox(this);
  connect(lb,TQT_SIGNAL(selected(int)), this, TQT_SLOT(lbSelected(int)));
  TQHBoxLayout *lay = new TQHBoxLayout(this);
  lay->addWidget(lb);
  lbFinish = false;
}

void indexWindow::lbSelected(int index)
{
  lbFinish = true;
  hide();
  lbFinish = false;
  finish(index);
}

void indexWindow::finish(int index)
{
  itemSelected = index;
  tqApp->exit_loop();
}


void indexWindow::insertItem(TQString txt)
{
  lb->insertItem(txt);
}

void indexWindow::hideEvent(TQHideEvent *h)
{
  TQWidget::hideEvent(h);
  if (!lbFinish)
    finish(-1);
}


int indexWindow::exec(const TQPoint & /*start*/, int /*width*/)
{
    // This code is not xinerama safe, on the other hand this part of the widget
    // is not used in KRegExpEditor. This is part of a widget which has
    // never been completed, but which is used partly by KRegExpEditor.
    // 23 Feb. 2003 11:28 -- Jesper K. Pedersen
/*
    // calculate the height of all the elements together.
  // I need to do it this way, as sizeHint doesn't report the correct size
  // and itemHeight doesn't neither.
  int elm_h = lb->item(0)->height(lb) * lb->count();
  elm_h += 2*lb->frameWidth();

  TQWidget *desktop = TQApplication::desktop();
  int desktop_h = desktop->height();
  int rest_h = desktop_h - start.y();
  int below_h = TQMAX(rest_h, 200);

  int start_y = start.y();

  if (rest_h < 200 && elm_h > 200) {
    start_y = desktop_h-TQMIN(elm_h,200);
  }

  setGeometry(start.x(), start_y, width, TQMIN(elm_h, below_h));
  show();
  tqApp->enter_loop();
  return itemSelected;
*/
    return 0;
}

