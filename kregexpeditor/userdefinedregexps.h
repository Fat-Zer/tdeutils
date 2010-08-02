/*
 *  Copyright (c) 2002-2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/
#ifndef __USERDEFINEDREGEXPS_H
#define __USERDEFINEDREGEXPS_H
#include <tqdockwindow.h>
#include <tqlistview.h>

#include "compoundregexp.h"

class TQPoint;
class RegExp;

class UserDefinedRegExps :public QDockWindow
{
Q_OBJECT

public:
  UserDefinedRegExps( TQWidget *parent, const char *name = 0 );
  const TQPtrList<CompoundRegExp> regExps() const;

public slots:
  void slotSelectNewAction();

protected slots:
  void slotLoad(TQListViewItem* item);
  void slotEdit( TQListViewItem* item, const TQPoint& pos );
  void slotPopulateUserRegexps();
  void slotUnSelect();

protected:
  void createItems( const TQString& title, const TQString& dir, bool usersRegExp );

signals:
  void load( RegExp* );

private:
  TQListView* _userDefined;
  TQPtrList<CompoundRegExp> _regExps;
};

class WidgetWinItem :public QListViewItem
{
public:
  WidgetWinItem( TQString name, RegExp* regexp, bool users, TQListViewItem* parent );
  static TQString path();

  TQString fileName() const;
  RegExp* regExp() const;
  TQString name() const;
  void setName( const TQString& );
  bool isUsersRegExp() const { return _usersRegExp; }

private:
  TQString _name;
  RegExp* _regexp;
  bool _usersRegExp;
};


#endif // __USERDEFINEDREGEXPS_H
