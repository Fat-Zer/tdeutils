/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 2000 Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */
#ifndef _SEARCHBAR_H_
#define _SEARCHBAR_H_

#include <tqframe.h>

#include "hexbuffer.h"

class TQComboBox;
class TQLineEdit;
class TQPushButton;
class TQCheckBox;
class CHexValidator;

class CSearchBar : public TQFrame
{
  Q_OBJECT
  
  
  public:
    CSearchBar( TQWidget *parent=0, const char *name=0, WFlags f=0 );
    virtual bool eventFilter( TQObject *o, TQEvent *e );

  public slots:
    void cursorMoved();

  protected:
    virtual void showEvent( TQShowEvent * );

  private slots:
    void hideWidget();
    void selectorChanged( int index );
    void textChanged( const TQString &text );
    void start();

  signals:
    void hidden();
    void findData( SSearchControl &sc, uint mode, bool navigator );

  private:
    TQPushButton *mCloseButton;
    TQPushButton *mFindButton;
    TQComboBox   *mTypeCombo;
    TQLineEdit   *mInputEdit;
    TQCheckBox   *mBackwards;
    TQCheckBox   *mIgnoreCase;

    TQString       mFindString[5];
    TQByteArray    mFindData;
    CHexValidator *mValidator;
    uint          mSearchMode;
};

#endif


