/*
 
 ark -- archiver for the KDE project

 Copyright (C)

 1997-1999: Rob Palmbos palm9744@kettering.edu
 1999: Francois-Xavier Duranceau duranceau@kde.org
 1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
 2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)

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
#ifndef LHA_H
#define LHA_H

#include "arch.h"

class TQString;
class TQCString;
class TQStringList;

class ArkWidget;

class LhaArch : public Arch
{
  Q_OBJECT
  public:
    LhaArch( ArkWidget *, const TQString & );
    virtual ~LhaArch() { }

    virtual void open();
    virtual void create();

    virtual void addFile( const TQStringList & );
    virtual void addDir( const TQString & );

    virtual void remove( TQStringList * );
    virtual void unarchFileInternal();

  protected slots:
    virtual bool processLine( const TQCString &line );

  private:
    void setHeaders();
};

#endif // LHA_H
