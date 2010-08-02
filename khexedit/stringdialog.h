/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999-2000 Espen Sand, espensa@online.no
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

#ifndef _STRING_DIALOG_H_
#define _STRING_DIALOG_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

class TQCheckBox;
class TQLabel;
class TQLineEdit;
class TQListView;
class TQSpinBox;

#include <tqregexp.h> 
#include <kdialogbase.h>

class CListView;
#include "hexbuffer.h"
#include "progress.h"

class CStringDialog : public KDialogBase
{
  Q_OBJECT
  
  public:
    CStringDialog( TQWidget *parent=0, const char *name = 0, bool modal=false );
    ~CStringDialog( void );

    int  updateList( CProgress &p ); 
    inline CStringCollectControl &stringData( void );

  signals:
    void markText( uint offset, uint size, bool moveCursor );
    void collect( void );

  public slots:
    void clearList( void );  // Removes list entries
    void removeList( void ); // Removes list entries and the string list.
    void setDirty( void );
    void setClean( void );

  protected slots:
    virtual void slotUser1( void );
    virtual void startGoto( TQListViewItem * );
    virtual void selectionChanged( );

  protected:
    virtual void timerEvent( TQTimerEvent *e );
    virtual void resizeEvent( TQResizeEvent *e );
    virtual void showEvent( TQShowEvent *e );

  private:
    void enableList( bool state );
    void appendListItem( const TQString &str, uint offsetSize );
    void updateListInfo( void );
    void setColumnWidth( void );
    void readConfiguration( void );
    void writeConfiguration( void );

  private:
    CStringCollectControl mStringData;
    TQRegExp mRegExp;
    uint mMaxLength;
    bool mBusy;
    bool mDirty;

    TQSpinBox  *mLengthSpin;
    TQLineEdit *mFilter;
    TQCheckBox *mIgnoreCaseCheck;
    TQCheckBox *mOffsetDecimalCheck;
    CListView *mStringList;
    TQLabel    *mListSizeLabel;
    TQLabel    *mDisplaySizeLabel;
    TQLabel    *mDirtyLabel;
};


CStringCollectControl &CStringDialog::stringData( void )
{
  return( mStringData );
}

#endif







