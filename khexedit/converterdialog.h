/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999-2000 Espen Sand, espensa@online.no
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

#ifndef _CONVERTER_DIALOG_H_
#define _CONVERTER_DIALOG_H_

#include <tqlineedit.h>
#include <kdialogbase.h>

class CHexValidator;


class CValidateLineEdit : public TQLineEdit 
{
  Q_OBJECT
   
  public:
    CValidateLineEdit( TQWidget *parent, int validateType, const char *name=0 );
    ~CValidateLineEdit( void );

  public slots:
    void setData( const TQByteArray &buf );

  private slots:
    void convertText( const TQString &text );

  signals:
    void dataChanged( const TQByteArray &buf );

  private:
    bool mBusy;
    CHexValidator *mValidator;
};


class CConverterDialog : public KDialogBase 
{
  Q_OBJECT
  
  public:
    CConverterDialog( TQWidget *parent, const char *name=0, bool modal=true );
    ~CConverterDialog( void );
    
  protected:
    void showEvent( TQShowEvent *e );

  protected slots:
    virtual void slotUser1( void );
    virtual void slotUser2( void );

  private slots:
    void setData( const TQByteArray &data );

  signals:
    void probeCursorValue( TQByteArray &buf, uint size );

  private:
    CValidateLineEdit *mHexInput;
    CValidateLineEdit *mDecInput;
    CValidateLineEdit *mOctInput;
    CValidateLineEdit *mBinInput;
    CValidateLineEdit *mTxtInput;
};

#endif

