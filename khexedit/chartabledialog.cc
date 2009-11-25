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

#include <limits.h>

#include <qlabel.h>
#include <qheader.h>
#include <qlayout.h>
#include <qspinbox.h>

#include <kglobalsettings.h>
#include <klocale.h>
#include <kstdguiitem.h>

#include "chartabledialog.h"


CCharTableDialog::CCharTableDialog( QWidget *parent, const char *name,
				    bool modal )
  :KDialogBase( Plain, i18n("Character Table"), Help|User1|Cancel, User1,
		parent, name, modal, true, KStdGuiItem::insert())
{
  updateGeometry();
  setHelp( "khexedit/khexedit.html", QString::null );

  QString text;
  QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  mCharacterList = new CListView( plainPage(), "characterList" );
  mCharacterList->setFont( KGlobalSettings::fixedFont() );
  mCharacterList->addColumn( i18n("Decimal") );
  mCharacterList->addColumn( i18n("Hexadecimal") );
  mCharacterList->addColumn( i18n("Octal") );
  mCharacterList->addColumn( i18n("Binary") );
  mCharacterList->addColumn( i18n("Text") );
  mCharacterList->setAllColumnsShowFocus( true );
  mCharacterList->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );
  mCharacterList->setSorting( -1 );

  //
  // I am setting the min. size below
  //
  //connect( mCharacterList, SIGNAL(doubleClicked(QListViewItem *) ),
  //SLOT(startAssign(QListViewItem *)));

  topLayout->addWidget( mCharacterList, 10 );

  QHBoxLayout *hbox = new QHBoxLayout();
  if( hbox == 0 ) { return; }
  topLayout->addLayout( hbox );

  text = i18n("Insert this number of characters:");
  QLabel *label = new QLabel( text, plainPage() );
  label->setFixedWidth( label->sizeHint().width() );
  hbox->addWidget( label );

  mInputCountSpin = new QSpinBox( plainPage(), "spin" );
  mInputCountSpin->setMinimumWidth( fontMetrics().maxWidth()*7 );
  mInputCountSpin->setRange( 1, INT_MAX );
  mInputCountSpin->setValue( 1 );
  hbox->addWidget( mInputCountSpin );

  hbox->addStretch ( 10 );

  createListData();
  setColumnWidth();
  mCharacterList->setVisibleItem( 15 );
}



CCharTableDialog::~CCharTableDialog( void )
{
}


const char *printBinary( uint val )
{
  static char buf[9];
  for( int i = 0; i < 8; i++ )
  {
    buf[7-i] = (val&(1<<i)) ? '1' : '0';
  }
  buf[8] = 0;
  return( buf );
}


void CCharTableDialog::createListData( void )
{
  QString d, h, o, b, c;
  QListViewItem *item = 0;

  char buf[10];
  memset( buf, 0, sizeof( buf ) );

  for( uint i=0; i<256; i++ )
  {
    d.sprintf("%03d", i );
    h.sprintf("0x%02x", i );
    o.sprintf("%03o", i );
    b.sprintf("%s", printBinary(i) );

    if( QChar((char)i).isPrint() == true )
    {
      c = QChar((char)i);
    }
    else
    {
      c = QChar('.');
    }

    item = new QListViewItem( mCharacterList, item, d, h, o, b, c );
    if( i == 0 )
    {
      mCharacterList->setSelected( item, true );
    }

  }
}


void CCharTableDialog::slotUser1( void ) // Insert
{
 QListViewItem *item = mCharacterList->selectedItem();
 if( item != 0 )
 {
   startAssign( item );
 }

}

void CCharTableDialog::startAssign( QListViewItem *item )
{
  QByteArray buf;
  if( buf.fill( item->text(0).toInt(), mInputCountSpin->value() ) == false )
  {
    return;
  }
  emit assign( buf );
}


void CCharTableDialog::setColumnWidth( void )
{
  const QFontMetrics &fm = mCharacterList->fontMetrics();
  int w0, w1, w2, w3, w4;

  w0 = -fm.minLeftBearing() - fm.minRightBearing() + 8 + fm.maxWidth();
  w3 = 0;

  w1  = fm.width( mCharacterList->header()->label(0) ) + w0;
  w2  = fm.width('0')*5;
  w3 += w1 > w2 ? w1 : w2;
  mCharacterList->setColumnWidth( 0, w1 > w2 ? w1 : w2 );

  w1  = fm.boundingRect( mCharacterList->header()->label(1) ).width() + w0;
  w2  = fm.width('0')*6;
  w3 += w1 > w2 ? w1 : w2;
  mCharacterList->setColumnWidth( 1, w1 > w2 ? w1 : w2 );

  w1  = fm.boundingRect( mCharacterList->header()->label(2) ).width() + w0;
  w2  = fm.width('0')*5;
  w3 += w1 > w2 ? w1 : w2;
  mCharacterList->setColumnWidth( 2, w1 > w2 ? w1 : w2 );

  w1  = fm.boundingRect( mCharacterList->header()->label(3) ).width() + w0;
  w2  = fm.width('0')*10;
  w3 += w1 > w2 ? w1 : w2;
  mCharacterList->setColumnWidth( 3, w1 > w2 ? w1 : w2 );

  w4 = mCharacterList->viewport()->width() - w3;
  w1 = fm.boundingRect( mCharacterList->header()->label(4) ).width() + w0;
  w2 = fm.width('0')*3;
  w1 = w1 > w2 ? w1 : w2;
  mCharacterList->setColumnWidth( 4, w1 > w4 ? w1 : w4 );
}


void CCharTableDialog::resizeEvent( QResizeEvent * )
{
  setColumnWidth();
}


void CCharTableDialog::showEvent( QShowEvent *e )
{
  KDialogBase::showEvent(e);
  setColumnWidth();
  mCharacterList->setFocus();
}


#include "chartabledialog.moc"
