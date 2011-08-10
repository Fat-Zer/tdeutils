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

#ifndef _HEX_MANAGER_WIDGET_H_
#define _HEX_MANAGER_WIDGET_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 

#include <tqtabbar.h>
#include <tqvaluelist.h> 
#include <tqwidget.h>

#include "hexeditorwidget.h"
#include "hextoolwidget.h"

class CSearchBar;


class CFileKey
{
  public:
    CFileKey( const TQString &fname, int fid )
    {
      mFilename = fname;
      mId       = fid;
    }

    CFileKey( const CFileKey & k ) 
    {
      mFilename = k.mFilename;
      mId       = k.mId;
    }

    CFileKey( void ) 
    {
    }

    TQString filename( void ) const 
    { 
      return( mFilename );
    }

    int id() const               
    { 
      return( mId );
    }

  private:
    TQString mFilename;
    int     mId;
};




class CTabBar : public TQTabBar
{
  Q_OBJECT
  TQ_OBJECT

  public:
    CTabBar( TQWidget *parent=0, char *name=0 );
    void addName( const TQString &name );
    void removeName( const TQString &name );
    void changeName( const TQString &curName, const TQString &newName );
    int  count( void );

  protected slots:
    void slotSelected( int id );

  private:
    TQTab *find( const TQString &name );

  private:
    TQValueList<CFileKey> mFileList;

  signals:
    void selected( const TQString &filename );
};


class CHexManagerWidget : public TQWidget
{
  Q_OBJECT
  TQ_OBJECT

  public:
    enum EConversionPosition
    {
      Hide,
      Float,
      Embed
    };

    enum EPosition
    {
      HideItem = 0,
      AboveEditor,
      BelowEditor
    };      

  public:
    CHexManagerWidget( TQWidget *parent = 0, const char *name = 0, 
		       EConversionPosition state = Embed,
		       EPosition tabBarPosition = HideItem,
		       EPosition searchBarPosition = HideItem );
    ~CHexManagerWidget( void );

    inline bool isValid( void );
    inline CHexEditorWidget *editor( void );
    inline CHexToolWidget *converter( void );
    inline EConversionPosition conversionPosition( void );
    inline EPosition tabBarPosition( void );
    inline EPosition searchBarPosition( void );

    int preferredWidth( void );

  public slots:
    void setConversionVisibility( EConversionPosition position );
    void setTabBarPosition( EPosition position );
    void setSearchBarPosition( EPosition position );
    void addName( const TQString &name );
    void removeName( const TQString &name );
    void changeName( const TQString &curName, const TQString &newName );
    
  protected slots:
    void open( const TQString &name );
    void searchBarHidden( void );
    
  private:
    void updateLayout( void );

  signals:
    void conversionClosed( void );
    void searchBarClosed( void );

  private:
    bool mValid;
    bool mEnableTabBar;
    EConversionPosition mConversionPosition;
    EPosition        mTabPosition;
    EPosition        mSearchBarPosition;
    CHexEditorWidget *mEditor;
    CTabBar          *mTabBar;
    CHexToolWidget   *mConverter;
    CSearchBar       *mSearchBar;
};


inline bool CHexManagerWidget::isValid( void )
{
  return( mValid );
}

inline CHexEditorWidget *CHexManagerWidget::editor( void )
{
  return( mEditor );
}

inline CHexToolWidget *CHexManagerWidget::converter( void )
{
  return( mConverter );
}

inline CHexManagerWidget::EConversionPosition 
  CHexManagerWidget::conversionPosition( void )
{
  return( mConversionPosition );
}

inline CHexManagerWidget::EPosition CHexManagerWidget::tabBarPosition( void )
{
  return( mTabPosition );
}

inline CHexManagerWidget::EPosition CHexManagerWidget::searchBarPosition(void)
{
  return( mSearchBarPosition );
}




#endif

