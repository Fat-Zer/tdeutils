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

#ifndef _FILE_INFO_DIALOG_H_
#define _FILE_INFO_DIALOG_H_


#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

class TQLabel;

#include <kdialogbase.h>

class CListView;
#include "hexbuffer.h"

class CFileInfoDialog : public KDialogBase
{
  Q_OBJECT
  TQ_OBJECT
  
  public:
    CFileInfoDialog( TQWidget *tqparent=0, const char *name=0,bool modal=false );
    ~CFileInfoDialog( void );

    void setStatistics( void );
    void setStatistics( SStatisticControl &sc );

  public slots:
    void setDirty( void );
    void setClean( void );

  protected slots:
    virtual void slotUser1( void );

  protected:
    virtual void resizeEvent( TQResizeEvent * );
    virtual void showEvent( TQShowEvent * );
    virtual void timerEvent( TQTimerEvent * );

  private:
    void setColumnWidth( void );

  signals:
    void collectStatistic( SStatisticControl &sc );

  private:
    bool mBusy;
    bool mDirty;
    CListView *mFrequencyList;
    TQLabel *mFileNameLabel;
    TQLabel *mFileSizeLabel;
    TQLabel *mDirtyLabel;
};

#endif
