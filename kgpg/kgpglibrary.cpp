/***************************************************************************
                          kgpglibrary.cpp  -  description
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

#include <tqhbox.h>
#include <tqvbox.h>

#include <klocale.h>
#include <kapplication.h>
#include <tdeconfig.h>
#include <kmessagebox.h>
#include <krun.h>
#include <tqfile.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include "kgpglibrary.h"
#include "popuppublic.h"
#include "kgpginterface.h"
#include <tdeio/renamedlg.h>

KgpgLibrary::KgpgLibrary(TQWidget *parent, bool pgpExtension)
{
        if (pgpExtension)
                extension=".pgp";
        else
                extension=".gpg";
	popIsActive=false;
	panel=parent;
}

KgpgLibrary::~KgpgLibrary()
{}


void KgpgLibrary::slotFileEnc(KURL::List urls,TQStringList opts,TQStringList defaultKey,KShortcut goDefaultKey)
{
        /////////////////////////////////////////////////////////////////////////  encode file file
        if (!urls.empty()) {
                urlselecteds=urls;
                if (defaultKey.isEmpty()) {
			TQString fileNames=urls.first().fileName();
			if (urls.count()>1) fileNames+=",...";
                        popupPublic *dialogue=new popupPublic(0,"Public keys",fileNames,true,goDefaultKey);
                        connect(dialogue,TQT_SIGNAL(selectedKey(TQStringList,TQStringList,bool,bool)),this,TQT_SLOT(startencode(TQStringList,TQStringList,bool,bool)));
                        dialogue->exec();
                        delete dialogue;
                } else
                        startencode(defaultKey,opts,false,false);
        }
}

void KgpgLibrary::startencode(TQStringList encryptKeys,TQStringList encryptOptions,bool shred,bool symetric)
{
	popIsActive=false;
        //KURL::List::iterator it;
	//filesToEncode=urlselecteds.count();
	_encryptKeys=encryptKeys;
	_encryptOptions=encryptOptions;
	_shred=shred;
	_symetric=symetric;
		fastencode(urlselecteds.first(),encryptKeys,encryptOptions,symetric);
}


void KgpgLibrary::fastencode(KURL &fileToCrypt,TQStringList selec,TQStringList encryptOptions,bool symetric)
{
        //////////////////              encode from file
        if ((selec.isEmpty()) && (!symetric)) {
                KMessageBox::sorry(0,i18n("You have not chosen an encryption key."));
                return;
        }
        urlselected=fileToCrypt;
        KURL dest;
        if (encryptOptions.find("--armor")!=encryptOptions.end())
                dest.setPath(urlselected.path()+".asc");
        else
                dest.setPath(urlselected.path()+extension);

        TQFile fgpg(dest.path());

        if (fgpg.exists()) {
			TDEIO::RenameDlg *over=new TDEIO::RenameDlg(0,i18n("File Already Exists"),TQString(),dest.path(),TDEIO::M_OVERWRITE);
		    	if (over->exec()==TQDialog::Rejected)
	    		{
                	delete over;
			emit systemMessage(TQString(),true);
                	return;
            		}
	    		dest=over->newDestURL();
	    		delete over;
        }
	int filesToEncode=urlselecteds.count();
	if (filesToEncode>1)
	emit systemMessage(i18n("<b>%1 Files left.</b>\nEncrypting </b>%2").arg(filesToEncode).arg(urlselecteds.first().path()));
	else emit systemMessage(i18n("<b>Encrypting </b>%2").arg(urlselecteds.first().path()));
        KgpgInterface *cryptFileProcess=new KgpgInterface();
	pop = new KPassivePopup(panel);
        cryptFileProcess->KgpgEncryptFile(selec,urlselected,dest,encryptOptions,symetric);
         if (!popIsActive) 
	{
	//connect(cryptFileProcess,TQT_SIGNAL(processstarted(TQString)),this,TQT_SLOT(processpopup2(TQString)));
	popIsActive=true;	
	}
	connect(cryptFileProcess,TQT_SIGNAL(encryptionfinished(KURL)),this,TQT_SLOT(processenc(KURL)));
        connect(cryptFileProcess,TQT_SIGNAL(errormessage(TQString)),this,TQT_SLOT(processencerror(TQString)));
}

void KgpgLibrary::processpopup2(TQString fileName)
{
        
	//pop->setTimeout(0);
        pop->setView(i18n("Processing encryption (%1)").arg(fileName),i18n("Please wait..."),TDEGlobal::iconLoader()->loadIcon("kgpg",KIcon::Desktop));
        pop->show();
        /*TQRect qRect(TQApplication::desktop()->screenGeometry());
        int iXpos=qRect.width()/2-pop->width()/2;
        int iYpos=qRect.height()/2-pop->height()/2;
        pop->move(iXpos,iYpos);*/

}

void KgpgLibrary::shredpreprocessenc(KURL fileToShred)
{
	popIsActive=false;
	emit systemMessage(TQString());
	shredprocessenc(fileToShred);
}

void KgpgLibrary::shredprocessenc(KURL::List filesToShred)
{
emit systemMessage(i18n("Shredding %n file","Shredding %n files",filesToShred.count()));

TDEIO::Job *job;
job = TDEIO::del( filesToShred, true );
connect( job, TQT_SIGNAL( result( TDEIO::Job * ) ),TQT_SLOT( slotShredResult( TDEIO::Job * ) ) );	
}

void KgpgLibrary::slotShredResult( TDEIO::Job * job )
{
    emit systemMessage(TQString());
    if (job && job->error())
    {
    job->showErrorDialog( (TQWidget*)parent() );
    emit systemMessage(TQString(),true);
    KPassivePopup::message(i18n("KGpg Error"),i18n("Process halted, not all files were shredded."),TDEGlobal::iconLoader()->loadIcon("kgpg",KIcon::Desktop),panel,"kgpg_error",0);
    }
}


void KgpgLibrary::processenc(KURL)
{
	emit systemMessage(TQString());
	if (_shred) shredprocessenc(urlselecteds.first());
	urlselecteds.pop_front ();
	if (urlselecteds.count()>0)
	fastencode(urlselecteds.first(),_encryptKeys,_encryptOptions,_symetric);
}

void KgpgLibrary::processencerror(TQString mssge)
{
	popIsActive=false;
	emit systemMessage(TQString(),true);
	KMessageBox::detailedSorry(panel,i18n("<b>Process halted</b>.<br>Not all files were encrypted."),mssge);
}



void KgpgLibrary::slotFileDec(KURL srcUrl,KURL destUrl,TQStringList customDecryptOption)
{
        //////////////////////////////////////////////////////////////////    decode file from konqueror or menu
        KgpgInterface *decryptFileProcess=new KgpgInterface();
        pop = new KPassivePopup();
	urlselected=srcUrl;
        decryptFileProcess->KgpgDecryptFile(srcUrl,destUrl,customDecryptOption);
        connect(decryptFileProcess,TQT_SIGNAL(processaborted(bool)),this,TQT_SLOT(processdecover()));
        connect(decryptFileProcess,TQT_SIGNAL(processstarted(TQString)),this,TQT_SLOT(processpopup(TQString)));
        connect(decryptFileProcess,TQT_SIGNAL(decryptionfinished()),this,TQT_SLOT(processdecover()));
        connect(decryptFileProcess,TQT_SIGNAL(errormessage(TQString)),this,TQT_SLOT(processdecerror(TQString)));
}

void KgpgLibrary::processpopup(TQString fileName)
{
	emit systemMessage(i18n("Decrypting %1").arg(fileName));
	pop->setTimeout(0);
        pop->setView(i18n("Processing decryption"),i18n("Please wait..."),TDEGlobal::iconLoader()->loadIcon("kgpg",KIcon::Desktop));
        pop->show();
        TQRect qRect(TQApplication::desktop()->screenGeometry());
        int iXpos=qRect.width()/2-pop->width()/2;
        int iYpos=qRect.height()/2-pop->height()/2;
        pop->move(iXpos,iYpos);
}

void KgpgLibrary::processdecover()
{
	emit systemMessage(TQString());
	delete pop;
        emit decryptionOver();
}


void KgpgLibrary::processdecerror(TQString mssge)
{
	delete pop;
	emit systemMessage(TQString());
        ///// test if file is a public key
        TQFile qfile(TQFile::encodeName(urlselected.path()));
        if (qfile.open(IO_ReadOnly)) {
                TQTextStream t( &qfile );
                TQString result(t.read());
                qfile.close();
                //////////////     if  pgp data found, decode it
                if (result.startsWith("-----BEGIN PGP PUBLIC KEY BLOCK")) {//////  dropped file is a public key, ask for import
                        int result=KMessageBox::warningContinueCancel(0,i18n("<p>The file <b>%1</b> is a public key.<br>Do you want to import it ?</p>").arg(urlselected.path()),i18n("Warning"));
                        if (result==KMessageBox::Cancel)
                                return;
                        else {
                                KgpgInterface *importKeyProcess=new KgpgInterface();
                                importKeyProcess->importKeyURL(urlselected);
				connect(importKeyProcess,TQT_SIGNAL(importfinished(TQStringList)),this,TQT_SIGNAL(importOver(TQStringList)));
                                return;
                        }
                } else if (result.startsWith("-----BEGIN PGP PRIVATE KEY BLOCK")) {//////  dropped file is a public key, ask for import
                        qfile.close();
                        KMessageBox::information(0,i18n("<p>The file <b>%1</b> is a private key block. Please use KGpg key manager to import it.</p>").arg(urlselected.path()));
                        return;
                }
        }
        KMessageBox::detailedSorry(0,i18n("Decryption failed."),mssge);
}



#include "kgpglibrary.moc"
