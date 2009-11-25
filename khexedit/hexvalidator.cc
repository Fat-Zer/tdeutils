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

#include <ctype.h>
#include <stdio.h>
#include <qwidget.h>
#include "hexvalidator.h"

CHexValidator::CHexValidator( QWidget *parent, EState state,
			      const char *name )
  :QValidator( parent, name )
{
  setState( state );
}

CHexValidator::~CHexValidator( void )
{
}

QValidator::State CHexValidator::validate( QString &string, int &/*pos*/ ) const
{
  if( mState == hexadecimal )
  {
    for( uint i=0; i < string.length(); i++ )
    {
      int val = string[i].latin1();
      if( isxdigit( val ) == 0 && isspace( val ) == 0 )
      {
	return( QValidator::Invalid );
      }
    }
    return( QValidator::Valid );
  }
  if( mState == decimal )
  {
    for( uint i=0; i < string.length(); i++ )
    {
      int val = string[i].latin1();
      if( isdigit( val ) == 0 && isspace( val ) == 0 )
      {
	return( QValidator::Invalid );
      }
    }
    return( QValidator::Valid );
  }
  else if( mState == octal )
  {
    for( uint i=0; i < string.length(); i++ )
    {
      int val = string[i].latin1();
      if( (isdigit( val ) == 0 || val == '8' || val == '9') &&
	  isspace( val ) == 0 )
      {
	return( QValidator::Invalid );
      }
    }
    return( QValidator::Valid );
  }
  else if( mState == binary )
  {
    for( uint i=0; i < string.length(); i++ )
    {
      int val = string[i].latin1();
      if( val != '0' && val != '1' && isspace( val ) == 0 )
      {
	return( QValidator::Invalid );
      }
    }
    return( QValidator::Valid );
  }
  else if( mState == regularText )
  {
    return( QValidator::Valid );
  }
  else
  {
    return( QValidator::Invalid );
  }

}


void CHexValidator::setState( EState state )
{
  mState = state;
}


void CHexValidator::convert( QByteArray &dest, const QString &src )
{
  uint value;
  uint k=0;

  if( mState == hexadecimal )
  {
    dest.resize(0);

    char buf[3];
    for( uint i=0; i < src.length(); i++ )
    {
      int val = src[i].latin1();
      if( isxdigit(val) )
      {
	buf[k++] = val;
	if( k == 2 )
	{
	  buf[k] = 0;
	  sscanf( buf, "%X", &value );

	  dest.resize( dest.size()+1 );
	  dest[ dest.size()-1 ] = value;
	  k = 0;
	}
      }
    }

    if( k == 1 )
    {
      buf[1] = buf[0];
      buf[0] = '0';
      buf[2] = 0;
      sscanf( buf, "%X", &value );

      dest.resize( dest.size()+1 );
      dest[ dest.size()-1 ] = value;
    }

  }
  else if( mState == decimal )
  {
    dest.resize(0);

    char buf[4];
    for( uint i=0; i < src.length(); i++ )
    {
      int val = src[i].latin1();
      if( isdigit(val) )
      {
	buf[k++] = val;
	if( k == 3 )
	{
	  buf[k] = 0;
	  sscanf( buf, "%u", &value );

	  dest.resize( dest.size()+1 );
	  dest[ dest.size()-1 ] = value;
	  k = 0;
	}
      }
    }

    if( k == 1 || k == 2 )
    {
      if( k == 1 )
      {
	buf[2] = buf[0];
	buf[0] = buf[1] = '0';
      }
      else
      {
	buf[2] = buf[1];
	buf[1] = buf[0];
	buf[0] = '0';
      }
      buf[3] = 0;
      sscanf( buf, "%u", &value );

      dest.resize( dest.size()+1 );
      dest[ dest.size()-1 ] = value;
    }
  }

  else if( mState == octal )
  {
    dest.resize(0);

    char buf[4];
    for( uint i=0; i < src.length(); i++ )
    {
      int val = src[i].latin1();
      if( isdigit(val) )
      {
	buf[k++] = val;
	if( k == 3 )
	{
	  if( buf[0] > '3' ) { buf[0] = '3'; }
	  buf[k] = 0;
	  sscanf( buf, "%o", &value );

	  dest.resize( dest.size()+1 );
	  dest[ dest.size()-1 ] = value;
	  k = 0;
	}
      }
    }

    if( k == 1 || k == 2 )
    {
      if( k == 1 )
      {
	buf[2] = buf[0];
	buf[0] = buf[1] = '0';
      }
      else
      {
	buf[2] = buf[1];
	buf[1] = buf[0];
	buf[0] = '0';
      }
      buf[3] = 0;
      sscanf( buf, "%o", &value );

      dest.resize( dest.size()+1 );
      dest[ dest.size()-1 ] = value;
    }
  }
  else if( mState == binary )
  {
    dest.resize(0);

    char buf[9];
    for( uint i=0; i < src.length(); i++ )
    {
      int val = src[i].latin1();
      if( isdigit(val) )
      {
	buf[k++] = val;
	if( k == 8 )
	{
	  value = 0;
	  for( uint j=0; j < 8; j++ )
	  {
	    value |= (buf[8-j-1] == '1') ? 1<<j : 0;
	  }

	  dest.resize( dest.size()+1 );
	  dest[ dest.size()-1 ] = value;
	  k = 0;
	}
      }
    }

    if( k > 0 )
    {
      value = 0;
      for( uint j=0; j < k; j++ )
      {
	value |= (buf[k-j-1] == '1') ? 1<<j : 0;
      }

      dest.resize( dest.size()+1 );
      dest[ dest.size()-1 ] = value;
    }
  }
  else if( mState == regularText )
  {
    dest.resize(src.length());
    for( uint i=0; i < src.length(); i++ )
    {
      dest[i] = src[i].latin1();
    }

  }
  else
  {
    dest.resize(0);
  }
}



void CHexValidator::format( QString &dest, const QByteArray &src )
{
  for( uint i=0; i<src.size(); ++i )
  {
    unsigned char srcCh = (unsigned char)src[i];
    QString formattedCh;

    switch( mState )
    {
      case hexadecimal:
        formattedCh = zeroExtend( QString::number( srcCh, 16 ), 2 );
        break;
      case decimal:
        formattedCh = zeroExtend( QString::number( srcCh, 10), 3 );
        break;
      case octal:
        formattedCh = zeroExtend( QString::number( srcCh, 8), 3 );
        break;
      case binary:
        formattedCh = zeroExtend( QString::number( srcCh, 2), 8 );
        break;
      case regularText:
        formattedCh = QString( QChar( srcCh ) );
        break;
    }

    dest += formattedCh + " ";
  }
}


QString CHexValidator::zeroExtend( const QString &src, unsigned destLen) const
{
  if( src.length() >= destLen )
    return src;

  QString zeroes;
  zeroes.fill( '0', destLen - src.length() );
  return zeroes + src;
}
#include "hexvalidator.moc"
