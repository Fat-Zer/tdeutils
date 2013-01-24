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

#include <tqheader.h>
#include <tqlabel.h>
#include <tqlayout.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <klocale.h>

#include "fileinfodialog.h"
#include "listview.h"

// quick'n'dirty hack to have the occurrence column sorted correctly
class CStatisticListViewItem : public TQListViewItem
{
  public:
    CStatisticListViewItem( TQListView * parent, TQListViewItem * after,
                            TQString label1, TQString label2, TQString label3, TQString label4,
                            TQString label5, TQString label6, TQString label7, int i, int o)
    : TQListViewItem( parent, after, label1, label2, label3, label4, label5, label6, label7),
      item( i ),
      occurrence( o )
    {}

    virtual int compare( TQListViewItem *i, int col, bool ascending/*TQt doc says: ignore this one*/ ) const
    {
      // occurrence column (or the percent one)?
      if( col == 5 || col == 6 )
      {
        const int otherOccurrence = ((CStatisticListViewItem*)i)->occurrence;
        return occurrence < otherOccurrence ? -1 : occurrence == otherOccurrence ? 0 : 1;
      }
      // char column?
      else if( col == 4 )
      {
        const int otherItem = ((CStatisticListViewItem*)i)->item;
        return item < otherItem ? -1 : item == otherItem ? 0 : 1;
      }
      // default
      else
        return TQListViewItem::compare(i,col,ascending);
    }

  protected:
    // no of byte
    int item;
    // number of the byte's occurrence
    int occurrence;
};



CFileInfoDialog::CFileInfoDialog( TQWidget *parent,const char *name,bool modal)
  :KDialogBase( Plain, i18n("Statistics"), Help|User1|Cancel, User1,
		parent, name, modal, true, i18n("&Update") ),
   mBusy(false), mDirty(false)
{
  setHelp( "khexedit/khexedit.html", TQString() );

  TQString text;
  TQVBoxLayout *topLayout = new TQVBoxLayout( plainPage(), 0, spacingHint() );
  if( topLayout == 0 ) { return; }


  TQGridLayout *gbox = new TQGridLayout( 2, 2, spacingHint() );
  if( gbox == 0 ) { return; }
  topLayout->addLayout( gbox );
  gbox->setColStretch( 1, 10 );

  text = i18n("File name: ");
  TQLabel *label = new TQLabel( text, plainPage() );
  gbox->addWidget( label, 0, 0 );

  text = i18n("Size [bytes]: ");
  label = new TQLabel( text, plainPage() );
  gbox->addWidget( label, 1, 0 );

  mFileNameLabel = new TQLabel( plainPage() );
  mFileSizeLabel = new TQLabel( plainPage() );
  gbox->addWidget( mFileNameLabel, 0, 1 );
  gbox->addWidget( mFileSizeLabel, 1, 1 );

  mFrequencyList = new CListView( plainPage(), "stringList" );
  mFrequencyList->setFont( TDEGlobalSettings::fixedFont() );

  mFrequencyList->addColumn( i18n("Hexadecimal") );
  mFrequencyList->addColumn( i18n("Decimal") );
  mFrequencyList->addColumn( i18n("Octal") );
  mFrequencyList->addColumn( i18n("Binary") );
  mFrequencyList->addColumn( i18n("Text") );
  mFrequencyList->addColumn( i18n("Occurrence") );
  mFrequencyList->addColumn( i18n("Percent") );
  mFrequencyList->setAllColumnsShowFocus( true );
  mFrequencyList->setFrameStyle( TQFrame::WinPanel + TQFrame::Sunken );
  topLayout->addWidget( mFrequencyList, 10 );

  mDirtyLabel = new TQLabel( plainPage() );
  mDirtyLabel->setFixedHeight( fontMetrics().height() );
  topLayout->addWidget( mDirtyLabel );

  setStatistics();
  setColumnWidth();
  mFrequencyList->setVisibleItem( 15 );

  //
  // Load the first set of data when this timer expires. I do it this
  // way so that the dialog will be visible when the load operation starts.
  //
  startTimer( 0 );
}


CFileInfoDialog::~CFileInfoDialog( void )
{
}


void CFileInfoDialog::slotUser1( void ) // Update
{
  if( mBusy )
    return;

  SStatisticControl *sc = new SStatisticControl;
  if( sc == 0 ) { return; }

  mBusy = true;
  emit collectStatistic( *sc );
  mBusy = false;

  delete sc;

}


void CFileInfoDialog::setDirty( void )
{
  if( mDirty )
    return;

  mDirtyLabel->setText(
    i18n("Warning: Document has been modified since last update"));
  mDirty = true;
}


void CFileInfoDialog::setClean( void )
{
  if( !mDirty )
    return;

  mDirtyLabel->setText("");
  mDirty = false;
}


const char *printBin( uint val )
{
  static char buf[9];
  for( int i = 0; i < 8; i++ )
    buf[7-i] = (val&(1<<i)) ? '1' : '0';
  buf[8] = 0;
  return( buf );
}


void CFileInfoDialog::setStatistics() // Default
{
  setClean();
  mFrequencyList->clear();
  mFileNameLabel->clear();
  mFileSizeLabel->clear();

  static const TQString u("?");
  TQString d, h, o, b, c;
  TQListViewItem *item = 0;

  char buf[10];
  memset( buf, 0, sizeof( buf ) );

  for( uint i=0; i<256; i++ )
  {
    h.sprintf("0x%02x", i );
    d.sprintf("%03d", i );
    o.sprintf("%03o", i );
    b.sprintf("%s", printBin(i) );

    const TQChar _i((char)i);
    c = _i.isPrint() ? _i : TQChar('.'); 

    item = new CStatisticListViewItem( mFrequencyList, item, h, d, o, b, c, u, u, i, -1 );
    if( i == 0 )
      mFrequencyList->setSelected( item, true );
  }
}



void CFileInfoDialog::setStatistics( SStatisticControl &sc )
{
  setClean();
  mFrequencyList->clear();
  mFileNameLabel->setText( sc.documentName );
  mFileSizeLabel->setText( TDEGlobal::locale()->formatNumber(sc.documentSize, 0) );

  TQString d, h, o, b, c, n, p;
  TQListViewItem *item = 0;

  uint size, pre, i;
  // find width of occurrence
  for( i=size=0; i<256; i++ )
    if( sc.occurrence[i] > size ) 
      size = sc.occurrence[i];
  for( pre = 1; size > 0 ; pre++ )
    size /= 10;

  for( i=0; i<256; i++ )
  {
    h.sprintf("0x%02x", i );
    d.sprintf("%03d", i );
    o.sprintf("%03o", i );
    b.sprintf("%s", printBin(i) );

    n = TQString("%1").arg( sc.occurrence[i], pre );
    if( sc.documentSize == 0 )
      p = "0.00";
    else
    {
      double val = 100.0*((double)sc.occurrence[i]/(double)sc.documentSize);
      p = TQString("%1").arg( val, 6, 'f', 2 );
    }

    const TQChar _i((char)i);
    c = _i.isPrint() ? _i : TQChar('.'); 

    item = new CStatisticListViewItem( mFrequencyList, item, h, d, o, b, c, n, p, i, sc.occurrence[i] );
    if( i == 0 )
      mFrequencyList->setSelected( item, true );
  }
}



void CFileInfoDialog::setColumnWidth( void )
{
  const TQFontMetrics &fm = mFrequencyList->fontMetrics();
  int w0, w1, w2, w3, w4;

  w0 = -fm.minLeftBearing() - fm.minRightBearing() + 8 + fm.maxWidth();
  w3 = 0;

  w1  = fm.width( mFrequencyList->header()->label(0) ) + w0;
  w2  = fm.width('0')*6;
  w3 += w1 > w2 ? w1 : w2;
  mFrequencyList->setColumnWidth( 0, w1 > w2 ? w1 : w2 );

  w1  = fm.boundingRect( mFrequencyList->header()->label(1) ).width() + w0;
  w2  = fm.width('0')*5;
  w3 += w1 > w2 ? w1 : w2;
  mFrequencyList->setColumnWidth( 1, w1 > w2 ? w1 : w2 );

  w1  = fm.boundingRect( mFrequencyList->header()->label(2) ).width() + w0;
  w2  = fm.width('0')*5;
  w3 += w1 > w2 ? w1 : w2;
  mFrequencyList->setColumnWidth( 2, w1 > w2 ? w1 : w2 );

  w1  = fm.boundingRect( mFrequencyList->header()->label(3) ).width() + w0;
  w2  = fm.width('0')*10;
  w3 += w1 > w2 ? w1 : w2;
  mFrequencyList->setColumnWidth( 3, w1 > w2 ? w1 : w2 );

  w1  = fm.boundingRect( mFrequencyList->header()->label(4) ).width() + w0;
  w2  = fm.width('0')*3;
  w3 += w1 > w2 ? w1 : w2;
  mFrequencyList->setColumnWidth( 4, w1 > w2 ? w1 : w2 );

  w1  = fm.boundingRect( mFrequencyList->header()->label(5) ).width() + w0;
  w2  = fm.width('0')*10;
  w3 += w1 > w2 ? w1 : w2;
  mFrequencyList->setColumnWidth( 5, w1 > w2 ? w1 : w2 );

  w4 = mFrequencyList->viewport()->width() - w3;
  w1 = fm.boundingRect( mFrequencyList->header()->label(6) ).width() + w0;
  w2 = fm.width('0')*3;
  w1 = w1 > w2 ? w1 : w2;
  mFrequencyList->setColumnWidth( 6, w1 > w4 ? w1 : w4 );
}


void CFileInfoDialog::resizeEvent( TQResizeEvent * )
{
  setColumnWidth();
}


void CFileInfoDialog::showEvent( TQShowEvent *e )
{
  KDialogBase::showEvent(e);
  setColumnWidth();
  mFrequencyList->setFocus();
}


void CFileInfoDialog::timerEvent( TQTimerEvent * )
{
  TQT_TQOBJECT(this)->killTimers();
  slotUser1();
}



#include "fileinfodialog.moc"
