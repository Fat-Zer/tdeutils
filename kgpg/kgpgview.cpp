/***************************************************************************
                          kgpgview.cpp  -  description
                             -------------------
    begin                : Tue Jul  2 12:31:38 GMT 2002
    copyright          : (C) 2002 by Jean-Baptiste Mardelle
    email                : bj@altern.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//////////////////////////////////////////////////////////////  code for the main view (editor)

// include files for Qt
//#include <tqprinter.h>

// application specific includes

#include <tqscrollview.h>
#include <tqregexp.h>

#include <kio/netaccess.h>
#include <klocale.h>
#include <kstdaction.h>
#include <kurldrag.h>

#include <kmessagebox.h>
#include <tqfile.h>
#include <tqlayout.h>
#include <kbuttonbox.h>
#include <unistd.h>
#include <kaction.h>
#include <klineedit.h>
#include <tqtextcodec.h>


#include "kgpgsettings.h"
#include "kgpginterface.h"
#include "kgpgview.h"
#include "keyservers.h"
#include "keyserver.h"
#include "kgpg.h"
#include "kgpgeditor.h"
#include "listkeys.h"
#include "popuppublic.h"
#include "detailedconsole.h"

//////////////// configuration for editor

MyEditor::MyEditor( TQWidget *parent, const char *name )
                : KTextEdit( parent, name )
{
        setTextFormat(PlainText);
	setCheckSpellingEnabled (true);
        setAcceptDrops(true);
}

void MyEditor::contentsDragEnterEvent( TQDragEnterEvent *e )
{
        ////////////////   if a file is dragged into editor ...
        e->accept (KURLDrag::canDecode(e) || TQTextDrag::canDecode (e));
        //e->accept (TQTextDrag::canDecode (e));
}




void MyEditor::contentsDropEvent( TQDropEvent *e )
{
        /////////////////    decode dropped file
        KURL::List list;
        TQString text;
        if ( KURLDrag::decode( e, list ) )
                slotDroppedFile(list.first());
        else if ( TQTextDrag::decode(e, text) )
                insert(text);
}

void MyEditor::slotDroppedFile(KURL url)
{
        /////////////////    decide what to do with dropped file
        TQString text;
        if (!tempFile.isEmpty()) {
                KIO::NetAccess::removeTempFile(tempFile);
                tempFile=TQString::null;
        }

        if (url.isLocalFile())
                tempFile = url.path();
        else {
                if (KMessageBox::warningContinueCancel(0,i18n("<qt><b>Remote file dropped</b>.<br>The remote file will now be copied to a temporary file to process requested operation. This temporary file will be deleted after operation.</qt>"),0,KStdGuiItem::cont(),"RemoteFileWarning")!=KMessageBox::Continue)
                        return;
                if (!KIO::NetAccess::download (url, tempFile,this)) {
                        KMessageBox::sorry(this,i18n("Could not download file."));
                        return;
                }
        }


                /////////////  if dropped filename ends with gpg, pgp or asc, try to decode it
                if ((tempFile.endsWith(".gpg")) || (tempFile.endsWith(".asc")) || (tempFile.endsWith(".pgp"))) {
                        slotDecodeFile(tempFile);
                }
		else slotCheckContent(tempFile);
}


bool MyEditor::slotCheckContent(TQString fileToCheck, bool checkForPgpMessage)
{
TQFile qfile(fileToCheck);
        if (qfile.open(IO_ReadOnly)) {
                //////////   open file

                        TQTextStream t( &qfile );
                        TQString result(t.read());
                        //////////////     if  pgp data found, decode it
                        if ((checkForPgpMessage) && (result.startsWith("-----BEGIN PGP MESSAGE"))) {
                                qfile.close();
                                slotDecodeFile(fileToCheck);
                                return true;
                        } else
                                if (result.startsWith("-----BEGIN PGP PUBLIC KEY BLOCK")) {//////  dropped file is a public key, ask for import
                                        qfile.close();
                                        int result=KMessageBox::warningContinueCancel(this,i18n("<p>The file <b>%1</b> is a public key.<br>Do you want to import it ?</p>").arg(fileToCheck),i18n("Warning"));
                                        if (result==KMessageBox::Cancel) {
                                                KIO::NetAccess::removeTempFile(fileToCheck);
                                                return true;
                                        } else {
                                                KgpgInterface *importKeyProcess=new KgpgInterface();
                                                importKeyProcess->importKeyURL(KURL(fileToCheck));
                                                connect(importKeyProcess,TQT_SIGNAL(importfinished(TQStringList)),this,TQT_SLOT(slotProcessResult(TQStringList)));
                                                return true;
                                        }
                                } else {
                                        if (result.startsWith("-----BEGIN PGP PRIVATE KEY BLOCK")) {
						qfile.close();
                                                KMessageBox::information(0,i18n("This file is a private key.\nPlease use kgpg key management to import it."));
                                                KIO::NetAccess::removeTempFile(fileToCheck);
                                                return true;
                                        }

                                        setText(result);
                                        qfile.close();
                                        KIO::NetAccess::removeTempFile(fileToCheck);
                                }
                }
		return false;
}


void MyEditor::editorUpdateDecryptedtxt(TQString newtxt)
{
	setText(newtxt);
}

void MyEditor::editorFailedDecryptedtxt(TQString newtxt)
{
	if (!slotCheckContent(tempFile,false))
	KMessageBox::detailedSorry(this,i18n("Decryption failed."),newtxt);
}


void MyEditor::slotDecodeFile(TQString fname)
{
        ////////////////     decode file from given url into editor
        TQFile qfile(TQFile::encodeName(fname));
        if (qfile.open(IO_ReadOnly)) {
	KgpgInterface *txtDecrypt=new KgpgInterface();
        connect (txtDecrypt,TQT_SIGNAL(txtdecryptionfinished(TQString)),this,TQT_SLOT(editorUpdateDecryptedtxt(TQString)));
	connect (txtDecrypt,TQT_SIGNAL(txtdecryptionfailed(TQString)),this,TQT_SLOT(editorFailedDecryptedtxt(TQString)));
        txtDecrypt->KgpgDecryptFileToText(KURL(fname),TQStringList::split(TQString(" "),KGpgSettings::customDecrypt().simplifyWhiteSpace()));
        } else
                KMessageBox::sorry(this,i18n("Unable to read file."));
}


void MyEditor::slotProcessResult(TQStringList iKeys)
{
	emit refreshImported(iKeys);
        KIO::NetAccess::removeTempFile(tempFile);
        tempFile=TQString::null;
}


////////////////////////// main view configuration

KgpgView::KgpgView(TQWidget *parent, const char *name) : TQWidget(parent, name)
{
        editor=new MyEditor(this);
	windowAutoClose=true;

        /////    layout

        TQVBoxLayout *vbox=new TQVBoxLayout(this,3);

        editor->setReadOnly( false );
        editor->setUndoRedoEnabled(true);
        editor->setUndoDepth(5);

        setAcceptDrops(true);

        KButtonBox *boutonbox=new KButtonBox(this,KButtonBox::Horizontal,15,12);
        boutonbox->addStretch(1);

        bouton0=boutonbox->addButton(i18n("S&ign/Verify"),this,TQT_SLOT(clearSign()),TRUE);
        bouton1=boutonbox->addButton(i18n("En&crypt"),this,TQT_SLOT(popuppublic()),TRUE);
        bouton2=boutonbox->addButton(i18n("&Decrypt"),this,TQT_SLOT(slotdecode()),TRUE);

        TQObject::connect(editor,TQT_SIGNAL(textChanged()),this,TQT_SLOT(modified()));

        boutonbox->layout();
        editor->resize(editor->maximumSize());
        vbox->addWidget(editor);
        vbox->addWidget(boutonbox);
}


void KgpgView::modified()
{
        /////////////// notify for changes in editor window
        KgpgApp *win=(KgpgApp *) parent();
        if (win->fileSave->isEnabled()==false) {
                TQString capt=win->Docname.fileName();
                if (capt.isEmpty())
                        capt=i18n("untitled");
                win->setCaption(capt,true);
                win->fileSave->setEnabled(true);
                win->editUndo->setEnabled(true);
        }

}

void KgpgView::slotAskForImport(TQString ID)
{
if (KMessageBox::questionYesNo(0,i18n("<qt><b>Missing signature:</b><br>Key id: %1<br><br>"
	"Do you want to import this key from a keyserver?</qt>").arg(ID),i18n("Missing Key"), TQString::null, i18n("Import"), i18n("Do Not Import"))==KMessageBox::Yes) {
        keyServer *kser=new keyServer(0,"server_dialog",false,true);
        kser->page->kLEimportid->setText(ID);
        kser->slotImport();
	windowAutoClose=false;
        }
	else emit verifyFinished();
}

void KgpgView::slotVerifyResult(TQString mssge,TQString log)
{
emit verifyFinished();
//KMessageBox::information(0,mssge);
(void) new KDetailedInfo(0,"verify_result",mssge,log);
}

void KgpgView::clearSign()
{
        TQString mess=editor->text();
        if (mess.startsWith("-----BEGIN PGP SIGNED")) {
                //////////////////////   this is a signed message, verify it
		KgpgInterface *verifyProcess=new KgpgInterface();
		connect(verifyProcess,TQT_SIGNAL(missingSignature(TQString)),this,TQT_SLOT(slotAskForImport(TQString)));
		connect(verifyProcess,TQT_SIGNAL(verifyOver(TQString,TQString)),this,TQT_SLOT(slotVerifyResult(TQString,TQString)));
		verifyProcess->KgpgVerifyText(mess);
        	}
		else {
                /////    Sign the text in Editor
		TQString signKeyID;
		///// open key selection dialog
                KgpgSelKey *opts=new KgpgSelKey(this,0);

                if (opts->exec()==TQDialog::Accepted) {
                        signKeyID=opts->getkeyID();
                } else {
                        delete opts;
                        return;
                }
                delete opts;

		KgpgInterface *signProcess=new KgpgInterface();
		connect(signProcess,TQT_SIGNAL(txtSignOver(TQString)),this,TQT_SLOT(slotSignResult(TQString)));
		TQStringList options=NULL;
		if (KGpgSettings::pgpCompatibility())
                        options<<"--pgp6";
		signProcess->KgpgSignText(mess,signKeyID,options);
}
}


void KgpgView::slotSignResult(TQString signResult)
{
if (signResult.isEmpty())
KMessageBox::sorry(this,i18n("Signing not possible: bad passphrase or missing key"));
else
{
	editor->setText(signResult);
	emit resetEncoding(false);

	KgpgApp *win=(KgpgApp *) parent();
	win->editRedo->setEnabled(false);
	win->editUndo->setEnabled(false);
}
}



void KgpgView::popuppublic()
{
        /////    popup dialog to select public key for encryption

        ////////  open dialog --> popuppublic.cpp
        popupPublic *dialogue=new popupPublic(this, "public_keys", 0,false,((KgpgApp *) parent())->goDefaultKey);
        connect(dialogue,TQT_SIGNAL(selectedKey(TQStringList,TQStringList,bool,bool)),this,TQT_SLOT(encodetxt(TQStringList,TQStringList,bool,bool)));
        dialogue->exec();
        delete dialogue;
}


//////////////////////////////////////////////////////////////////////////////////////     decode

void KgpgView::slotdecode()
{
        ///////////////    decode data from the editor. triggered by the decode button

        TQString dests,encUsers;
        messages=TQString::null;

        //TQString resultat=KgpgInterface::KgpgDecryptText(editor->text(),encUsers);
	KgpgInterface *txtDecrypt=new KgpgInterface();
        connect (txtDecrypt,TQT_SIGNAL(txtdecryptionfinished(TQString)),this,TQT_SLOT(updateDecryptedtxt(TQString)));
	connect (txtDecrypt,TQT_SIGNAL(txtdecryptionfailed(TQString)),this,TQT_SLOT(failedDecryptedtxt(TQString)));
        txtDecrypt->KgpgDecryptText(editor->text(),TQStringList::split(TQString(" "),KGpgSettings::customDecrypt().simplifyWhiteSpace()));

	/*
        KgpgApp *win=(KgpgApp *) parent();
        if (!resultat.isEmpty()) {
                editor->setText(resultat);
                win->editRedo->setEnabled(false);
                win->editUndo->setEnabled(false);
        }*/
}

void KgpgView::updateDecryptedtxt(TQString newtxt)
{
	//kdDebug(2100)<<"UTF8 Test Result--------------"<<KgpgView::checkForUtf8()<<endl;
	editor->setText(newtxt);
	emit resetEncoding(false);
}

bool KgpgView::checkForUtf8(TQString text)
{  //// try to guess if the decrypted text uses utf-8 encoding
    TQTextCodec *codec =TQTextCodec::codecForLocale ();
    if (codec->canEncode(text)) return true;
    return false;
}

void KgpgView::failedDecryptedtxt(TQString newtxt)
{
	KMessageBox::detailedSorry(this,i18n("Decryption failed."),newtxt);
}


void KgpgView::encodetxt(TQStringList selec,TQStringList encryptOptions,bool, bool symmetric)
{
        //////////////////              encode from editor
        if (KGpgSettings::pgpCompatibility())
                encryptOptions<<"--pgp6";

	if (symmetric) selec.clear();

	KgpgInterface *txtCrypt=new KgpgInterface();
        connect (txtCrypt,TQT_SIGNAL(txtencryptionfinished(TQString)),this,TQT_SLOT(updatetxt(TQString)));
        txtCrypt->KgpgEncryptText(editor->text(),selec,encryptOptions);
        //KMessageBox::sorry(0,"OVER");

        //KgpgInterface::KgpgEncryptText(editor->text(),selec,encryptOptions);
        //if (!resultat.isEmpty()) editor->setText(resultat);
        //else KMessageBox::sorry(this,i18n("Decryption failed."));
}

void KgpgView::updatetxt(TQString newtxt)
{
        if (!newtxt.isEmpty())
                editor->setText(newtxt);
        else
                KMessageBox::sorry(this,i18n("Encryption failed."));
}


KgpgView::~KgpgView()
{
delete editor;
}

/*
void KgpgView::print(TQPrinter *pPrinter)
{
  TQPainter printpainter;
  printpainter.begin(pPrinter);

  // TODO: add your printing code here

  printpainter.end();
}
*/
#include "kgpgview.moc"

