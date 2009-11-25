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

#ifndef _BITSWAP_WIDGET_H_
#define _BITSWAP_WIDGET_H_

#include <qlabel.h>
#include <qlayout.h> 
#include <qstring.h> 


class CDigitLabel : public QLabel
{
  Q_OBJECT

  public:
    CDigitLabel( QWidget *parent, uint digit = 0, const char *name=0 );
    ~CDigitLabel( void );

    virtual QSize sizeHint() const;
    inline uint value( void );

  signals:
    void stepCell( const QObject *obj, bool next );
    void valueChanged( const QObject *obj, uint newVal, bool after );

  public slots:
    void setValue( uint digit, bool notify );
    void setDotPosition( uint dotPosition );

  protected:  
    void paletteChange( const QPalette & );
    void drawContents( QPainter * );
    virtual void keyPressEvent( QKeyEvent *e );

  private:
    void initialize( void );

  private:
    uint mDigit;
    uint mDotPosition;
};


inline uint CDigitLabel::value( void )
{
  return( mDigit );
}




class CByteWidget : public QWidget
{
  Q_OBJECT

  public:
    CByteWidget( QWidget *parent, const char *name=0 );
    ~CByteWidget( void );

    bool flag( QByteArray &buf );

  public slots:
    void reset( void );

  private slots:
    void stepCell( const QObject *obj, bool next );
    void valueChanged( const QObject *obj, uint newVal, bool after );
    
  private:
    void setBuddy( const QObject *obj );

  private:
    QHBoxLayout *mHBox;
    CDigitLabel *mDigit[8];
};







#endif
