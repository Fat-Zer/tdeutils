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
#include <qbuttongroup.h>
#include <qlineedit.h> 

#include <kfiledialog.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "dialog.h"
#include "exportdialog.h"
#include <qpushbutton.h>


CExportDialog::CExportDialog( QWidget *parent, char *name, bool modal )
  :KDialogBase( Tabbed, i18n("Export Document"), Help|Ok|Cancel, Ok,
		parent, name,  modal )
{
  setHelp( "khexedit/khexedit.html", QString::null );

  mFrame[ page_destination ] = addPage( i18n("Destination") );
  mFrame[ page_option ] = addPage( i18n("Options") );

  setupDestinationPage();
  setupOptionPage();
 
  mConfig = 0;
  readConfiguration();

  QString path = mDestination.fileInput->text();
  int index = path.findRev( '/' );
  if( index != -1 ) { mWorkDir = path.left( index+1 ); }
}


CExportDialog::~CExportDialog( void )
{
  writeConfiguration();
  delete mConfig; mConfig = 0;
}


void CExportDialog::showEvent( QShowEvent *e )
{
  KDialogBase::showEvent(e);
  showPage(0);
  mDestination.fileInput->setFocus();
}


void CExportDialog::readConfiguration( void )
{
  if( mConfig != 0 ) { return; }

  mConfig = new KSimpleConfig( QString("hexexport") );
  if( mConfig == 0 ) { return; }

  mConfig->setGroup( "Destination" );

  int val = mConfig->readNumEntry( "Format", 0 );
  mDestination.formatCombo->setCurrentItem( val );
  formatChanged( val < 0 || val > option_html ? 0 : val );

  mConfig->setGroup( "Option" );
  val = mConfig->readNumEntry( "HtmlLine", 80 );
  mHtml.lineSpin->setValue( val );
  QString text = mConfig->readEntry( "HtmlPrefix", "table" );
  mHtml.prefixInput->setText( text );
  val = mConfig->readNumEntry( "HtmlHeader", 1 );
  mHtml.topCombo->setCurrentItem( val < 0 || val >= 4 ? 0 : val );
  val = mConfig->readNumEntry( "HtmlFooter", 3 );
  mHtml.bottomCombo->setCurrentItem( val < 0 || val >= 4 ? 0 : val );

  bool state = mConfig->readBoolEntry( "HtmlSymlink", true );
  mHtml.symlinkCheck->setChecked( state );  
  state = mConfig->readBoolEntry( "HtmlNavigatorBar", true );
  mHtml.navigatorCheck->setChecked( state );
  state = mConfig->readBoolEntry( "HtmlBlackWhite", false );
  mHtml.bwCheck->setChecked( state );

  text = mConfig->readEntry( "ArrayName", "buffer" );
  mArray.nameInput->setText( text );
  text = mConfig->readEntry( "ArrayElementType", "char" );
  for( int i=0; i < mArray.typeCombo->count(); i++ )
  { 
    if( mArray.typeCombo->text(i) == text )
    {
      mArray.typeCombo->setCurrentItem(i);
      break;
    }
  }
  val = mConfig->readNumEntry( "ArrayElementPerLine", 20 );
  val = QMAX( val, mArray.lineSizeSpin->minValue() );  
  val = QMIN( val, mArray.lineSizeSpin->maxValue() );
  mArray.lineSizeSpin->setValue( val );
  state = mConfig->readBoolEntry( "ArrayUnsignedAsHex", true );
  mArray.hexadecimalCheck->setChecked( state );
}


void CExportDialog::writeConfiguration( void )
{
  if( mConfig == 0 )
  {
    return;
  }

  mConfig->setGroup( "Destination" );
  mConfig->writeEntry( "Format", mDestination.formatCombo->currentItem() );

  mConfig->setGroup( "Option" );
  mConfig->writeEntry( "HtmlLine", mHtml.lineSpin->value() );
  mConfig->writeEntry( "HtmlPrefix", mHtml.prefixInput->text() );
  mConfig->writeEntry( "HtmlHeader", mHtml.topCombo->currentItem() );
  mConfig->writeEntry( "HtmlFooter", mHtml.bottomCombo->currentItem() );
  mConfig->writeEntry( "HtmlSymlink", mHtml.symlinkCheck->isChecked());
  mConfig->writeEntry( "HtmlNavigatorBar",mHtml.navigatorCheck->isChecked());
  mConfig->writeEntry( "HtmlBlackWhite", mHtml.bwCheck->isChecked() );


  mConfig->writeEntry( "ArrayName", mArray.nameInput->text() );
  mConfig->writeEntry( "ArrayElementType", mArray.typeCombo->currentText() );
  mConfig->writeEntry( "ArrayElementPerLine", mArray.lineSizeSpin->value() );
  mConfig->writeEntry( "ArrayUnsignedAsHex", 
		       mArray.hexadecimalCheck->isChecked() );
  mConfig->sync();
}


void CExportDialog::setupDestinationPage( void )
{
  QString text;
  QFrame *page = mFrame[ page_destination ];

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );  
  if( topLayout == 0 ) { return; }

  QStringList formatList;
  formatList.append( i18n("Plain Text") );
  formatList.append( i18n("HTML Tables") );
  formatList.append( i18n("Rich Text (RTF)") );
  formatList.append( i18n("C Array") );

  mDestination.formatCombo = new QComboBox( false, page );
  mDestination.formatCombo->insertStringList( formatList );
  mDestination.formatCombo->setMinimumWidth( fontMetrics().maxWidth()*10 );
  connect( mDestination.formatCombo, SIGNAL(activated(int)),
	   SLOT(formatChanged(int)) );

  text = i18n("&Format:");
  QLabel *label = new QLabel( mDestination.formatCombo, text, page );
  topLayout->addWidget( label );
  topLayout->addWidget( mDestination.formatCombo );

  QHBoxLayout *hbox = new QHBoxLayout();
  topLayout->addLayout( hbox );

  text = i18n("&Destination:");
  label = new QLabel( text, page );
  hbox->addWidget( label );
 
  text = i18n("(Package folder)"); 
  mDestination.fileExtraLabel = new QLabel( text, page );
  hbox->addWidget( mDestination.fileExtraLabel, 10, AlignLeft|AlignVCenter );

  hbox = new QHBoxLayout();
  topLayout->addLayout( hbox );

  mDestination.fileInput = new QLineEdit( page );
  hbox->addWidget( mDestination.fileInput );
  connect(mDestination.fileInput, SIGNAL(textChanged ( const QString & )),this,SLOT(destinationChanged(const QString &)));
  text = i18n("Choose...");
  QPushButton *browseButton = new QPushButton( text, page, "browse" );
  hbox->addWidget( browseButton );
  connect( browseButton, SIGNAL(clicked()), SLOT(browserClicked()) );
  mDestination.fileInput->setMinimumWidth( fontMetrics().maxWidth()*15 );

  label->setBuddy(mDestination.fileInput);

  hbox = new QHBoxLayout();
  topLayout->addLayout( hbox, 10 );

  mDestination.rangeBox = new QButtonGroup( i18n("Export Range"), page );
  hbox->addWidget( mDestination.rangeBox );

  QButtonGroup *group = mDestination.rangeBox; // convenience

  QVBoxLayout *vbox = new QVBoxLayout( group, spacingHint() );  
  vbox->addSpacing( fontMetrics().lineSpacing() );
  
  QRadioButton *radio1 = new QRadioButton( i18n("&Everything"), group );
  radio1->setFixedSize( radio1->sizeHint() );
  mDestination.rangeBox->insert( radio1, 0 );
  vbox->addWidget( radio1, 0, AlignLeft );

  QRadioButton *radio2 = new QRadioButton( i18n("&Selection"), group );
  radio2->setFixedSize( radio2->sizeHint() );
  mDestination.rangeBox->insert( radio2, 1 );
  vbox->addWidget( radio2, 0, AlignLeft );

  QRadioButton *radio3 = new QRadioButton( i18n("&Range"), group );
  radio3->setFixedSize( radio3->sizeHint() );
  mDestination.rangeBox->insert( radio3, 2 );
  vbox->addWidget( radio3, 0, AlignLeft );

  QGridLayout *gbox = new QGridLayout( 2, 2, spacingHint() );
  vbox->addLayout( gbox );

  mDestination.fromInput = new QLineEdit( group );
  text = i18n("&From offset:");
  mDestination.fromLabel = new QLabel( mDestination.fromInput, text, group );
  gbox->addWidget( mDestination.fromLabel, 0, 0 );
  gbox->addWidget( mDestination.fromInput, 0, 1 );

  mDestination.toInput = new QLineEdit( group );
  text = i18n("&To offset:");
  mDestination.toLabel = new QLabel( mDestination.toInput, text, group );
  gbox->addWidget( mDestination.toLabel, 1, 0 );
  gbox->addWidget( mDestination.toInput, 1, 1 );

  connect( group, SIGNAL(clicked(int)), SLOT(rangeChanged(int)) );
  group->setButton(0);
  rangeChanged(0);
  enableButtonOK( !mDestination.fileInput->text().isEmpty() );
}

void CExportDialog::destinationChanged(const QString &_text)
{
  enableButtonOK( !_text.isEmpty() );
}

void CExportDialog::setupOptionPage( void )
{
  QFrame *page = mFrame[ page_option ];
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );  
  if( topLayout == 0 ) { return; }

  mOptionStack = new QWidgetStack( page, "stack" );
  if( mOptionStack == 0 ) { return; }
  topLayout->addWidget( mOptionStack );

  makeTextOption();
  makeHtmlOption();
  makeRtfOption();
  makeCArrayOption();
  mOptionStack->raiseWidget( (int)option_text );

  QSize size = mOptionStack->sizeHint();
  size += QSize(spacingHint()*2, spacingHint()*2);
  page->setMinimumSize( size ); 
}


void CExportDialog::makeTextOption( void )
{
  QFrame *page = new QFrame( mFrame[ page_option ] );
  if( page == 0 ) { return; }
  mOptionStack->addWidget( page, option_text );

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
  QString text = i18n("No options for this format.");
  QLabel *label = new QLabel( text, page );
  topLayout->addWidget( label, 0, AlignCenter );
}


void CExportDialog::makeHtmlOption( void )
{
  QFrame *page = new QFrame( mFrame[ page_option ] );
  if( page == 0 ) { return; }
  mOptionStack->addWidget( page, option_html );

  QString text;
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );  
  if( topLayout == 0 ) { return; }

  text = i18n("HTML Options (one table per page)");
  QLabel *label = new QLabel( text, page );
  topLayout->addWidget( label );

  QFrame *hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addWidget( hline );

  QFrame *frame = new QFrame( page );
  if( frame == 0 ) { return; }  
  topLayout->addWidget( frame );

  QGridLayout *gbox = new QGridLayout( frame, 4, 2, 0, spacingHint() );
  if( gbox == 0 ) { return; }
  gbox->setColStretch( 1, 10 );  

  mHtml.lineSpin = new QSpinBox( frame );
  mHtml.lineSpin->setMinimumWidth( fontMetrics().maxWidth()*10 );
  mHtml.lineSpin->setRange( 5, INT_MAX );
  gbox->addWidget( mHtml.lineSpin, 0, 1 );
  
  text = i18n("&Lines per table:");
  label = new QLabel( mHtml.lineSpin, text, frame );
  gbox->addWidget( label, 0, 0 );

  mHtml.prefixInput = new QLineEdit( frame, "prefix" );
  mHtml.prefixInput->setMinimumWidth( fontMetrics().maxWidth()*10 );
  gbox->addWidget( mHtml.prefixInput, 1, 1 );

  text = i18n("Filename &prefix (in package):");
  label = new QLabel( mHtml.prefixInput, text, frame );
  gbox->addWidget( label, 1, 0 );

  QStringList headerList;
  headerList.append( i18n("None") );
  headerList.append( i18n("Filename with Path") );
  headerList.append( i18n("Filename") );
  headerList.append( i18n("Page Number") );

  mHtml.topCombo = new QComboBox( false, frame );
  mHtml.topCombo->insertStringList( headerList );
  gbox->addWidget( mHtml.topCombo, 2, 1 );

  text = i18n("Header &above text:");
  label = new QLabel( mHtml.topCombo, text, frame );
  gbox->addWidget( label, 2, 0 );

  mHtml.bottomCombo = new QComboBox( false, frame );
  mHtml.bottomCombo->insertStringList( headerList );
  gbox->addWidget( mHtml.bottomCombo, 3, 1 );

  text = i18n("&Footer below text:");
  label = new QLabel( mHtml.bottomCombo, text, frame );
  gbox->addWidget( label, 3, 0 );

  text = i18n("Link \"index.html\" to &table of contents file");
  mHtml.symlinkCheck = new QCheckBox( text, page );
  topLayout->addWidget( mHtml.symlinkCheck );

  text = i18n("&Include navigator bar");
  mHtml.navigatorCheck = new QCheckBox( text, page );
  topLayout->addWidget( mHtml.navigatorCheck );

  text = i18n("&Use black and white only");
  mHtml.bwCheck = new QCheckBox( text, page );
  topLayout->addWidget( mHtml.bwCheck );

  topLayout->addStretch(10);
}


void CExportDialog::makeRtfOption( void )
{
  QFrame *page = new QFrame( mFrame[ page_option ] );
  if( page == 0 ) { return; }
  mOptionStack->addWidget( page, option_rtf );

  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );
  QString text = i18n("No options for this format.");
  QLabel *label = new QLabel( text, page );
  topLayout->addWidget( label, 0, AlignCenter );
}


void CExportDialog::makeCArrayOption( void )
{
  QFrame *page = new QFrame( mFrame[ page_option ] );
  mOptionStack->addWidget( page, option_carray );

  QString text;
  QVBoxLayout *topLayout = new QVBoxLayout( page, 0, spacingHint() );  

  text = i18n("C Array Options");
  QLabel *label = new QLabel( text, page );
  topLayout->addWidget( label, 0, AlignLeft );

  QFrame *hline = new QFrame( page );
  hline->setFrameStyle( QFrame::Sunken | QFrame::HLine );
  topLayout->addWidget( hline );

  QGridLayout *gbox = new QGridLayout( 3, 2, spacingHint() );
  topLayout->addLayout( gbox );
  gbox->setColStretch( 1, 10 );

  mArray.nameInput = new QLineEdit( page );
  gbox->addWidget( mArray.nameInput, 0, 1 );
  text = i18n("Array name:"); 
  label = new QLabel( mArray.nameInput, text, page );
  gbox->addWidget( label, 0, 0 );

  QStringList typeList;
  typeList.append( i18n("char") );
  typeList.append( i18n("unsigned char") );
  typeList.append( i18n("short") );
  typeList.append( i18n("unsigned short") );
  typeList.append( i18n("int") );
  typeList.append( i18n("unsigned int") );
  typeList.append( i18n("float") );
  typeList.append( i18n("double") );
  mArray.typeCombo = new QComboBox( false, page );
  mArray.typeCombo->insertStringList( typeList );
  mArray.typeCombo->setMinimumWidth( fontMetrics().maxWidth()*10 );
  gbox->addWidget( mArray.typeCombo, 1, 1 );
  text = i18n("Element type:"); 
  label = new QLabel( mArray.typeCombo, text, page );
  gbox->addWidget( label, 1, 0 );
  
  mArray.lineSizeSpin = new QSpinBox( page );
  mArray.lineSizeSpin->setMinimumWidth( fontMetrics().maxWidth()*10 );
  mArray.lineSizeSpin->setRange( 1, INT_MAX );
  gbox->addWidget( mArray.lineSizeSpin, 2, 1 );
  text = i18n("Elements per line:"); 
  label = new QLabel( mArray.lineSizeSpin, text, page );
  gbox->addWidget( label, 2, 0 );

  text = i18n("Print unsigned values as hexadecimal");
  mArray.hexadecimalCheck = new QCheckBox( text, page );
  topLayout->addWidget( mArray.hexadecimalCheck );

  topLayout->addStretch(10);
}





void CExportDialog::formatChanged( int index )
{
  mDestination.formatCombo->setCurrentItem(index);
  mDestination.fileExtraLabel->setEnabled( index == option_html );
  mOptionStack->raiseWidget( index );
}


void CExportDialog::rangeChanged( int id )
{
  bool state = id == 2 ? true : false;
  mDestination.toLabel->setEnabled( state );
  mDestination.fromLabel->setEnabled( state );
  mDestination.toInput->setEnabled( state );
  mDestination.fromInput->setEnabled( state );
}


void CExportDialog::browserClicked( void )
{
  QString url;
  if( mDestination.formatCombo->currentItem() == option_html )
  {
    url = KFileDialog::getExistingDirectory( mWorkDir, topLevelWidget() );
  }
  else
  {
    url = KFileDialog::getSaveFileName( mWorkDir, "*", topLevelWidget() );
  }

  if( url.isEmpty() )
  {
    return;
  }
  
  int index = url.findRev( '/' );
  if( index != -1 ) 
  { 
    mWorkDir = url.left( index+1 ); 
  }
  mDestination.fileInput->setText( url );
}


void CExportDialog::slotOk( void )
{
  QString path( mDestination.fileInput->text() );

  int format = mDestination.formatCombo->currentItem();
  if( format == option_text )
  {
    if( verifyFileDestnation( this, i18n("Export Document"), path ) == false )
    {
      return;
    }

    SExportText e;
    uint mode;
    if( collectRange( mode, e.range.start, e.range.stop ) == false )
    {
      showEntryFailure( this, QString("") );
      return;
    }
    e.range.mode = (SExportRange::EMode)mode; // FIXME
    e.destFile   = path;

    hide();
    emit exportText(e);
  }
  else if( format == option_html )
  {
    SExportHtml e;
    uint mode;
    if( collectRange( mode, e.range.start, e.range.stop ) == false )
    {
      showEntryFailure( this, QString("") );
      return;
    }
    e.range.mode = (SExportRange::EMode)mode; // FIXME

    const QString str = mHtml.prefixInput->text().stripWhiteSpace();
    mHtml.prefixInput->setText( str );
    if( mHtml.prefixInput->text().isEmpty() == true )
    {
      mHtml.prefixInput->setText( "table" );
    }

    const QString prefix = mHtml.prefixInput->text();
    for( uint i=0; i<prefix.length(); i++ )
    {
      QChar c = prefix[i];
      if( c.isSpace() == true || c.isPunct() == true )
      {
	QString msg = i18n("The filename prefix can not contain empty letters "
	  "or punctuation marks.");
	KMessageBox::sorry( this, msg, i18n("Export Document") );
	return;
      }
    }

    if( verifyPackage( path ) == false )
    {
      return;
    }

    e.package       = path;
    e.prefix        = prefix;
    e.linePerPage   = mHtml.lineSpin->value();
    e.topCaption    = mHtml.topCombo->currentItem();
    e.bottomCaption = mHtml.bottomCombo->currentItem();
    e.symLink       = mHtml.symlinkCheck->isChecked();
    e.navigator     = mHtml.navigatorCheck->isChecked();
    e.blackWhite    = mHtml.bwCheck->isChecked();

    hide();
    emit exportHtml(e);
  }
  else if( format == option_rtf )
  {
    QString msg = i18n("This format is not yet supported.");
    KMessageBox::sorry( this, msg );
  }
  else if( format == option_carray )
  {
    if( verifyFileDestnation( this, i18n("Export Document"), path ) == false )
    {
      return;
    }

    SExportCArray e;
    uint mode;
    if( collectRange( mode, e.range.start, e.range.stop ) == false )
    {
      showEntryFailure( this, QString("") );
      return;
    }
    e.range.mode = (SExportRange::EMode)mode; // FIXME
    e.destFile   = path;
    e.arrayName = mArray.nameInput->text();
    e.elementType = mArray.typeCombo->currentItem();
    e.elementPerLine = mArray.lineSizeSpin->value(); 
    e.unsignedAsHexadecimal = mArray.hexadecimalCheck->isChecked();

    emit exportCArray( e );
  }
}




bool CExportDialog::collectRange( uint &mode, uint &start, uint &stop )
{
  QButton *b = mDestination.rangeBox->selected();
  if( b == 0 )
  {
    return( false );
  }
  
  int id =  mDestination.rangeBox->id( b );
  if( id == 0 )
  {
    mode = SExportRange::All;
  }
  else if( id == 1 )
  {
    mode = SExportRange::Selection;
  }
  else if( id == 2 )
  {
    mode = SExportRange::Range;
    bool ok1 = stringToOffset( mDestination.fromInput->text(), start );
    bool ok2 = stringToOffset( mDestination.toInput->text(), stop );
    if( ok1 == false || ok2 == false || start >= stop )
    {
      return( false );
    }
  }
  else
  {
    return( false );
  }

  return( true );
}


//
// This one will attempt to create a directory if 'path'
// specifies a nonexistent name.
//
bool CExportDialog::verifyPackage( const QString &path )
{
  const QString title = i18n("Export Document");

  if( path.isEmpty() == true )
  {
    QString msg = i18n("You must specify a destination.");
    KMessageBox::sorry( this, msg, title );
    return( false );
  }

  QFileInfo info( path );
  if( info.exists() == false )
  {
    QDir directory;
    if( directory.mkdir( path ) == false )
    {
      QString msg;
      msg += i18n("Unable to create a new folder");
      msg += "\n";
      msg += path;
      KMessageBox::sorry( this, msg, title );
      return( false );
    }
  }
  else
  {
    if( info.isDir() == false ) 
    {
      QString msg = i18n("You have specified an existing file");
      KMessageBox::sorry( this, msg, title );
      return( false );
    }
    else
    {
      if( info.isWritable() == false ) 
      {
	QString msg = i18n( ""
          "You do not have write permission to this folder.");
	KMessageBox::sorry( this, msg, title );
	return( false );
      }

      const QString prefix = mHtml.prefixInput->text();
      QString f1 = QString("%1%2.html").arg(prefix).arg("00000000");
      QString f2 = QString("%1%2.html").arg(prefix).arg("99999999");

      QString msg = i18n( ""
        "You have specified an existing folder.\n"
	"If you continue, any existing file in the range "
	"\"%1\" to \"%2\" can be lost.\n"
	"Continue?").arg(f1).arg(f2);
      int reply = KMessageBox::warningContinueCancel( this, msg, title );
      if( reply != KMessageBox::Continue )
      {
	return( false );
      }

    }
  }

  return( true );
}

#include "exportdialog.moc"
