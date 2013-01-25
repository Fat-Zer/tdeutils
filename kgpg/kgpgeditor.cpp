/***************************************************************************
                          kgpgeditor.cpp  -  description
                             -------------------
    begin                : Mon Jul 8 2002
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

#include <stdlib.h>

#include <kaction.h>
#include <kfiledialog.h>
#include <klocale.h>
#include <dcopclient.h>
#include <tqpaintdevicemetrics.h>
#include <tqcstring.h>

#include <kencodingfiledialog.h>

#include <tqradiobutton.h>
#include <tqclipboard.h>
#include <tqtextcodec.h>
#include <tqpainter.h>
#include <kprinter.h>
#include <kmessagebox.h>
#include <kdebug.h>
#include <klineedit.h>
#include <tqcheckbox.h>
#include <kurlrequester.h>
#include <ktempfile.h>
#include <kio/netaccess.h>
#include <kio/renamedlg.h>
#include <kedittoolbar.h>


#include "kgpgsettings.h"
#include "kgpgeditor.h"
#include "sourceselect.h"
#include "keyexport.h"
#include "keyserver.h"
#include "keyservers.h"
#include "kgpg.h"
#include "kgpgview.h"
#include "listkeys.h"
#include "kgpglibrary.h"

KgpgApp::KgpgApp(TQWidget *parent, const char *name, WFlags f,KShortcut goHome,bool mainWindow):KMainWindow(parent, name,f)
{
	isMainWindow=mainWindow;
	textEncoding=TQString();
        readOptions();
	goDefaultKey=goHome;
        // call inits to invoke all other construction parts

        initActions();
        initView();

	slotSetFont(KGpgSettings::font());
        setupGUI(( ToolBar | Keys | StatusBar | Save | Create ), "kgpg.rc");
	setAutoSaveSettings("Editor",true);

}

KgpgApp::~KgpgApp()
{
delete view;
}

void KgpgApp::slotOptions()
{
TQByteArray data;
if (!kapp->dcopClient()->send("kgpg", "KeyInterface", "showOptions()",data))
kdDebug(2100) <<"there was some error using DCOP."<<endl;
}

void KgpgApp::slotKeyManager()
{
TQByteArray data;
if (!kapp->dcopClient()->send("kgpg", "KeyInterface", "showKeyManager()",data))
kdDebug(2100) <<"there was some error using DCOP."<<endl;
}

void KgpgApp::closeEvent ( TQCloseEvent * e )
{
        if (!isMainWindow)
	{
	 kapp->ref();
	KMainWindow::closeEvent( e );
	}
  else e->accept();
}

void KgpgApp::closeWindow()
{
if (view->windowAutoClose) close();
kdDebug(2100) << "Close requested"<<endl;
}

void KgpgApp::saveOptions()
{
        KGpgSettings::setEditorGeometry(size());
        KGpgSettings::setFirstRun(false);
        KGpgSettings::writeConfig();
}

void KgpgApp::readOptions(bool doresize)
{
	customDecrypt=TQStringList::split(TQString(" "), KGpgSettings::customDecrypt().simplifyWhiteSpace());

        if (doresize) {
                TQSize size= KGpgSettings::editorGeometry();
                if (!size.isEmpty())
                        resize(size);
        }

}

void KgpgApp::initActions()
{
        KStdAction::openNew(TQT_TQOBJECT(this), TQT_SLOT(slotFileNew()), actionCollection());
        KStdAction::open(TQT_TQOBJECT(this), TQT_SLOT(slotFileOpen()), actionCollection());
        KStdAction::saveAs(TQT_TQOBJECT(this), TQT_SLOT(slotFileSaveAs()), actionCollection());
        KStdAction::quit(TQT_TQOBJECT(this), TQT_SLOT(slotFileQuit()), actionCollection());
        KStdAction::cut(TQT_TQOBJECT(this), TQT_SLOT(slotEditCut()), actionCollection());
        KStdAction::copy(TQT_TQOBJECT(this), TQT_SLOT(slotEditCopy()), actionCollection());
        KStdAction::paste(TQT_TQOBJECT(this), TQT_SLOT(slotEditPaste()), actionCollection());
	KStdAction::selectAll(TQT_TQOBJECT(this), TQT_SLOT(slotSelectAll()), actionCollection());
	KStdAction::preferences(TQT_TQOBJECT(this), TQT_SLOT(slotOptions()), actionCollection(),"kgpg_config");

        //KStdAction::keyBindings(guiFactory(), TQT_SLOT(configureShortcuts()), actionCollection());
        //KStdAction::configureToolbars(TQT_TQOBJECT(this), TQT_SLOT(slotConfigureToolbars()), actionCollection());

        fileSave = KStdAction::save(TQT_TQOBJECT(this), TQT_SLOT(slotFileSave()), actionCollection());
        (void) new KAction(i18n("&Encrypt File..."), "encrypted", 0,TQT_TQOBJECT(this), TQT_SLOT(slotFilePreEnc()), actionCollection(),"file_encrypt");
        (void) new KAction(i18n("&Decrypt File..."), "decrypted", 0,TQT_TQOBJECT(this), TQT_SLOT(slotFilePreDec()), actionCollection(),"file_decrypt");
	(void) new KAction(i18n("&Open Key Manager"), "kgpg", 0,TQT_TQOBJECT(this), TQT_SLOT(slotKeyManager()), actionCollection(),"key_manage");
        editUndo = KStdAction::undo(TQT_TQOBJECT(this), TQT_SLOT(slotundo()), actionCollection());
        editRedo = KStdAction::redo(TQT_TQOBJECT(this), TQT_SLOT(slotredo()), actionCollection());
        //(void) new KAction(i18n("&Manage Keys"), "kgpg_manage", CTRL+Key_K,TQT_TQOBJECT(this), TQT_SLOT(slotManageKey()), actionCollection(),"keys_manage");
        (void) new KAction(i18n("&Generate Signature..."),0, TQT_TQOBJECT(this), TQT_SLOT(slotPreSignFile()), actionCollection(), "sign_generate");
        (void) new KAction(i18n("&Verify Signature..."),0, TQT_TQOBJECT(this), TQT_SLOT(slotPreVerifyFile()), actionCollection(), "sign_verify");
        (void) new KAction(i18n("&Check MD5 Sum..."), 0,TQT_TQOBJECT(this), TQT_SLOT(slotCheckMd5()), actionCollection(), "sign_check");
	KStdAction::print(TQT_TQOBJECT(this), TQT_SLOT(slotFilePrint()), actionCollection());

    // comment out for now, only confusing
	//encodingAction=new KToggleAction(i18n("&Unicode (utf-8) Encoding"), 0, 0,this, TQT_SLOT(slotSetCharset()),actionCollection(),"charsets");
}

void KgpgApp::slotSetFont(TQFont myFont)
{
view->editor->setFont (myFont);
}


void KgpgApp::slotSetCharset()
{
////////  work in progress
//if (encodingAction->isChecked())
//view->editor->setText(TQString::fromUtf8(view->editor->text().ascii()));
//else
{
if (checkEncoding(TQTextCodec::codecForLocale ())) return;
view->editor->setText(view->editor->text().utf8());
}
}

void KgpgApp::initView()
{
        ////////////////////////////////////////////////////////////////////
        // create the main widget here that is managed by KTMainWindow's view-region and
        // connect the widget to your document to display document contents.

        view = new KgpgView(this,0);
        //  doc->addView(view);
	connect(view,TQT_SIGNAL(resetEncoding(bool)),this,TQT_SLOT(slotResetEncoding(bool)));
        setCentralWidget(view);
        setCaption(i18n("Untitled"),false); ///   doc->URL().fileName(),false);

}

void KgpgApp::slotFileQuit()
{
        saveOptions();
        exit(1);
}

void KgpgApp::slotResetEncoding(bool enc)
{
//kdDebug(2100)<<"Resetting encoding--------------------"<<endl;
//encodingAction->setChecked(enc);
//if (enc) slotSetCharset();
}

void KgpgApp::slotFileNew()
{
        //////  delete all text from editor

        view->editor->setText(TQString());
        editRedo->setEnabled(false);
        editUndo->setEnabled(false);
        setCaption(i18n("Untitled"), false);
        fileSave->setEnabled(false);
        Docname=TQString();
	slotResetEncoding(false);
}

void KgpgApp::slotFilePreEnc()
{
        TQStringList opts;
	KURL::List urls=KFileDialog::getOpenURLs(TQString(),
                                         i18n("*|All Files"), this, i18n("Open File to Encode"));
        if (urls.isEmpty())
                return;
	emit encryptFiles(urls);
}

void KgpgApp::slotFilePreDec()
{

        KURL url=KFileDialog::getOpenURL(TQString(),
                                         i18n("*|All Files"), this, i18n("Open File to Decode"));

        if (url.isEmpty())
                return;
        TQString oldname=url.fileName();

        TQString newname;

        if (oldname.endsWith(".gpg") || oldname.endsWith(".asc") || oldname.endsWith(".pgp"))
                oldname.truncate(oldname.length()-4);
        else
                oldname.append(".clear");
        oldname.prepend(url.directory(0,0));

	KDialogBase *popn=new KDialogBase( KDialogBase::Swallow, i18n("Decrypt File To"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, this, "file_decrypt",true);

	SrcSelect *page=new SrcSelect();
	popn->setMainWidget(page);
	page->newFilename->setURL(oldname);
	page->newFilename->setMode(KFile::File);
	page->newFilename->setCaption(i18n("Save File"));

	page->checkClipboard->setText(i18n("Editor"));
	page->resize(page->minimumSize());
	popn->resize(popn->minimumSize());
        if (popn->exec()==TQDialog::Accepted) {
                if (page->checkFile->isChecked())
                        newname=page->newFilename->url();
        } else {
                delete popn;
                return;
        }
        delete popn;


        if (!newname.isEmpty()) {
                TQFile fgpg(newname);
                if (fgpg.exists()) {
			TDEIO::RenameDlg *over=new TDEIO::RenameDlg(0,i18n("File Already Exists"),TQString(),newname,TDEIO::M_OVERWRITE);
		    	if (over->exec()==TQDialog::Rejected)
	    		{
                	delete over;
                	return;
            		}
	    		newname=over->newDestURL().path();
	    		delete over;
                }
                KgpgLibrary *lib=new KgpgLibrary(this);
                lib->slotFileDec(url,KURL(newname), customDecrypt);
		connect(lib,TQT_SIGNAL(importOver(TQStringList)),this,TQT_SIGNAL(refreshImported(TQStringList)));
        } else
                openEncryptedDocumentFile(url);
}

void KgpgApp::slotFileOpen()
{
	KEncodingFileDialog::Result loadResult;
	loadResult=KEncodingFileDialog::getOpenURLAndEncoding(TQString(),TQString(),TQString(),this);
	KURL url=loadResult.URLs.first();
	textEncoding=loadResult.encoding;

        if(!url.isEmpty()) {
                openDocumentFile(url,textEncoding);
                Docname=url;
                fileSave->setEnabled(false);
                //fileSaveAs->setEnabled(true);
                setCaption(url.fileName(), false);
        }

}

bool KgpgApp::checkEncoding(TQTextCodec *codec)
{
 /////////////          TDEGlobal::locale()->encoding()->name()
return codec->canEncode(view->editor->text());
}

void KgpgApp::slotFileSave()
{
    TQString filn=Docname.path();
    if (filn.isEmpty()) {
    	slotFileSaveAs();
    	return;
    }
    TQTextCodec*cod=TQTextCodec::codecForName (textEncoding.ascii());
        // slotStatusMsg(i18n("Saving file..."));
    if (!checkEncoding(cod)) {
	KMessageBox::sorry(this,i18n("The document could not been saved, as the selected encoding cannot encode every unicode character in it."));
	return;
    }

    KTempFile tmpfile;
    if (Docname.isLocalFile()) {
    TQFile f(filn);
    if ( !f.open( IO_WriteOnly ) ) {
	KMessageBox::sorry(this,i18n("The document could not be saved, please check your permissions and disk space."));
        return;
    }
    TQTextStream t( &f );
    t.setCodec(cod);
    //t.setEncoding( TQTextStream::Latin1 );
    t << view->editor->text();//.utf8();
    f.close();
    }
    else {
	/*FIXME  use following code:
	 TQFile f( fName );
00983         if ( !f.open( IO_ReadOnly ) )
00984             return;
00985         TQFileInfo info ( f );
00986         smModificationTime = new TQTime( info.lastModified().time() );
00987         TQTextStream t(&f);
00988         t.setEncoding( TQTextStream::Latin1 );
00989         TQString s = t.readLine();
00990         f.close();

*/
	TQTextStream *stream = tmpfile.textStream();
	stream->setCodec(cod);
    	*stream << view->editor->text();//.utf8();
   	tmpfile.close();
	if(!TDEIO::NetAccess::upload(tmpfile.name(), Docname,this)) {
	    KMessageBox::sorry(this,i18n("The document could not be saved, please check your permissions and disk space."));
	    tmpfile.unlink();
            return;
	}
	tmpfile.unlink();
    }

    fileSave->setEnabled(false);
    setCaption(Docname.fileName(),false);
}


void KgpgApp::slotFileSaveAs()
{

        //KURL url=KFileDialog::getSaveURL(TQDir::currentDirPath(),i18n("*|All Files"), this, i18n("Save As"));
	KEncodingFileDialog::Result saveResult;
	saveResult=KEncodingFileDialog::getSaveURLAndEncoding (TQString(),TQString(),TQString(),this);
	KURL url=saveResult.URLs.first();
	TQString selectedEncoding=saveResult.encoding;

	if(!url.isEmpty()) {
		if (url.isLocalFile())
		{
                TQString filn=url.path();
                TQFile f(filn);
                if (f.exists()) {
                        TQString message=i18n("Overwrite existing file %1?").arg(url.fileName());
                        int result=KMessageBox::warningContinueCancel(this,TQString(message),i18n("Warning"),i18n("Overwrite"));
                        if (result==KMessageBox::Cancel)
                                return;
                }
                f.close();
		}
		else if (TDEIO::NetAccess::exists(url,false,this))
		{
		TQString message=i18n("Overwrite existing file %1?").arg(url.fileName());
                        int result=KMessageBox::warningContinueCancel(this,TQString(message),i18n("Warning"),i18n("Overwrite"));
                        if (result==KMessageBox::Cancel)
                                return;
		}
	Docname=url;
	textEncoding=selectedEncoding;
	slotFileSave();
	}
}

void KgpgApp::openDocumentFile(const KURL& url,TQString encoding)
{
TQString tempOpenFile;
        /////////////////////////////////////////////////
if( TDEIO::NetAccess::download( url, tempOpenFile,this ) ) {
        TQFile qfile(tempOpenFile);
        if (qfile.open(IO_ReadOnly)) {
                TQTextStream t( &qfile );
		t.setCodec(TQTextCodec::codecForName (encoding.ascii()));
                view->editor->setText(t.read());
                qfile.close();
                fileSave->setEnabled(false);
                editRedo->setEnabled(false);
                editUndo->setEnabled(false);
        }
}
}

void KgpgApp::slotFilePrint()
{
        KPrinter prt;
        //kdDebug(2100)<<"Printing..."<<endl;
        if (prt.setup(this)) {
                TQPainter painter(&prt);
                TQPaintDeviceMetrics metrics(painter.device());
                painter.drawText( 0, 0, metrics.width(), metrics.height(), AlignLeft|AlignTop|DontClip,view->editor->text() );
        }
}

void KgpgApp::slotEditCut()
{
        view->editor->cut();
}

void KgpgApp::slotEditCopy()
{
        view->editor->copy();
}

void KgpgApp::slotEditPaste()
{
        view->editor->paste();
}

void KgpgApp::slotSelectAll()
{
        view->editor->selectAll();
}

void KgpgApp::slotundo()
{
        view->editor->undo();
        editRedo->setEnabled(true);
}

void KgpgApp::slotredo()
{
        view->editor->redo();
}

/////////////    file signature slots


void KgpgApp::slotCheckMd5()
{
        /////////////////////////////////////////////////////////////////////////  display md5 sum for a chosen file

        KURL url=KFileDialog::getOpenURL(TQString(),
                                         i18n("*|All Files"), this, i18n("Open File to Verify"));
        if (!url.isEmpty()) {

                Md5Widget *mdwidget=new Md5Widget(this,0,url);
                mdwidget->exec();
                delete mdwidget;
                //      KMessageBox::information(this,TQString("MD5 sum for "+url.fileName()+" is:\n"+checkfile.hexDigest().data()));
        }
}


void KgpgApp::slotPreSignFile()
{
        //////////////////////////////////////   create a detached signature for a chosen file
        KURL url=KFileDialog::getOpenURL(TQString(),i18n("*|All Files"), this, i18n("Open File to Sign"));
        if (!url.isEmpty())
                slotSignFile(url);
}

void KgpgApp::slotSignFile(KURL url)
{
        //////////////////////////////////////   create a detached signature for a chosen file
        TQString signKeyID;
        if (!url.isEmpty()) {
                //////////////////   select a private key to sign file --> listkeys.cpp
                KgpgSelKey *opts=new KgpgSelKey(this,"select_secret");
                if (opts->exec()==TQDialog::Accepted)
                        signKeyID=opts->getkeyID();
                else {
                        delete opts;
                        return;
                }
                delete opts;
                TQString Options;
                if (KGpgSettings::asciiArmor())
                        Options="--armor";
                if (KGpgSettings::pgpCompatibility())
                        Options+="--pgp6";
                KgpgInterface *signFileProcess=new KgpgInterface();
                signFileProcess->KgpgSignFile(signKeyID,url,Options);
        }
}

void KgpgApp::slotPreVerifyFile()
{
        KURL url=KFileDialog::getOpenURL(TQString(),
                                         i18n("*|All Files"), this, i18n("Open File to Verify"));
        slotVerifyFile(url);
}

void KgpgApp::slotVerifyFile(KURL url)
{
        ///////////////////////////////////   check file signature
        TQString sigfile=TQString();
        if (!url.isEmpty()) {
                //////////////////////////////////////       try to find detached signature.
                if (!url.fileName().endsWith(".sig")) {
                        sigfile=url.path()+".sig";
                        TQFile fsig(sigfile);
                        if (!fsig.exists()) {
                                sigfile=url.path()+".asc";
                                TQFile fsig(sigfile);
                                //////////////   if no .asc or .sig signature file included, assume the file is internally signed
                                if (!fsig.exists())
                                        sigfile=TQString();
                        }
                }
                ///////////////////////// pipe gpg command
                KgpgInterface *verifyFileProcess=new KgpgInterface();
                verifyFileProcess->KgpgVerifyFile(url,KURL(sigfile));
                connect (verifyFileProcess,TQT_SIGNAL(verifyquerykey(TQString)),this,TQT_SLOT(importSignatureKey(TQString)));
        }
}

void KgpgApp::importSignatureKey(TQString ID)
{
        keyServer *kser=new keyServer(0,"server_dialog",false);
        kser->page->kLEimportid->setText(ID);
        kser->slotImport();
}

void KgpgApp::openEncryptedDocumentFile(const KURL& url)
{
        view->editor->slotDroppedFile(url);
}


#include "kgpgeditor.moc"
