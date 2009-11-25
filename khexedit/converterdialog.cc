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


#include <qlabel.h>
#include <qlayout.h>

#include <klocale.h>

#include "converterdialog.h"
#include "hexvalidator.h"


CValidateLineEdit::CValidateLineEdit( QWidget *parent, int validateType,
				      const char *name )
  :QLineEdit( parent, name ), mBusy(false)
{
  mValidator = new CHexValidator( this, (CHexValidator::EState)validateType );
  setValidator( mValidator );
  connect( this, SIGNAL(textChanged(const QString &)),
	   this, SLOT(convertText(const QString &)) );
}


CValidateLineEdit::~CValidateLineEdit( void )
{
}


void CValidateLineEdit::setData( const QByteArray &buf )
{
  if( mBusy == false )
  {
    QString text;
    mValidator->format( text, buf );
    setText( text );
  }
}


void CValidateLineEdit::convertText( const QString &text )
{
  QByteArray buf;
  mValidator->convert( buf, text );
  mBusy = true; // Don't update while editing
  emit dataChanged( buf );
  mBusy = false;
}



CConverterDialog::CConverterDialog( QWidget *parent, const char *name, 
				    bool modal )
  :KDialogBase( parent, name, modal, i18n("Converter"), Cancel|User2|User1, 
		Cancel, true, KStdGuiItem::clear(), i18n("&On Cursor") )
{
  QWidget *page = new QWidget( this );
  setMainWidget( page );

  QGridLayout *topLayout = new QGridLayout( page, 6, 2, 0, spacingHint() );
  topLayout->setRowStretch( 5, 10 );
  topLayout->setColStretch( 1, 10 );

  QLabel *label = new QLabel( i18n("Hexadecimal:"), page );
  topLayout->addWidget( label, 0, 0 );
  label = new QLabel( i18n("Decimal:"), page );
  topLayout->addWidget( label, 1, 0 );
  label = new QLabel( i18n("Octal:"), page );
  topLayout->addWidget( label, 2, 0 );
  label = new QLabel( i18n("Binary:"), page );
  topLayout->addWidget( label, 3, 0 );
  label = new QLabel( i18n("Text:"), page );
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

  connect( mHexInput, SIGNAL(dataChanged(const QByteArray &)),
	   this, SLOT(setData(const QByteArray &)) );
  connect( mDecInput, SIGNAL(dataChanged(const QByteArray &)),
	   this, SLOT(setData(const QByteArray &)) );
  connect( mOctInput, SIGNAL(dataChanged(const QByteArray &)),
	   this, SLOT(setData(const QByteArray &)) );
  connect( mBinInput, SIGNAL(dataChanged(const QByteArray &)),
	   this, SLOT(setData(const QByteArray &)) );
  connect( mTxtInput, SIGNAL(dataChanged(const QByteArray &)),
	   this, SLOT(setData(const QByteArray &)) );

}


CConverterDialog::~CConverterDialog( void )
{
}


void CConverterDialog::showEvent( QShowEvent *e )  
{
  KDialogBase::showEvent(e);
  mHexInput->setFocus();
}


void CConverterDialog::setData( const QByteArray &data )
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
  QByteArray buf;
  setData( buf );
}

void CConverterDialog::slotUser2( void ) // On Cursor
{
  QByteArray buf;
  emit probeCursorValue( buf, 1 );
  setData( buf );
}


#include "converterdialog.moc"
