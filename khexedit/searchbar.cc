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


#include <klocale.h>

#include "dialog.h"
#include "hexvalidator.h"
#include "searchbar.h"
#include <qpushbutton.h>

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

CSearchBar::CSearchBar( QWidget *parent, const char *name, WFlags f )
  :QFrame( parent, name, f )
{
  setFrameStyle( QFrame::Panel | QFrame::Raised );
  setLineWidth( 1 );

  mTypeCombo = new QComboBox( this );
  connect( mTypeCombo, SIGNAL(activated(int)), SLOT(selectorChanged(int)) );
  QStringList list;
  list << i18n("Hex") << i18n("Dec") << i18n("Oct") << i18n("Bin")
       << i18n("Txt");
  mTypeCombo->insertStringList( list );

  mInputEdit = new QLineEdit( this );
  connect( mInputEdit, SIGNAL(textChanged(const QString&)),
	   SLOT(textChanged(const QString&)) );
  mValidator = new CHexValidator( this, CHexValidator::regularText );
  mInputEdit->setValidator( mValidator );

  mFindButton = new QPushButton( i18n("Find"), this );
  mFindButton->setAutoDefault(false);
  connect( mFindButton, SIGNAL(clicked()), this, SLOT(start()) );
  connect(mInputEdit,SIGNAL(returnPressed()),mFindButton,SLOT(animateClick()));
  mFindButton->setFixedHeight( mTypeCombo->sizeHint().height() );

  mBackwards = new QCheckBox( i18n("Backwards"), this );
  mIgnoreCase = new QCheckBox( i18n("Ignore case"), this );

  mCloseButton = new QPushButton( this );
  mCloseButton->setAutoDefault(false);
  mCloseButton->setPixmap( QPixmap( close_xpm ) );
  connect( mCloseButton, SIGNAL(clicked()), this, SLOT(hideWidget()) );

  //
  // Make layout
  //
  QHBoxLayout *hlay = new QHBoxLayout( this, 4, 6 );
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
// Qt 2.1: Seems like the QLineEdit::returnPressed() does not work when
// I install a validator. So I catch the event manually
//
bool CSearchBar::eventFilter( QObject *o, QEvent *e )
{
  if( o == mInputEdit && e->type() == QEvent::KeyPress )
  {
    QKeyEvent *ke = (QKeyEvent*)e;
    if( ke->key() == Key_Return )
    {
      mFindButton->animateClick();
      return true;
    }
  }
  return QFrame::eventFilter( o, e );
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


void CSearchBar::textChanged( const QString &text )
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
    showEntryFailure( this, QString("") );
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


void CSearchBar::showEvent( QShowEvent *e )
{
  QFrame::showEvent(e);
  mInputEdit->setFocus();
}


#include "searchbar.moc"
