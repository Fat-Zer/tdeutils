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
#include <qlayout.h>
#include "hexmanagerwidget.h"
#include "searchbar.h"

CHexManagerWidget::CHexManagerWidget( QWidget *parent, const char *name,
				      EConversionPosition conversionPosition,
				      EPosition tabBarPosition,
				      EPosition searchBarPosition )
  : QWidget( parent, name )
{
  mValid = false;

  mEditor    = new CHexEditorWidget( this );
  mTabBar    = new CTabBar( this );
  mTabBar->hide();
  mSearchBar = 0;

  mConverter = new CHexToolWidget( this );

  connect( mEditor->view(), SIGNAL(fileName(const QString &, bool)),
	   this, SLOT( addName(const QString &)));
  connect( mEditor->view(), SIGNAL( fileClosed(const QString &)),
	   this, SLOT( removeName(const QString &)));
  connect( mEditor->view(),SIGNAL(fileRename(const QString &,const QString &)),
	   this, SLOT(changeName(const QString &,const QString &)));
  connect( mEditor->view(), SIGNAL( cursorChanged( SCursorState & ) ),
	   mConverter, SLOT( cursorChanged( SCursorState & ) ) );
  connect( mConverter, SIGNAL( closed(void) ),
	   this, SIGNAL( conversionClosed(void) ) );
  connect( mTabBar, SIGNAL(selected(const QString &)),
	   this, SLOT(open(const QString &)));

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

  delete layout();
  QVBoxLayout *vlay = new QVBoxLayout( this, 0, 0 );

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
    QPoint point = mapToGlobal( QPoint(0,0) );
    QRect  rect  = geometry();
    QPoint p;

    p.setX(point.x() + rect.width()/2 - mConverter->minimumSize().width()/2);
    p.setY(point.y() + rect.height()/2 - mConverter->minimumSize().height()/2);
    mConverter->resize( mConverter->minimumSize() );
    mConverter->reparent( 0, WStyle_Customize | WStyle_DialogBorder, p, true );
    mConverter->setCaption(kapp->makeStdCaption(i18n("Conversion")));
  }
  else
  {
    mConversionPosition = Embed;
    uint utilHeight = mConverter->minimumSize().height();
    QPoint p( 0, height() - utilHeight );
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
      mTabBar->setShape( QTabBar::RoundedAbove );
    }
    else
    {
      mTabBar->setShape( QTabBar::RoundedBelow );
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
      connect( mSearchBar, SIGNAL(hidden()), this, SLOT(searchBarHidden()) );
      connect( mSearchBar, SIGNAL(findData(SSearchControl &, uint, bool)),
	       mEditor, SLOT(findData(SSearchControl &, uint, bool)) );
      connect( editor()->view(), SIGNAL( cursorChanged( SCursorState & ) ),
	       mSearchBar, SLOT( cursorMoved() ) );
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


void CHexManagerWidget::addName( const QString &name )
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


void CHexManagerWidget::removeName( const QString &name )
{
  mTabBar->removeName( name );
  if( mTabBar->isVisible() == true && mTabBar->count() == 0 )
  {
    setTabBarPosition( mTabPosition );
  }
}


void CHexManagerWidget::changeName( const QString &curName,
				    const QString &newName )
{
  mTabBar->changeName( curName, newName );
}


void CHexManagerWidget::open( const QString &name )
{
  mEditor->open( name, false, 0 );
}


int CHexManagerWidget::preferredWidth( void )
{
  int w = mEditor->defaultTextWidth();
  if( mConversionPosition == Embed )
  {
    int converterWidth = mConverter->sizeHint().width();
    w = QMAX( w, converterWidth );
  }
  return( w );
}



CTabBar::CTabBar( QWidget *parent, char *name )
  :QTabBar( parent, name )
{
  connect( this, SIGNAL(selected(int)), this, SLOT(slotSelected(int)) );
}


void CTabBar::addName( const QString &name )
{
  QString n( name.right(name.length()-name.findRev('/')-1) );

  QTab *t = find( n );
  if( t == 0 )
  {
    t = new QTab();
    t->setText( n);
    int id = addTab( t );
    mFileList.append( CFileKey(name,id) );
  }
  setCurrentTab(t);
}


void CTabBar::removeName( const QString &name )
{
  QString n( name.right(name.length()-name.findRev('/')-1) );
  QTab *t = find(n);
  if( t == 0 )
  {
    return;
  }

  QValueList<CFileKey>::Iterator it;
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


void CTabBar::changeName( const QString &curName, const QString &newName )
{
  QString n( curName.right(curName.length()-curName.findRev('/')-1) );
  QTab *t = find(n);
  if( t == 0 )
  {
    return;
  }

  QValueList<CFileKey>::Iterator it;
  for( it = mFileList.begin(); it != mFileList.end(); ++it )
  {
    if( (*it).id() == t->identifier() )
    {
      QString m( newName.right(newName.length()-newName.findRev('/')-1) );
      t->setText(m);

      mFileList.remove(it);
      mFileList.append( CFileKey(newName,t->identifier()) );
      layoutTabs();
      update(); // Seems to be necessary
      break;
    }
  }
}


QTab *CTabBar::find( const QString &name )
{
  QPtrList<QTab> &list = *tabList();
  for( QTab *t = list.first(); t != 0; t = list.next() )
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
  QValueList<CFileKey>::Iterator it;
  for( it = mFileList.begin(); it != mFileList.end(); ++it )
  {
    if( (*it).id() == id )
    {
      emit selected( (*it).filename() );
    }
  }
}




#include "hexmanagerwidget.moc"
