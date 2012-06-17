/*

  ark -- archiver for the KDE project

  Copyright (C)

  2004: Henrique Pinto <henrique.pinto@kdemail.net>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/
#ifndef SEVENZIP_H
#define SEVENZIP_H 

#include "arch.h"

class SevenZipArch : public Arch
{
  Q_OBJECT
  
  public:
    SevenZipArch( ArkWidget *, const TQString & );
    virtual ~SevenZipArch();
  
    virtual void open();
    virtual void create();
    virtual void test();

    virtual void addFile( const TQStringList & );
    virtual void addDir( const TQString & );

    virtual void remove( TQStringList * );
    virtual void unarchFileInternal( );
    virtual bool passwordRequired();
    virtual void createPassword();

  protected slots:
    virtual bool processLine( const TQCString& line );
    virtual void slotReceivedTOC( KProcess*, char*, int );

  private:
    void setHeaders();
    int m_nameColumnPos;
};

#endif // SEVENZIP_H
