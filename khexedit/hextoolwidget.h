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

#ifndef _HEX_TOOL_WIDGET_H_
#define _HEX_TOOL_WIDGET_H_

#include <kconfig.h>

class TQGridLayout;
class TQComboBox;
class TQLineEdit;
class TQCheckBox;

#include "hexbuffer.h"
#include <tqframe.h>

class CHexToolWidget : public TQFrame
{
  Q_OBJECT
  

  public:
    CHexToolWidget( TQWidget *parent = 0, const char *name = 0 );
    ~CHexToolWidget( void );

    void writeConfiguration( KConfig &config );
    void readConfiguration( KConfig &config );
    unsigned long bitValue( SCursorState &state, int n );

  protected:
    void resizeEvent( TQResizeEvent *e );
    void closeEvent( TQCloseEvent *e );

  public slots:
    void cursorChanged( SCursorState &state );
    void fontChanged( void );
    void intelFormat( void );
    void unsignedFormat( void );
    void bitWidthChanged( int i );

  signals:
    void closed( void );


  private:
 
  private:
    SCursorState mCursorState;
    bool mViewHexCaps;

    TQGridLayout *mUtilBox;
    TQLineEdit *mText1[4];
    TQLineEdit *mText2[4];
    TQLineEdit *mText3[4];
    TQCheckBox *mCheckIntelFormat;
    TQCheckBox *mCheckHexadecimal;
    TQComboBox *mBitCombo;
};






#endif
