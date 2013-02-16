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

#include <ctype.h>
#include <iostream>
#include <limits.h>

#include <tqbuttongroup.h>
#include <tqfileinfo.h>
#include <tqlayout.h>
#include <tqvalidator.h>

#include <tdefiledialog.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kseparator.h>
#include <kstdguiitem.h>

#include "dialog.h"
#include <tqpushbutton.h>

static const TQStringList &formatStrings( void );
static const TQStringList &operationStrings( void );


CGotoDialog::CGotoDialog( TQWidget *parent, const char *name, bool modal )
  :KDialogBase( Plain, i18n("Goto Offset"), Ok|Cancel, Ok, parent, name,
		modal )
{
  TQVBoxLayout *topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  TQVBoxLayout *vbox = new TQVBoxLayout();
  if( vbox == 0 ) { return; }
  topLayout->addLayout( vbox );

  mComboBox = new TQComboBox( true, plainPage() );
  if( mComboBox == 0 ) { return; }
  mComboBox->setMaxCount( 10 );
  mComboBox->setInsertionPolicy( TQComboBox::AtTop );
  mComboBox->setMinimumWidth( fontMetrics().maxWidth()*17 );

  TQLabel *label = new TQLabel( mComboBox, i18n("O&ffset:"), plainPage() );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mComboBox );

  TQButtonGroup *group = new TQButtonGroup( i18n("Options"), plainPage() );
  if( group == 0 ) { return; }
  topLayout->addWidget( group, 10 );

  TQGridLayout *gbox = new TQGridLayout( group, 4, 2, spacingHint() );
  if( gbox == 0 ) { return; }
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  mCheckFromCursor = new TQCheckBox( i18n("&From cursor"), group );
  gbox->addWidget( mCheckFromCursor, 1, 0 );
  mCheckBackward = new TQCheckBox( i18n("&Backwards"), group );
  gbox->addWidget( mCheckBackward, 1, 1 );
  mCheckVisible = new TQCheckBox( i18n("&Stay visible"), group );
  gbox->addWidget( mCheckVisible, 2, 0 );
  gbox->setRowStretch( 3, 10 );

  TDEConfig &config = *kapp->config();
  config.setGroup("Goto Dialog");
  mCheckFromCursor->setChecked( config.readBoolEntry( "FromCursor", false ) );
  mCheckVisible->setChecked( config.readBoolEntry( "StayVisible", true ) );
  mCheckBackward->setChecked( config.readBoolEntry( "Backwards", false ) );
}



CGotoDialog::~CGotoDialog( void )
{
  TDEConfig &config = *kapp->config();
  config.setGroup("Goto Dialog");
  config.writeEntry( "FromCursor", mCheckFromCursor->isChecked() );
  config.writeEntry( "StayVisible", mCheckVisible->isChecked() );
  config.writeEntry( "Backwards", mCheckBackward->isChecked() );
  config.sync();
}



void CGotoDialog::showEvent( TQShowEvent *e )
{
  KDialogBase::showEvent(e);
  mComboBox->setFocus();
}



//
// Format of input string:
// 0x<HexNumber>|<DecimalNumber>s<Bit>
// s = :,. or space
//
void CGotoDialog::slotOk( void )
{
  uint offset;
  bool success = stringToOffset( mComboBox->currentText(), offset );
  if( success == false )
  {
    showEntryFailure( this, TQString("") );
    return;
  }

  if( mCheckVisible->isChecked() == false )
  {
    hide();
  }
  emit gotoOffset( offset, 7, mCheckFromCursor->isChecked(),
		   mCheckBackward->isChecked() == true ? false : true );

  #if 0
  const char *p = mComboBox->currentText();
  if( strlen( p ) == 0 )
  {
    return;
  }

  //
  // Skip any whitespaces in front of string
  //
  for( ; *p != 0 && isspace( *p ) ; p++ );

  uint offset, bit;
  int match;
  if( strncmp( p, "0x", 2 ) == 0 || strncmp( p, "0X", 2 ) == 0 )
  {
    match = sscanf( p+2, "%x", &offset );
  }
  else
  {
    match = sscanf( p, "%u", &offset );
  }

  if( match == 0 )
  {
    return;
  }

  bit = 7;

  p = strpbrk( p, ":,. " );
  if( p != 0 )
  {
    match = sscanf( p+1, "%u", &bit );
    if( match == 0 )
    {
      return;
    }
    if( bit > 7 ) { bit = 7; }
  }
  #endif
}



CFindDialog::CFindDialog( TQWidget *parent, const char *name, bool modal )
  :KDialogBase( Plain, i18n("Find"), Ok|Cancel, Ok, parent, name, modal )
{

  TQVBoxLayout *topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  TQVBoxLayout *vbox = new TQVBoxLayout();
  if( vbox == 0 ) { return; }
  topLayout->addLayout( vbox );

  mSelector = new TQComboBox( false, plainPage() );
  if( mSelector == 0 ) { return; }
  mSelector->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mSelector->insertStringList( formatStrings() );
  connect( mSelector, TQT_SIGNAL(activated(int)), TQT_SLOT(selectorChanged(int)) );

  TQLabel *label = new TQLabel( mSelector, i18n("Fo&rmat:"), plainPage() );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mSelector );

  mInput = new TQLineEdit( plainPage() );
  if( mInput == 0 ) { return; }
  mInput->setMinimumWidth( fontMetrics().maxWidth()*17 );
  connect( mInput, TQT_SIGNAL(textChanged(const TQString&)),
	   TQT_SLOT(inputChanged(const TQString&)) );
  mFindValidator = new CHexValidator( this, CHexValidator::regularText );
  if( mFindValidator == 0 ) { return; }
  mInput->setValidator( mFindValidator );

  label = new TQLabel( mInput, i18n("F&ind:"), plainPage() );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mInput );

  TQButtonGroup *group = new TQButtonGroup( i18n("Options"), plainPage() );
  if( group == 0 ) { return; }
  topLayout->addWidget( group, 10 );

  TQGridLayout *gbox = new TQGridLayout( group, 5, 2, spacingHint() );
  if( gbox == 0 ) { return; }
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

  mCheckFromCursor = new TQCheckBox( i18n("&From cursor"), group );
  gbox->addWidget( mCheckFromCursor, 1, 0 );
  mCheckBackward = new TQCheckBox( i18n("&Backwards"), group );
  gbox->addWidget( mCheckBackward, 1, 1 );
  mCheckInSelection = new TQCheckBox( i18n("&In selection"), group );
  gbox->addWidget( mCheckInSelection, 2, 0 );
  mCheckUseNavigator = new TQCheckBox( i18n("&Use navigator"),group);
  gbox->addWidget( mCheckUseNavigator, 2, 1 );
  mCheckIgnoreCase = new TQCheckBox( i18n("Ignore c&ase"),group);
  gbox->addWidget( mCheckIgnoreCase, 3, 0 );
  gbox->setRowStretch( 4, 10 );

  TDEConfig &config = *kapp->config();
  config.setGroup("Find Dialog");
  mCheckFromCursor->setChecked( config.readBoolEntry( "FromCursor", true ) );
  mCheckInSelection->setChecked( config.readBoolEntry( "InSelection", false) );
  mCheckIgnoreCase->setChecked( config.readBoolEntry( "IgnoreCase", false ) );
  mCheckBackward->setChecked( config.readBoolEntry( "Backwards", false ) );
  mCheckUseNavigator->setChecked( config.readBoolEntry( "UseNavigator", true));
  uint val = config.readUnsignedNumEntry( "Format", 0 );
  mSelector->setCurrentItem(TQMIN(4,val) );
  selectorChanged( mSelector->currentItem() );
  enableButtonOK(!mInput->text().isEmpty());
}


CFindDialog::~CFindDialog( void )
{
  TDEConfig &config = *kapp->config();
  config.setGroup("Find Dialog");
  config.writeEntry( "FromCursor", mCheckFromCursor->isChecked() );
  config.writeEntry( "InSelection", mCheckInSelection->isChecked() );
  config.writeEntry( "IgnoreCase", mCheckIgnoreCase->isChecked() );
  config.writeEntry( "Backwards", mCheckBackward->isChecked() );
  config.writeEntry( "UseNavigator", mCheckUseNavigator->isChecked() );
  config.writeEntry( "Format", mSelector->currentItem() );
  config.sync();
}


void CFindDialog::selectorChanged( int index )
{
  mFindValidator->setState( (CHexValidator::EState)index );
  mInput->setText( mFindString[ index ] );
  mCheckIgnoreCase->setEnabled( index == 4 );
}


void CFindDialog::inputChanged( const TQString &text )
{
  mFindString[ mSelector->currentItem() ] = text;
  mFindValidator->convert( mFindData,
    mFindString[ mSelector->currentItem() ] );
  enableButtonOK(!text.isEmpty());
}


void CFindDialog::showEvent( TQShowEvent *e )
{
  KDialogBase::showEvent(e);
  mInput->setFocus();
}


bool CFindDialog::isEmpty( void )
{
  return( mFindData.isEmpty() );
}


void CFindDialog::slotOk( void )
{
  if( isEmpty() == true )
  {
    showEntryFailure( this, TQString("") );
    return;
  }

  SSearchControl sc;
  sc.key = mFindData;
  sc.keyType = mSelector->currentItem();
  sc.fromCursor = mCheckFromCursor->isChecked();
  sc.inSelection = mCheckInSelection->isChecked();
  sc.forward = mCheckBackward->isChecked() == true ? false : true;
  sc.ignoreCase = mCheckIgnoreCase->isEnabled() && mCheckIgnoreCase->isChecked();

  hide();
  emit findData( sc, Find_First, mCheckUseNavigator->isChecked() );
}


void CFindDialog::findAgain( EOperation operation )
{
  if( isEmpty() == true )
  {
    showEntryFailure( this, TQString("") );
    return;
  }

  SSearchControl sc;
  sc.key = mFindData;
  sc.fromCursor = true;
  sc.inSelection = mCheckInSelection->isChecked();
  sc.ignoreCase = mCheckIgnoreCase->isEnabled() && mCheckIgnoreCase->isChecked();
  if( operation == find_Next )
  {
    sc.forward = true;
  }
  else if( operation == find_Previous )
  {
    sc.forward = false;
  }
  else
  {
    sc.forward = mCheckBackward->isChecked() == true ? false : true;
  }

  hide();
  emit findData( sc, Find_Next, false );
}



CFindNavigatorDialog::CFindNavigatorDialog( TQWidget *parent, const char *name,
					     bool modal )
  :KDialogBase( Plain, i18n("Find (Navigator)"), User3|User2|User1|Close,
		User2, parent, name, modal, true, i18n("New &Key"),
		i18n("&Next"), i18n("&Previous") )
{
  TQString text;
  TQBoxLayout *topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  topLayout->addSpacing( spacingHint() ); // A little bit extra space

  TQHBoxLayout *hbox = new TQHBoxLayout();
  if( hbox == 0 ) { return; }
  topLayout->addLayout( hbox );

  text = i18n("Searching for:");
  TQLabel *label = new TQLabel( text, plainPage() );
  hbox->addWidget( label );

  mKey = new TQLineEdit( plainPage() );
  mKey->setMinimumWidth( fontMetrics().width("M") * 20 );
  mKey->setFocusPolicy( TQ_NoFocus );
  hbox->addWidget( mKey );

  topLayout->addSpacing( spacingHint() ); // A little bit extra space
  topLayout->addStretch(10);
}


CFindNavigatorDialog::~CFindNavigatorDialog( void )
{
}


void CFindNavigatorDialog::defineData( SSearchControl &sc )
{
  mSearchControl = sc;
  mSearchControl.key.duplicate( sc.key );

  if( mSearchControl.key.isEmpty() == true )
  {
    mKey->setText("");
    return;
  }

  if( mSearchControl.keyType == 0 )
  {
    TQString str;
    for( uint i=0; i<mSearchControl.key.size(); i++ )
    {
      str += mSearchControl.key[i];
    }
    mKey->setText( str );

  }
  else if( mSearchControl.keyType == 1 )
  {
    TQString str("0x ");
    for( uint i=0; i<mSearchControl.key.size(); i++ )
    {
      str += TQString().sprintf("%02X ", (unsigned char)mSearchControl.key[i]);
    }
    mKey->setText( str );
  }
  else if( mSearchControl.keyType == 2 )
  {
    TQString str;
    for( uint i=0; i<mSearchControl.key.size(); i++ )
    {
      str += TQString().sprintf("%03o ", (unsigned char)mSearchControl.key[i]);
    }
    mKey->setText( str );
  }
  else
  {
    char buf[10];
    memset( buf, 0, sizeof( buf ) ); buf[8] = ' ';

    TQString str;
    for( uint i=0; i<mSearchControl.key.size(); i++ )
    {
      unsigned char data = (unsigned char)mSearchControl.key[i];
      for( int j = 0; j < 8; j++ )
      {
	buf[7-j] = (data&(1<<j)) ? '1' : '0';
      }
      str += buf;
    }
    mKey->setText( str );
  }
}


void CFindNavigatorDialog::slotUser3( void ) // Previous
{
  done( repPrevious );
}


void CFindNavigatorDialog::slotUser2( void ) // Next
{
  done( repNext );
}


void CFindNavigatorDialog::slotUser1( void ) // New key
{
  done( repNewKey );
}


void CFindNavigatorDialog::slotClose( void )
{
  done( repClose );
}


void CFindNavigatorDialog::done( int resultCode )
{
  setResult( resultCode );
  if( resultCode == repClose || resultCode == repNewKey )
  {
    if( resultCode == repNewKey )
    {
      emit makeKey();
    }
    hide();
    return;
  }

  mSearchControl.forward = resultCode == repNext ? true : false;
  emit findData( mSearchControl, Find_Next, true );
}




CReplaceDialog::CReplaceDialog( TQWidget *parent, const char *name, bool modal )
  :KDialogBase( Plain, i18n("Find & Replace"), Ok|Cancel, Ok,
		parent, name, modal )
{
  TQString text;
  TQVBoxLayout *topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  TQVBoxLayout *vbox = new TQVBoxLayout();
  if( vbox == 0 ) { return; }
  topLayout->addLayout( vbox );


  mFindSelector = new TQComboBox( false, plainPage() );
  if( mFindSelector == 0 ) { return; }
  mFindSelector->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mFindSelector->insertStringList( formatStrings() );
  connect( mFindSelector, TQT_SIGNAL(activated(int)),
	   TQT_SLOT(findSelectorChanged(int)) );

  text = i18n("Fo&rmat (find):");
  TQLabel *label = new TQLabel( mFindSelector, text, plainPage() );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mFindSelector );

  mFindInput = new TQLineEdit( plainPage() );
  if( mFindInput == 0 ) { return; }
  mFindInput->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mFindValidator = new CHexValidator( this, CHexValidator::regularText );
  if( mFindValidator == 0 ) { return; }
  mFindInput->setValidator( mFindValidator );
  connect( mFindInput, TQT_SIGNAL(textChanged(const TQString&)),
	   TQT_SLOT(findInputChanged(const TQString&)) );

  label = new TQLabel( mFindInput, i18n("F&ind:"), plainPage() );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mFindInput  );

  mReplaceSelector = new TQComboBox( false, plainPage() );
  if( mReplaceSelector == 0 ) { return; }
  mReplaceSelector->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mReplaceSelector->insertStringList( formatStrings() );
  connect( mReplaceSelector, TQT_SIGNAL(activated(int)),
	   TQT_SLOT(replaceSelectorChanged(int)) );

  text = i18n("For&mat (replace):");
  label = new TQLabel( mReplaceSelector, text, plainPage() );
  if( label == 0 ) { return; }
  label->setFixedHeight( label->sizeHint().height() );

  vbox->addWidget( label );
  vbox->addWidget( mReplaceSelector );

  mReplaceInput = new TQLineEdit( plainPage() );
  if( mReplaceInput == 0 ) { return; }
  mReplaceInput->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mReplaceValidator = new CHexValidator( this, CHexValidator::regularText );
  if( mReplaceValidator == 0 ) { return; }
  mReplaceInput->setValidator( mReplaceValidator );
  connect( mReplaceInput, TQT_SIGNAL(textChanged(const TQString&)),
	   TQT_SLOT(replaceInputChanged(const TQString&)) );

  label = new TQLabel( mReplaceInput, i18n("Rep&lace:"), plainPage() );
  if( label == 0 ) { return; }
  label->setFixedHeight( label->sizeHint().height() );

  vbox->addWidget( label );
  vbox->addWidget( mReplaceInput );

  TQButtonGroup *group = new TQButtonGroup( i18n("Options"), plainPage() );
  if( group == 0 ) { return; }
  topLayout->addWidget( group, 10 );

  TQGridLayout *gbox = new TQGridLayout( group, 5, 2, spacingHint() );
  if( gbox == 0 ) { return; }
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  mCheckFromCursor = new TQCheckBox( i18n("&From cursor"), group );
  gbox->addWidget( mCheckFromCursor, 1, 0 );
  mCheckBackward = new TQCheckBox( i18n("&Backwards"), group );
  gbox->addWidget( mCheckBackward, 1, 1 );
  mCheckInSelection = new TQCheckBox( i18n("&In selection"), group );
  gbox->addWidget( mCheckInSelection, 2, 0 );
  mCheckPrompt = new TQCheckBox( i18n("&Prompt"), group );
  gbox->addWidget( mCheckPrompt, 2, 1 );
  mCheckIgnoreCase = new TQCheckBox( i18n("Ignore c&ase"), group );
  gbox->addWidget( mCheckIgnoreCase, 3, 0 );
  gbox->setRowStretch( 4, 10 );

  TDEConfig &config = *kapp->config();
  config.setGroup("Replace Dialog");
  mCheckFromCursor->setChecked( config.readBoolEntry( "FromCursor", true ) );
  mCheckInSelection->setChecked( config.readBoolEntry( "InSelection", false) );
  mCheckIgnoreCase->setChecked( config.readBoolEntry( "IgnoreCase", false ) );
  mCheckBackward->setChecked( config.readBoolEntry( "Backwards", false ) );
  mCheckPrompt->setChecked( config.readBoolEntry( "Prompt", true));
  uint val = config.readUnsignedNumEntry( "FindFormat", 0 );
  mFindSelector->setCurrentItem(TQMIN(4,val) );
  findSelectorChanged( mFindSelector->currentItem() );
  val = config.readUnsignedNumEntry( "ReplaceFormat", 0 );
  mReplaceSelector->setCurrentItem(TQMIN(4,val) );
  replaceSelectorChanged( mReplaceSelector->currentItem() );
  enableButtonOK(!mFindInput->text().isEmpty());
}


CReplaceDialog::~CReplaceDialog( void )
{
  TDEConfig &config = *kapp->config();
  config.setGroup("Replace Dialog");
  config.writeEntry( "FromCursor", mCheckFromCursor->isChecked() );
  config.writeEntry( "InSelection", mCheckInSelection->isChecked() );
  config.writeEntry( "IgnoreCase", mCheckIgnoreCase->isChecked() );
  config.writeEntry( "Backwards", mCheckBackward->isChecked() );
  config.writeEntry( "Prompt", mCheckPrompt->isChecked() );
  config.writeEntry( "FindFormat", mFindSelector->currentItem() );
  config.writeEntry( "ReplaceFormat", mReplaceSelector->currentItem() );
  config.sync();
}


void CReplaceDialog::findSelectorChanged( int index )
{
  mFindValidator->setState( (CHexValidator::EState)index );
  mFindInput->setText( mFindString[ index ] );
  mCheckIgnoreCase->setEnabled( index == 4 );
}


void CReplaceDialog::findInputChanged( const TQString &text )
{
  mFindString[ mFindSelector->currentItem() ] = text;
  mFindValidator->convert( mFindData,
    mFindString[ mFindSelector->currentItem() ] );
  enableButtonOK(!text.isEmpty());
}


void CReplaceDialog::replaceSelectorChanged( int index )
{
  mReplaceValidator->setState( (CHexValidator::EState)index );
  mReplaceInput->setText( mReplaceString[ index ] );
}


void CReplaceDialog::replaceInputChanged( const TQString &text )
{
  mReplaceString[ mReplaceSelector->currentItem() ] = text;
  mReplaceValidator->convert( mReplaceData,
    mReplaceString[ mReplaceSelector->currentItem() ] );
}


void CReplaceDialog::showEvent( TQShowEvent *e )
{
  KDialogBase::showEvent(e);
  mFindInput->setFocus();
}


void CReplaceDialog::slotOk( void )
{
  if( mFindData.isEmpty() == true )
  {
    showEntryFailure( this, TQString("") );
    return;
  }

  if( mFindData == mReplaceData )
  {
    showEntryFailure( this,i18n("Source and target values can not be equal."));
    return;
  }

  hide();

  SSearchControl sc;
  sc.key = mFindData;
  sc.val = mReplaceData;
  sc.fromCursor = mCheckFromCursor->isChecked();
  sc.inSelection = mCheckInSelection->isChecked();
  sc.forward = mCheckBackward->isChecked() == true ? false : true;

  sc.ignoreCase = mCheckIgnoreCase->isEnabled() && mCheckIgnoreCase->isChecked();

  emit replaceData( sc, mCheckPrompt->isChecked() ? Replace_First: Replace_AllInit );
}





CReplacePromptDialog::CReplacePromptDialog( TQWidget *parent, const char *name,
					     bool modal )
  :KDialogBase( Plain, i18n("Find & Replace"), User3|User2|User1|Close,
		User2, parent, name, modal, true, i18n("Replace &All"),
		i18n("Do Not Replace"), i18n("Replace"))
{
  TQString text;
  TQBoxLayout *topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  topLayout->addSpacing( spacingHint() ); // A little bit extra space

  text = i18n("Replace marked data at cursor position?");
  TQLabel* label = new TQLabel( text, plainPage() );
  topLayout->addWidget( label );

  topLayout->addSpacing( spacingHint() ); // A little bit extra space
  topLayout->addStretch(10);
}


CReplacePromptDialog::~CReplacePromptDialog( void )
{
}


void CReplacePromptDialog::defineData( SSearchControl &sc )
{
  mSearchControl = sc;
  mSearchControl.key.duplicate( sc.key );
  mSearchControl.val.duplicate( sc.val );
}


void CReplacePromptDialog::slotUser3( void )
{
  done( repYes );
}


void CReplacePromptDialog::slotUser2( void )
{
  done( repNo );
}


void CReplacePromptDialog::slotUser1( void )
{
  done( repAll );
}


void CReplacePromptDialog::slotClose( void )
{
  done( repClose );
}


void CReplacePromptDialog::done( int returnCode )
{
  if( returnCode == repClose )
  {
    hide();
  }
  else if( returnCode == repYes )
  {
    emit replaceData( mSearchControl, Replace_Next );
  }
  else if( returnCode == repNo )
  {
    emit replaceData( mSearchControl, Replace_Ignore );
  }
  else
  {
    emit replaceData( mSearchControl, Replace_All );
  }
}




CFilterDialog::CFilterDialog( TQWidget *parent, const char *name, bool modal )
  :KDialogBase( Plain, i18n("Binary Filter"), Ok|Cancel, Ok,
		parent, name, modal )
{
  TQString text;
  TQVBoxLayout *topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  TQVBoxLayout *vbox = new TQVBoxLayout();
  if( vbox == 0 ) { return; }
  topLayout->addLayout( vbox );


  mOperationSelector = new TQComboBox( false, plainPage() );
  if( mOperationSelector == 0 ) { return; }
  mOperationSelector->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mOperationSelector->insertStringList( operationStrings() );
  connect( mOperationSelector, TQT_SIGNAL(activated(int)),
	   TQT_SLOT(operationSelectorChanged(int)) );

  text = i18n("O&peration:");
  TQLabel *label = new TQLabel( mOperationSelector, text, plainPage() );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mOperationSelector );

  KSeparator *separator = new KSeparator( plainPage() );
  separator->setOrientation( TQFrame::HLine );
  vbox->addWidget( separator );


  mWidgetStack = new TQWidgetStack( plainPage(), "pagestack" );
  if( mWidgetStack == 0 ) { return; }
  vbox->addWidget( mWidgetStack );

  makeEmptyLayout();
  makeOperandLayout();
  makeBitSwapLayout();
  makeRotateLayout();
  mWidgetStack->raiseWidget( (int)OperandPage );


  TQButtonGroup *group = new TQButtonGroup( i18n("Options"), plainPage() );
  if( group == 0 ) { return; }
  topLayout->addWidget( group, 10 );

  TQGridLayout *gbox = new TQGridLayout( group, 4, 2, spacingHint() );
  if( gbox == 0 ) { return; }
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );
  mCheckFromCursor = new TQCheckBox( i18n("&From cursor"), group );
  gbox->addWidget( mCheckFromCursor, 1, 0 );
  mCheckBackward = new TQCheckBox( i18n("&Backwards"), group );
  gbox->addWidget( mCheckBackward, 1, 1 );
  mCheckInSelection = new TQCheckBox( i18n("&In selection"), group );
  gbox->addWidget( mCheckInSelection, 2, 0 );
  mCheckVisible = new TQCheckBox( i18n("&Stay visible"), group );
  gbox->addWidget( mCheckVisible, 2, 1 );
  gbox->setRowStretch( 3, 10 );
}


CFilterDialog::~CFilterDialog( void )
{
  delete mOperandValidator;
}


void CFilterDialog::makeEmptyLayout( void )
{
  TQFrame *page = new TQFrame( plainPage() );
  if( page == 0 ) { return; }
  mWidgetStack->addWidget( page, EmptyPage );
}


void CFilterDialog::makeOperandLayout( void )
{
  TQString text;

  TQFrame *page = new TQFrame( plainPage() );
  if( page == 0 ) { return; }
  mWidgetStack->addWidget( page, OperandPage );

  TQVBoxLayout *vbox = new TQVBoxLayout( page, 0, spacingHint() );
  if( vbox == 0 ) { return; }

  mOperandSelector = new TQComboBox( false, page );
  if( mOperandSelector == 0 ) { return; }
  mOperandSelector->setFixedHeight( mOperandSelector->sizeHint().height());
  mOperandSelector->setMinimumWidth( fontMetrics().width("M")*20 );
  mOperandSelector->insertStringList( formatStrings() );
  connect( mOperandSelector, TQT_SIGNAL(activated(int)),
	   TQT_SLOT(operandSelectorChanged(int)) );

  text = i18n("Fo&rmat (operand):");
  mOperandFormatLabel = new TQLabel( mOperandSelector, text, page );
  if( mOperandFormatLabel == 0 ) { return; }

  vbox->addWidget( mOperandFormatLabel );
  vbox->addWidget( mOperandSelector );

  mOperandInput = new TQLineEdit( page );
  if( mOperandInput == 0 ) { return; }
  mOperandInput->setMinimumWidth( fontMetrics().width("M") * 20 );
  mOperandValidator = new CHexValidator( this, CHexValidator::regularText );
  if( mOperandValidator == 0 ) { return; }
  mOperandInput->setValidator( mOperandValidator );
  connect( mOperandInput, TQT_SIGNAL(textChanged(const TQString&)),
	   TQT_SLOT(operandInputChanged(const TQString&)) );

  mOperandInputLabel = new TQLabel( mOperandInput, i18n("O&perand:"), page );
  if( mOperandInputLabel == 0 ) { return; }

  vbox->addWidget( mOperandInputLabel );
  vbox->addWidget( mOperandInput  );
  vbox->addSpacing( 1 );
}


void CFilterDialog::makeBitSwapLayout( void )
{
  TQString text;

  TQFrame *page = new TQFrame( plainPage() );
  if( page == 0 ) { return; }
  mWidgetStack->addWidget( page, BitSwapPage );

  TQVBoxLayout *vbox = new TQVBoxLayout( page, 0, spacingHint() );
  if( vbox == 0 ) { return; }

  text = i18n("Swap rule");
  TQLabel *label = new TQLabel( text, page );
  if( label == 0 ) { return; }
  label->setFixedHeight( label->sizeHint().height() );
  vbox->addWidget( label );

  mByteWidget = new CByteWidget( page );
  vbox->addWidget( mByteWidget );

  TQHBoxLayout *hbox = new TQHBoxLayout( 0 );
  vbox->addLayout( hbox );

  text = i18n("&Reset");
  TQPushButton *resetButton = new TQPushButton( text, page );
  resetButton->setFixedHeight( resetButton->sizeHint().height() );
  connect( resetButton, TQT_SIGNAL(clicked()), mByteWidget, TQT_SLOT(reset()) );

  hbox->addWidget( resetButton );
  hbox->addStretch( 10 );
}


void CFilterDialog::makeRotateLayout( void )
{
  TQString text;

  TQFrame *page = new TQFrame( plainPage() );
  if( page == 0 ) { return; }
  mWidgetStack->addWidget( page, RotatePage );

  TQVBoxLayout *vbox = new TQVBoxLayout( page, 0, spacingHint() );
  if( vbox == 0 ) { return; }

  mGroupSpin = new TQSpinBox( page );
  if( mGroupSpin == 0 ) { return; }
  mGroupSpin->setMinimumWidth( fontMetrics().width("M")*20 );
  mGroupSpin->setRange(1, INT_MAX );

  text = i18n("&Group size [bytes]");
  TQLabel *label = new TQLabel( mGroupSpin, text, page );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mGroupSpin );

  mBitSpin = new TQSpinBox( page );
  if( mBitSpin == 0 ) { return; }
  mBitSpin->setMinimumWidth( fontMetrics().width("M")*20 );
  mBitSpin->setRange(INT_MIN, INT_MAX);

  text = i18n("S&hift size [bits]");
  label = new TQLabel( mBitSpin, text, page );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mBitSpin );
}



void CFilterDialog::showEvent( TQShowEvent *e )
{
  KDialogBase::showEvent(e);
  mOperandInput->setFocus();
}



void CFilterDialog::slotOk( void )
{
  SFilterControl fc;
  switch( mOperationSelector->currentItem() )
  {
    case SFilterControl::OperandAndData:
    case SFilterControl::OperandOrData:
    case SFilterControl::OperandXorData:
      if( mOperandData.isEmpty() == true )
      {
	showEntryFailure( this, TQString("") );
	return;
      }
      fc.operand = mOperandData;
    break;

    case SFilterControl::InvertData:
    case SFilterControl::ReverseData:
    break;

    case SFilterControl::RotateData:
    case SFilterControl::ShiftData:
      fc.rotate[0] = mGroupSpin->value();
      fc.rotate[1] = mBitSpin->value();
      if( fc.rotate[1] == 0 )
      {
	TQString msg = i18n("Shift size is zero.");
	showEntryFailure( this, msg );
	return;
      }
    break;

    case SFilterControl::SwapBits:
      if( mByteWidget->flag( fc.operand ) == false )
      {
	TQString msg = i18n("Swap rule does not define any swapping.");
	showEntryFailure( this, msg );
	return;
      }
    break;

    default:
      return;
    break;

  }

  if( mCheckVisible->isChecked() == false )
  {
    hide();
  }

  fc.operation = (SFilterControl::Operation)mOperationSelector->currentItem();
  fc.fromCursor = mCheckFromCursor->isChecked();
  fc.inSelection = mCheckInSelection->isChecked();
  if( mCheckBackward->isEnabled() == true )
  {
    fc.forward = mCheckBackward->isChecked() == true ? false : true;
  }
  else
  {
    fc.forward = true;
  }

  emit filterData( fc );
}


void CFilterDialog::operandSelectorChanged( int index )
{
  mOperandValidator->setState( (CHexValidator::EState)index );
  mOperandInput->setText( mOperandString[ index ] );
}


void CFilterDialog::operandInputChanged( const TQString &text )
{
  mOperandString[ mOperandSelector->currentItem() ] = text;
  mOperandValidator->convert( mOperandData,
    mOperandString[ mOperandSelector->currentItem() ] );
}


void CFilterDialog::operationSelectorChanged( int index )
{
  if( index <= 2 )
  {
    mWidgetStack->raiseWidget( OperandPage );
    mCheckBackward->setEnabled( true );
  }
  else if( index <= 4 )
  {
    mWidgetStack->raiseWidget( EmptyPage );
    mCheckBackward->setEnabled( true );
  }
  else if( index <= 6 )
  {
    mWidgetStack->raiseWidget( RotatePage );
    mCheckBackward->setEnabled( false );
  }
  else
  {
    mWidgetStack->raiseWidget( BitSwapPage );
    mCheckBackward->setEnabled( true );
  }
}




CInsertDialog::CInsertDialog( TQWidget *parent, const char *name, bool modal )
  :KDialogBase( Plain, i18n("Insert Pattern"), Ok|Cancel, Ok,
		parent, name, modal )
{
  setButtonOKText(i18n("&Insert"));

  TQString text;
  TQVBoxLayout *topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  if( topLayout == 0 ) { return; }

  TQVBoxLayout *vbox = new TQVBoxLayout();
  if( vbox == 0 ) { return; }
  topLayout->addLayout( vbox );

  mSizeBox = new TQSpinBox( plainPage() );
  if( mSizeBox == 0 ) { return; }
  mSizeBox->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mSizeBox->setRange( 1, INT_MAX );
  mSizeBox->setValue( 1 );

  TQLabel *label = new TQLabel( mSizeBox, i18n("&Size:"), plainPage() );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mSizeBox );

  mPatternSelector = new TQComboBox( false, plainPage() );
  if( mPatternSelector == 0 ) { return; }
  mPatternSelector->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mPatternSelector->insertStringList( formatStrings() );
  connect( mPatternSelector, TQT_SIGNAL(activated(int)),
	   TQT_SLOT(patternSelectorChanged(int)) );

  text = i18n("Fo&rmat (pattern):");
  label = new TQLabel( mPatternSelector, text, plainPage() );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mPatternSelector );

  mPatternInput = new TQLineEdit( plainPage() );
  if( mPatternInput == 0 ) { return; }
  mPatternInput->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mPatternValidator = new CHexValidator( this, CHexValidator::regularText );
  if( mPatternValidator == 0 ) { return; }
  mPatternInput->setValidator( mPatternValidator );
  connect( mPatternInput, TQT_SIGNAL(textChanged(const TQString&)),
	   TQT_SLOT(patternInputChanged(const TQString&)) );

  label = new TQLabel( mPatternInput, i18n("&Pattern:"), plainPage() );
  if( label == 0 ) { return; }

  vbox->addWidget( label );
  vbox->addWidget( mPatternInput );

  mOffsetInput = new TQLineEdit( plainPage() );
  mOffsetInput->setMinimumWidth( fontMetrics().maxWidth()*17 );

  mOffsetLabel = new TQLabel( mOffsetInput, i18n("&Offset:"), plainPage() );
  if( mOffsetLabel == 0 ) { return; }

  vbox->addWidget( mOffsetLabel );
  vbox->addWidget( mOffsetInput );

  TQButtonGroup *group = new TQButtonGroup( i18n("Options"), plainPage() );
  if( group == 0 ) { return; }
  topLayout->addWidget( group, 10 );


  TQGridLayout *gbox = new TQGridLayout( group, 4, 2, spacingHint() );
  if( gbox == 0 ) { return; }
  gbox->addRowSpacing( 0, fontMetrics().lineSpacing() );

  mCheckPattern = new TQCheckBox( i18n("R&epeat pattern"), group );
  gbox->addWidget( mCheckPattern, 1, 0 );
  mCheckOnCursor = new TQCheckBox( i18n("&Insert on cursor position"), group );
  gbox->addWidget( mCheckOnCursor, 2, 0 );
  connect( mCheckOnCursor, TQT_SIGNAL(clicked()), TQT_SLOT(cursorCheck()) );
  gbox->setRowStretch( 3, 10 );

  TDEConfig &config = *kapp->config();
  config.setGroup("Insert Pattern Dialog");
  mCheckPattern->setChecked( config.readBoolEntry( "RepeatPattern", false ) );
  mCheckOnCursor->setChecked( config.readBoolEntry( "InsertOnCursor", false) );
  cursorCheck();
  uint val = config.readUnsignedNumEntry( "Format", 0 );
  mPatternSelector->setCurrentItem( TQMIN(4,val) );
  patternSelectorChanged( mPatternSelector->currentItem() );
}


CInsertDialog::~CInsertDialog( void )
{
  TDEConfig &config = *kapp->config();
  config.setGroup("Insert Pattern Dialog");
  config.writeEntry( "RepeatPattern", mCheckPattern->isChecked() );
  config.writeEntry( "InsertOnCursor", mCheckOnCursor->isChecked() );
  config.writeEntry( "Format", mPatternSelector->currentItem() );
  config.sync();
}


void CInsertDialog::showEvent( TQShowEvent *e )
{
  KDialogBase::showEvent(e);
  mPatternInput->setFocus();
}


void CInsertDialog::patternSelectorChanged( int index )
{
  mPatternValidator->setState( (CHexValidator::EState)index );
  mPatternInput->setText( mPatternString[ index ] );
}


void CInsertDialog::patternInputChanged( const TQString &text )
{
  mPatternString[ mPatternSelector->currentItem() ] = text;
  mPatternValidator->convert( mPatternData,
    mPatternString[ mPatternSelector->currentItem() ] );
}


void CInsertDialog::slotOk( void )
{
  if( mPatternData.isEmpty() == true )
  {
    showEntryFailure( this, TQString("") );
    return;
  }

  SInsertData id;
  id.size = mSizeBox->value();
  id.repeatPattern = mCheckPattern->isChecked();
  id.onCursor = mCheckOnCursor->isChecked();
  id.pattern = mPatternData;

  if( id.onCursor == false )
  {
    bool success = stringToOffset( mOffsetInput->text(), id.offset );
    if( success == false )
    {
      showEntryFailure( this, TQString("") );
      return;
    }
  }

  hide();
  execute( id );
}


void CInsertDialog::cursorCheck( void )
{
  bool state = mCheckOnCursor->isChecked() == true ? false : true;
  mOffsetLabel->setEnabled( state );
  mOffsetInput->setEnabled( state );
}




void centerDialog( TQWidget *widget, TQWidget *centerParent )
{
  if( centerParent == 0 || widget == 0 )
  {
    return;
  }

  TQPoint point = centerParent->mapToGlobal( TQPoint(0,0) );
  TQRect pos    = centerParent->geometry();

  widget->setGeometry( point.x() + pos.width()/2  - widget->width()/2,
		       point.y() + pos.height()/2 - widget->height()/2,
		       widget->width(), widget->height() );
}


void centerDialogBottom( TQWidget *widget, TQWidget *centerParent )
{
  if( centerParent == 0 || widget == 0 )
  {
    return;
  }

  TQPoint point = centerParent->mapToGlobal( TQPoint(0,0) );
  TQRect pos    = centerParent->geometry();

  widget->setGeometry( point.x() + pos.width()/2  - widget->width()/2,
		       point.y() + pos.height() - widget->height(),
		       widget->width(), widget->height() );
}


void comboMatchText( TQComboBox *combo, const TQString &text )
{
  for( int i=0; i < combo->count(); i++ )
  {
    if( combo->text(i) == text )
    {
      combo->setCurrentItem(i);
      return;
    }
  }
  combo->setCurrentItem(0);
}





void showEntryFailure( TQWidget *parent, const TQString &msg )
{
  TQString message;
  message += i18n("Your request can not be processed.");
  message += "\n";
  if( msg.isNull() == true || msg.isEmpty() == true )
  {
    message += i18n("Examine argument(s) and try again.");
  }
  else
  {
    message += msg;
  }
  KMessageBox::sorry( parent, message, i18n("Invalid argument(s)") );
}


bool verifyFileDestnation( TQWidget *parent, const TQString &title,
			   const TQString &path )
{
  if( path.isEmpty() == true )
  {
    TQString msg = i18n("You must specify a destination file.");
    KMessageBox::sorry( parent, msg, title );
    return( false );
  }

  TQFileInfo info( path );
  if( info.exists() == true )
  {
    if( info.isDir() == true )
    {
      TQString msg = i18n("You have specified an existing folder.");
      KMessageBox::sorry( parent, msg, title );
      return( false );
    }

    if( info.isWritable() == false )
    {
      TQString msg = i18n("You do not have write permission to this file.");
      KMessageBox::sorry( parent, msg, title );
      return( false );
    }

    TQString msg = i18n( ""
      "You have specified an existing file.\n"
      "Overwrite current file?" );
    int reply = KMessageBox::warningContinueCancel( parent, msg, title, i18n("Overwrite") );
    if( reply != KMessageBox::Continue )
    {
      return( false );
    }
  }

  return( true );
}



bool stringToOffset( const TQString & text, uint &offset )
{
  if( text.isEmpty() )
  {
    return( false );
  }

  const char * p = text.ascii(); // ####: Is this correct?

  //
  // Skip any whitespaces in front of string
  //
  for( ; *p != 0 && isspace( *p ) ; p++ );

  int match = 0;
  bool space = false;
  if( strncmp( p, "0x", 2 ) == 0 || strncmp( p, "0X", 2 ) == 0 )
  {
    for( const char *q = p+2; *q != 0; q++ )
    {
      if( isxdigit( *q ) == 0 || space == true )
      {
	if( isspace( *q ) == 0 )
	{
	  return( false );
	}
	space = true;
      }
    }
    match = sscanf( p+2, "%x", &offset );
  }
  else
  {
    for( const char *q = p; *q != 0; q++ )
    {
      if( isdigit( *q ) == 0 || space == true )
      {
	if( isspace( *q ) == 0 )
	{
	  return( false );
	}
	space = true;
      }
    }
    match = sscanf( p, "%u", &offset );
  }

  if( match == 0 )
  {
    return( false );
  }

  return( true );
}


static const TQStringList &formatStrings( void )
{
  static TQStringList list;
  if( list.isEmpty() == true )
  {
    list.append( i18n( "Hexadecimal" ) );
    list.append( i18n( "Decimal" ) );
    list.append( i18n( "Octal" ) );
    list.append( i18n( "Binary" ) );
    list.append( i18n( "Regular Text" ) );
  }
  return( list );
}


static const TQStringList &operationStrings( void )
{
  static TQStringList list;
  if( list.isEmpty() == true )
  {
    list.append( i18n( "operand AND data" ) );
    list.append( i18n( "operand OR data" ) );
    list.append( i18n( "operand XOR data" ) );
    list.append( i18n( "INVERT data" ) );
    list.append( i18n( "REVERSE data" ) );
    list.append( i18n( "ROTATE data" ) );
    list.append( i18n( "SHIFT data" ) );
    list.append( i18n( "Swap Individual Bits" ) );
  }

  return( list );
}





#include "dialog.moc"
