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

#include <klocale.h>
#include <tqlayout.h>
#include "hexmanagerwidget.h"
#include "searchbar.h"

CHexManagerWidget::CHexManagerWidget( TQWidget *tqparent, const char *name,
				      EConversionPosition conversionPosition,
				      EPosition tabBarPosition,
				      EPosition searchBarPosition )
  : TQWidget( tqparent, name )
{
  mValid = false;

  mEditor    = new CHexEditorWidget( this );
  mTabBar    = new CTabBar( this );
  mTabBar->hide();
  mSearchBar = 0;

  mConverter = new CHexToolWidget( this );

  connect( mEditor->view(), TQT_SIGNAL(fileName(const TQString &, bool)),
	   this, TQT_SLOT( addName(const TQString &)));
  connect( mEditor->view(), TQT_SIGNAL( fileClosed(const TQString &)),
	   this, TQT_SLOT( removeName(const TQString &)));
  connect( mEditor->view(),TQT_SIGNAL(fileRename(const TQString &,const TQString &)),
	   this, TQT_SLOT(changeName(const TQString &,const TQString &)));
  connect( mEditor->view(), TQT_SIGNAL( cursorChanged( SCursorState & ) ),
	   mConverter, TQT_SLOT( cursorChanged( SCursorState & ) ) );
  connect( mConverter, TQT_SIGNAL( closed(void) ),
	   this, TQT_SIGNAL( conversionClosed(void) ) );
  connect( mTabBar, TQT_SIGNAL(selected(const TQString &)),
	   this, TQT_SLOT(open(const TQString &)));

  mValid = true;
  setConversionVisibility( conversionPosition );
  setTabBarPosition( tabBarPosition );
  setSearchBarPosition( searchBarPosition );
}


CHexManagerWidget::~CHexManagerWidget( void )
{
  delete mEditor;
  delete mTabBar;
  delete mConverter;
}


void CHexManagerWidget::updateLayout( void )
{
  if( mValid == false ) { return; }

  delete tqlayout();
  TQVBoxLayout *vlay = new TQVBoxLayout( this, 0, 0 );

  if( mSearchBar && mSearchBarPosition == AboveEditor )
  {
    vlay->addWidget( mSearchBar );
  }

  if( mTabPosition == AboveEditor )
  {
    vlay->addWidget( mTabBar );
    vlay->addWidget( mEditor, 1 );
  }
  else
  {
    vlay->addWidget( mEditor, 1 );
    vlay->addWidget( mTabBar );
  }

  if( mSearchBar && mSearchBarPosition == BelowEditor )
  {
    vlay->addWidget( mSearchBar );
  }

  if( mConversionPosition == Embed )
  {
    vlay->addWidget( mConverter );
  }
  vlay->activate(); // Required in this case
}


void CHexManagerWidget::setConversionVisibility( EConversionPosition position )
{
  if( mValid == false )
  {
    return;
  }

  if( mConversionPosition == position )
  {
    if( mConversionPosition == Float )
    {
      mConverter->raise();
    }
    return;
  }

  mConversionPosition = position;
  if( mConversionPosition == Hide )
  {
    mConverter->hide();
  }
  else if( mConversionPosition == Float )
  {
    TQPoint point = mapToGlobal( TQPoint(0,0) );
    TQRect  rect  = tqgeometry();
    TQPoint p;

    p.setX(point.x() + rect.width()/2 - mConverter->tqminimumSize().width()/2);
    p.setY(point.y() + rect.height()/2 - mConverter->tqminimumSize().height()/2);
    mConverter->resize( mConverter->tqminimumSize() );
    mConverter->reparent( 0, WStyle_Customize | WStyle_DialogBorder, p, true );
    mConverter->setCaption(kapp->makeStdCaption(i18n("Conversion")));
  }
  else
  {
    mConversionPosition = Embed;
    uint utilHeight = mConverter->tqminimumSize().height();
    TQPoint p( 0, height() - utilHeight );
    mConverter->reparent( this, 0, p, true );
  }

  updateLayout();
}


void CHexManagerWidget::setTabBarPosition( EPosition position )
{
  mTabPosition = position;
  if( mTabPosition != HideItem && mTabBar->count() > 0 )
  {
    if( mTabPosition == AboveEditor )
    {
      mTabBar->setShape( TQTabBar::RoundedAbove );
    }
    else
    {
      mTabBar->setShape( TQTabBar::RoundedBelow );
    }
    mTabBar->show();
  }
  else
  {
    mTabBar->hide();
  }

  updateLayout();
}


void CHexManagerWidget::setSearchBarPosition( EPosition position )
{
  mSearchBarPosition = position;
  if( position != HideItem )
  {
    if( mSearchBar == 0 )
    {
      mSearchBar = new CSearchBar( this );
      connect( mSearchBar, TQT_SIGNAL(hidden()), this, TQT_SLOT(searchBarHidden()) );
      connect( mSearchBar, TQT_SIGNAL(findData(SSearchControl &, uint, bool)),
	       mEditor, TQT_SLOT(findData(SSearchControl &, uint, bool)) );
      connect( editor()->view(), TQT_SIGNAL( cursorChanged( SCursorState & ) ),
	       mSearchBar, TQT_SLOT( cursorMoved() ) );
    }
    mSearchBar->show();
  }
  else
  {
    if( mSearchBar != 0 )
    {
      mSearchBar->hide();
    }
  }

  updateLayout();
}


void CHexManagerWidget::searchBarHidden( void )
{
  updateLayout();
  mSearchBarPosition = HideItem;
  emit searchBarClosed();
}


void CHexManagerWidget::addName( const TQString &name )
{
  if( name.isEmpty() == true )
  {
    return;
  }

  mTabBar->addName( name );
  if( mTabBar->isVisible() == false && mTabPosition != HideItem )
  {
    setTabBarPosition( mTabPosition );
  }
}


void CHexManagerWidget::removeName( const TQString &name )
{
  mTabBar->removeName( name );
  if( mTabBar->isVisible() == true && mTabBar->count() == 0 )
  {
    setTabBarPosition( mTabPosition );
  }
}


void CHexManagerWidget::changeName( const TQString &curName,
				    const TQString &newName )
{
  mTabBar->changeName( curName, newName );
}


void CHexManagerWidget::open( const TQString &name )
{
  mEditor->open( name, false, 0 );
}


int CHexManagerWidget::preferredWidth( void )
{
  int w = mEditor->defaultTextWidth();
  if( mConversionPosition == Embed )
  {
    int converterWidth = mConverter->tqsizeHint().width();
    w = TQMAX( w, converterWidth );
  }
  return( w );
}



CTabBar::CTabBar( TQWidget *tqparent, char *name )
  :TQTabBar( tqparent, name )
{
  connect( this, TQT_SIGNAL(selected(int)), this, TQT_SLOT(slotSelected(int)) );
}


void CTabBar::addName( const TQString &name )
{
  TQString n( name.right(name.length()-name.tqfindRev('/')-1) );

  TQTab *t = tqfind( n );
  if( t == 0 )
  {
    t = new TQTab();
    t->setText( n);
    int id = addTab( t );
    mFileList.append( CFileKey(name,id) );
  }
  setCurrentTab(t);
}


void CTabBar::removeName( const TQString &name )
{
  TQString n( name.right(name.length()-name.tqfindRev('/')-1) );
  TQTab *t = tqfind(n);
  if( t == 0 )
  {
    return;
  }

  TQValueList<CFileKey>::Iterator it;
  for( it = mFileList.begin(); it != mFileList.end(); ++it )
  {
    if( (*it).id() == t->identifier() )
    {
      mFileList.remove(it);
      removeTab(t);
      layoutTabs();
      break;
    }
  }
}


void CTabBar::changeName( const TQString &curName, const TQString &newName )
{
  TQString n( curName.right(curName.length()-curName.tqfindRev('/')-1) );
  TQTab *t = tqfind(n);
  if( t == 0 )
  {
    return;
  }

  TQValueList<CFileKey>::Iterator it;
  for( it = mFileList.begin(); it != mFileList.end(); ++it )
  {
    if( (*it).id() == t->identifier() )
    {
      TQString m( newName.right(newName.length()-newName.tqfindRev('/')-1) );
      t->setText(m);

      mFileList.remove(it);
      mFileList.append( CFileKey(newName,t->identifier()) );
      layoutTabs();
      update(); // Seems to be necessary
      break;
    }
  }
}


TQTab *CTabBar::tqfind( const TQString &name )
{
  TQPtrList<TQTab> &list = *tabList();
  for( TQTab *t = list.first(); t != 0; t = list.next() )
  {
    if( t->text() == name )
    {
      return( t );
    }
  }

  return( 0 );
}


int CTabBar::count( void )
{
  return( tabList()->count() );
}


void CTabBar::slotSelected( int id )
{
  TQValueList<CFileKey>::Iterator it;
  for( it = mFileList.begin(); it != mFileList.end(); ++it )
  {
    if( (*it).id() == id )
    {
      emit selected( (*it).filename() );
    }
  }
}




#include "hexmanagerwidget.moc"
