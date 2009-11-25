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

#ifndef _HEX_VALIDATOR_H_
#define _HEX_VALIDATOR_H_

#include <qvalidator.h> 

class CHexValidator: public QValidator
{
  Q_OBJECT
  
  public:
    enum EState
    {
      hexadecimal = 0,
      decimal,
      octal,
      binary,
      regularText
    };

  public:
    CHexValidator( QWidget *parent, EState state, const char *name = 0 );
    ~CHexValidator( void );
    QValidator::State validate( QString &string, int &pos ) const;
    void setState( EState state );
    void convert( QByteArray &dest, const QString &src );
    void format( QString &dest, const QByteArray &src );

  private:
    /**
     * Returns a string that is at least as long as @p destLen number of characters,
     * by adding zeroes to the left as necessary.
     *
     * e.g. zeroExtend( "32", 3 ) => "032"
     */
    QString zeroExtend( const QString &src, unsigned destLen ) const;

    EState mState;


};


#endif
