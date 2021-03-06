/*
 * kdfconfig.h
 *
 * Copyright (c) 1999 Michael Kropfberger <michael.kropfberger@gmx.net>
 *
 * Requires the TQt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#ifndef __KDFCONFIG_H__
#define __KDFCONFIG_H__

#include <tqmemarray.h>
#include <tqstring.h>

#include "stdoption.h"

class TQCheckBox;
class TQLabel;
class TQLCDNumber;
class TQLineEdit;
class TQListViewItem;
class TQScrollBar;

class CListView;

class KDFConfigWidget : public TQWidget
{
  Q_OBJECT
  

  class CTabName
  {
    public:
      CTabName( const TQString &res, const TQString &name )
      {
        mRes     = res;
        mName    = name;
      }
      CTabName( void ) { }
      ~CTabName( void ) { }

      TQString mRes;
      TQString mName;
  };

  public:
    KDFConfigWidget( TQWidget *parent=0, const char *name=0, bool init=false);
    ~KDFConfigWidget();

  public slots:
    void loadSettings( void );
    void applySettings( void );
    void defaultsBtnClicked( void );

  protected slots:
    void slotChanged();

  private slots:
    void toggleListText( TQListViewItem *item, const TQPoint &, int column );

  protected:
    void closeEvent( TQCloseEvent * );

  private:
    TQMemArray<CTabName*> mTabName;
    CListView  *mList;
    TQScrollBar *mScroll;
    TQLCDNumber *mLCD;
    TQLineEdit  *mFileManagerEdit;
    TQCheckBox  *mOpenMountCheck;
    TQCheckBox  *mPopupFullCheck;
    CStdOption mStd;

  signals:
    void configChanged();
};


#endif



