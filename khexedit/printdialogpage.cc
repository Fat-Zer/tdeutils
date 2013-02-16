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

#include <tqlayout.h>
#include <tqbuttongroup.h>
#include <tqspinbox.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqstringlist.h>
#include <tqcombobox.h>

#include <tdelocale.h>


#include "printdialogpage.h"

LayoutDialogPage::LayoutDialogPage( TQWidget *parent, const char *name )
 : KPrintDialogPage( parent, name )
{
  mConfig = 0;
  setTitle( i18n( "Page Layout" ) );
  setupLayoutPage();

  readConfiguration();
}


LayoutDialogPage::~LayoutDialogPage( void )
{
  writeConfiguration();
  
  delete mConfig; mConfig = 0;
}


void LayoutDialogPage::setupLayoutPage( void )
{
  TQString text;
  TQVBoxLayout *topLayout = new TQVBoxLayout( this, 0, 6 /*KDialog::mSpacingSize*/ );
  if( topLayout == 0 ) { return; }

  text = i18n("Margins [millimeter]");
  TQButtonGroup *group = new TQButtonGroup( text, this );
  if( group == 0 ) { return; }
  topLayout->addWidget( group );

  TQGridLayout *gbox = new TQGridLayout( group, 3, 6, 6 /*KDialog::mSpacingSize*/ );
  if( gbox == 0 ) { return; }
  gbox->addRowSpacing( 0, group->fontMetrics().height() );
  gbox->setColStretch( 5, 10 );

  TQString name[4];
  int i;

  name[0] = i18n("&Top:");
  name[1] = i18n("&Bottom:");
  name[2] = i18n("&Left:");
  name[3] = i18n("&Right:");

  for( i=0; i<4; i++ )
  {
    mLayout.marginSpin[i] = new TQSpinBox( group );
    mLayout.marginSpin[i]->setFixedHeight(
      mLayout.marginSpin[i]->sizeHint().height() );
    mLayout.marginSpin[i]->setMinimumWidth(
      mLayout.marginSpin[i]->fontMetrics().width("M")*10 );
    mLayout.marginSpin[i]->setRange( 0, INT_MAX );

    TQLabel *label = new TQLabel( mLayout.marginSpin[i], name[i], group );
    label->setFixedHeight( mLayout.marginSpin[i]->sizeHint().height() );
    label->setFixedWidth( label->sizeHint().width() );

    if( i < 2 )
    {
      gbox->addWidget( label, i+1, 0, AlignLeft );
      gbox->addWidget( mLayout.marginSpin[i], i+1, 1, AlignLeft );
    }
    else
    {
      gbox->addWidget( label, i-1, 3, AlignLeft );
      gbox->addWidget( mLayout.marginSpin[i], i-1, 4, AlignLeft );
    }
  }
  
  text = i18n("Draw h&eader above text");
  mLayout.headerCheck = new TQCheckBox( text, this );
  mLayout.headerCheck->setFixedSize( mLayout.headerCheck->sizeHint() );
  connect( mLayout.headerCheck, TQT_SIGNAL( toggled(bool)),
	   TQT_SLOT( slotDrawHeader(bool)));
  topLayout->addWidget( mLayout.headerCheck, 0, AlignLeft );

  gbox = new TQGridLayout( 5, 6, 0 );
  if( gbox == 0 ) { return; }
  topLayout->addLayout( gbox );

  gbox->setColStretch ( 5, 10 );
  gbox->addColSpacing( 1, 6 /*KDialog::mSpacingSize*/ );
  gbox->addColSpacing( 3, 6 /*KDialog::mSpacingSize*/ );
  gbox->addRowSpacing( 2, 6 /*KDialog::mSpacingSize*/ );

  name[0] = i18n("Left:");
  name[1] = i18n("Center:");
  name[2] = i18n("Right:");
  name[3] = i18n("Border:");

  TQStringList textList;
  textList.append(i18n("None"));
  textList.append(i18n("Date & Time"));
  textList.append(i18n("Page Number"));
  textList.append(i18n("Filename"));

  TQStringList lineList;
  lineList.append(i18n("None"));
  lineList.append(i18n("Single Line"));
  lineList.append(i18n("Rectangle"));

  for( i=0; i<4; i++ )
  {
    mLayout.headerCombo[i] = new TQComboBox( false, this );
    mLayout.headerCombo[i]->setFixedHeight(
      mLayout.headerCombo[i]->sizeHint().height() );
    mLayout.headerCombo[i]->setMinimumWidth(
      mLayout.headerCombo[i]->fontMetrics().width("M")*10 );
 
    mLayout.headerLabel[i] = new TQLabel( mLayout.headerCombo[i], name[i],
					 this );
    mLayout.headerLabel[i]->setFixedHeight( 
      mLayout.headerLabel[i]->sizeHint().height() );
    mLayout.headerLabel[i]->setFixedWidth( 
      mLayout.headerLabel[i]->sizeHint().width() );

    if( i<3 )
    {
      mLayout.headerCombo[i]->insertStringList( textList );
      gbox->addWidget( mLayout.headerLabel[i], 0, i*2, AlignLeft );
      gbox->addWidget( mLayout.headerCombo[i], 1, i*2, AlignLeft );
    }
    else
    {
      mLayout.headerCombo[i]->insertStringList( lineList );
      gbox->addWidget( mLayout.headerLabel[i], 3, 0, AlignLeft );
      gbox->addWidget( mLayout.headerCombo[i], 4, 0, AlignLeft );
    }
  }


  text = i18n("Draw &footer below text");
  mLayout.footerCheck = new TQCheckBox( text, this );
  mLayout.footerCheck->setFixedSize( mLayout.footerCheck->sizeHint() );
  connect( mLayout.footerCheck, TQT_SIGNAL( toggled(bool)),
	   TQT_SLOT( slotDrawFooter(bool)));
  topLayout->addWidget( mLayout.footerCheck, 0, AlignLeft );

  gbox = new TQGridLayout( 5, 6, 0 );
  if( gbox == 0 ) { return; }
  topLayout->addLayout( gbox );

  gbox->setColStretch ( 5, 10 );
  gbox->addColSpacing( 1, 6 /*KDialog::mSpacingSize*/ );
  gbox->addColSpacing( 3, 6 /*KDialog::mSpacingSize*/ );
  gbox->addRowSpacing( 2, 6 /*KDialog::mSpacingSize*/ );

  for( i=0; i<4; i++ )
  {
    mLayout.footerCombo[i] = new TQComboBox( false, this );
    mLayout.footerCombo[i]->setFixedHeight(
      mLayout.footerCombo[i]->sizeHint().height() );
    mLayout.footerCombo[i]->setMinimumWidth(
      mLayout.footerCombo[i]->fontMetrics().width("M")*10 );

    mLayout.footerLabel[i] = new TQLabel( mLayout.footerCombo[i], name[i], 
					 this );
    mLayout.footerLabel[i]->setFixedHeight( 
      mLayout.footerLabel[i]->sizeHint().height() );
    mLayout.footerLabel[i]->setFixedWidth( 
      mLayout.footerLabel[i]->sizeHint().width() );

    if( i<3 )
    {
      mLayout.footerCombo[i]->insertStringList( textList );
      gbox->addWidget( mLayout.footerLabel[i], 0, i*2, AlignLeft );
      gbox->addWidget( mLayout.footerCombo[i], 1, i*2, AlignLeft );
    }
    else
    {
      mLayout.footerCombo[i]->insertStringList( lineList );
      gbox->addWidget( mLayout.footerLabel[i], 3, 0, AlignLeft );
      gbox->addWidget( mLayout.footerCombo[i], 4, 0, AlignLeft );
    }
  }

  for( i=0; i<4; i++ )
  {
    mLayout.marginSpin[i]->setValue( 15 );
  }
  mLayout.headerCheck->setChecked( true );
  slotDrawHeader( true );
  mLayout.footerCheck->setChecked( true );
  slotDrawFooter( true );

  topLayout->addStretch(10);
}


void LayoutDialogPage::readConfiguration( void )
{
  if( mConfig != 0 ) { return; }

  mConfig = new KSimpleConfig( TQString("hexprinter") );
  if( mConfig == 0 ) { return; }

  mConfig->setGroup( "PageLayout" );
  int val = mConfig->readNumEntry( "MarginTop", 15 );
  mLayout.marginSpin[0]->setValue( val );
  val = mConfig->readNumEntry( "MarginBottom", 15 );
  mLayout.marginSpin[1]->setValue( val );
  val = mConfig->readNumEntry( "MarginLeft", 15 );
  mLayout.marginSpin[2]->setValue( val );
  val = mConfig->readNumEntry( "MarginRight", 15 );
  mLayout.marginSpin[3]->setValue( val );
  bool state = mConfig->readBoolEntry( "DrawHeader", true );
  mLayout.headerCheck->setChecked( state );
  slotDrawHeader( state );
  state = mConfig->readBoolEntry( "DrawFooter", true );
  mLayout.footerCheck->setChecked( state );
  slotDrawFooter( state );

  mLayout.headerCombo[0]->setCurrentItem(
    headerTextIndex( mConfig->readEntry("HeaderLeft","DateTime")));
  mLayout.headerCombo[1]->setCurrentItem(
    headerTextIndex( mConfig->readEntry("HeaderCenter")));
  mLayout.headerCombo[2]->setCurrentItem(
    headerTextIndex( mConfig->readEntry("HeaderRight","FileName")));
  mLayout.headerCombo[3]->setCurrentItem(
    headerLineIndex( mConfig->readEntry("HeaderLine","SingleLine")));

  mLayout.footerCombo[0]->setCurrentItem(
    headerTextIndex( mConfig->readEntry("FooterLeft")));
  mLayout.footerCombo[1]->setCurrentItem(
    headerTextIndex( mConfig->readEntry("FooterCenter","PageNumber")));
  mLayout.footerCombo[2]->setCurrentItem(
    headerTextIndex( mConfig->readEntry("FooterRight")));
  mLayout.footerCombo[3]->setCurrentItem(
    headerLineIndex( mConfig->readEntry("FooterLine","SingleLine")));

}


void LayoutDialogPage::writeConfiguration( void )
{
  if( mConfig == 0 )
  {
    return;
  }

  mConfig->setGroup( "PageLayout" );
  mConfig->writeEntry( "MarginTop", mLayout.marginSpin[0]->value() );
  mConfig->writeEntry( "MarginBottom", mLayout.marginSpin[1]->value() );
  mConfig->writeEntry( "MarginLeft", mLayout.marginSpin[2]->value() );
  mConfig->writeEntry( "MarginRight", mLayout.marginSpin[3]->value() );
  mConfig->writeEntry( "DrawHeader", mLayout.headerCheck->isChecked() );
  mConfig->writeEntry( "DrawFooter", mLayout.footerCheck->isChecked() );

  mConfig->writeEntry( "HeaderLeft",
    headerText( mLayout.headerCombo[0]->currentItem() ) );
  mConfig->writeEntry( "HeaderCenter",
    headerText( mLayout.headerCombo[1]->currentItem() ) );
  mConfig->writeEntry( "HeaderRight",
    headerText( mLayout.headerCombo[2]->currentItem() ) );
  mConfig->writeEntry( "HeaderLine",
    headerLine( mLayout.headerCombo[3]->currentItem() ) );

  mConfig->writeEntry( "FooterLeft",
    headerText( mLayout.footerCombo[0]->currentItem() ) );
  mConfig->writeEntry( "FooterCenter",
    headerText( mLayout.footerCombo[1]->currentItem() ) );
  mConfig->writeEntry( "FooterRight",
    headerText( mLayout.footerCombo[2]->currentItem() ) );
  mConfig->writeEntry( "FooterLine",
    headerLine( mLayout.footerCombo[3]->currentItem() ) );

  mConfig->sync();
}


TQString LayoutDialogPage::headerText( uint index )
{
  static const TQString text[4] = {"None","DateTime","PageNumber","FileName"};
  return( text[ index > 3 ? 0 : index ] );
}


int LayoutDialogPage::headerTextIndex( const TQString & headerText )
{
  static const TQString text[4] = {"None","DateTime","PageNumber","FileName"};
  if( headerText != 0 )
  {
    for( int i=0; i<4; i++ )
    {
      if( headerText == text[i] ) { return( i ); }
    }
  }
  return( 0 );
}


TQString LayoutDialogPage::headerLine( uint index )
{
  static const TQString text[3] = {"None","SingleLine","Rectangle"};
  return( text[ index > 2 ? 0 : index ] );
}


int LayoutDialogPage::headerLineIndex( const TQString & headerLine )
{
  static const TQString text[3] = {"None","SingleLine","Rectangle"};
  if( headerLine != 0 )
  {
    for( int i=0; i<3; i++ )
    {
      if( headerLine == text[i] ) { return( i ); }
    }
  }
  return( 0 );
}


void LayoutDialogPage::slotDrawHeader( bool state )
{
  for( int i=0; i<4; i++ )
  {
    mLayout.headerLabel[i]->setEnabled( state );
    mLayout.headerCombo[i]->setEnabled( state );
  }
}


void LayoutDialogPage::slotDrawFooter( bool state )
{
  for( int i=0; i<4; i++ )
  {
    mLayout.footerLabel[i]->setEnabled( state );
    mLayout.footerCombo[i]->setEnabled( state );
  }
}


void LayoutDialogPage::getOptions( TQMap<TQString,TQString>& opts, bool /*incldef*/ )
{
   opts[ "kde-khexedit-topmarginmm" ]    = TQString::number( mLayout.marginSpin[0]->value() );
   opts[ "kde-khexedit-bottommarginmm" ] = TQString::number( mLayout.marginSpin[1]->value() );
   opts[ "kde-khexedit-leftmarginmm" ]   = TQString::number( mLayout.marginSpin[2]->value() );
   opts[ "kde-khexedit-rightmarginmm" ]  = TQString::number( mLayout.marginSpin[3]->value() );

   opts[ "kde-khexedit-headercheck" ]  = (mLayout.headerCheck->isChecked() ? "true" : "false");
   opts[ "kde-khexedit-headercombo0" ] = TQString::number( mLayout.headerCombo[0]->currentItem() );
   opts[ "kde-khexedit-headercombo1" ] = TQString::number( mLayout.headerCombo[1]->currentItem() );
   opts[ "kde-khexedit-headercombo2" ] = TQString::number( mLayout.headerCombo[2]->currentItem() );
   opts[ "kde-khexedit-headercombo3" ] = TQString::number( mLayout.headerCombo[3]->currentItem() );

   opts[ "kde-khexedit-footercheck" ]  = (mLayout.footerCheck->isChecked() ? "true" : "false");
   opts[ "kde-khexedit-footercombo0" ] = TQString::number( mLayout.footerCombo[0]->currentItem() );
   opts[ "kde-khexedit-footercombo1" ] = TQString::number( mLayout.footerCombo[1]->currentItem() );
   opts[ "kde-khexedit-footercombo2" ] = TQString::number( mLayout.footerCombo[2]->currentItem() );
   opts[ "kde-khexedit-footercombo3" ] = TQString::number( mLayout.footerCombo[3]->currentItem() );
}


#include "printdialogpage.moc"
