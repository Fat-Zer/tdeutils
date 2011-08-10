/*
 * Copyright (C) 2005 Petri Damstén <petri.damsten@iki.fi>
 *
 * This file is part of SuperKaramba.
 *
 *  SuperKaramba is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  SuperKaramba is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SuperKaramba; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/
#include "kwidgetlistbox.h"
#include <kdebug.h>
#include <kglobalsettings.h>

KWidgetListbox::KWidgetListbox(TQWidget *parent, const char *name)
 : TQTable(parent, name)
{
  setNumRows(0);
  setNumCols(1);
  setColumnStretchable(0, true);
  setLeftMargin(0);
  setTopMargin(0);
  horizontalHeader()->hide();
  verticalHeader()->hide();
  setSelectionMode(TQTable::NoSelection);
  setFocusStyle(TQTable::FollowStyle);
  connect(this, TQT_SIGNAL(currentChanged(int, int)),
          this, TQT_SLOT(selectionChanged(int, int)));
  setHScrollBarMode(TQScrollView::AlwaysOff);
  setVScrollBarMode(TQScrollView::Auto);
}

KWidgetListbox::~KWidgetListbox()
{
  clear();
}

void KWidgetListbox::clear()
{
  for(int i = 0; i < numRows(); ++i)
    clearCellWidget(i, 0);
  setNumRows(0);
}

int KWidgetListbox::insertItem(TQWidget* item, int index)
{
  int row;

  if(index == -1)
  {
    row = numRows();
    setNumRows(row + 1);
  }
  else
    return -1;

  setRowHeight(row, item->height());
  setCellWidget(row, 0, item);
  setItemColors(row, even(row));
  return row;
}

void KWidgetListbox::setSelected(TQWidget* item)
{
  setSelected(index(item));
}

void KWidgetListbox::selectionChanged(int row, int col)
{
  ensureCellVisible(row, col);
  updateColors();
  emit selected(row);
}

void KWidgetListbox::removeItem(TQWidget* item)
{
  removeItem(index(item));
}

void KWidgetListbox::removeItem(int index)
{
  removeRow(index);
  updateColors();
}

void KWidgetListbox::setSelected(int index)
{
  setCurrentCell(index, 0);
}

int KWidgetListbox::selected() const
{
  return currentRow();
}

TQWidget* KWidgetListbox::selectedItem() const
{
  return item(selected());
}

TQWidget* KWidgetListbox::item(int index) const
{
  return cellWidget(index, 0);
}

int KWidgetListbox::index(TQWidget* itm) const
{
  for(int i = 0; i < numRows(); ++i)
    if(item(i) == itm)
      return i;
  return -1;
}

bool KWidgetListbox::even(int index)
{
  int v = 0;
  for(int i = 0; i < numRows(); ++i)
  {
    if(index == i)
      break;
    if(!isRowHidden(i))
      ++v;
  }
  return (v%2 == 0);
}

void KWidgetListbox::updateColors()
{
  int v = 0;
  for(int i = 0; i < numRows(); ++i)
  {
    if(!isRowHidden(i))
    {
      setItemColors(i, (v%2 == 0));
      ++v;
    }
  }
}

void KWidgetListbox::setItemColors(int index, bool even)
{
  TQWidget* itm = item(index);

  if(index == selected())
  {
    itm->setPaletteBackgroundColor(KGlobalSettings::highlightColor());
    itm->setPaletteForegroundColor(KGlobalSettings::highlightedTextColor());
  }
  else if(even)
  {
    itm->setPaletteBackgroundColor(KGlobalSettings::baseColor());
    itm->setPaletteForegroundColor(KGlobalSettings::textColor());
  }
  else
  {
    itm->setPaletteBackgroundColor(
        KGlobalSettings::alternateBackgroundColor());
    itm->setPaletteForegroundColor(KGlobalSettings::textColor());
  }
}

void KWidgetListbox::showItems(show_callback func, void* data)
{
  for(int i = 0; i < numRows(); ++i)
  {
    if(func == 0)
      showRow(i);
    else
    {
      if(func(i, item(i), data))
        showRow(i);
      else
        hideRow(i);
    }
  }
  updateColors();
}

void KWidgetListbox::showEvent(TQShowEvent*)
{
  //kdDebug() << k_funcinfo << endl;
  repaintContents(false);
}

void KWidgetListbox::paintCell(TQPainter*, int, int, const TQRect&,
                               bool, const TQColorGroup&)
{
  //kdDebug() << k_funcinfo << endl;
}

#include "kwidgetlistbox.moc"
