/*
  kcmdf.cpp - KcmDiskFree

  Copyright (C) 1998 by Michael Kropfberger <michael.kropfberger@gmx.net>
  
  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.
  
  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.
  
  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
   
  */

//
// 1999-12-05 Espen Sand 
// Modified to use TDECModule instead of the old and obsolete 
// KControlApplication
//


#include <kdialog.h>

#include <tqlayout.h>

#include "kcmdf.h"

KDiskFreeWidget::KDiskFreeWidget( TQWidget *parent, const char *name )
 : TDECModule( parent, name )
{
  setButtons(Help);

  TQVBoxLayout *topLayout = new TQVBoxLayout( this, 0, KDialog::spacingHint() );

  mKdf = new KDFWidget( this, "kdf", false );
  topLayout->addWidget( mKdf );
}

KDiskFreeWidget::~KDiskFreeWidget()
{
  mKdf->applySettings();
}

TQString KDiskFreeWidget::quickHelp() const
{
    return i18n("<h3>Hardware Information</h3><br> All the information modules return information"
    " about a certain aspect of your computer hardware or your operating system."
    " Not all modules are available on all hardware architectures and/or operating systems.");
}

extern "C"
{
  KDE_EXPORT TDECModule* create_kdf( TQWidget *parent, const char * /*name*/ )
  {
    return new KDiskFreeWidget( parent , "kdf" );
  }
}

#include "kcmdf.moc"
