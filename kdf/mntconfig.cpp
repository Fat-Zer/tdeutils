/*
 * mntconfig.cpp
 *
 * Copyright (c) 1999 Michael Kropfberger <michael.kropfberger@gmx.net>
 *
 * Requires the TQt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


//
// 1999-11-29 Espen Sand
// Converted to TQLayout and TQListView + cleanups
//

#include <tqgroupbox.h>
#include <tqheader.h>
#include <tqlayout.h>
#include <tqlineedit.h>

#undef Unsorted

#include <kapplication.h>
#include <kfiledialog.h>
#include <kicondialog.h>
#include <kmessagebox.h>

#include "listview.h"
#include "mntconfig.h"

#ifndef KDE_USE_FINAL
static bool GUI;
#endif


MntConfigWidget::MntConfigWidget(TQWidget *parent, const char *name, bool init)
  : TQWidget(parent, name)
{
  mInitializing = false;

  GUI = !init;
  if (GUI)
  {
    //tabList fillup waits until disklist.readDF() is done...
    mDiskList.readFSTAB();
    mDiskList.readDF();
    mInitializing = true;
    connect( &mDiskList,TQT_SIGNAL(readDFDone()),this,TQT_SLOT(readDFDone()));

    TQString text;
    TQVBoxLayout *topLayout = new TQVBoxLayout( this, 0, KDialog::spacingHint());

    mList = new CListView( this, "list", 8 );
    mList->setAllColumnsShowFocus( true );
    mList->addColumn( i18n("Icon") );
    mList->addColumn( i18n("Device") );
    mList->addColumn( i18n("Mount Point") );
    mList->addColumn( i18n("Mount Command") );
    mList->addColumn( i18n("Unmount Command") );
    mList->setFrameStyle( TQFrame::WinPanel + TQFrame::Sunken );
    connect( mList, TQT_SIGNAL(selectionChanged(TQListViewItem *)),
	     this, TQT_SLOT(clicked(TQListViewItem *)));

    topLayout->addWidget( mList );

    text = TQString("%1: %2  %3: %4").
      arg(mList->header()->label(DEVCOL)).
      arg(i18n("None")).
      arg(mList->header()->label(MNTPNTCOL)).
      arg(i18n("None"));
    mGroupBox = new TQGroupBox( text, this );
    TQ_CHECK_PTR(mGroupBox);
    topLayout->addWidget(mGroupBox);

    TQGridLayout *gl = new TQGridLayout(mGroupBox, 3, 4, KDialog::spacingHint());
    if( gl == 0 ) { return; }
    gl->addRowSpacing( 0, fontMetrics().lineSpacing() );

    mIconLineEdit = new TQLineEdit(mGroupBox);
    TQ_CHECK_PTR(mIconLineEdit);
    mIconLineEdit->setMinimumWidth( fontMetrics().maxWidth()*10 );
    connect( mIconLineEdit, TQT_SIGNAL(textChanged(const TQString&)),
	     this,TQT_SLOT(iconChanged(const TQString&)));
    connect( mIconLineEdit, TQT_SIGNAL(textChanged(const TQString&)),
	     this,TQT_SLOT(slotChanged()));
    gl->addWidget( mIconLineEdit, 2, 0 );

    mIconButton = new KIconButton(mGroupBox);
    mIconButton->setIconType(KIcon::Small, KIcon::Device);
    TQ_CHECK_PTR(mIconButton);
    mIconButton->setFixedWidth( mIconButton->sizeHint().height() );
    connect(mIconButton,TQT_SIGNAL(iconChanged(TQString)),this,TQT_SLOT(iconChangedButton(TQString)));
    gl->addWidget( mIconButton, 2, 1 );

    //Mount
    mMountButton = new TQPushButton( i18n("Get Mount Command"), mGroupBox );
    TQ_CHECK_PTR(mMountButton);
    connect(mMountButton,TQT_SIGNAL(clicked()),this,TQT_SLOT(selectMntFile()));
    gl->addWidget( mMountButton, 1, 2 );

    mMountLineEdit = new TQLineEdit(mGroupBox);
    TQ_CHECK_PTR(mMountLineEdit);
    mMountLineEdit->setMinimumWidth( fontMetrics().maxWidth()*10 );
    connect(mMountLineEdit,TQT_SIGNAL(textChanged(const TQString&)),
	    this,TQT_SLOT(mntCmdChanged(const TQString&)));
    connect( mMountLineEdit, TQT_SIGNAL(textChanged(const TQString&)),
	     this,TQT_SLOT(slotChanged()));
    gl->addWidget( mMountLineEdit, 1, 3 );

    //Umount
    mUmountButton = new TQPushButton(i18n("Get Unmount Command"), mGroupBox );
    TQ_CHECK_PTR( mUmountButton );
    connect(mUmountButton,TQT_SIGNAL(clicked()),this,TQT_SLOT(selectUmntFile()));
    gl->addWidget( mUmountButton, 2, 2 );

    mUmountLineEdit=new TQLineEdit(mGroupBox);
    TQ_CHECK_PTR(mUmountLineEdit);
    mUmountLineEdit->setMinimumWidth( fontMetrics().maxWidth()*10 );
    connect(mUmountLineEdit,TQT_SIGNAL(textChanged(const TQString&)),
	    this,TQT_SLOT(umntCmdChanged(const TQString&)));
    connect( mUmountLineEdit, TQT_SIGNAL(textChanged(const TQString&)),
	     this,TQT_SLOT(slotChanged()));
    gl->addWidget( mUmountLineEdit, 2, 3 );

  }

  loadSettings();
  if(init)
  {
    applySettings();
    mDiskLookup.resize(0);
  }

  mGroupBox->setEnabled( false );
}


MntConfigWidget::~MntConfigWidget( void )
{
}


void MntConfigWidget::readDFDone( void )
{
  mInitializing = false;
  mList->clear();
  mDiskLookup.resize(mDiskList.count());

  int i=0;
  TQListViewItem *item = 0;
  for( DiskEntry *disk=mDiskList.first(); disk!=0; disk=mDiskList.next(),++i )
  {
     item = new TQListViewItem( mList, item, TQString(), disk->deviceName(),
      disk->mountPoint(), disk->mountCommand(), disk->umountCommand() );
     item->setPixmap( ICONCOL, SmallIcon( disk->iconName() ) );
     mDiskLookup[i] = item;
  }

  loadSettings();
  applySettings();
}


void MntConfigWidget::applySettings( void )
{
  mDiskList.applySettings();

  KConfig &config = *kapp->config();
  config.setGroup("MntConfig");
  if(GUI )
  {
   config.writeEntry("Width", width() );
   config.writeEntry("Height", height() );
  }
  config.sync();
}


void MntConfigWidget::loadSettings( void )
{
  KConfig &config = *kapp->config();
  if( mInitializing == false && GUI )
  {
    config.setGroup("MntConfig");
    if( isTopLevel() )
    {
      int w = config.readNumEntry("Width",this->width() );
      int h = config.readNumEntry("Height",this->height() );
      resize(w,h);
    }

    TQListViewItem *item = mList->selectedItem();
    if( item != 0 )
    {
      clicked( item );
    }
  }
}


void MntConfigWidget::clicked( TQListViewItem *item )
{
  mGroupBox->setEnabled( true );
  mGroupBox->setTitle( TQString("%1: %2  %3: %4").
    arg(mList->header()->label(DEVCOL)).
    arg(item->text(DEVCOL)).
    arg(mList->header()->label(MNTPNTCOL)).
    arg(item->text(MNTPNTCOL)) );


  const TQPixmap *pix = item->pixmap(ICONCOL);
  if( pix != 0 )
  {
    mIconButton->setPixmap( *pix );
  }

  for(unsigned i=0 ; i < mDiskList.count() ; ++i) 
    {
      if (mDiskLookup[i] == item) 
	{
	  DiskEntry *disk = mDiskList.at(i);
	  if( disk != 0 )
	    {
	      mIconLineEdit->setText( disk->iconName() );
	    }
	  break;
	}
    }
  mMountLineEdit->setText( item->text(MNTCMDCOL) );
  mUmountLineEdit->setText( item->text(UMNTCMDCOL) );
}


void MntConfigWidget::iconChangedButton(TQString iconName)
{
  iconChanged(iconName);
}
void MntConfigWidget::iconChanged(const TQString &iconName)
{
  if( iconName.findRev('_') == 0 ||
      (iconName.right(iconName.length()-iconName.findRev('_'))!="_mount" &&
      iconName.right(iconName.length()-iconName.findRev('_'))!="_unmount"))
    {
      TQString msg = i18n(""
			 "This filename is not valid: %1\n"
			 "It must end with "
			 "\"_mount\" or \"_unmount\".").arg(iconName);
      KMessageBox::sorry( this, msg );
      return;
    }

  TQListViewItem *item = mList->selectedItem();
  for(unsigned i=0 ; i < mDiskList.count() ; ++i) 
    {
      if (mDiskLookup[i] == item) 
	{
	  DiskEntry *disk = mDiskList.at(i);
	  if( disk != 0 )
	    {
	      disk->setIconName(iconName);
	      mIconLineEdit->setText(iconName);
	      KIconLoader &loader = *KGlobal::iconLoader();
	      item->setPixmap( ICONCOL, loader.loadIcon( iconName, KIcon::Small));
	    }
	  break;
	}
    }
}


void MntConfigWidget::selectMntFile()
{
  KURL url = KFileDialog::getOpenURL( "","*", this );

  if( url.isEmpty() )
    return;

  if( !url.isLocalFile() )
  {
    KMessageBox::sorry( 0L, i18n( "Only local files supported." ) );
    return;
  }

  mMountLineEdit->setText( url.path() );
}

void MntConfigWidget::selectUmntFile()
{
  KURL url = KFileDialog::getOpenURL( "", "*", this );

  if( url.isEmpty() )
    return;

  if( !url.isLocalFile() )
  {
    KMessageBox::sorry( 0L, i18n( "Only local files are currently supported." ) );
    return;
  }

  mUmountLineEdit->setText( url.path() );
}

void MntConfigWidget::mntCmdChanged( const TQString &data )
{
  TQListViewItem *item = mList->selectedItem();
  for(unsigned  i=0 ; i < mDiskList.count() ; ++i) 
    {
      if (mDiskLookup[i] == item)
	{
	  DiskEntry *disk = mDiskList.at(i);
	  if( disk != 0 )
	    {
	      disk->setMountCommand(data);
	      item->setText( MNTCMDCOL, data );
	    }
	  break;
	}
    }
}


void MntConfigWidget::umntCmdChanged( const TQString &data )
{
  TQListViewItem *item = mList->selectedItem();
  for(unsigned i=0 ; i < mDiskList.count() ; ++i) 
    {
    if (mDiskLookup[i] == item) 
      {
	DiskEntry *disk = mDiskList.at(i);
	if( disk != 0 )
	  {
	    disk->setUmountCommand(data);
	    item->setText( UMNTCMDCOL, data );
	  }
	break;
      }
    }
}


void MntConfigWidget::closeEvent(TQCloseEvent *)
{
}

void MntConfigWidget::slotChanged()
{
  emit configChanged();
}

#include "mntconfig.moc"
