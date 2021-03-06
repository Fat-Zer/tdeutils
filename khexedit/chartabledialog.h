/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
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

#ifndef _CHAR_TABLE_DIALOG_H_
#define _CHAR_TABLE_DIALOG_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

class TQLabel;
class TQSpinBox;

#include <kdialogbase.h>
#include "listview.h"


class CCharTableDialog : public KDialogBase
{
  Q_OBJECT
  
  
  public:
    CCharTableDialog( TQWidget *parent=0, const char *name=0,bool modal=false );
    ~CCharTableDialog( void );

  signals:
    void assign( const TQByteArray &buf );

  protected slots:
    virtual void slotUser1( void );
    virtual void startAssign( TQListViewItem * );

  protected:
    virtual void resizeEvent( TQResizeEvent *e );
    virtual void showEvent( TQShowEvent *e );

  private:
    void createListData( void );
    void setColumnWidth( void );

  private:
    TQSpinBox  *mInputCountSpin;
    CListView *mCharacterList;
};

#endif









