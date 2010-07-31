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

#ifndef SKLINEEDIT_H
#define SKLINEEDIT_H

#include <tqlineedit.h>
#include <tqwidget.h>
#include <tqevent.h>
#include <tqpainter.h>
#include <tqcolor.h>

class Input;

class SKLineEdit : public QLineEdit
{
  public:
    SKLineEdit(TQWidget *w, Input *i);
    ~SKLineEdit();

    void drawFrame(TQPainter *p);
    void drawContents(TQPainter *p);

    void setFrameColor(TQColor c);
    TQColor getFrameColor() const;

    void setBackgroundColor(TQColor c);
    
    Input* getInput();

  protected:
    virtual void keyReleaseEvent(TQKeyEvent* e);
    virtual void keyPressEvent(TQKeyEvent* e);

  private:
    TQColor frameColor;
    Input* m_input;
};

#endif
