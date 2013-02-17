/*
 * mntconfig.h
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


#ifndef __MNTCONFIG_H__
#define __MNTCONFIG_H__

#include <tdecmodule.h>
#include <tdeconfig.h>
#include <kiconloader.h>

#include "disks.h"
#include "disklist.h"

class TQGroupBox;
class TQPushButton;
class TQRadioButton;
class TQLineEdit;
class TDEIconButton;

class CListView;
class TQListViewItem;

/**************************************************************/

class MntConfigWidget : public TQWidget
{
  Q_OBJECT
  

  public:
    enum ColType
    {
      ICONCOL=0,
      DEVCOL=1,
      MNTPNTCOL=2,
      MNTCMDCOL=3,
      UMNTCMDCOL=4
    };

  public:
    MntConfigWidget( TQWidget *parent=0, const char *name=0, bool init=false );
    ~MntConfigWidget();

  public slots:
    void loadSettings( void );
    void applySettings( void );

  protected slots:
    void slotChanged();

  private slots:
    void readDFDone( void );
    void clicked( TQListViewItem *item );
    void selectMntFile( void );
    void selectUmntFile( void );
    void iconChangedButton(TQString);
    void iconChanged( const TQString & );
    void mntCmdChanged( const TQString & );
    void umntCmdChanged( const TQString & );

  protected:       
    void closeEvent( TQCloseEvent * );

  private:
    CListView   *mList;
    TQGroupBox   *mGroupBox;
    TQLineEdit   *mIconLineEdit;
    TQLineEdit   *mMountLineEdit;
    TQLineEdit   *mUmountLineEdit;
    TQPushButton *mMountButton;
    TQPushButton *mUmountButton;
    TDEIconButton *mIconButton;
    DiskList    mDiskList;
    bool        mInitializing;
    TQMemArray<TQListViewItem*> mDiskLookup;

  signals:
    void configChanged();
};


#endif
