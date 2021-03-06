/***************************************************************************
                          kgpg.cpp  -  description
                             -------------------
    begin                : Mon Nov 18 2002
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

#include <tqlabel.h>
#include <tqclipboard.h>
#include <tqfile.h>
#include <tqpopupmenu.h>
#include <tqwidget.h>
#include <tqcheckbox.h>
#include <tqmovie.h>
#include <tqcstring.h>
#include <tqhbuttongroup.h>
#include <tdeglobal.h>
#include <kactivelabel.h>
#include <tdeversion.h>
#include <tdelocale.h>
#include <tdeconfig.h>
#include <tdeapplication.h>
#include <tdemessagebox.h>
#include <kcombobox.h>
#include <tqregexp.h>
#include <tdecmdlineargs.h>
#include <tqtooltip.h>
#include <kdebug.h>
#include <kurlrequesterdlg.h>
#include <klineedit.h>
#include <tdeio/renamedlg.h>
#include <kpassivepopup.h>
#include <tqlayout.h>
#include <tqbuttongroup.h>
#include <kiconloader.h>
#include <tdetempfile.h>
#include <twin.h>
#include <kprocess.h>
#include <kprocio.h>
#include <tdeaboutapplication.h>
#include <kurlrequester.h>
#include <ktip.h>
#include <kurldrag.h>
#include <ktar.h>
#include <kzip.h>
#include <dcopclient.h>
#include <kstandarddirs.h>
#include <tdefiledialog.h>
#include <tdepopupmenu.h>
#include <tqcursor.h>
#include <kdesktopfile.h>

#include "kgpgeditor.h"

#include "kgpg.h"
#include "kgpgsettings.h"
#include "listkeys.h"
#include "keyserver.h"
#include "keyservers.h"
#include "popuppublic.h"
#include "kgpgview.h"
#include "kgpglibrary.h"
#include "kgpgwizard.h"

MyView::MyView( TQWidget *parent, const char *name )
                : TQLabel( parent, name )
{
        setBackgroundMode( X11ParentRelative );
	openTasks=0;

	TDEAction *saveDecrypt=new TDEAction(i18n("&Decrypt && Save File"),"decrypted",0,TQT_TQOBJECT(this), TQT_SLOT(decryptDroppedFile()),TQT_TQOBJECT(this),"decrypt_file");
        TDEAction *showDecrypt=new TDEAction(i18n("&Show Decrypted File"),"edit",0,TQT_TQOBJECT(this), TQT_SLOT(showDroppedFile()),TQT_TQOBJECT(this),"show_file");
        TDEAction *encrypt=new TDEAction(i18n("&Encrypt File"),"encrypted",0,TQT_TQOBJECT(this), TQT_SLOT(encryptDroppedFile()),TQT_TQOBJECT(this),"encrypt_file");
        TDEAction *sign=new TDEAction(i18n("&Sign File"), "signature",0,TQT_TQOBJECT(this), TQT_SLOT(signDroppedFile()),TQT_TQOBJECT(this),"sign_file");
        //TQToolTip::add(this,i18n("KGpg drag & drop encryption applet"));

        readOptions();
	resize(24,24);
	setPixmap( KSystemTray::loadIcon("kgpg_docked"));
        setAcceptDrops(true);

        droppopup=new TQPopupMenu();
        showDecrypt->plug(droppopup);
        saveDecrypt->plug(droppopup);

        udroppopup=new TQPopupMenu();
        encrypt->plug(udroppopup);
        sign->plug(udroppopup);
	TQToolTip::add(this, i18n("KGpg - encryption tool"));
}

MyView::~MyView()
{

        delete droppopup;
        droppopup = 0;
        delete udroppopup;
        udroppopup = 0;
}


void  MyView::clipEncrypt()
{
        popupPublic *dialoguec=new popupPublic(0, "public_keys", 0,false,goDefaultKey);
        connect(dialoguec,TQT_SIGNAL(selectedKey(TQStringList,TQStringList,bool,bool)),TQT_TQOBJECT(this),TQT_SLOT(encryptClipboard(TQStringList,TQStringList,bool,bool)));
        dialoguec->exec();
        delete dialoguec;
}

void  MyView::clipDecrypt()
{
        TQString clippie=kapp->clipboard()->text(clipboardMode).stripWhiteSpace();
	droppedtext(clippie,false);
}

void  MyView::clipSign(bool openEditor)
{
        TQString clippie=kapp->clipboard()->text(clipboardMode).stripWhiteSpace();
        if (!clippie.isEmpty()) {
                KgpgApp *kgpgtxtedit = new KgpgApp(0, "editor",WDestructiveClose,goDefaultKey);
		connect(this,TQT_SIGNAL(setFont(TQFont)),kgpgtxtedit,TQT_SLOT(slotSetFont(TQFont)));
		connect(kgpgtxtedit,TQT_SIGNAL(encryptFiles(KURL::List)),TQT_TQOBJECT(this),TQT_SLOT(encryptFiles(KURL::List)));
		if (!openEditor)
		connect(kgpgtxtedit->view,TQT_SIGNAL(verifyFinished()),kgpgtxtedit,TQT_SLOT(closeWindow()));
                kgpgtxtedit->view->editor->setText(clippie);
                kgpgtxtedit->view->clearSign();
		kgpgtxtedit->show();

        } else
                KMessageBox::sorry(this,i18n("Clipboard is empty."));
}

void MyView::encryptDroppedFolder()
{
	compressionScheme=0;
        kgpgfoldertmp=new KTempFile(TQString());
        kgpgfoldertmp->setAutoDelete(true);
        if (KMessageBox::warningContinueCancel(0,i18n("<qt>KGpg will now create a temporary archive file:<br><b>%1</b> to process the encryption. The file will be deleted after the encryption is finished.</qt>").arg(kgpgfoldertmp->name()),i18n("Temporary File Creation"),KStdGuiItem::cont(),"FolderTmpFile")==KMessageBox::Cancel)
                return;

	dialogue=new popupPublic(0,"Public keys",droppedUrls.first().fileName(),true,goDefaultKey);

	TQHButtonGroup *bGroup = new TQHButtonGroup(dialogue->plainPage());
                (void) new TQLabel(i18n("Compression method for archive:"),bGroup);
                KComboBox *optionbx=new KComboBox(bGroup);
		optionbx->insertItem(i18n("Zip"));
		optionbx->insertItem(i18n("Gzip"));
		optionbx->insertItem(i18n("Bzip2"));
	bGroup->show();
	connect(dialogue,TQT_SIGNAL(keyListFilled ()),dialogue,TQT_SLOT(slotSetVisible()));
	connect(optionbx,TQT_SIGNAL(activated (int)),TQT_TQOBJECT(this),TQT_SLOT(slotSetCompression(int)));
        connect(dialogue,TQT_SIGNAL(selectedKey(TQStringList,TQStringList,bool,bool)),TQT_TQOBJECT(this),TQT_SLOT(startFolderEncode(TQStringList,TQStringList,bool,bool)));
        dialogue->CBshred->setEnabled(false);
        dialogue->exec();
	dialogue=0L;
}

void MyView::slotSetCompression(int cp)
{
compressionScheme=cp;
}

void MyView::startFolderEncode(TQStringList selec,TQStringList encryptOptions,bool ,bool symetric)
{
TQString extension;

if (compressionScheme==0)
	extension=".zip";
	else if (compressionScheme==1)
	extension=".tar.gz";
	else
	extension=".tar.bz2";

if (encryptOptions.find("armor")!=encryptOptions.end () )
                extension+=".asc";
        else if (KGpgSettings::pgpExtension())
                extension+=".pgp";
        else
                extension+=".gpg";

KURL encryptedFile(droppedUrls.first().path()+extension);
TQFile encryptedFolder(droppedUrls.first().path()+extension);
if (encryptedFolder.exists()) {
			dialogue->hide();
			TDEIO::RenameDlg *over=new TDEIO::RenameDlg(0,i18n("File Already Exists"),TQString(),encryptedFile.path(),TDEIO::M_OVERWRITE);
		    	if (over->exec()==TQDialog::Rejected)
	    		{
                	delete over;
                	return;
            		}
	    		encryptedFile=over->newDestURL();
	    		delete over;
			dialogue->show();   /////// strange, but if dialogue is hidden, the passive popup is not displayed...
                }

pop = new KPassivePopup();
	pop->setView(i18n("Processing folder compression and encryption"),i18n("Please wait..."),TDEGlobal::iconLoader()->loadIcon("kgpg",TDEIcon::Desktop));
	pop->setAutoDelete(false);
	pop->show();
	kapp->processEvents();
	dialogue->slotAccept();
	dialogue=0L;

	KArchive *arch;
	if (compressionScheme==0)
	arch=new KZip(kgpgfoldertmp->name());
	else if (compressionScheme==1)
	arch=new KTar(kgpgfoldertmp->name(), "application/x-gzip");
	else
	arch=new KTar(kgpgfoldertmp->name(), "application/x-bzip2");

        if (!arch->open( IO_WriteOnly )) {
                KMessageBox::sorry(0,i18n("Unable to create temporary file"));
		delete arch;
                return;
        }
        arch->addLocalDirectory (droppedUrls.first().path(),droppedUrls.first().fileName());
        arch->close();
        delete arch;

        KgpgInterface *folderprocess=new KgpgInterface();
        folderprocess->KgpgEncryptFile(selec,KURL(kgpgfoldertmp->name()),encryptedFile,encryptOptions,symetric);
        connect(folderprocess,TQT_SIGNAL(encryptionfinished(KURL)),TQT_TQOBJECT(this),TQT_SLOT(slotFolderFinished(KURL)));
        connect(folderprocess,TQT_SIGNAL(errormessage(TQString)),TQT_TQOBJECT(this),TQT_SLOT(slotFolderFinishedError(TQString)));
}

void  MyView::slotFolderFinished(KURL)
{
        delete pop;
        delete kgpgfoldertmp;
}

void  MyView::slotFolderFinishedError(TQString errmsge)
{
        delete pop;
        delete kgpgfoldertmp;
        KMessageBox::sorry(0,errmsge);
}

void MyView::busyMessage(TQString mssge,bool reset)
{
if (reset) openTasks=0;
if (!mssge.isEmpty())
{
openTasks++;
TQToolTip::remove(this);
TQToolTip::add(this, mssge);
setMovie(TQMovie(locate("appdata","pics/kgpg_docked.gif")));
}
else openTasks--;

//kdDebug(2100) << "Emit message: "<<openTasks<<endl;

if (openTasks<=0)
{
setPixmap( KSystemTray::loadIcon("kgpg_docked"));
TQToolTip::remove(this);
TQToolTip::add(this, i18n("KGpg - encryption tool"));
openTasks=0;
}
}

void  MyView::encryptDroppedFile()
{
        TQStringList opts;
        KgpgLibrary *lib=new KgpgLibrary(this,KGpgSettings::pgpExtension());
	connect(lib,TQT_SIGNAL(systemMessage(TQString,bool)),TQT_TQOBJECT(this),TQT_SLOT(busyMessage(TQString,bool)));
        if (KGpgSettings::fileKey()!=TQString()) {
                if (KGpgSettings::allowUntrustedKeys())
                        opts<<"--always-trust";
                if (KGpgSettings::asciiArmor())
                        opts<<"--armor";
                if (KGpgSettings::hideUserID())
                        opts<<"--throw-keyid";
                if (KGpgSettings::pgpCompatibility())
                        opts<<"--pgp6";
                lib->slotFileEnc(droppedUrls,opts,TQStringList::split(" ",KGpgSettings::fileKey()),goDefaultKey);
        } else
                lib->slotFileEnc(droppedUrls,TQString(),TQString(),goDefaultKey);
}

void  MyView::encryptFiles(KURL::List urls)
{
droppedUrls=urls;
encryptDroppedFile();
}

void  MyView::shredDroppedFile()
{
KDialogBase *shredConfirm=new KDialogBase( this, "confirm_shred", true,i18n("Shred Files"),KDialogBase::Ok | KDialogBase::Cancel);
TQWidget *page = new TQWidget(shredConfirm);
shredConfirm->setMainWidget(page);
TQBoxLayout *layout=new TQBoxLayout(page,TQBoxLayout::TopToBottom,0);
layout->setAutoAdd(true);

(void) new KActiveLabel( i18n("Do you really want to <a href=\"whatsthis:%1\">shred</a> these files?").arg(i18n( "<qt><p>You must be aware that <b>shredding is not secure</b> on all file systems, and that parts of the file may have been saved in a temporary file or in the spooler of your printer if you previously opened it in an editor or tried to print it. Only works on files (not on folders).</p></qt>")),page);
TDEListBox *lb=new TDEListBox(page);
lb->insertStringList(droppedUrls.toStringList());
if (shredConfirm->exec()==TQDialog::Accepted)
	{
	KgpgLibrary *lib=new KgpgLibrary(this);
	connect(lib,TQT_SIGNAL(systemMessage(TQString,bool)),TQT_TQOBJECT(this),TQT_SLOT(busyMessage(TQString,bool)));
	lib->shredprocessenc(droppedUrls);
	}
delete shredConfirm;
}


void  MyView::slotVerifyFile()
{
        ///////////////////////////////////   check file signature
        if (droppedUrl.isEmpty())
                return;

        TQString sigfile=TQString();
        //////////////////////////////////////       try to find detached signature.
        if (!droppedUrl.fileName().endsWith(".sig")) {
                sigfile=droppedUrl.path()+".sig";
                TQFile fsig(sigfile);
                if (!fsig.exists()) {
                        sigfile=droppedUrl.path()+".asc";
                        TQFile fsig(sigfile);
                        //////////////   if no .asc or .sig signature file included, assume the file is internally signed
                        if (!fsig.exists())
                                sigfile=TQString();
                }
        } else {
                sigfile=droppedUrl.path();
                droppedUrl=KURL(sigfile.left(sigfile.length()-4));
        }

        ///////////////////////// pipe gpg command
        KgpgInterface *verifyFileProcess=new KgpgInterface();
        verifyFileProcess->KgpgVerifyFile(droppedUrl,KURL(sigfile));
        connect (verifyFileProcess,TQT_SIGNAL(verifyquerykey(TQString)),TQT_TQOBJECT(this),TQT_SLOT(importSignature(TQString)));
}

void  MyView::importSignature(TQString ID)
{
        keyServer *kser=new keyServer(0,"server_dialog",false);
        kser->page->kLEimportid->setText(ID);
        kser->slotImport();
}

void  MyView::signDroppedFile()
{
        //////////////////////////////////////   create a detached signature for a chosen file
        if (droppedUrl.isEmpty())
                return;

        TQString signKeyID;
        //////////////////   select a private key to sign file --> listkeys.cpp
        KgpgSelKey *opts=new KgpgSelKey(0,"select_secret");
        if (opts->exec()==TQDialog::Accepted)
                signKeyID=opts->getkeyID();
        else {
                delete opts;
                return;
        }
        delete opts;
        TQStringList Options;
        if (KGpgSettings::asciiArmor())
                Options<<"--armor";
        if (KGpgSettings::pgpCompatibility())
                Options<<"--pgp6";
        KgpgInterface *signFileProcess=new KgpgInterface();
        signFileProcess->KgpgSignFile(signKeyID,droppedUrl,Options);
}

void  MyView::decryptDroppedFile()
{
        //bool isFolder=false;  // droppedUrls
        KURL swapname;

        if (!droppedUrls.first().isLocalFile()) {
                showDroppedFile();
                decryptNextFile();
        }

        TQString oldname=droppedUrls.first().fileName();
        if (oldname.endsWith(".gpg") || oldname.endsWith(".asc") || oldname.endsWith(".pgp"))
                oldname.truncate(oldname.length()-4);
        else
                oldname.append(".clear");
	/*
        if (oldname.endsWith(".tar.gz")) {
                isFolder=true;
                kgpgFolderExtract=new KTempFile(TQString(),".tar.gz");
                kgpgFolderExtract->setAutoDelete(true);
                swapname=KURL(kgpgFolderExtract->name());
                if (KMessageBox::warningContinueCancel(0,i18n("<qt>The file to decrypt is an archive. KGpg will create a temporary unencrypted archive file:<br><b>%1</b> before processing the archive extraction. This temporary file will be deleted after the decryption is finished.</qt>").arg(kgpgFolderExtract->name()),i18n("Temporary File Creation"),KStdGuiItem::cont(),"FolderTmpDecFile")==KMessageBox::Cancel)
                        return;
        } else*/ {
                swapname=KURL(droppedUrls.first().directory(0,0)+oldname);
                TQFile fgpg(swapname.path());
                if (fgpg.exists()) {
	    TDEIO::RenameDlg *over=new TDEIO::RenameDlg(0,i18n("File Already Exists"),TQString(),swapname.path(),TDEIO::M_OVERWRITE);
	    if (over->exec()==TQDialog::Rejected)
	    {
                delete over;
                decryptNextFile();
		return;
            }
	    swapname=over->newDestURL();
	    delete over;
                }
        }
        KgpgLibrary *lib=new KgpgLibrary(this);
        lib->slotFileDec(droppedUrls.first(),swapname,KGpgSettings::customDecrypt());
	connect(lib,TQT_SIGNAL(importOver(TQStringList)),TQT_TQOBJECT(this),TQT_SIGNAL(importedKeys(TQStringList)));
	connect(lib,TQT_SIGNAL(systemMessage(TQString,bool)),TQT_TQOBJECT(this),TQT_SLOT(busyMessage(TQString,bool)));
//        if (isFolder)
        connect(lib,TQT_SIGNAL(decryptionOver()),TQT_TQOBJECT(this),TQT_SLOT(decryptNextFile()));

}

void  MyView::decryptNextFile()
{
if (droppedUrls.count()>1)
{
droppedUrls.pop_front();
decryptDroppedFile();
}
}

void  MyView::unArchive()
{
        KTar compressedFolder(kgpgFolderExtract->name(),"application/x-gzip");
        if (!compressedFolder.open( IO_ReadOnly )) {
                KMessageBox::sorry(0,i18n("Unable to read temporary archive file"));
                return;
        }
        const KArchiveDirectory *archiveDirectory=compressedFolder.directory();
        //KURL savePath=KURL::getURL(droppedUrl,this,i18n(""));
        KURLRequesterDlg *savePath=new KURLRequesterDlg(droppedUrl.directory(false),i18n("Extract to: "),0,"extract");
        savePath->fileDialog()->setMode(KFile::Directory);
        if (!savePath->exec()==TQDialog::Accepted) {
                delete kgpgFolderExtract;
                return;
        }
        archiveDirectory->KArchiveDirectory::copyTo(savePath->selectedURL().path());
        compressedFolder.close();
        delete savePath;
        delete kgpgFolderExtract;
}


void  MyView::showDroppedFile()
{
kdDebug(2100)<<"------Show dropped file"<<endl;
        KgpgApp *kgpgtxtedit = new KgpgApp(0, "editor",WDestructiveClose,goDefaultKey);
        kgpgtxtedit->view->editor->slotDroppedFile(droppedUrls.first());
	connect(kgpgtxtedit,TQT_SIGNAL(encryptFiles(KURL::List)),TQT_TQOBJECT(this),TQT_SLOT(encryptFiles(KURL::List)));
	connect(this,TQT_SIGNAL(setFont(TQFont)),kgpgtxtedit,TQT_SLOT(slotSetFont(TQFont)));
	connect(kgpgtxtedit,TQT_SIGNAL(refreshImported(TQStringList)),TQT_TQOBJECT(this),TQT_SIGNAL(importedKeys(TQStringList)));
	connect(kgpgtxtedit->view->editor,TQT_SIGNAL(refreshImported(TQStringList)),TQT_TQOBJECT(this),TQT_SIGNAL(importedKeys(TQStringList)));
        kgpgtxtedit->show();
}


void  MyView::droppedfile (KURL::List url)
{
        droppedUrls=url;
        droppedUrl=url.first();
        if (KMimeType::findByURL(droppedUrl)->name()=="inode/directory") {
		encryptDroppedFolder();
                //KMessageBox::sorry(0,i18n("Sorry, only file operations are currently supported."));
                return;
        }
        if (!droppedUrl.isLocalFile()) {
                showDroppedFile();
                return;
        }

        if ((droppedUrl.path().endsWith(".asc")) || (droppedUrl.path().endsWith(".pgp")) || (droppedUrl.path().endsWith(".gpg"))) {
                switch (KGpgSettings::encryptedDropEvent()) {
                case KGpgSettings::EnumEncryptedDropEvent::DecryptAndSave:
                        decryptDroppedFile();
                        break;
                case KGpgSettings::EnumEncryptedDropEvent::DecryptAndOpen:
                        showDroppedFile();
                        break;
                case KGpgSettings::EnumEncryptedDropEvent::Ask:
                        droppopup->exec(TQCursor::pos ());
			kdDebug(2100)<<"Drop menu--------"<<endl;
                        break;
                }
        } else if (droppedUrl.path().endsWith(".sig")) {
                slotVerifyFile();
        } else
                switch (KGpgSettings::unencryptedDropEvent()) {
                case KGpgSettings::EnumUnencryptedDropEvent::Encrypt:
                        encryptDroppedFile();
                        break;
                case KGpgSettings::EnumUnencryptedDropEvent::Sign:
                        signDroppedFile();
                        break;
                case KGpgSettings::EnumUnencryptedDropEvent::Ask:
                        udroppopup->exec(TQCursor::pos ());
                        break;
                }
}


void  MyView::droppedtext (TQString inputText,bool allowEncrypt)
{

        if (inputText.startsWith("-----BEGIN PGP MESSAGE")) {
                KgpgApp *kgpgtxtedit = new KgpgApp(0, "editor",WDestructiveClose,goDefaultKey);
		connect(kgpgtxtedit,TQT_SIGNAL(encryptFiles(KURL::List)),TQT_TQOBJECT(this),TQT_SLOT(encryptFiles(KURL::List)));
		connect(this,TQT_SIGNAL(setFont(TQFont)),kgpgtxtedit,TQT_SLOT(slotSetFont(TQFont)));
                kgpgtxtedit->view->editor->setText(inputText);
                kgpgtxtedit->view->slotdecode();
                kgpgtxtedit->show();
                return;
        }
        if (inputText.startsWith("-----BEGIN PGP PUBLIC KEY")) {
                int result=KMessageBox::warningContinueCancel(0,i18n("<p>The dropped text is a public key.<br>Do you want to import it ?</p>"),i18n("Warning"));
                if (result==KMessageBox::Cancel)
                        return;
                else {
                        KgpgInterface *importKeyProcess=new KgpgInterface();
                        importKeyProcess->importKey(inputText);
			connect(importKeyProcess,TQT_SIGNAL(importfinished(TQStringList)),TQT_TQOBJECT(this),TQT_SIGNAL(importedKeys(TQStringList)));
                        return;
                }
        }
	        if (inputText.startsWith("-----BEGIN PGP SIGNED MESSAGE")) {
                clipSign(false);
                return;
        }
        if (allowEncrypt) clipEncrypt();
	else KMessageBox::sorry(this,i18n("No encrypted text found."));
}


void  MyView::dragEnterEvent(TQDragEnterEvent *e)
{
        e->accept (KURLDrag::canDecode(e) || TQTextDrag::canDecode (e));
}


void  MyView::dropEvent (TQDropEvent *o)
{
        KURL::List list;
        TQString text;
        if ( KURLDrag::decode( o, list ) )
                droppedfile(list);
        else if ( TQTextDrag::decode(o, text) )
		{
	        TQApplication::clipboard()->setText(text,clipboardMode);
                droppedtext(text);
		}
}


void  MyView::readOptions()
{
	clipboardMode=TQClipboard::Clipboard;
        if ( KGpgSettings::useMouseSelection() && kapp->clipboard()->supportsSelection())
                clipboardMode=TQClipboard::Selection;

	if (KGpgSettings::firstRun()) {
                firstRun();
        } else {
                TQString path = KGpgSettings::gpgConfigPath();
                if (path.isEmpty()) {
                        if (KMessageBox::questionYesNo(0,i18n("<qt>You have not set a path to your GnuPG config file.<br>This may cause some surprising results in KGpg's execution.<br>Would you like to start KGpg's Wizard to fix this problem?</qt>"),TQString(),i18n("Start Wizard"),i18n("Do Not Start"))==KMessageBox::Yes)
                                startWizard();
                } else {
                        TQStringList groups=KgpgInterface::getGpgGroupNames(path);
                        if (!groups.isEmpty())
                                KGpgSettings::setGroups(groups.join(","));
                }
        }
}


void  MyView::firstRun()
{
        KProcIO *p=new KProcIO();
        *p<<"gpg"<<"--no-tty"<<"--list-secret-keys";
        p->start(TDEProcess::Block);  ////  start gnupg so that it will create a config file
        startWizard();
}


static TQString getGpgHome()
{
    char    *env=getenv("GNUPGHOME");
    TQString gpgHome(env ? env : TQDir::homeDirPath()+"/.gnupg/");

    gpgHome.replace("//", "/");

    if(!gpgHome.endsWith("/"))
        gpgHome.append('/');

    TDEStandardDirs::makeDir(gpgHome, 0700);
    return gpgHome;
}


void  MyView::startWizard()
{
        kdDebug(2100)<<"Starting Wizard"<<endl;
        wiz=new KgpgWizard(0,"wizard");
        TQString gpgHome(getGpgHome());
        TQString confPath=gpgHome+"options";
        if (!TQFile(confPath).exists()) {
                confPath=gpgHome+"gpg.conf";
                if (!TQFile(confPath).exists()) {
                        if (KMessageBox::questionYesNo(this,i18n("<qt><b>The GnuPG configuration file was not found</b>. Please make sure you have GnuPG installed. Should KGpg try to create a config file ?</qt>"),TQString(),i18n("Create Config"),i18n("Do Not Create"))==KMessageBox::Yes) {
                                confPath=gpgHome+"options";
                                TQFile file(confPath);
                                if ( file.open( IO_WriteOnly ) ) {
                                        TQTextStream stream( &file );
                                        stream <<"# GnuPG config file created by KGpg"<< "\n";
                                        file.close();
                                }
                        } else {
                                wiz->text_optionsfound->setText(i18n("<qt><b>The GnuPG configuration file was not found</b>. Please make sure you have GnuPG installed and give the path to the config file.</qt>"));
                                confPath=TQString();
                        }
                }
        }

	int gpgVersion=KgpgInterface::getGpgVersion();
	if (gpgVersion<120) wiz->txtGpgVersion->setText(i18n("Your GnuPG version seems to be older than 1.2.0. Photo Id's and Key Groups will not work properly. Please consider upgrading GnuPG (http://gnupg.org)."));
	else wiz->txtGpgVersion->setText(TQString());

        wiz->kURLRequester1->setURL(confPath);
        /*
	wiz->kURLRequester2->setURL(TDEGlobalSettings::desktopPath());
        wiz->kURLRequester2->setMode(2);*/

        FILE *fp,*fp2;
        TQString tst,tst2,name,trustedvals="idre-";
        TQString firstKey=TQString();
        char line[300];
        bool counter=false;

        fp = popen("gpg --display-charset=utf-8 --no-tty --with-colon --list-secret-keys", "r");
        while ( fgets( line, sizeof(line), fp)) {
                tst=TQString::fromUtf8(line);
                if (tst.startsWith("sec")) {
                        name=KgpgInterface::checkForUtf8(tst.section(':',9,9));
                        if (!name.isEmpty()) {
                                fp2 = popen("gpg --display-charset=utf-8 --no-tty --with-colon --list-keys "+TQFile::encodeName(tst.section(':',4,4)), "r");
                                while ( fgets( line, sizeof(line), fp2)) {
                                        tst2=TQString::fromUtf8(line);
                                        if (tst2.startsWith("pub") && (trustedvals.find(tst2.section(':',1,1))==-1)) {
                                                counter=true;
                                                wiz->CBdefault->insertItem(tst.section(':',4,4).right(8)+": "+name);
                                                if (firstKey.isEmpty())
                                                        firstKey=tst.section(':',4,4).right(8)+": "+name;
						break;
                                        }
                                }
                                pclose(fp2);
                        }
                }
        }
        pclose(fp);
	wiz->CBdefault->setCurrentItem(firstKey);
        //connect(wiz->pushButton4,TQT_SIGNAL(clicked()),TQT_TQOBJECT(this),TQT_SLOT(slotGenKey()));
        if (!counter)
                connect(wiz->finishButton(),TQT_SIGNAL(clicked()),TQT_TQOBJECT(this),TQT_SLOT(slotGenKey()));
        else {
                wiz->textGenerate->hide();
		wiz->setTitle(wiz->page_4,i18n("Step Three: Select your Default Private Key"));
                connect(wiz->finishButton(),TQT_SIGNAL(clicked()),TQT_TQOBJECT(this),TQT_SLOT(slotSaveOptionsPath()));
        }
        connect(wiz->nextButton(),TQT_SIGNAL(clicked()),TQT_TQOBJECT(this),TQT_SLOT(slotWizardChange()));
        connect( wiz , TQT_SIGNAL( destroyed() ) , this, TQT_SLOT( slotWizardClose()));
	connect(wiz,TQT_SIGNAL(helpClicked()),TQT_TQOBJECT(this),TQT_SLOT(help()));

        wiz->setFinishEnabled(wiz->page_4,true);
        wiz->show();
}

void  MyView::slotWizardChange()
{
        TQString tst,name;
        char line[300];
        FILE *fp;

        if (wiz->indexOf(wiz->currentPage())==2) {
                TQString defaultID=KgpgInterface::getGpgSetting("default-key",wiz->kURLRequester1->url());
                if (defaultID.isEmpty())
                        return;
                fp = popen("gpg --display-charset=utf-8 --no-tty --with-colon --list-secret-keys "+TQFile::encodeName(defaultID), "r");
                while ( fgets( line, sizeof(line), fp)) {
                        tst=TQString::fromUtf8(line);
                        if (tst.startsWith("sec")) {
				name=KgpgInterface::checkForUtf8(tst.section(':',9,9));
                                wiz->CBdefault->setCurrentItem(tst.section(':',4,4).right(8)+": "+name);
                        }
                }
                pclose(fp);
        }
}

void  MyView::installShred()
{
                KURL path;
		path.setPath(TDEGlobalSettings::desktopPath());
		path.addPath("shredder.desktop");
                KDesktopFile configl2(path.path(), false);
                if (configl2.isImmutable() ==false) {
                        configl2.setGroup("Desktop Entry");
                        configl2.writeEntry("Type", "Application");
                        configl2.writeEntry("Name",i18n("Shredder"));
                        configl2.writeEntry("Icon","editshred");
                        configl2.writeEntry("Exec","kgpg -X %U");
                }
}

void  MyView::slotSaveOptionsPath()
{
tqWarning("Save wizard settings...");
        if (wiz->checkBox1->isChecked()) installShred();

        KGpgSettings::setAutoStart( wiz->checkBox2->isChecked() );

        KGpgSettings::setGpgConfigPath( wiz->kURLRequester1->url() );
        KGpgSettings::setFirstRun( false );

        TQString defaultID=wiz->CBdefault->currentText().section(':',0,0);
/*        if (!defaultID.isEmpty()) {
        	KGpgSettings::setDefaultKey(defaultID);
        }*/

        KGpgSettings::writeConfig();

        emit updateDefault("0x"+defaultID);
        if (wiz)
                delete wiz;
}

void  MyView::slotWizardClose()
{
        wiz=0L;
}

void  MyView::slotGenKey()
{
        slotSaveOptionsPath();
        emit createNewKey();
}

void  MyView::about()
{
        TDEAboutApplication dialog(kapp->aboutData());//_aboutData);
        dialog.exec();
}

void  MyView::help()
{
        kapp->invokeHelp(0,"kgpg");
}

kgpgapplet::kgpgapplet(TQWidget *parent, const char *name)
                : KSystemTray(parent,name)
{
        w=new MyView(this);
        w->show();
        TDEPopupMenu *conf_menu=contextMenu();
        KgpgEncryptClipboard = new TDEAction(i18n("&Encrypt Clipboard"), "kgpg", 0,TQT_TQOBJECT(w), TQT_SLOT(clipEncrypt()),actionCollection(),"clip_encrypt");
        KgpgDecryptClipboard = new TDEAction(i18n("&Decrypt Clipboard"), 0, 0,TQT_TQOBJECT(w), TQT_SLOT(clipDecrypt()),actionCollection(),"clip_decrypt");
	KgpgSignClipboard = new TDEAction(i18n("&Sign/Verify Clipboard"), "signature", 0,TQT_TQOBJECT(w), TQT_SLOT(clipSign()),actionCollection(),"clip_sign");
        TDEAction *KgpgOpenEditor;
	if (KGpgSettings::leftClick()==KGpgSettings::EnumLeftClick::KeyManager)
	KgpgOpenEditor = new TDEAction(i18n("&Open Editor"), "edit", 0,TQT_TQOBJECT(parent), TQT_SLOT(slotOpenEditor()),actionCollection(),"kgpg_editor");
	else
	KgpgOpenEditor = new TDEAction(i18n("&Open Key Manager"), "kgpg", 0,TQT_TQOBJECT(this), TQT_SLOT(slotOpenKeyManager()),actionCollection(),"kgpg_editor");

	TDEAction *KgpgOpenServer = new TDEAction(i18n("&Key Server Dialog"), "network", 0,TQT_TQOBJECT(this), TQT_SLOT(slotOpenServerDialog()),actionCollection(),"kgpg_server");
        TDEAction *KgpgPreferences=KStdAction::preferences(TQT_TQOBJECT(this), TQT_SLOT(showOptions()), actionCollection());

	connect (conf_menu,TQT_SIGNAL(aboutToShow()),TQT_TQOBJECT(this),TQT_SLOT(checkMenu()));

        KgpgEncryptClipboard->plug(conf_menu);
        KgpgDecryptClipboard->plug(conf_menu);
	KgpgSignClipboard->plug(conf_menu);
        KgpgOpenEditor->plug(conf_menu);
	KgpgOpenServer->plug(conf_menu);
        conf_menu->insertSeparator();
        KgpgPreferences->plug(conf_menu);
}


void kgpgapplet::checkMenu()
{
	KgpgDecryptClipboard->setEnabled(false);
	if ((kapp->clipboard()->text(w->clipboardMode).isEmpty()))
	{
		KgpgEncryptClipboard->setEnabled(false);
		KgpgSignClipboard->setEnabled(false);
	}
	else
	{
		KgpgEncryptClipboard->setEnabled(true);
		if (kapp->clipboard()->text(w->clipboardMode).stripWhiteSpace().startsWith("-----BEGIN"))
		KgpgDecryptClipboard->setEnabled(true);
		KgpgSignClipboard->setEnabled(true);
	}
}

void kgpgapplet::showOptions()
{
TQByteArray data;
if (!kapp->dcopClient()->send("kgpg", "KeyInterface", "showOptions()",data))
kdDebug(2100) <<"there was some error using DCOP."<<endl;
}

void kgpgapplet::slotOpenKeyManager()
{
TQByteArray data;
if (!kapp->dcopClient()->send("kgpg", "KeyInterface", "showKeyManager()",data))
kdDebug(2100) <<"there was some error using DCOP."<<endl;
}

void kgpgapplet::slotOpenServerDialog()
{
TQByteArray data;
if (!kapp->dcopClient()->send("kgpg", "KeyInterface", "showKeyServer()",data))
kdDebug(2100) <<"there was some error using DCOP."<<endl;
}



kgpgapplet::~kgpgapplet()
{
        delete w;
        w = 0L;
}

KgpgAppletApp::KgpgAppletApp()
                : KUniqueApplication()//, kgpg_applet( 0 )
{

        running=false;
}


KgpgAppletApp::~KgpgAppletApp()
{
        delete s_keyManager;
        s_keyManager=0L;
        delete kgpg_applet;
        kgpg_applet = 0L;
}

void KgpgAppletApp::slotHandleQuit()
{
s_keyManager->keysList2->saveLayout(TDEGlobal::config(),"KeyView");
        KGpgSettings::setPhotoProperties(s_keyManager->photoProps->currentItem());
	KGpgSettings::setShowTrust(s_keyManager->sTrust->isChecked());
	KGpgSettings::setShowExpi(s_keyManager->sExpi->isChecked());
	KGpgSettings::setShowCreat(s_keyManager->sCreat->isChecked());
	KGpgSettings::setShowSize(s_keyManager->sSize->isChecked());
        KGpgSettings::writeConfig();
	KSimpleConfig ("kgpgrc").sync();
        quit();
}

void KgpgAppletApp::wizardOver(TQString defaultKeyId)
{
        if (defaultKeyId.length()==10)
                s_keyManager->slotSetDefaultKey(defaultKeyId);
        s_keyManager->show();
        s_keyManager->raise();
}

int KgpgAppletApp::newInstance()
{
        kdDebug(2100)<<"New instance"<<endl;
        args = TDECmdLineArgs::parsedArgs();
        if (running) {
                kdDebug(2100)<<"Already running"<<endl;
                kgpg_applet->show();
        } else {
                kdDebug(2100) << "Starting KGpg"<<endl;
                running=true;

                s_keyManager=new listKeys(0, "key_manager");

		TQString gpgPath= KGpgSettings::gpgConfigPath();
		if (!gpgPath.isEmpty() && KURL(gpgPath).directory(false)!=TQDir::homeDirPath()+"/.gnupg/")
		setenv("GNUPGHOME", TQFile::encodeName(KURL::fromPathOrURL(gpgPath).directory(false)), 1);

        s_keyManager->refreshkey();

		if (KGpgSettings::leftClick()==KGpgSettings::EnumLeftClick::KeyManager)
                kgpg_applet=new kgpgapplet(s_keyManager,"kgpg_systrayapplet");
		else
		{
		kgpg_applet=new kgpgapplet(s_keyManager->s_kgpgEditor,"kgpg_systrayapplet");
		}
		connect(s_keyManager,TQT_SIGNAL(encryptFiles(KURL::List)),kgpg_applet->w,TQT_SLOT(encryptFiles(KURL::List)));
		connect(s_keyManager,TQT_SIGNAL(installShredder()),kgpg_applet->w,TQT_SLOT(installShred()));
		connect(s_keyManager->s_kgpgEditor,TQT_SIGNAL(encryptFiles(KURL::List)),kgpg_applet->w,TQT_SLOT(encryptFiles(KURL::List)));

                connect( kgpg_applet, TQT_SIGNAL(quitSelected()), this, TQT_SLOT(slotHandleQuit()));
                connect(s_keyManager,TQT_SIGNAL(readAgainOptions()),kgpg_applet->w,TQT_SLOT(readOptions()));
                connect(kgpg_applet->w,TQT_SIGNAL(updateDefault(TQString)),TQT_TQOBJECT(this),TQT_SLOT(wizardOver(TQString)));
                connect(kgpg_applet->w,TQT_SIGNAL(createNewKey()),s_keyManager,TQT_SLOT(slotgenkey()));
		connect(s_keyManager,TQT_SIGNAL(fontChanged(TQFont)),kgpg_applet->w,TQT_SIGNAL(setFont(TQFont)));
		connect(kgpg_applet->w,TQT_SIGNAL(importedKeys(TQStringList)),s_keyManager->keysList2,TQT_SLOT(slotReloadKeys(TQStringList)));
                kgpg_applet->show();


                if (!gpgPath.isEmpty()) {
                        if ((KgpgInterface::getGpgBoolSetting("use-agent",gpgPath)) && (!getenv("GPG_AGENT_INFO")))
                                KMessageBox::sorry(0,i18n("<qt>The use of <b>GnuPG Agent</b> is enabled in GnuPG's configuration file (%1).<br>"
                                                          "However, the agent does not seem to be running. This could result in problems with signing/decryption.<br>"
                                                          "Please disable GnuPG Agent from KGpg settings, or fix the agent.</qt>").arg(gpgPath));
                }

        }
	goHome=s_keyManager->actionCollection()->action("go_default_key")->shortcut();
	kgpg_applet->w->goDefaultKey=goHome;

        ////////////////////////   parsing of command line args
        if (args->isSet("k")!=0) {
                s_keyManager->show();
                KWin::setOnDesktop( s_keyManager->winId() , KWin::currentDesktop() );  //set on the current desktop
                KWin::deIconifyWindow( s_keyManager->winId());  //de-iconify window
                s_keyManager->raise();  // set on top
        } else
                if (args->count()>0) {
                        kdDebug(2100) << "KGpg: found files"<<endl;

                        urlList.clear();

                        for (int ct=0;ct<args->count();ct++)
                                urlList.append(args->url(ct));

                        if (urlList.empty())
                                return 0;

                        kgpg_applet->w->droppedUrl=urlList.first();

                        bool directoryInside=false;
                        TQStringList lst=urlList.toStringList();
                        for ( TQStringList::Iterator it = lst.begin(); it != lst.end(); ++it ) {
                                if (KMimeType::findByURL(KURL( *it ))->name()=="inode/directory")
                                        directoryInside=true;
                        }

                        if ((directoryInside) && (lst.count()>1)) {
                                KMessageBox::sorry(0,i18n("Unable to perform requested operation.\nPlease select only one folder, or several files, but do not mix files and folders."));
                                return 0;
                        }

                        kgpg_applet->w->droppedUrls=urlList;

                        if (args->isSet("e")!=0) {
                                if (!directoryInside)
                                        kgpg_applet->w->encryptDroppedFile();
                                else
                                        kgpg_applet->w->encryptDroppedFolder();
                        } else if (args->isSet("X")!=0) {
                                if (!directoryInside)
                                        kgpg_applet->w->shredDroppedFile();
                                else
                                        KMessageBox::sorry(0,i18n("Cannot shred folder."));
                        } else if (args->isSet("s")!=0) {
                                if (!directoryInside)
                                        kgpg_applet->w->showDroppedFile();
                                else
                                        KMessageBox::sorry(0,i18n("Cannot decrypt and show folder."));
                        } else if (args->isSet("S")!=0) {
                                if (!directoryInside)
                                        kgpg_applet->w->signDroppedFile();
                                else
                                        KMessageBox::sorry(0,i18n("Cannot sign folder."));
                        } else if (args->isSet("V")!=0) {
                                if (!directoryInside)
                                        kgpg_applet->w->slotVerifyFile();
                                else
                                        KMessageBox::sorry(0,i18n("Cannot verify folder."));
                        } else if (kgpg_applet->w->droppedUrl.fileName().endsWith(".sig"))
                                kgpg_applet->w->slotVerifyFile();
                        else
                                kgpg_applet->w->decryptDroppedFile();
                }
        return 0;
}




//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void MyView::encryptClipboard(TQStringList selec,TQStringList encryptOptions,bool,bool symmetric)
{
        if (kapp->clipboard()->text(clipboardMode).isEmpty()) {
	KPassivePopup::message(i18n("Clipboard is empty."),TQString(),TDEGlobal::iconLoader()->loadIcon("kgpg",TDEIcon::Desktop),this);
	return;
	}
                if (KGpgSettings::pgpCompatibility())
                        encryptOptions<<"--pgp6";
                encryptOptions<<"--armor";

		if (symmetric) selec.clear();
                KgpgInterface *txtEncrypt=new KgpgInterface();
                connect (txtEncrypt,TQT_SIGNAL(txtencryptionfinished(TQString)),TQT_TQOBJECT(this),TQT_SLOT(slotSetClip(TQString)));
		connect (txtEncrypt,TQT_SIGNAL(txtencryptionstarted()),TQT_TQOBJECT(this),TQT_SLOT(slotPassiveClip()));
                txtEncrypt->KgpgEncryptText(kapp->clipboard()->text(clipboardMode),selec,encryptOptions);
}

void MyView::slotPassiveClip()
{
TQString newtxt=kapp->clipboard()->text(clipboardMode);
if (newtxt.length()>300)
                        newtxt=TQString(newtxt.left(250).stripWhiteSpace())+"...\n"+TQString(newtxt.right(40).stripWhiteSpace());

                newtxt.replace(TQRegExp("<"),"&lt;");   /////   disable html tags
                newtxt.replace(TQRegExp("\n"),"<br>");

pop = new KPassivePopup( this);
                pop->setView(i18n("Encrypted following text:"),newtxt,TDEGlobal::iconLoader()->loadIcon("kgpg",TDEIcon::Desktop));
                pop->setTimeout(3200);
                pop->show();
                TQRect qRect(TQApplication::desktop()->screenGeometry());
                int iXpos=qRect.width()/2-pop->width()/2;
                int iYpos=qRect.height()/2-pop->height()/2;
                pop->move(iXpos,iYpos);
}

void MyView::slotSetClip(TQString newtxt)
{
        if (newtxt.isEmpty()) return;
                TQApplication::clipboard()->setText(newtxt,clipboardMode);//,TQClipboard::Clipboard);    QT 3.1 only
}



/////////////////////////////////////////////////////////////////////////////

#include "kgpg.moc"


