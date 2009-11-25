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

#include <qvbox.h>
#include <qvbuttongroup.h>
#include <qpainter.h>

#include <qcheckbox.h>
#include <qlayout.h>
#include <qlabel.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qwhatsthis.h>

#include <kcolordialog.h>
#include <kcolordrag.h>
#include <kfontdialog.h> // For KFontChooser
#include <kiconloader.h>
#include <klocale.h>
#include <kglobalsettings.h>

#include "optiondialog.h"
#include <qpushbutton.h>
#include <qcombobox.h>

#if 0
#include <qobjectlist.h>
static void enableWidget( QWidget *w, bool state )
{
  if( w->children() )
  {
    QObjectList *l = (QObjectList*)w->children(); // silence please
    for( uint i=0; i < l->count(); i++ )
    {
      QObject *o = l->at(i);
      if( o->isWidgetType() )
      {
	enableWidget( (QWidget*)o, state );
      }
    }
  }
  w->setEnabled( state );
}
#endif



COptionDialog::COptionDialog( QWidget *parent, char *name, bool modal )
  :KDialogBase( IconList, i18n("Configure"), Help|Default|Apply|Ok|Cancel,
		Ok, parent, name, modal, true )
{
  setHelp( "khexedit/khexedit.html", QString::null );

  setupLayoutPage();
  setupCursorPage();
  setupFontPage();
  setupColorPage();
  setupFilePage();
  setupMiscPage();
  enableButton( Apply, false );
  configChanged = false;
}


COptionDialog::~COptionDialog( void )
{
}

void COptionDialog::slotChanged()
{
  enableButton( Apply, true );
  configChanged = true;
}

void COptionDialog::showEvent( QShowEvent *e )
{
  KDialogBase::showEvent(e);
  showPage(0);
  mLayout.lineSizeSpin->setFocus();
}


void COptionDialog::setupLayoutPage( void )
{
  QString text;
  QFrame *page = addPage( i18n("Layout"), i18n("Data Layout in Editor"),
			  BarIcon("khexedit", KIcon::SizeMedium ) );

  QGridLayout *gbox = new QGridLayout( page, 15, 2, 0, spacingHint() );
  gbox->setColStretch( 1, 10 );

  mLayout.formatCombo  = new QComboBox( false, page );
  QStringList modeList;
  modeList.append( i18n("Hexadecimal Mode") );
  modeList.append( i18n("Decimal Mode") );
  modeList.append( i18n("Octal Mode") );
  modeList.append( i18n("Binary Mode") );
  modeList.append( i18n("Text Only Mode") );
  mLayout.formatCombo->insertStringList( modeList );
  connect( mLayout.formatCombo, SIGNAL(activated(int)),
	   SLOT(slotModeSelectorChanged(int)) );
  connect( mLayout.formatCombo, SIGNAL(activated(int)),
    SLOT(slotChanged()) );
  gbox->addWidget( mLayout.formatCombo, 0, 1 );

  //
  // I am setting the min. width for one widget in the second column
  // This widtk will be used by every widget in this column.
  //
  mLayout.lineSizeSpin = new QSpinBox( page );
  mLayout.lineSizeSpin->setMinimumWidth( fontMetrics().width("M") * 10 );
  mLayout.lineSizeSpin->setRange( 1, 10000 );
  connect( mLayout.lineSizeSpin, SIGNAL(valueChanged(int)),
	   SLOT(slotLineSizeChanged(int) ) );
  connect( mLayout.lineSizeSpin, SIGNAL(valueChanged(int)),
    SLOT(slotChanged()) );
  gbox->addWidget( mLayout.lineSizeSpin, 1, 1 );

  mLayout.columnSizeSpin = new QSpinBox( page );
  mLayout.columnSizeSpin->setRange( 1, 10000 );
  connect( mLayout.columnSizeSpin, SIGNAL(valueChanged(int)),
	   SLOT(slotColumnSizeChanged(int) ) );
  connect( mLayout.columnSizeSpin, SIGNAL(valueChanged(int)),
    SLOT(slotChanged() ) );
  gbox->addWidget( mLayout.columnSizeSpin, 2, 1 );

  text = i18n("Default l&ine size [bytes]:");
  QLabel *label = new QLabel( mLayout.lineSizeSpin, text, page );
  gbox->addWidget( label, 1, 0 );

  text = i18n("Colu&mn size [bytes]:");
  label = new QLabel( mLayout.columnSizeSpin, text, page );
  gbox->addWidget( label, 2, 0 );

  QFrame *hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  gbox->addMultiCellWidget( hline, 3, 3, 0, 1 );

  text = i18n("Line size is &fixed (use scrollbar when required)");
  mLayout.lockLineCheck = new QCheckBox( text, page );
  gbox->addMultiCellWidget( mLayout.lockLineCheck, 4, 4, 0, 1, AlignLeft );
  connect( mLayout.lockLineCheck, SIGNAL(toggled(bool)),
    SLOT(slotChanged()) );

  text = i18n("Loc&k column at end of line (when column size>1)");
  mLayout.lockColumnCheck = new QCheckBox( text, page );
  gbox->addMultiCellWidget( mLayout.lockColumnCheck, 5, 5, 0, 1 );
  connect( mLayout.lockColumnCheck, SIGNAL(toggled(bool)),
    SLOT(slotChanged()) );

  hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  gbox->addMultiCellWidget( hline, 6, 6, 0, 1 );

  QStringList gridList;
  gridList.append( i18n("None") );
  gridList.append( i18n("Vertical Only") );
  gridList.append( i18n("Horizontal Only") );
  gridList.append( i18n("Both Directions") );

  mLayout.gridCombo = new QComboBox( false, page );
  mLayout.gridCombo->insertStringList( gridList );
  connect( mLayout.gridCombo, SIGNAL(activated(int)),
    SLOT(slotChanged()) );

  text = i18n("&Gridlines between text:");
  label = new QLabel( mLayout.gridCombo, text, page );

  gbox->addWidget( label, 7, 0 );
  gbox->addWidget( mLayout.gridCombo, 7, 1 );

  mLayout.leftSepWidthSpin = new QSpinBox( page );
  mLayout.leftSepWidthSpin->setRange( 0, 20 );
  gbox->addWidget( mLayout.leftSepWidthSpin, 8, 1 );
  connect( mLayout.leftSepWidthSpin, SIGNAL(valueChanged(int)),
    SLOT(slotChanged()) );

  mLayout.rightSepWidthSpin = new QSpinBox( page );
  mLayout.rightSepWidthSpin->setRange( 0, 20 );
  gbox->addWidget( mLayout.rightSepWidthSpin, 9, 1 );
  connect( mLayout.rightSepWidthSpin, SIGNAL(valueChanged(int)),
    SLOT(slotChanged()) );

  text = i18n("&Left separator width [pixels]:");
  mLayout.leftSepLabel = new QLabel( mLayout.leftSepWidthSpin, text, page );
  gbox->addWidget( mLayout.leftSepLabel, 8, 0 );

  text = i18n("&Right separator width [pixels]:");
  mLayout.rightSepLabel = new QLabel( mLayout.rightSepWidthSpin, text, page );
  gbox->addWidget( mLayout.rightSepLabel, 9, 0 );

  mLayout.separatorSpin = new QSpinBox( page );
  mLayout.separatorSpin->setRange( 0, 20 );
  gbox->addWidget( mLayout.separatorSpin, 10, 1 );
  connect( mLayout.separatorSpin, SIGNAL(valueChanged(int)),
    SLOT(slotChanged()) );

  mLayout.edgeSpin = new QSpinBox( page );
  mLayout.edgeSpin->setRange( 0, 20 );
  gbox->addWidget( mLayout.edgeSpin, 11, 1 );
  connect( mLayout.edgeSpin, SIGNAL(valueChanged(int)),
    SLOT(slotChanged()) );

  text = i18n("&Separator margin width [pixels]:");
  label = new QLabel( mLayout.separatorSpin, text, page );
  gbox->addWidget( label, 10, 0 );

  text = i18n("&Edge margin width [pixels]:");
  label = new QLabel( mLayout.edgeSpin, text, page );
  gbox->addWidget( label, 11, 0 );

  text = i18n("Column separation is e&qual to one character");
  mLayout.columnCheck = new QCheckBox( text, page );
  gbox->addMultiCellWidget( mLayout.columnCheck, 12, 12, 0, 1, AlignLeft );
  connect( mLayout.columnCheck, SIGNAL(toggled(bool)),
	   SLOT( slotColumnSepCheck(bool)));
  connect( mLayout.columnCheck, SIGNAL(toggled(bool)),
    SLOT( slotChanged()));

  mLayout.columnSepSpin = new QSpinBox( page );
  mLayout.columnSepSpin->setRange( 1, 100 );
  connect( mLayout.columnSepSpin, SIGNAL(valueChanged(int)),
    SLOT(slotChanged()) );

  text = i18n("Column separa&tion [pixels]:");
  mLayout.columnSepLabel = new QLabel( mLayout.columnSepSpin, text, page );

  gbox->addWidget( mLayout.columnSepLabel, 13, 0 );
  gbox->addWidget( mLayout.columnSepSpin, 13, 1 );

  gbox->setRowStretch( 14, 10 );
}


void COptionDialog::setupCursorPage( void )
{
  QString text;
  QFrame *page = addPage( i18n("Cursor"),
    i18n("Cursor Behavior (only valid for editor)"),
    BarIcon("mouse", KIcon::SizeMedium ) );
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  QVButtonGroup *group = new QVButtonGroup( i18n("Blinking"), page );
  group->layout()->setMargin( spacingHint() );
  topLayout->addWidget( group );

  text = i18n("Do not b&link");
  mCursor.blinkCheck = new QCheckBox( i18n("Do not b&link"), group );
  connect( mCursor.blinkCheck, SIGNAL(toggled(bool)),
	   SLOT( slotBlinkIntervalCheck(bool)));
  connect( mCursor.blinkCheck, SIGNAL(toggled(bool)),
    SLOT( slotChanged()));

  QHBox *hbox = new QHBox( group );
  mCursor.blinkLabel = new QLabel( i18n("&Blink interval [ms]:" ), hbox );
  mCursor.blinkSpin = new QSpinBox( hbox );
  mCursor.blinkSpin->setMinimumWidth( fontMetrics().width("M") * 10 );
  mCursor.blinkSpin->setRange( 100, 1000 );
  mCursor.blinkSpin->setSteps( 100, 100 );
  mCursor.blinkSpin->setValue( 500 );
  mCursor.blinkLabel->setBuddy(mCursor.blinkSpin);
  connect( mCursor.blinkSpin, SIGNAL(valueChanged(int)),
    SLOT( slotChanged()));

  group = new QVButtonGroup( i18n("Shape"), page );
  group->layout()->setMargin( spacingHint() );
  topLayout->addWidget( group );

  text = i18n("Always &use block (rectangular) cursor");
  mCursor.blockCheck = new QCheckBox( text, group );
  connect( mCursor.blockCheck, SIGNAL(toggled(bool)),
	   SLOT( slotBlockCursorCheck(bool)));
  connect( mCursor.blockCheck, SIGNAL(toggled(bool)),
    SLOT( slotChanged()));
  text = i18n("Use &thick cursor in insert mode");
  mCursor.thickCheck = new QCheckBox( text, group );
  connect( mCursor.thickCheck, SIGNAL(toggled(bool)),
    SLOT( slotChanged()));

  text = i18n("Cursor Behavior When Editor Loses Focus");
  group = new QVButtonGroup( text, page );
  group->layout()->setMargin( spacingHint() );
  topLayout->addWidget( group );

  text = i18n("&Stop blinking (if blinking is enabled)");
  mCursor.stopRadio = new QRadioButton( text, group, "radio1" );
  mCursor.hideRadio = new QRadioButton( i18n("H&ide"), group, "radio2" );
  text = i18n("Do &nothing");
  mCursor.nothingRadio = new QRadioButton( text, group, "radio3" );
  connect( mCursor.stopRadio, SIGNAL(toggled(bool)),
    SLOT( slotChanged()));
  connect( mCursor.hideRadio, SIGNAL(toggled(bool)),
    SLOT( slotChanged()));
  connect( mCursor.nothingRadio, SIGNAL(toggled(bool)),
    SLOT( slotChanged()));

  topLayout->addStretch(10);
}


void COptionDialog::setupColorPage( void )
{
  QString text;
  QFrame *page = addPage( i18n("Colors"),
    i18n("Editor Colors (system selection color is always used)"),
    BarIcon("colorize", KIcon::SizeMedium ) );

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  text = i18n("&Use system colors (as chosen in Control Center)");
  mColor.checkSystem = new QCheckBox( text, page );
  connect( mColor.checkSystem, SIGNAL(toggled(bool)),
	   SLOT( slotColorSystem(bool)));
  connect( mColor.checkSystem, SIGNAL(toggled(bool)),
    SLOT( slotChanged()));
  topLayout->addWidget( mColor.checkSystem );

  QFrame *hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addWidget( hline );

  QStringList modeList;
  modeList.append( i18n("First, Third ... Line Background") );
  modeList.append( i18n("Second, Fourth ... Line Background") );
  modeList.append( i18n("Offset Background") );
  modeList.append( i18n("Inactive Background") );
  modeList.append( i18n("Even Column Text") );
  modeList.append( i18n("Odd Column Text") );
  modeList.append( i18n("Non Printable Text") );
  modeList.append( i18n("Offset Text") );
  modeList.append( i18n("Secondary Text") );
  modeList.append( i18n("Marked Background") );
  modeList.append( i18n("Marked Text") );
  modeList.append( i18n("Cursor Background") );
  modeList.append( i18n("Cursor Text (block shape)") );
  modeList.append( i18n("Bookmark Background") );
  modeList.append( i18n("Bookmark Text") );
  modeList.append( i18n("Separator") );
  modeList.append( i18n("Grid Lines") );

  mColor.colorList = new CColorListBox( page );
  topLayout->addWidget( mColor.colorList, 10 );
  for( uint i=0; i<17; i++ )
  {
    CColorListItem *listItem = new CColorListItem( modeList[i] );
    mColor.colorList->insertItem( listItem );
  }
  mColor.colorList->setCurrentItem(0);
  connect( mColor.colorList, SIGNAL( dataChanged() ), this, SLOT( slotChanged() ) );
}


void COptionDialog::setupFontPage( void )
{
  QString text;
  QFrame *page = addPage( i18n("Font"),
    i18n("Font Selection (editor can only use a fixed font)"),
    BarIcon("fonts", KIcon::SizeMedium ) );

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  text = i18n("&Use system font (as chosen in Control Center)");
  mFont.checkSystem = new QCheckBox( text, page );
  connect( mFont.checkSystem, SIGNAL(toggled(bool)),
	   this, SLOT( slotFontSystem(bool)));
  connect( mFont.checkSystem, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));
  topLayout->addWidget( mFont.checkSystem );

  QFrame *hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addWidget( hline );

  mFont.chooser = new KFontChooser( page, "font", true, QStringList(), false, 4 );
  topLayout->addWidget( mFont.chooser );
  QFont fixFont( KGlobalSettings::fixedFont() );
  fixFont.setBold(true);
  mFont.chooser->setFont( fixFont, true );
  mFont.chooser->setSampleText( i18n("KHexEdit editor font") );
  connect( mFont.chooser, SIGNAL(fontSelected(const QFont &)),
    this, SLOT( slotChanged()));

  hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addWidget( hline );

  QHBoxLayout *hbox = new QHBoxLayout();
  topLayout->addLayout( hbox );

  mFont.nonPrintInput = new QLineEdit( page );
  mFont.nonPrintInput->setMaxLength( 1 );
  connect( mFont.nonPrintInput, SIGNAL(textChanged(const QString &)),
    this, SLOT( slotChanged()));

  text = i18n("&Map non printable characters to:");
  QLabel *nonPrintLabel = new QLabel( mFont.nonPrintInput, text, page );

  hbox->addWidget( nonPrintLabel, 0, AlignLeft );
  hbox->addSpacing( spacingHint() );
  hbox->addWidget( mFont.nonPrintInput, 10 );

  topLayout->addStretch(10);
}


void COptionDialog::setupFilePage( void )
{
  QString text;
  QFrame *page = addPage( i18n("Files"), i18n("File Management"),
			  BarIcon("kmultiple", KIcon::SizeMedium ) );

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );

  QGridLayout *gbox = new QGridLayout( 2, 2, spacingHint() );
  topLayout->addLayout( gbox );

  mFile.openCombo = new QComboBox( false, page );
  QStringList modeList;
  modeList.append( i18n("None") );
  modeList.append( i18n("Most Recent Document") );
  modeList.append( i18n("All Recent Documents") );
  mFile.openCombo->insertStringList( modeList );
  mFile.openCombo->setMinimumWidth( mFile.openCombo->sizeHint().width() );
  connect( mFile.openCombo, SIGNAL(activated(int)),
    this, SLOT( slotChanged()));

  text = i18n("Open doc&uments on startup:");
  QLabel *label = new QLabel( mFile.openCombo, text, page );

  gbox->addWidget( label, 0, 0 );
  gbox->addWidget( mFile.openCombo, 0, 1 );

  text = i18n("&Jump to previous cursor position on startup");
  mFile.gotoOffsetCheck = new QCheckBox( text, page );
  // ### TODO: this is currently not available.
  mFile.gotoOffsetCheck->setChecked( false );
  mFile.gotoOffsetCheck->setEnabled( false );
  // ### END
  topLayout->addWidget( mFile.gotoOffsetCheck, 0, AlignLeft );
  connect( mFile.gotoOffsetCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  QFrame *hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addWidget( hline );

  text = i18n("Open document with &write protection enabled");
  mFile.writeProtectCheck = new QCheckBox( text, page );
  topLayout->addWidget( mFile.writeProtectCheck, 0, AlignLeft );
  connect( mFile.writeProtectCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  text = i18n("&Keep cursor position after reloading document");
  mFile.reloadOffsetCheck = new QCheckBox( text, page );
  topLayout->addWidget( mFile.reloadOffsetCheck, 0, AlignLeft );
  connect( mFile.reloadOffsetCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  text = i18n("&Make a backup when saving document");
  mFile.backupCheck = new QCheckBox( text, page );
  topLayout->addWidget( mFile.backupCheck, 0, AlignLeft );
  connect( mFile.backupCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addWidget( hline );

  text = i18n("Don't &save \"Recent\" document list on exit");
  mFile.discardRecentCheck = new QCheckBox( text, page );
  topLayout->addWidget( mFile.discardRecentCheck, 0, AlignLeft );
  QWhatsThis::add( mFile.discardRecentCheck,
                   i18n( "Clicking this check box makes KHexEdit forget his recent document list "
                          "when the program is closed.\n"
                          "Note: it will not erase any document of the recent document list "
                          "created by KDE." ) );
  connect( mFile.discardRecentCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  text = i18n("Cl&ear \"Recent\" Document List");
  QPushButton *discardRecentButton = new QPushButton( page );
  discardRecentButton->setText( text );
  QWhatsThis::add( discardRecentButton,
                   i18n( "Clicking this button makes KHexEdit forget his recent document list.\n"
                          "Note: it will not erase any document of the recent document list "
                          "created by KDE." ) );
  topLayout->addWidget( discardRecentButton, 0, AlignCenter );
  connect( discardRecentButton, SIGNAL(clicked()),
	   SIGNAL(removeRecentFiles()) );

  topLayout->addStretch(10);
}



void COptionDialog::setupMiscPage( void )
{
  QString text;
  QLabel *label;

  QFrame *page = addPage( i18n("Miscellaneous"), i18n("Various Properties"),
			  BarIcon("gear", KIcon::SizeMedium ) );

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );


  text = i18n("Auto&matic copy to clipboard when selection is ready");
  mMisc.autoCheck = new QCheckBox( text, page );
  topLayout->addWidget( mMisc.autoCheck, 0, AlignLeft );
  connect( mMisc.autoCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  text = i18n("&Editor starts in \"insert\" mode" );
  mMisc.insertCheck = new QCheckBox( text, page );
  topLayout->addWidget( mMisc.insertCheck, 0, AlignLeft );
  connect( mMisc.insertCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  text = i18n("Confirm &wrapping (to beginning or end) during search");
  mMisc.confirmWrapCheck = new QCheckBox( text, page );
  topLayout->addWidget( mMisc.confirmWrapCheck, 0, AlignLeft );
  connect( mMisc.confirmWrapCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  text = i18n("Cursor jumps to &nearest byte when moved");
  mMisc.cursorJumpCheck = new QCheckBox( text, page );
  topLayout->addWidget( mMisc.cursorJumpCheck, 0, AlignLeft );
  connect( mMisc.cursorJumpCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  QVButtonGroup *group = new QVButtonGroup( i18n("Sounds"), page );
  group->layout()->setMargin( spacingHint() );
  topLayout->addWidget( group );
  text = i18n("Make sound on data &input (eg. typing) failure");
  mMisc.inputCheck = new QCheckBox( text, group );
  connect( mMisc.inputCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));
  text = i18n("Make sound on &fatal failure");
  mMisc.fatalCheck = new QCheckBox( text, group );
  connect( mMisc.fatalCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  group = new QVButtonGroup( i18n("Bookmark Visibility"), page );
  group->layout()->setMargin( spacingHint() );
  topLayout->addWidget( group );
  text = i18n("Use visible bookmarks in the offset column");
  mMisc.bookmarkColumnCheck = new QCheckBox( text, group );
  connect( mMisc.bookmarkColumnCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));
  text = i18n("Use visible bookmarks in the editor fields");
  mMisc.bookmarkEditorCheck = new QCheckBox( text, group );
  connect( mMisc.bookmarkEditorCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));

  text = i18n("Confirm when number of printed pages will e&xceed limit");
  mMisc.thresholdCheck = new QCheckBox( text, page );
  connect( mMisc.thresholdCheck, SIGNAL(clicked()),
	   SLOT( slotThresholdConfirm()));
  connect( mMisc.thresholdCheck, SIGNAL(toggled(bool)),
    this, SLOT( slotChanged()));
  topLayout->addWidget( mMisc.thresholdCheck, 0, AlignLeft );

  QGridLayout *glay = new QGridLayout( 3, 3 );
  glay->setColStretch(2,10);
  topLayout->addLayout( glay );

  mMisc.thresholdSpin = new QSpinBox( page );
  mMisc.thresholdSpin->setMinimumWidth( fontMetrics().width("M") * 10 );
  mMisc.thresholdSpin->setRange( 5, INT_MAX );
  mMisc.thresholdSpin->setSteps( 5, 5 );
  mMisc.thresholdSpin->setValue( 5 );
  connect( mMisc.thresholdSpin, SIGNAL(valueChanged(int)),
    this, SLOT( slotChanged()));

  text = i18n("&Threshold [pages]:" );
  mMisc.thresholdLabel = new QLabel( mMisc.thresholdSpin, text, page );

  glay->addWidget( mMisc.thresholdLabel, 0, 0 );
  glay->addWidget( mMisc.thresholdSpin, 0, 1 );

  QFrame *hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  glay->addMultiCellWidget( hline, 1, 1, 0, 2 );

  mMisc.undoSpin = new QSpinBox( page );
  mMisc.undoSpin->setRange( 10, 10000 );
  mMisc.undoSpin->setSteps( 5, 5 );
  mMisc.undoSpin->setValue( 50 );
  connect( mMisc.undoSpin, SIGNAL(valueChanged(int)),
    this, SLOT( slotChanged()));

  label = new QLabel( mMisc.undoSpin, i18n("&Undo limit:"), page );
  glay->addWidget( label, 2, 0 );
  glay->addWidget( mMisc.undoSpin, 2, 1 );

  topLayout->addStretch(10);
}



void COptionDialog::slotModeSelectorChanged( int index )
{
  int lineSize = mDisplayState.line.getLineSize( index );
  int colSize  = mDisplayState.line.getColumnSize( index );
  mLayout.lineSizeSpin->setValue( lineSize );
  mLayout.columnSizeSpin->setValue( colSize );
}

void COptionDialog::slotLineSizeChanged( int value )
{
  int index = mLayout.formatCombo->currentItem();
  mDisplayState.line.setLineSize( index, value );
  if( index == SDisplayLine::textOnly )
  {
    mLayout.columnSizeSpin->setValue( value );
  }
}

void COptionDialog::slotColumnSizeChanged( int value )
{
  int index = mLayout.formatCombo->currentItem();
  mDisplayState.line.setColumnSize( index, value );
  if( index == SDisplayLine::textOnly )
  {
    mLayout.lineSizeSpin->setValue( value );
  }
}

void COptionDialog::slotColumnSepCheck( bool state )
{
  state = state == true ? false: true;
  mLayout.columnSepLabel->setEnabled( state );
  mLayout.columnSepSpin->setEnabled( state );
}


void COptionDialog::slotBlinkIntervalCheck( bool state )
{
  state = state == true ? false : true;
  mCursor.blinkSpin->setEnabled( state );
  mCursor.blinkLabel->setEnabled( state );
}

void COptionDialog::slotBlockCursorCheck( bool state )
{
  state = state == true ? false : true;
  mCursor.thickCheck->setEnabled( state );
}

void COptionDialog::slotFontSystem( bool state )
{
  mFont.chooser->setEnabled( state == true ? false : true );
}

void COptionDialog::slotColorSystem( bool state )
{
  state = state == true ? false : true;
  mColor.colorList->setEnabled( state );
}



void COptionDialog::slotThresholdConfirm( void )
{
  bool state = mMisc.thresholdCheck->isChecked();
  mMisc.thresholdLabel->setEnabled( state );
  mMisc.thresholdSpin->setEnabled( state );
}


void COptionDialog::slotDefault( void )
{
  //
  // We use the constructor settings as default values
  //
  switch( activePageIndex() )
  {
    case page_layout:
    {
      SDisplayLayout layout;
      SDisplayLine line;
      setLayout( layout, line );
    }
    break;

    case page_cursor:
    {
      SDisplayCursor cursor;
      setCursor( cursor );
    }
    break;

    case page_color:
    {
      SDisplayColor color;
      setColor( color );
    }
    break;

    case page_font:
    {
      SDisplayFont font;
      setFont( font );
    }
    break;

    case page_file:
    {
      SDisplayMisc misc;
      setFile( misc );
    }
    break;

    case page_misc:
    {
      SDisplayMisc misc;
      setMisc( misc );
    }
    break;
  }
}


void COptionDialog::slotOk( void )
{
  if( configChanged )
    slotApply();
  accept();
}

void COptionDialog::slotApply( void )
{
  switch( activePageIndex() )
  {
    case page_layout:
    {
      int index = mLayout.formatCombo->currentItem();
      mDisplayState.line.setLineSize(index, mLayout.lineSizeSpin->value());
      mDisplayState.line.setColumnSize(index, mLayout.columnSizeSpin->value());
      emit lineSizeChoice( mDisplayState.line );

      SDisplayLayout &l = mDisplayState.layout;
      l.lockLine = mLayout.lockLineCheck->isChecked();
      l.lockColumn = mLayout.lockColumnCheck->isChecked();
      l.leftSeparatorWidth = mLayout.leftSepWidthSpin->value();
      l.rightSeparatorWidth = mLayout.rightSepWidthSpin->value();
      l.separatorMarginWidth = mLayout.separatorSpin->value();
      l.edgeMarginWidth = mLayout.edgeSpin->value();
      l.leftSeparatorWidth = mLayout.leftSepWidthSpin->value();
      l.rightSeparatorWidth = mLayout.rightSepWidthSpin->value();
      l.columnCharSpace = mLayout.columnCheck->isChecked();
      l.columnSpacing = mLayout.columnSepSpin->value();

      if( mLayout.gridCombo->currentItem() == 0 )
      {
	l.horzGridWidth = l.vertGridWidth = 0;
      }
      else if( mLayout.gridCombo->currentItem() == 1 )
      {
	l.horzGridWidth = 0;
	l.vertGridWidth = 1;
      }
      else if( mLayout.gridCombo->currentItem() == 2 )
      {
	l.horzGridWidth = 1;
	l.vertGridWidth = 0;
      }
      else
      {
	l.horzGridWidth = l.vertGridWidth = 1;
      }

      emit layoutChoice( mDisplayState.layout );
    }
    break;


    case page_cursor:
    {
      SDisplayCursor &c = mDisplayState.cursor;
      c.alwaysVisible    = mCursor.blinkCheck->isChecked();
      c.interval         = mCursor.blinkSpin->value();
      c.alwaysBlockShape = mCursor.blockCheck->isChecked();
      c.thickInsertShape = mCursor.thickCheck->isChecked();
      c.focusMode        = cursorFocusMode();
      emit cursorChoice( mDisplayState.cursor );
    }
    break;

    case page_color:
    {
      SDisplayColor &c = mDisplayState.color;
      c.useSystemColor = mColor.checkSystem->isChecked();
      c.textBg = mColor.colorList->color( SColorWidgets::FirstTextBg );
      c.secondTextBg = mColor.colorList->color( SColorWidgets::SecondTextBg );
      c.offsetBg = mColor.colorList->color( SColorWidgets::OffsetBg );
      c.inactiveBg = mColor.colorList->color( SColorWidgets::InactiveBg );
      c.markBg = mColor.colorList->color( SColorWidgets::MarkedBg );
      c.markFg = mColor.colorList->color( SColorWidgets::MarkedFg );
      c.primaryFg[0] = mColor.colorList->color( SColorWidgets::EvenColumnFg );
      c.primaryFg[1] = mColor.colorList->color( SColorWidgets::OddColumnFg );
      c.offsetFg  = mColor.colorList->color( SColorWidgets::OffsetFg );
      c.secondaryFg = mColor.colorList->color( SColorWidgets::SecondaryFg );
      c.cursorBg  = mColor.colorList->color( SColorWidgets::CursorBg );
      c.cursorFg  = mColor.colorList->color( SColorWidgets::CursorFg );
      c.leftSeparatorFg = mColor.colorList->color(SColorWidgets::SeparatorFg);
      c.rightSeparatorFg = mColor.colorList->color(SColorWidgets::SeparatorFg);
      c.bookmarkBg = mColor.colorList->color( SColorWidgets::BookmarkBg );
      c.bookmarkFg = mColor.colorList->color( SColorWidgets::BookmarkFg );
      c.nonPrintFg  = mColor.colorList->color( SColorWidgets::NonPrintFg );
      c.gridFg = mColor.colorList->color( SColorWidgets::GridFg );
      emit colorChoice( mDisplayState.color );
    }
    break;

    case page_font:
    {
      SDisplayFont &f = mDisplayState.font;
      f.useSystemFont = mFont.checkSystem->isChecked();
      f.localFont = mFont.chooser->font();

      QString nonPrintText = mFont.nonPrintInput->text();
      if( nonPrintText.isEmpty() )
      {
	f.nonPrintChar = ' ';
      }
      else
      {
	f.nonPrintChar = nonPrintText[0];
      }

      emit fontChoice( mDisplayState.font );
    }
    break;

    case page_file:
    case page_misc:
    {
      SDisplayMisc &m = mDisplayState.misc;
      m.undoLevel = mMisc.undoSpin->value();
      m.autoCopyToClipboard = mMisc.autoCheck->isChecked();
      m.insertMode = mMisc.insertCheck->isChecked();
      m.inputSound = mMisc.inputCheck->isChecked();
      m.confirmWrap = mMisc.confirmWrapCheck->isChecked();
      m.cursorJump = mMisc.cursorJumpCheck->isChecked();
      m.fatalSound = mMisc.fatalCheck->isChecked();
      m.confirmThreshold = mMisc.thresholdCheck->isChecked();
      m.thresholdValue = mMisc.thresholdSpin->value();
      m.bookmarkOffsetColumn = mMisc.bookmarkColumnCheck->isChecked();
      m.bookmarkEditor = mMisc.bookmarkEditorCheck->isChecked();

      m.openFile  = (SDisplayMisc::EOpenFile)mFile.openCombo->currentItem();
      m.gotoOnStartup = mFile.gotoOffsetCheck->isChecked();
      m.writeProtect = mFile.writeProtectCheck->isChecked();
      m.gotoOnReload = mFile.reloadOffsetCheck->isChecked();
      m.makeBackup = mFile.backupCheck->isChecked();
      m.discardRecent = mFile.discardRecentCheck->isChecked();

      emit miscChoice( mDisplayState.misc );
    }
    break;
  }
  enableButton( Apply, false );
  configChanged = false;
}

void COptionDialog::setLayout( SDisplayLayout &layout, SDisplayLine &line )
{
  mDisplayState.line = line;
  mDisplayState.layout = layout;

  slotModeSelectorChanged( mLayout.formatCombo->currentItem() );
  mLayout.lockLineCheck->setChecked( layout.lockLine );
  mLayout.lockColumnCheck->setChecked( layout.lockColumn );
  mLayout.leftSepWidthSpin->setValue( layout.leftSeparatorWidth );
  mLayout.rightSepWidthSpin->setValue( layout.rightSeparatorWidth );
  mLayout.separatorSpin->setValue( layout.separatorMarginWidth );
  mLayout.edgeSpin->setValue( layout.edgeMarginWidth );
  mLayout.leftSepWidthSpin->setValue( layout.leftSeparatorWidth );
  mLayout.rightSepWidthSpin->setValue( layout.rightSeparatorWidth );
  mLayout.columnCheck->setChecked( layout.columnCharSpace );
  slotColumnSepCheck( layout.columnCharSpace );
  mLayout.columnSepSpin->setValue( layout.columnSpacing );

  if( layout.horzGridWidth == 0 && layout.vertGridWidth == 0 )
  {
    mLayout.gridCombo->setCurrentItem(0);
  }
  else if( layout.horzGridWidth != 0 && layout.vertGridWidth != 0 )
  {
    mLayout.gridCombo->setCurrentItem(3);
  }
  else
  {
    mLayout.gridCombo->setCurrentItem( layout.vertGridWidth != 0 ? 1 : 2 );
  }

}


void COptionDialog::setCursor( SDisplayCursor &cursor )
{
  mDisplayState.cursor = cursor;

  mCursor.blinkCheck->setChecked( cursor.alwaysVisible );
  mCursor.blinkSpin->setValue( cursor.interval );
  mCursor.blockCheck->setChecked( cursor.alwaysBlockShape );
  mCursor.thickCheck->setChecked( cursor.thickInsertShape );
  bool state = cursor.focusMode == SDisplayCursor::stopBlinking ? true : false;
  mCursor.stopRadio->setChecked( state );
  state = cursor.focusMode == SDisplayCursor::hide ? true : false;
  mCursor.hideRadio->setChecked( state );
  state = cursor.focusMode == SDisplayCursor::ignore ? true : false;
  mCursor.nothingRadio->setChecked( state );
}


void COptionDialog::setColor( SDisplayColor &color )
{
  mDisplayState.color = color;

  mColor.checkSystem->setChecked( color.useSystemColor );
  mColor.colorList->setColor( SColorWidgets::FirstTextBg, color.textBg );
  mColor.colorList->setColor( SColorWidgets::SecondTextBg, color.secondTextBg);
  mColor.colorList->setColor( SColorWidgets::OffsetBg, color.offsetBg );
  mColor.colorList->setColor( SColorWidgets::InactiveBg, color.inactiveBg );
  mColor.colorList->setColor( SColorWidgets::MarkedBg, color.markBg );
  mColor.colorList->setColor( SColorWidgets::MarkedFg, color.markFg );
  mColor.colorList->setColor( SColorWidgets::EvenColumnFg, color.primaryFg[0]);
  mColor.colorList->setColor( SColorWidgets::OddColumnFg, color.primaryFg[1] );
  mColor.colorList->setColor( SColorWidgets::OffsetFg, color.offsetFg );
  mColor.colorList->setColor( SColorWidgets::SecondaryFg, color.secondaryFg );
  mColor.colorList->setColor( SColorWidgets::CursorBg, color.cursorBg );
  mColor.colorList->setColor( SColorWidgets::CursorFg, color.cursorFg );
  mColor.colorList->setColor(SColorWidgets::SeparatorFg,color.leftSeparatorFg);
  mColor.colorList->setColor( SColorWidgets::BookmarkBg, color.bookmarkBg );
  mColor.colorList->setColor( SColorWidgets::BookmarkFg, color.bookmarkFg );
  mColor.colorList->setColor( SColorWidgets::NonPrintFg, color.nonPrintFg );
  mColor.colorList->setColor( SColorWidgets::GridFg, color.gridFg );
}


void COptionDialog::setFont( SDisplayFont &font )
{
  mDisplayState.font = font;

  mFont.checkSystem->setChecked( font.useSystemFont );
  mFont.chooser->setFont( font.localFont, true );
  QString buf = font.nonPrintChar;
  mFont.nonPrintInput->setText( buf );
}


void COptionDialog::setMisc( SDisplayMisc &misc )
{
  mDisplayState.misc.undoLevel            = misc.undoLevel;
  mDisplayState.misc.inputSound           = misc.inputSound;
  mDisplayState.misc.fatalSound           = misc.fatalSound;
  mDisplayState.misc.autoCopyToClipboard  = misc.autoCopyToClipboard;
  mDisplayState.misc.insertMode           = misc.insertMode;
  mDisplayState.misc.confirmWrap          = misc.confirmWrap;
  mDisplayState.misc.cursorJump           = misc.cursorJump;
  mDisplayState.misc.confirmThreshold     = misc.confirmThreshold;
  mDisplayState.misc.thresholdValue       = misc.thresholdValue;
  mDisplayState.misc.bookmarkOffsetColumn = misc.bookmarkOffsetColumn;
  mDisplayState.misc.bookmarkEditor       = misc.bookmarkEditor;

  mMisc.autoCheck->setChecked( misc.autoCopyToClipboard );
  mMisc.insertCheck->setChecked( misc.insertMode );
  mMisc.confirmWrapCheck->setChecked( misc.confirmWrap );
  mMisc.cursorJumpCheck->setChecked( misc.cursorJump );

  mMisc.inputCheck->setChecked( misc.inputSound );
  mMisc.fatalCheck->setChecked( misc.fatalSound );
  mMisc.bookmarkColumnCheck->setChecked( misc.bookmarkOffsetColumn );
  mMisc.bookmarkEditorCheck->setChecked( misc.bookmarkEditor );

  mMisc.thresholdCheck->setChecked( misc.confirmThreshold );
  mMisc.thresholdSpin->setValue( misc.thresholdValue );
  slotThresholdConfirm();

  mMisc.undoSpin->setValue( misc.undoLevel );
}

void COptionDialog::setFile( SDisplayMisc &misc )
{
  mDisplayState.misc.openFile      = misc.openFile;
  mDisplayState.misc.gotoOnStartup = misc.gotoOnStartup;
  mDisplayState.misc.writeProtect  = misc.writeProtect;
  mDisplayState.misc.gotoOnReload  = misc.gotoOnReload;
  mDisplayState.misc.discardRecent = misc.discardRecent;
  mDisplayState.misc.makeBackup    = misc.makeBackup;

  mFile.openCombo->setCurrentItem( (uint)misc.openFile );
  mFile.gotoOffsetCheck->setChecked( misc.gotoOnStartup );
  mFile.writeProtectCheck->setChecked( misc.writeProtect );
  mFile.reloadOffsetCheck->setChecked( misc.gotoOnReload );
  mFile.discardRecentCheck->setChecked( misc.discardRecent );
  mFile.backupCheck->setChecked( misc.makeBackup );
}


void COptionDialog::setState( SDisplayState &state )
{
  setLayout( state.layout, state.line );
  setCursor( state.cursor );
  setColor( state.color );
  setFont( state.font );
  setFile( state.misc );
  setMisc( state.misc );
  enableButton( Apply, false );
  configChanged = false;
}


SDisplayCursor::EFocusMode COptionDialog::cursorFocusMode( void )
{
  if( mCursor.stopRadio->isChecked() == true )
  {
    return( SDisplayCursor::stopBlinking );
  }
  else if( mCursor.hideRadio->isChecked() == true )
  {
    return( SDisplayCursor::hide );
  }
  else
  {
    return( SDisplayCursor::ignore );
  }
}



CColorListBox::CColorListBox( QWidget *parent, const char *name, WFlags f )
  :KListBox( parent, name, f ), mCurrentOnDragEnter(-1)
{
  connect( this, SIGNAL(selected(int)), this, SLOT(newColor(int)) );
  setAcceptDrops( true);
}


void CColorListBox::setEnabled( bool state )
{
  if( state == isEnabled() )
  {
    return;
  }

  QListBox::setEnabled( state );
  for( uint i=0; i<count(); i++ )
  {
    updateItem( i );
  }
}


void CColorListBox::setColor( uint index, const QColor &color )
{
  if( index < count() )
  {
    CColorListItem *colorItem = (CColorListItem*)item(index);
    colorItem->setColor(color);
    updateItem( colorItem );
  }
}


const QColor CColorListBox::color( uint index )
{
  if( index < count() )
  {
    CColorListItem *colorItem = (CColorListItem*)item(index);
    return( colorItem->color() );
  }
  else
  {
    return( black );
  }
}


void CColorListBox::newColor( int index )
{
  if( isEnabled() == false )
  {
    return;
  }

  if( (uint)index < count() )
  {
    QColor c = color( index );
    if( KColorDialog::getColor( c, this ) != QDialog::Rejected )
    {
      setColor( index, c );
      emit dataChanged();
    }
  }
}


void CColorListBox::dragEnterEvent( QDragEnterEvent *e )
{
  if( KColorDrag::canDecode(e) && isEnabled() )
  {
    mCurrentOnDragEnter = currentItem();
    e->accept( true );
  }
  else
  {
    mCurrentOnDragEnter = -1;
    e->accept( false );
  }
}


void CColorListBox::dragLeaveEvent( QDragLeaveEvent * )
{
  if( mCurrentOnDragEnter != -1 )
  {
    setCurrentItem( mCurrentOnDragEnter );
    mCurrentOnDragEnter = -1;
  }
}


void CColorListBox::dragMoveEvent( QDragMoveEvent *e )
{
  if( KColorDrag::canDecode(e) && isEnabled() )
  {
    CColorListItem *item = (CColorListItem*)itemAt( e->pos() );
    if( item != 0 )
    {
      setCurrentItem ( item );
    }
  }
}


void CColorListBox::dropEvent( QDropEvent *e )
{
  QColor color;
  if( KColorDrag::decode( e, color ) )
  {
    int index = currentItem();
    if( index != -1 )
    {
      CColorListItem *colorItem = (CColorListItem*)item(index);
      colorItem->setColor(color);
      triggerUpdate( false ); // Redraw item
    }
    mCurrentOnDragEnter = -1;
  }
}



CColorListItem::CColorListItem( const QString &text, const QColor &color )
  : QListBoxItem(), mColor( color ), mBoxWidth( 30 )
{
  setText( text );
}


const QColor &CColorListItem::color( void )
{
  return( mColor );
}


void CColorListItem::setColor( const QColor &color )
{
  mColor = color;
}


void CColorListItem::paint( QPainter *p )
{
  QFontMetrics fm = p->fontMetrics();
  int h = fm.height();

  p->drawText( mBoxWidth+3*2, fm.ascent() + fm.leading()/2, text() );

  p->setPen( Qt::black );
  p->drawRect( 3, 1, mBoxWidth, h-1 );
  p->fillRect( 4, 2, mBoxWidth-2, h-3, mColor );
}


int CColorListItem::height(const QListBox *lb ) const
{
  return( lb->fontMetrics().lineSpacing()+1 );
}


int CColorListItem::width(const QListBox *lb ) const
{
  return( mBoxWidth + lb->fontMetrics().width( text() ) + 6 );
}






#include "optiondialog.moc"
