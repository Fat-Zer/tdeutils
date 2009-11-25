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

#include <qlabel.h>
#include <qvbox.h>

#include <klocale.h>
#include <kcharsets.h>
#include <kdiroperator.h>
#include <krecentdocument.h>
#include <ktoolbar.h>
#include <kpushbutton.h>

#include "ktextfiledlg.h"

KTextFileDialog::KTextFileDialog(const QString& startDir,
				 const QString& filter,
				 QWidget *parent, const char* name,
				 bool modal)
  : KFileDialog(startDir, filter, parent, name, modal)
{
  /*
  // insert encoding action into toolbar
  KSelectAction *mEncoding = new KSelectAction(
      i18n( "Set &Encoding" ), 0, this,
      SLOT( slotSetEncoding() ), this,
      "encoding" );

  QStringList encodings = KGlobal::charsets()->descriptiveEncodingNames();
  encodings.prepend( i18n( "Default encoding" ) );
  mEncoding->setItems( encodings );
  mEncoding->setCurrentItem(0);
  QStringList::Iterator it;
  int i = 0;
  for( it = encodings.begin(); it != encodings.end(); ++it) {
      if ( (*it).contains( encodingStr ) ) {
      mEncoding->setCurrentItem( i );
      break;
      }
    i++;
  }

  KToolBar *tb = toolBar();
  mEncoding->plug( tb, 7 );
  */

  KAction* mEncoding = new KAction(
      i18n("Select Encoding..."), 0,
      this, SLOT( slotShowEncCombo() ), this, "encoding");

  mEncoding->setIcon( QString::fromLatin1("charset") );

  KToolBar *tb = toolBar();
  mEncoding->plug( tb, pathComboIndex() - 1 );
}

KTextFileDialog::~KTextFileDialog() {}

void KTextFileDialog::setEncoding(const QString& encoding) {
  enc = encoding;
}



void KTextFileDialog::slotShowEncCombo()
{
  // Modal widget asking the user about charset
  //
  KDialogBase *encDlg;
  QLabel *label;
  QComboBox *encCombo;
  QVBox *vbox;

  // Create widgets, and display using geometry management
  encDlg = new KDialogBase( this,
			    "Encoding Dialog", true, i18n("Select Encoding"),
			    KDialogBase::Ok | KDialogBase::Cancel );
  vbox = new QVBox( encDlg );
  vbox->setSpacing( KDialog::spacingHint() );
  encDlg->setMainWidget( vbox );
  label = new QLabel( vbox );
  label->setAlignment( AlignLeft | AlignVCenter );
  label->setText(i18n("Select encoding for text file: "));

  encCombo = new QComboBox(vbox);
  encCombo->setInsertionPolicy(QComboBox::NoInsertion);
  encCombo->insertItem(i18n("Default Encoding"));

  QStringList encodings = KGlobal::charsets()->descriptiveEncodingNames();
  encodings.prepend( i18n( "Default encoding" ) );
  encCombo->insertStringList( encodings );
  encCombo->setCurrentItem(0);
  QStringList::Iterator it;
  int i = 1;
  for( it = encodings.begin(); it != encodings.end(); ++it) {

    if ( (*it).contains( encoding() ) ) {
      encCombo->setCurrentItem( i );
      break;
    }

    i++;
  }

  connect( encDlg->actionButton( KDialogBase::Ok ), SIGNAL(clicked()),
	   encDlg, SLOT(accept()) );
  connect( encDlg->actionButton( KDialogBase::Cancel ), SIGNAL(clicked()),
	   encDlg, SLOT(reject()) );

  encDlg->setMinimumSize( 300, 120);

  if ( encDlg->exec() == QDialog::Accepted ) {
    // set encoding
    if (encCombo->currentItem() == 0) { // Default
      setEncoding("");
    } else {
      setEncoding(KGlobal::charsets()->
		  encodingForName(encCombo->currentText()));
    }
  }


  delete encDlg;
}


KURL KTextFileDialog::getOpenURLwithEncoding(
     const QString& startDir,
     const QString& filter,
     QWidget *parent,
     const QString& caption,
     const QString& encoding,
     const QString& buttontext)
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
      KRecentDocument::add( url.path(-1) );
    else
      KRecentDocument::add( url.url(-1), true );
  }

  // append encoding to the URL params
  url.setFileEncoding(dlg.encoding());

  return url;
}

KURL KTextFileDialog::getSaveURLwithEncoding(
       const QString& dir, const QString& filter,
       QWidget *parent,
       const QString& caption,
       const QString& encoding)
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
      KRecentDocument::add( url.path(-1) );
    else
      KRecentDocument::add( url.url(-1) );
  }

  // append encoding to the URL params
  url.setFileEncoding(dlg.encoding());

  return url;
}

#include "ktextfiledlg.moc"







