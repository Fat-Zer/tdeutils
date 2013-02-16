/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 2000 Espen Sand, espensa@online.no
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


#include <tdelocale.h>

#include "dialog.h"
#include "hexvalidator.h"
#include "searchbar.h"
#include <tqpushbutton.h>

// crappy X11 headers
#undef KeyPress

static const char * close_xpm[] = {
"16 16 3 1",
"       s None  c None",
".      c #ffffff",
"X      c #707070",
"                ",
"                ",
"  .X        .X  ",
"  .XX      .XX  ",
"   .XX    .XX   ",
"    .XX  .XX    ",
"     .XX.XX     ",
"      .XXX      ",
"      .XXX      ",
"     .XX.XX     ",
"    .XX  .XX    ",
"   .XX    .XX   ",
"  .XX      .XX  ",
"  .X        .X  ",
"                ",
"                "};

CSearchBar::CSearchBar( TQWidget *parent, const char *name, WFlags f )
  :TQFrame( parent, name, f )
{
  setFrameStyle( TQFrame::Panel | TQFrame::Raised );
  setLineWidth( 1 );

  mTypeCombo = new TQComboBox( this );
  connect( mTypeCombo, TQT_SIGNAL(activated(int)), TQT_SLOT(selectorChanged(int)) );
  TQStringList list;
  list << i18n("Hex") << i18n("Dec") << i18n("Oct") << i18n("Bin")
       << i18n("Txt");
  mTypeCombo->insertStringList( list );

  mInputEdit = new TQLineEdit( this );
  connect( mInputEdit, TQT_SIGNAL(textChanged(const TQString&)),
	   TQT_SLOT(textChanged(const TQString&)) );
  mValidator = new CHexValidator( this, CHexValidator::regularText );
  mInputEdit->setValidator( mValidator );

  mFindButton = new TQPushButton( i18n("Find"), this );
  mFindButton->setAutoDefault(false);
  connect( mFindButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(start()) );
  connect(mInputEdit,TQT_SIGNAL(returnPressed()),mFindButton,TQT_SLOT(animateClick()));
  mFindButton->setFixedHeight( mTypeCombo->sizeHint().height() );

  mBackwards = new TQCheckBox( i18n("Backwards"), this );
  mIgnoreCase = new TQCheckBox( i18n("Ignore case"), this );

  mCloseButton = new TQPushButton( this );
  mCloseButton->setAutoDefault(false);
  mCloseButton->setPixmap( TQPixmap( close_xpm ) );
  connect( mCloseButton, TQT_SIGNAL(clicked()), this, TQT_SLOT(hideWidget()) );

  //
  // Make layout
  //
  TQHBoxLayout *hlay = new TQHBoxLayout( this, 4, 6 );
  hlay->addWidget( mTypeCombo );
  hlay->addWidget( mInputEdit );
  hlay->addWidget( mFindButton );
  hlay->addWidget( mBackwards );
  hlay->addWidget( mIgnoreCase );
  hlay->addWidget( mCloseButton );

  //
  // Read below why I do this.
  //
  mInputEdit->installEventFilter( this );
  selectorChanged(0);
}


//
// Espen 2000-04-21
// TQt 2.1: Seems like the TQLineEdit::returnPressed() does not work when
// I install a validator. So I catch the event manually
//
bool CSearchBar::eventFilter( TQObject *o, TQEvent *e )
{
  if( TQT_BASE_OBJECT(o) == TQT_BASE_OBJECT(mInputEdit) && e->type() == TQEvent::KeyPress )
  {
    TQKeyEvent *ke = (TQKeyEvent*)e;
    if( ke->key() == Key_Return )
    {
      mFindButton->animateClick();
      return true;
    }
  }
  return TQFrame::eventFilter( o, e );
}

//
// Seach for te first item each time the curso has moved. Note: The cursor
// will move when we search and get a match, but (in start() below) the
// mSearchMode is set to Find_Next after this slot has been called so it
// will work.
//
void CSearchBar::cursorMoved()
{
  mSearchMode = Find_First;
}


void CSearchBar::selectorChanged( int index )
{
  mValidator->setState( (CHexValidator::EState)index );
  mInputEdit->setText( mFindString[ index ] );
  mIgnoreCase->setEnabled( index == 4 ? true : false );
  mSearchMode = Find_First;
}


void CSearchBar::textChanged( const TQString &text )
{
  mFindString[ mTypeCombo->currentItem() ] = text;
  mValidator->convert( mFindData, mFindString[ mTypeCombo->currentItem() ] );
  mSearchMode = Find_First;
}


void CSearchBar::hideWidget()
{
  hide();
  emit hidden();
}


void CSearchBar::start( void )
{
  if( mFindData.isEmpty() == true )
  {
    showEntryFailure( this, TQString("") );
    return;
  }

  SSearchControl sc;
  sc.key = mFindData;
  sc.keyType = mTypeCombo->currentItem();
  sc.fromCursor = true;
  sc.inSelection = false;
  sc.forward = !mBackwards->isChecked();
  sc.ignoreCase = mIgnoreCase->isEnabled() && mIgnoreCase->isChecked();
  emit findData( sc, mSearchMode, false );
  mSearchMode = Find_Next;
}


void CSearchBar::showEvent( TQShowEvent *e )
{
  TQFrame::showEvent(e);
  mInputEdit->setFocus();
}


#include "searchbar.moc"
