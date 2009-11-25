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

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>

#include <kconfig.h>
#include <kglobalsettings.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "listview.h"
#include "stringdialog.h"
#include <qpushbutton.h>


CStringDialog::CStringDialog( QWidget *parent, const char *name, bool modal )
  : KDialogBase( Plain, i18n("Extract Strings"), Help|User1|Cancel, User1,
		 parent, name, modal, true, i18n("&Update") ),
  mMaxLength(0), mBusy(false), mDirty(false)
{
  setHelp( "khexedit/khexedit.html", QString::null );

  QString text;
  QVBoxLayout *topLayout = new QVBoxLayout( plainPage(), 0, spacingHint() );

  QGridLayout *glay = new QGridLayout( 3, 3, spacingHint() );
  topLayout->addLayout( glay );
  glay->setColStretch( 1, 10 );

  mLengthSpin = new QSpinBox( plainPage() );
  mLengthSpin->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mLengthSpin->setRange( 1, 1000000 );
  mLengthSpin->setValue( 4 );
  glay->addMultiCellWidget( mLengthSpin, 0, 0, 1, 2 );

  text = i18n("&Minimum length:");
  QLabel *label = new QLabel( mLengthSpin, text, plainPage() );
  glay->addWidget( label, 0, 0, AlignRight );

  mFilter = new QLineEdit( plainPage() );
  mFilter->setMinimumWidth( fontMetrics().maxWidth()*17 );
  mFilter->setText("*");
  glay->addWidget( mFilter, 1, 1 );

  text = i18n("&Filter:");
  label = new QLabel( mFilter, text, plainPage() );
  glay->addWidget( label, 1, 0, AlignRight );

  QPushButton *useButton = new QPushButton( plainPage(), "use" );
  useButton->setText( i18n("&Use") );
  useButton->setAutoDefault( false );
  connect( useButton, SIGNAL(clicked()),this,SLOT(slotUser1()) );
  glay->addWidget( useButton, 1, 2 );

  QWidget *helper = new QWidget( plainPage() );
  glay->addMultiCellWidget( helper, 2, 2, 1, 2 );
  QHBoxLayout *hlay = new QHBoxLayout( helper, 0, spacingHint() );

  text = i18n("&Ignore case");
  mIgnoreCaseCheck = new QCheckBox( text, helper );
  hlay->addWidget( mIgnoreCaseCheck );

  text = i18n("Show offset as &decimal");
  mOffsetDecimalCheck = new QCheckBox( text, helper );
  hlay->addWidget( mOffsetDecimalCheck );

  hlay->addStretch(10);

  //
  // Using listview as suggested by Dima Rogozin <dima@mercury.co.il>
  //
  mStringList = new CListView( plainPage(), "stringList" );
  mStringList->setFont( KGlobalSettings::fixedFont() );
  mStringList->addColumn( i18n("Offset") );
  mStringList->addColumn( i18n("String") );
  mStringList->setAllColumnsShowFocus( true );
  mStringList->setFrameStyle( QFrame::WinPanel + QFrame::Sunken );

  connect( mStringList, SIGNAL(selectionChanged()),
	   SLOT(selectionChanged()));
  topLayout->addWidget( mStringList, 10 );

  hlay = new QHBoxLayout( topLayout );

  text = i18n("Number of strings:");
  label = new QLabel( text, plainPage() );
  hlay->addWidget( label, AlignLeft );

  mListSizeLabel = new QLabel( QString(), plainPage() );
  mListSizeLabel->setFixedWidth( fontMetrics().maxWidth()*10 );
  hlay->addWidget( mListSizeLabel, AlignLeft|AlignHCenter );

  text = i18n("Displayed:");
  label = new QLabel( text, plainPage() );
  hlay->addWidget( label, AlignLeft );

  mDisplaySizeLabel = new QLabel( QString(""), plainPage() );
  mDisplaySizeLabel->setFixedWidth( fontMetrics().maxWidth()*10 );
  hlay->addWidget( mDisplaySizeLabel, AlignLeft|AlignHCenter);

  hlay->addStretch( 10 );

  mDirtyLabel = new QLabel( plainPage() );
  topLayout->addWidget( mDirtyLabel );

  readConfiguration();

  //
  // The initial width of the columns are set in updateListInfo()
  //
  updateListInfo();
  mStringList->setVisibleItem( 15 );
  startTimer(100);
}


CStringDialog::~CStringDialog( void )
{
  writeConfiguration();
}


void CStringDialog::showEvent( QShowEvent *e )
{
  KDialogBase::showEvent(e);
  mLengthSpin->setFocus();
}


void CStringDialog::readConfiguration( void )
{
  KConfig &config = *kapp->config();
  config.setGroup( "String Dialog" );

  mLengthSpin->setValue( config.readNumEntry("MinimumLength", 4)  );
  mFilter->setText( config.readEntry("FilterText", "*") );
  mIgnoreCaseCheck->setChecked( config.readBoolEntry("IgnoreCase", false) );
  mOffsetDecimalCheck->setChecked(
    config.readBoolEntry("OffsetAsDecimal", false ) );
}


void CStringDialog::writeConfiguration( void )
{
  KConfig &config = *kapp->config();
  config.setGroup( "String Dialog" );

  config.writeEntry( "MinimumLength", mLengthSpin->value() );
  config.writeEntry( "FilterText", mFilter->text() );
  config.writeEntry( "IgnoreCase", mIgnoreCaseCheck->isChecked() );
  config.writeEntry( "OffsetAsDecimal", mOffsetDecimalCheck->isChecked() );
  config.sync();
}


void CStringDialog::slotUser1( void ) // Start
{
  if( mBusy == true )
  {
    return;
  }

  mRegExp.setCaseSensitive( mIgnoreCaseCheck->isChecked() == false );
  mRegExp.setWildcard( true );
  if( mFilter->text().isEmpty() == true )
  {
    mRegExp.setPattern("*");
  }
  else
  {
    mRegExp.setPattern(mFilter->text());
  }

  if( mRegExp.isValid() == false )
  {
    QString msg = i18n(""
      "The filter expression you have specified is illegal. "
      "You must specify a valid regular expression.\n"
      "Continue without filter?");
    int reply = KMessageBox::warningContinueCancel( this, msg, i18n("Extract Strings"));
    if( reply != KMessageBox::Continue )
    {
      return;
    }
    mRegExp.setPattern( "*");
  }


  mStringData.minLength     = mLengthSpin->value();
  mStringData.decimalOffset = mOffsetDecimalCheck->isChecked();

  removeList();

  mBusy = true;
  emit collect();
  mBusy = false;
}


void CStringDialog::startGoto( QListViewItem *item )
{
  QString str_offset = item->text(0);
  QString string = item->text(1);

  uint offset;
  if( stringData().decimalOffset == true )
  {
    // sscanf( str_offset.ascii(), "%u", &offset );
    offset = str_offset.toInt();
  }
  else
  {
    uint val1,val2;
    // #### Rewrite it do it doesn't need the .ascii() call
    sscanf( str_offset.ascii(), "%X:%X", &val1, &val2 );
    offset = (val1<<16) + val2;
  }

  uint size = string.length();

  emit markText( offset, size, true );
}


void CStringDialog::setDirty( void )
{
  if( mDirty == true )
  {
    return;
  }

  mDirtyLabel->setText(
    i18n("Warning: Document has been modified since last update"));
  mDirty = true;
}


void CStringDialog::setClean( void )
{
  if( mDirty == false )
  {
    return;
  }

  mDirtyLabel->setText("");
  mDirty = false;
}


void CStringDialog::selectionChanged()
{
  killTimers();
  startTimer( 200 );
}


void CStringDialog::timerEvent( QTimerEvent * )
{
  killTimers();

  QListViewItem *item = mStringList->currentItem();
  if( item == 0 )
  {
    slotUser1();
    return;
  }
  startGoto( item );
}


void CStringDialog::resizeEvent( QResizeEvent * )
{
  setColumnWidth();
}



int CStringDialog::updateList( CProgress &p )
{
  clearList();
  enableList( false );

  int offsetLen = stringData().offsetLen();

  QPtrList<QString> &list = stringData().list();
  for( const QString *str = list.first(); str != 0; str = list.next() )
  {
    appendListItem( *str, offsetLen );
    mMaxLength = QMAX( mMaxLength, str->length() );

    if( p.expired() == true )
    {
      int errCode = p.step( (float)list.at()/(float)list.count() );
      if( errCode == Err_Stop )
      {
	p.finish();
	return( Err_Success );
      }
    }
  }

  p.finish();
  updateListInfo();
  enableList( true );

  return( Err_Success );
}




void CStringDialog::clearList( void )
{
  if( mStringList->childCount() > 0 )
  {
    mStringList->clear();
    mStringList->update();
  }
  mMaxLength = 0;

  updateListInfo();
  setClean();
}


void CStringDialog::removeList( void )
{
  mStringData.clear();
  clearList();
}


void CStringDialog::enableList( bool state )
{
  mStringList->setEnabled( state );
  if( state == true )
  {
    mStringList->update();
  }
}


void CStringDialog::appendListItem( const QString &str, uint offsetLen )
{
  if( mRegExp.search ( str, offsetLen ) != -1 )
  {
    new QListViewItem( mStringList, str.left(offsetLen), str.mid(offsetLen) );
  }
}


void CStringDialog::updateListInfo( void )
{
  mListSizeLabel->setText( QString("%1").arg(mStringData.count()) );
  mDisplaySizeLabel->setText( QString("%1").arg(mStringList->childCount()) );
  setColumnWidth();
}



void CStringDialog::setColumnWidth( void )
{
  const QFontMetrics &fm = mStringList->fontMetrics();

  if( mStringData.count() == 0 )
  {
    int w1 = fm.maxWidth() * (stringData().offsetLen() + 2);
    int w2 = mStringList->viewport()->width() - w1;
    mStringList->setColumnWidth( 0, w1 );
    mStringList->setColumnWidth( 1, w2 );
  }
  else
  {
    int w1  = fm.maxWidth() * (stringData().offsetLen() + 2);
    int w2  = fm.maxWidth() * mMaxLength - w1;
    if( w2 < 0 )
    {
      w2 = mStringList->viewport()->width() - w1;
    }

    mStringList->setColumnWidth( 0, w1 );
    mStringList->setColumnWidth( 1, w2 );
  }
}






#include "stringdialog.moc"
