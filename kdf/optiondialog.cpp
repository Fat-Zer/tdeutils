/*
 *   Copyright (C) 1999  Espen Sand, espen@kde.org
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

#include <layout.h>

#include "kdfconfig.h"
#include "mntconfig.h"
#include "optiondialog.h"

COptionDialog::COptionDialog( TQWidget *parent, const char *name, bool modal )
  :KDialogBase( Tabbed, i18n("Configure"), Help|Apply|Ok|Cancel, Ok,
	        parent, name, modal )
{
  setHelp( "kcontrol/kdf/index.html", TQString() );

  TQFrame *f1 = addPage( i18n("General Settings") );
  TQVBoxLayout *l1 = new TQVBoxLayout( f1 );  
  mConf = new KDFConfigWidget( f1, "kdfconf" );
  l1->addWidget(mConf);
  connect( mConf, TQT_SIGNAL( configChanged() ), this, TQT_SLOT( slotChanged() ) );

  TQFrame *f2 = addPage( i18n("Mount Commands") );
  TQVBoxLayout *l2 = new TQVBoxLayout( f2 );
  mMnt = new MntConfigWidget( f2, "mntconf");
  l2->addWidget(mMnt);
  connect( mMnt, TQT_SIGNAL( configChanged() ), this, TQT_SLOT( slotChanged() ) );
  enableButton( Apply, false );
  dataChanged = false;
}


COptionDialog::~COptionDialog( void )
{
}


void COptionDialog::slotOk( void )
{
  if( dataChanged )
    slotApply();
  accept();
}


void COptionDialog::slotApply( void )
{
  mConf->applySettings();
  mMnt->applySettings();
  emit valueChanged();
  enableButton( Apply, false );
  dataChanged = false;
}

void COptionDialog::slotChanged()
{
  enableButton( Apply, true );
  dataChanged = true;
}

#include "optiondialog.moc"









