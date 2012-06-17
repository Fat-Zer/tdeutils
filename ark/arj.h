/*

  ark -- archiver for the KDE project

  Copyright (C)

  1997-1999: Rob Palmbos palm9744@kettering.edu
  1999: Francois-Xavier Duranceau duranceau@kde.org
  1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
  2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)
  2007: ALT Linux (author: Sergey V Turchin, zerg@altlinux.org)

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

#ifndef ARJARCH_H
#define ARJARCH_H

#include "arch.h"

class TQString;
class TQStringList;

class ArkWidget;

class ArjArch : public Arch
{
  Q_OBJECT
  public:
    ArjArch( ArkWidget *_gui, const TQString & _fileName );
    virtual ~ArjArch() { }

    virtual void open();
    virtual void create();
    virtual void test();

    virtual void remove(TQStringList*);
    virtual void addFile(const TQStringList&);
    virtual void addDir(const TQString&);

    virtual void unarchFileInternal();
    virtual bool passwordRequired();
    virtual void createPassword();

  protected slots:
    virtual bool processLine( const TQCString & );
  private:
    TQCString file_entry;
    void setHeaders();
};

#endif /* ARJARCH_H */
