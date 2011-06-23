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


#include <tqlabel.h>
#include <tqlayout.h>

#include <klocale.h>

#include "converterdialog.h"
#include "hexvalidator.h"


CValidateLineEdit::CValidateLineEdit( TQWidget *tqparent, int validateType,
				      const char *name )
  :TQLineEdit( tqparent, name ), mBusy(false)
{
  mValidator = new CHexValidator( this, (CHexValidator::EState)validateType );
  setValidator( mValidator );
  connect( this, TQT_SIGNAL(textChanged(const TQString &)),
	   this, TQT_SLOT(convertText(const TQString &)) );
}


CValidateLineEdit::~CValidateLineEdit( void )
{
}


void CValidateLineEdit::setData( const TQByteArray &buf )
{
  if( mBusy == false )
  {
    TQString text;
    mValidator->format( text, buf );
    setText( text );
  }
}


void CValidateLineEdit::convertText( const TQString &text )
{
  TQByteArray buf;
  mValidator->convert( buf, text );
  mBusy = true; // Don't update while editing
  emit dataChanged( buf );
  mBusy = false;
}



CConverterDialog::CConverterDialog( TQWidget *tqparent, const char *name, 
				    bool modal )
  :KDialogBase( tqparent, name, modal, i18n("Converter"), Cancel|User2|User1, 
		Cancel, true, KStdGuiItem::clear(), i18n("&On Cursor") )
{
  TQWidget *page = new TQWidget( this );
  setMainWidget( page );

  TQGridLayout *topLayout = new TQGridLayout( page, 6, 2, 0, spacingHint() );
  topLayout->setRowStretch( 5, 10 );
  topLayout->setColStretch( 1, 10 );

  TQLabel *label = new TQLabel( i18n("Hexadecimal:"), page );
  topLayout->addWidget( label, 0, 0 );
  label = new TQLabel( i18n("Decimal:"), page );
  topLayout->addWidget( label, 1, 0 );
  label = new TQLabel( i18n("Octal:"), page );
  topLayout->addWidget( label, 2, 0 );
  label = new TQLabel( i18n("Binary:"), page );
  topLayout->addWidget( label, 3, 0 );
  label = new TQLabel( i18n("Text:"), page );
  topLayout->addWidget( label, 4, 0 );

  mHexInput = new CValidateLineEdit( page, CHexValidator::hexadecimal );
  mHexInput->setMinimumWidth( fontMetrics().maxWidth()*17 );
  topLayout->addWidget( mHexInput, 0, 1 );
  mDecInput = new CValidateLineEdit( page, CHexValidator::decimal );
  topLayout->addWidget( mDecInput, 1, 1 );
  mOctInput = new CValidateLineEdit( page, CHexValidator::octal );
  topLayout->addWidget( mOctInput, 2, 1 );
  mBinInput = new CValidateLineEdit( page, CHexValidator::binary );
  topLayout->addWidget( mBinInput, 3, 1 );
  mTxtInput = new CValidateLineEdit( page, CHexValidator::regularText );
  topLayout->addWidget( mTxtInput, 4, 1 );

  connect( mHexInput, TQT_SIGNAL(dataChanged(const TQByteArray &)),
	   this, TQT_SLOT(setData(const TQByteArray &)) );
  connect( mDecInput, TQT_SIGNAL(dataChanged(const TQByteArray &)),
	   this, TQT_SLOT(setData(const TQByteArray &)) );
  connect( mOctInput, TQT_SIGNAL(dataChanged(const TQByteArray &)),
	   this, TQT_SLOT(setData(const TQByteArray &)) );
  connect( mBinInput, TQT_SIGNAL(dataChanged(const TQByteArray &)),
	   this, TQT_SLOT(setData(const TQByteArray &)) );
  connect( mTxtInput, TQT_SIGNAL(dataChanged(const TQByteArray &)),
	   this, TQT_SLOT(setData(const TQByteArray &)) );

}


CConverterDialog::~CConverterDialog( void )
{
}


void CConverterDialog::showEvent( TQShowEvent *e )  
{
  KDialogBase::showEvent(e);
  mHexInput->setFocus();
}


void CConverterDialog::setData( const TQByteArray &data )
{
  mHexInput->blockSignals(true);
  mDecInput->blockSignals(true);
  mOctInput->blockSignals(true);
  mBinInput->blockSignals(true);
  mTxtInput->blockSignals(true);
  mHexInput->setData(data);
  mDecInput->setData(data);
  mOctInput->setData(data);
  mBinInput->setData(data);
  mTxtInput->setData(data);
  mHexInput->blockSignals(false);
  mDecInput->blockSignals(false);
  mOctInput->blockSignals(false);
  mBinInput->blockSignals(false);
  mTxtInput->blockSignals(false);
}

void CConverterDialog::slotUser1( void ) // Clear
{
  TQByteArray buf;
  setData( buf );
}

void CConverterDialog::slotUser2( void ) // On Cursor
{
  TQByteArray buf;
  emit probeCursorValue( buf, 1 );
  setData( buf );
}


#include "converterdialog.moc"
