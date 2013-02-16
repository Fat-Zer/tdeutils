// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2001 Wolfram Diestel <wolfram@steloj.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqlabel.h>
#include <tqvbox.h>

#include <tdelocale.h>
#include <kcharsets.h>
#include <tdediroperator.h>
#include <tderecentdocument.h>
#include <tdetoolbar.h>
#include <kpushbutton.h>

#include "ktextfiledlg.h"

KTextFileDialog::KTextFileDialog(const TQString& startDir,
				 const TQString& filter,
				 TQWidget *parent, const char* name,
				 bool modal)
  : KFileDialog(startDir, filter, parent, name, modal)
{
  /*
  // insert encoding action into toolbar
  TDESelectAction *mEncoding = new TDESelectAction(
      i18n( "Set &Encoding" ), 0, this,
      TQT_SLOT( slotSetEncoding() ), this,
      "encoding" );

  TQStringList encodings = TDEGlobal::charsets()->descriptiveEncodingNames();
  encodings.prepend( i18n( "Default encoding" ) );
  mEncoding->setItems( encodings );
  mEncoding->setCurrentItem(0);
  TQStringList::Iterator it;
  int i = 0;
  for( it = encodings.begin(); it != encodings.end(); ++it) {
      if ( (*it).contains( encodingStr ) ) {
      mEncoding->setCurrentItem( i );
      break;
      }
    i++;
  }

  TDEToolBar *tb = toolBar();
  mEncoding->plug( tb, 7 );
  */

  TDEAction* mEncoding = new TDEAction(
      i18n("Select Encoding..."), 0,
      TQT_TQOBJECT(this), TQT_SLOT( slotShowEncCombo() ), TQT_TQOBJECT(this), "encoding");

  mEncoding->setIcon( TQString::fromLatin1("charset") );

  TDEToolBar *tb = toolBar();
  mEncoding->plug( tb, pathComboIndex() - 1 );
}

KTextFileDialog::~KTextFileDialog() {}

void KTextFileDialog::setEncoding(const TQString& encoding) {
  enc = encoding;
}



void KTextFileDialog::slotShowEncCombo()
{
  // Modal widget asking the user about charset
  //
  KDialogBase *encDlg;
  TQLabel *label;
  TQComboBox *encCombo;
  TQVBox *vbox;

  // Create widgets, and display using geometry management
  encDlg = new KDialogBase( this,
			    "Encoding Dialog", true, i18n("Select Encoding"),
			    KDialogBase::Ok | KDialogBase::Cancel );
  vbox = new TQVBox( encDlg );
  vbox->setSpacing( KDialog::spacingHint() );
  encDlg->setMainWidget( vbox );
  label = new TQLabel( vbox );
  label->setAlignment( AlignLeft | AlignVCenter );
  label->setText(i18n("Select encoding for text file: "));

  encCombo = new TQComboBox(vbox);
  encCombo->setInsertionPolicy(TQComboBox::NoInsertion);
  encCombo->insertItem(i18n("Default Encoding"));

  TQStringList encodings = TDEGlobal::charsets()->descriptiveEncodingNames();
  encodings.prepend( i18n( "Default encoding" ) );
  encCombo->insertStringList( encodings );
  encCombo->setCurrentItem(0);
  TQStringList::Iterator it;
  int i = 1;
  for( it = encodings.begin(); it != encodings.end(); ++it) {

    if ( (*it).contains( encoding() ) ) {
      encCombo->setCurrentItem( i );
      break;
    }

    i++;
  }

  connect( encDlg->actionButton( KDialogBase::Ok ), TQT_SIGNAL(clicked()),
	   encDlg, TQT_SLOT(accept()) );
  connect( encDlg->actionButton( KDialogBase::Cancel ), TQT_SIGNAL(clicked()),
	   encDlg, TQT_SLOT(reject()) );

  encDlg->setMinimumSize( 300, 120);

  if ( encDlg->exec() == TQDialog::Accepted ) {
    // set encoding
    if (encCombo->currentItem() == 0) { // Default
      setEncoding("");
    } else {
      setEncoding(TDEGlobal::charsets()->
		  encodingForName(encCombo->currentText()));
    }
  }


  delete encDlg;
}


KURL KTextFileDialog::getOpenURLwithEncoding(
     const TQString& startDir,
     const TQString& filter,
     TQWidget *parent,
     const TQString& caption,
     const TQString& encoding,
     const TQString& buttontext)
{
  KTextFileDialog dlg(startDir, filter, parent, "filedialog", true);
  dlg.setEncoding(encoding);
  dlg.setOperationMode( Opening );

  dlg.setCaption(caption.isNull() ? i18n("Open") : caption);
  dlg.ops->clearHistory();
  if (!buttontext.isEmpty())
    dlg.okButton()->setText(buttontext);
  dlg.exec();

  KURL url = dlg.selectedURL();
  if (url.isValid()) {
    if ( url.isLocalFile() )
      TDERecentDocument::add( url.path(-1) );
    else
      TDERecentDocument::add( url.url(-1), true );
  }

  // append encoding to the URL params
  url.setFileEncoding(dlg.encoding());

  return url;
}

KURL KTextFileDialog::getSaveURLwithEncoding(
       const TQString& dir, const TQString& filter,
       TQWidget *parent,
       const TQString& caption,
       const TQString& encoding)
{
  KTextFileDialog dlg(dir, filter, parent, "filedialog", true);
  dlg.setEncoding(encoding);
  dlg.setOperationMode( Saving );

  dlg.setCaption(caption.isNull() ? i18n("Save As") : caption);
  dlg.setKeepLocation( true );

  dlg.exec();

  KURL url = dlg.selectedURL();
  if (url.isValid()) {
    if ( url.isLocalFile() )
      TDERecentDocument::add( url.path(-1) );
    else
      TDERecentDocument::add( url.url(-1) );
  }

  // append encoding to the URL params
  url.setFileEncoding(dlg.encoding());

  return url;
}

#include "ktextfiledlg.moc"







