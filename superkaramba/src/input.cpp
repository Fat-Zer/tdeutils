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

#include "input.h"
#include "kdebug.h"

Input::Input(karamba* k, int x, int y, int w, int h):
  Meter(k, x, y, w, h)
{
  edit = new SKLineEdit((TQWidget*)k, this);
  edit->setGeometry(x,y,w,h);
}

Input::~Input()
{
  delete edit;
}

void Input::mUpdate(TQPainter*)
{
  edit->tqrepaint();
}

void Input::setValue(TQString text)
{
  edit->setText(text);
}

TQString Input::getStringValue() const
{
  return edit->text();
}

void Input::setBGColor(TQColor c)
{
  edit->setBackgroundColor(c);
}

void Input::setColor(TQColor c)
{
  Meter::setColor(c);
  edit->setFrameColor(c);
}

TQColor Input::getBGColor() const
{
  return edit->backgroundColor();
}

TQColor Input::getColor() const
{
  return edit->getFrameColor();
}

void Input::hide()
{
  Meter::hide();
  edit->setHidden(true);
}

void Input::show()
{
  Meter::show();
  edit->setHidden(false);
}

void Input::setSize(int ix, int iy, int iw, int ih)
{
  Meter::setSize(ix, iy, iw, ih);
  edit->setGeometry(ix, iy, iw, ih);
}

void Input::setX(int ix)
{
  Meter::setX(ix);
  edit->setGeometry(ix, getY(), getWidth(), getHeight());
}

void Input::setY(int iy)
{
  Meter::setY(iy);
  edit->setGeometry(getX(), iy, getWidth(), getHeight());
}

void Input::setWidth(int iw)
{
  Meter::setWidth(iw);
  edit->setGeometry(getX(), getY(), iw, getHeight());
}

void Input::setHeight(int ih)
{
  Meter::setHeight(ih);
  edit->setGeometry(getX(), getY(), getWidth(), ih);
}

void Input::setFont(TQString f)
{
  font.setFamily(f);
  edit->setFont(font);
}

TQString Input::getFont() const
{
  return font.family();
}

void Input::setFontColor(TQColor fontColor)
{
  TQPalette palette = edit->palette();
  palette.setColor(TQColorGroup::Text, fontColor);
  edit->setPalette(palette);
}

TQColor Input::getFontColor() const
{
  const TQColorGroup &color = edit->colorGroup();
  return color.text();
}

void Input::setSelectionColor(TQColor selectionColor)
{
  TQPalette palette = edit->palette();
  palette.setColor(TQColorGroup::Highlight, selectionColor);
  edit->setPalette(palette);
}

TQColor Input::getSelectionColor() const
{
  const TQColorGroup &color = edit->colorGroup();
  return color.highlight();
}

void Input::setSelectedTextColor(TQColor selectedTextColor)
{
  TQPalette palette = edit->palette();
  palette.setColor(TQColorGroup::HighlightedText, selectedTextColor);
  edit->setPalette(palette);
}

TQColor Input::getSelectedTextColor() const
{
  const TQColorGroup &color = edit->colorGroup();
  return color.highlightedText();
}

void Input::setFontSize(int size)
{
  font.setPixelSize(size);
  edit->setFont(font);
}

int Input::getFontSize() const
{
  return font.pixelSize();
}

void Input::setTextProps(TextField* t)
{
  if(t)
  {
    setFontSize(t->getFontSize());
    setFont(t->getFont());
    setColor(t->getColor());
    setBGColor(t->getBGColor());
  }
}

void Input::setInputFocus()
{
  edit->setFocus();
}

void Input::clearInputFocus()
{
  edit->clearFocus();
}

#include "input.moc"
