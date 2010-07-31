/****************************************************************************
 * Copyright (c) 2005 Alexander Wiedenbruch <mail@wiedenbruch.de>
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

#ifndef INPUT_H
#define INPUT_H

#include <meter.h>
#include <sklineedit.h>

#include <tqpainter.h>
#include <tqcolor.h>
#include <tqlineedit.h>
#include <tqwidget.h>
#include <tqstring.h>
#include <tqfont.h>

#include "textfield.h"

class Input : public Meter
{
Q_OBJECT
public:
  Input(karamba* k, int ix, int iy, int iw, int ih);
  Input();

  ~Input();

  void mUpdate(TQPainter *p);

  void setValue(TQString text);
  TQString getStringValue() const;

  void setBGColor(TQColor c);
  TQColor getBGColor() const;
  void setColor(TQColor c);
  TQColor getColor() const;
  void setFontColor(TQColor fontColor);
  TQColor getFontColor() const;
  void setSelectionColor(TQColor selectionColor);
  TQColor getSelectionColor() const;
  void setSelectedTextColor(TQColor selectedTextColor);
  TQColor getSelectedTextColor() const;
  void setTextProps(TextField*);

  void hide();
  void show();

  void setSize(int ix, int iy, int iw, int ih);
  void setX(int ix);
  void setY(int iy);
  void setWidth(int iw);
  void setHeight(int ih);

  void setFont(TQString f);
  TQString getFont() const;
  void setFontSize(int size);
  int getFontSize() const;
  
  void setInputFocus();
  void clearInputFocus();

private:
  SKLineEdit *edit;
  TQFont font;
};

#endif
