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
#ifndef KWIDGETLISTBOX_H
#define KWIDGETLISTBOX_H

#include <tqtable.h>

/**
@author See README for the list of authors
*/

typedef bool (*show_callback) (int index, TQWidget* widget, void* data);

class KWidgetListbox : public TQTable
{
    Q_OBJECT
  

  public:
    KWidgetListbox(TQWidget *parent = 0, const char *name = 0);
    ~KWidgetListbox();

    int insertItem(TQWidget* item, int index = -1);
    void setSelected(TQWidget* item);
    void setSelected(int index);
    void removeItem(TQWidget* item);
    void removeItem(int index);
    void clear();
    int selected() const;
    TQWidget* selectedItem() const;
    TQWidget* item(int index) const;
    int index(TQWidget* itm) const;
    uint count() const { return numRows(); };

    void showItems(show_callback func = 0, void* data = 0);

    void paintCell(TQPainter* p, int row, int col, const TQRect& cr,
                   bool selected, const TQColorGroup& cg);
  protected:
    void setItemColors(int index, bool even);
    void updateColors();
    bool even(int index);
    virtual void showEvent(TQShowEvent* e);

  protected slots:
    void selectionChanged(int row, int col);

  signals:
    void selected(int index);
};

#endif
