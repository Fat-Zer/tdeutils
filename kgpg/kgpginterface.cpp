/***************************************************************************
                          kgpginterface.cpp  -  description
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

#include <stdio.h>

#include <tqdialog.h>
#include <tqclipboard.h>
#include <tqlayout.h>
#include <tqregexp.h>
#include <tqstring.h>
#include <tqlabel.h>
#include <tqapplication.h>
#include <kio/netaccess.h>
#include <tqcheckbox.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <kpassdlg.h>
#include <kmdcodec.h>
#include <klineedit.h>
#include <kcharsets.h>
#include <kpassivepopup.h>
#include <kiconloader.h>
#include <kaction.h>
#include <tqtextcodec.h>
#include <kprocess.h>
#include <kprocio.h>
#include <kconfig.h>
#include <tqfile.h>
#include <kled.h>
#include <kdebug.h>
#include <ktempfile.h>

#include "kgpginterface.h"
#include "listkeys.h"
#include "detailedconsole.h"

KgpgInterface::KgpgInterface()
{}


int KgpgInterface::getGpgVersion()
{
FILE *fp;
        TQString readResult,gpgString;
        char buffer[200];
	bool readLine=true;

        TQString gpgcmd="gpg --version";

        fp = popen(TQFile::encodeName(gpgcmd), "r");
        while ( fgets( buffer, sizeof(buffer), fp)) {
                readResult=buffer;
                if (readLine) {
		gpgString=readResult.stripWhiteSpace().section(' ',-1);
		readLine=false;
		}
        }
        pclose(fp);
	return (100*gpgString.section('.',0,0).toInt()+10*gpgString.section('.',1,1).toInt()+gpgString.section('.',2,2).toInt());
}

void KgpgInterface::updateIDs(TQString txtString)
{
 int cut=txtString.find(' ',22,false);
          txtString.remove(0,cut);
          if (txtString.find("(",0,false)!=-1)
            txtString=txtString.section('(',0,0)+txtString.section(')',-1);
	    txtString.replace(TQRegExp("<"),"&lt;");
          if (userIDs.find(txtString)==-1)
            {
              if (!userIDs.isEmpty())
                userIDs+=i18n(" or ");
              userIDs+=txtString;
            }
}

void KgpgInterface::KgpgEncryptFile(TQStringList encryptKeys,KURL srcUrl,KURL destUrl, TQStringList Options, bool symetrical)
{
        sourceFile=srcUrl;
        message=TQString();

        KProcIO *proc=new KProcIO(TQTextCodec::codecForLocale());
	*proc<<"gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--status-fd=2"<<"--command-fd=0"<<"--utf8-strings";
	for ( TQStringList::Iterator it = Options.begin(); it != Options.end(); ++it )
       		if (!TQFile::encodeName(*it).isEmpty()) *proc<< TQString(TQFile::encodeName(*it));

                *proc<<"--output"<<TQString(TQFile::encodeName(destUrl.path()));

		if (!symetrical) {
		*proc<<"-e";
		for ( TQStringList::Iterator it = encryptKeys.begin(); it != encryptKeys.end(); ++it )
       		*proc<<"--recipient"<< *it;
        	} else  ////////////   symetrical encryption, prompt for password
                *proc<<"-c";

		*proc<<TQString(TQFile::encodeName(srcUrl.path()));

        /////////  when process ends, update dialog infos
        TQObject::connect(proc, TQT_SIGNAL(processExited(KProcess *)),this,TQT_SLOT(encryptfin(KProcess *)));
        TQObject::connect(proc,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(readencprocess(KProcIO *)));
        proc->start(KProcess::NotifyOnExit,true);
}


KgpgInterface::~KgpgInterface()
{}


void KgpgInterface::encryptfin(KProcess *)
{
        if (message.find("END_ENCRYPTION")!=-1)
                emit encryptionfinished(sourceFile);
        else {
                emit errormessage(message);
        }
}

void KgpgInterface::readencprocess(KProcIO *p)
{
        TQString required;
        while (p->readln(required,true)!=-1) {
                if (required.find("BEGIN_ENCRYPTION",0,false)!=-1)
                        emit processstarted(sourceFile.path());
                if (required.find("GET_")!=-1) {
                        if (required.find("openfile.overwrite.okay")!=-1)
                                p->writeStdin(TQString("Yes"));
                        else if ((required.find("passphrase.enter")!=-1)) {
                                TQCString passphrase;
                                int code=KPasswordDialog::getNewPassword(passphrase,i18n("Enter passphrase for your file (symmetrical encryption):"));
                                if (code!=TQDialog::Accepted) {
                                        p->deleteLater();
                                        emit processaborted(true);
                                        return;
                                }
                                p->writeStdin(passphrase,true);
                        } else {
                                p->writeStdin(TQString("quit"));
                                p->closeWhenDone();
                        }
                }
                message+=required+"\n";
        }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////   File decryption

void KgpgInterface::KgpgDecryptFile(KURL srcUrl,KURL destUrl,TQStringList Options)
{
        message=TQString();
        step=3;
	decryptUrl=srcUrl.path();
        userIDs=TQString();
        anonymous=false;

        KProcIO *proc=new KProcIO(TQTextCodec::codecForLocale());

                *proc<<"gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--status-fd=2"<<"--command-fd=0"<<"--utf8-strings";

		for ( TQStringList::Iterator it = Options.begin(); it != Options.end(); ++it )
       		if (!TQFile::encodeName(*it).isEmpty()) *proc<< TQString(TQFile::encodeName(*it));

		if (!destUrl.fileName().isEmpty()) // a filename was entered
				*proc<<"-o"<<TQString(TQFile::encodeName(destUrl.path()));

                *proc<<"-d"<<TQString(TQFile::encodeName(srcUrl.path()));

        TQObject::connect(proc, TQT_SIGNAL(processExited(KProcess *)),this,TQT_SLOT(decryptfin(KProcess *)));
        TQObject::connect(proc,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(readdecprocess(KProcIO *)));
        proc->start(KProcess::NotifyOnExit,true);
}

void KgpgInterface::decryptfin(KProcess *)
{
        if ((message.find("DECRYPTION_OKAY")!=-1) && (message.find("END_DECRYPTION")!=-1)) //&& (message.find("GOODMDC")!=-1)
                emit decryptionfinished();
        else
	emit errormessage(message);
}


void KgpgInterface::readdecprocess(KProcIO *p)
{
        TQString required;
        while (p->readln(required,true)!=-1) {
                if (required.find("BEGIN_DECRYPTION",0,false)!=-1)
                        emit processstarted(decryptUrl);
                if (required.find("USERID_HINT",0,false)!=-1)
			updateIDs(required);

                if (required.find("ENC_TO")!=-1) {
                        if (required.find("0000000000000000")!=-1)
                                anonymous=true;
                }
                if (required.find("GET_")!=-1) {
                        if (required.find("openfile.overwrite.okay")!=-1)
                                p->writeStdin(TQString("Yes"));
                        else if ((required.find("passphrase.enter")!=-1)) {
                                if (userIDs.isEmpty())
                                        userIDs=i18n("[No user id found]");
                                userIDs.replace(TQRegExp("<"),"&lt;");
                                TQCString passphrase;
                                TQString passdlgmessage;
                                if (anonymous)
                                        passdlgmessage=i18n("<b>No user id found</b>. Trying all secret keys.<br>");
                                if ((step<3) && (!anonymous))
                                        passdlgmessage=i18n("<b>Bad passphrase</b>. You have %1 tries left.<br>").tqarg(step);

                                passdlgmessage+=i18n("Enter passphrase for <b>%1</b>").tqarg(userIDs);
                                int code=KPasswordDialog::getPassword(passphrase,passdlgmessage);
                                if (code!=TQDialog::Accepted) {
                                        p->deleteLater();
                                        emit processaborted(true);
                                        return;
                                }
                                p->writeStdin(passphrase,true);
                                userIDs=TQString();
                                if (step>1) step--;
				else step=3;
                        } else {
                                p->writeStdin(TQString("quit"));
                                p->closeWhenDone();
                        }
                }
                message+=required+"\n";
        }
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////    Text encryption


void KgpgInterface::KgpgEncryptText(TQString text,TQStringList userIDs, TQStringList Options)
{
        message=TQString();
	//TQTextCodec *codec = KGlobal::charsets()->codecForName(KGlobal::locale()->encoding());
	TQTextCodec *codec =TQTextCodec::codecForLocale ();
	if (codec->canEncode(text)) txtToEncrypt=text;
	else txtToEncrypt=text.utf8();

        KProcIO *proc=new KProcIO(TQTextCodec::codecForLocale());
        *proc<<"gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--command-fd=0"<<"--status-fd=1"<<"--utf8-strings";

	for ( TQStringList::Iterator it = Options.begin(); it != Options.end(); ++it )
       		if (!TQFile::encodeName(*it).isEmpty()) *proc<< TQString(TQFile::encodeName(*it));

	if (!userIDs.isEmpty())
	{
        *proc<<"-e";
	for ( TQStringList::Iterator it = userIDs.begin(); it != userIDs.end(); ++it )
       		*proc<<"--recipient"<< *it;
	}
	else
	  *proc<<"-c";

        /////////  when process ends, update dialog infos

        TQObject::connect(proc, TQT_SIGNAL(processExited(KProcess *)),this,TQT_SLOT(txtencryptfin(KProcess *)));
        TQObject::connect(proc,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(txtreadencprocess(KProcIO *)));
        proc->start(KProcess::NotifyOnExit,false);
	emit txtencryptionstarted();
}


void KgpgInterface::txtencryptfin(KProcess *)
{
        if (!message.isEmpty())
                emit txtencryptionfinished(message);
        else
                emit txtencryptionfinished(TQString());
}

void KgpgInterface::txtreadencprocess(KProcIO *p)
{
        TQString required;
        while (p->readln(required,true)!=-1) {
	  if (required.find("BEGIN_ENCRYPTION",0,false)!=-1)
	  {
	    p->writeStdin(txtToEncrypt,false);
	    txtToEncrypt=TQString();
	    p->closeWhenDone();
	  }
	  else
	if ((required.find("passphrase.enter")!=-1))
            {
              TQCString passphrase;
              TQString passdlgmessage=i18n("Enter passphrase (symmetrical encryption)");
              int code=KPasswordDialog::getNewPassword(passphrase,passdlgmessage);
	      if (code!=TQDialog::Accepted)
                {
                  p->deleteLater();
                  return;
                }
              p->writeStdin(passphrase,true);
            }
	    else
		if (!required.startsWith("[GNUPG:]")) message+=required+"\n";
        }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////     Text decryption

void KgpgInterface::KgpgDecryptText(TQString text,TQStringList Options)
{
  gpgOutput=TQString();
  log=TQString();

  message=TQString();
  userIDs=TQString();
  step=3;
  anonymous=false;
  decfinished=false;
  decok=false;
  badmdc=false;
  KProcess *proc=new KProcess();
  *proc<<"gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--command-fd=0"<<"--status-fd=2"<<"--no-batch"<<"--utf8-strings";
  for ( TQStringList::Iterator it = Options.begin(); it != Options.end(); ++it )
	  if (!TQFile::encodeName(*it).isEmpty()) *proc<< TQString(TQFile::encodeName(*it));
  *proc<<"-d";

  /////////  when process ends, update dialog infos

  TQObject::connect(proc, TQT_SIGNAL(processExited(KProcess *)),this,TQT_SLOT(txtdecryptfin(KProcess *)));
  connect(proc, TQT_SIGNAL(receivedStdout(KProcess *, char *, int)),this, TQT_SLOT(getOutput(KProcess *, char *, int)));
  connect(proc, TQT_SIGNAL(receivedStderr(KProcess *, char *, int)),this, TQT_SLOT(getCmdOutput(KProcess *, char *, int)));
  proc->start(KProcess::NotifyOnExit,KProcess::All);
  proc->writeStdin(text.utf8(), text.length());
}

void KgpgInterface::txtdecryptfin(KProcess *)
{
if ((decok) && (!badmdc))
emit txtdecryptionfinished(message);

else if (badmdc)
{
KMessageBox::sorry(0,i18n("Bad MDC detected. The encrypted text has been manipulated."));
emit txtdecryptionfailed(log);
}
else
emit txtdecryptionfailed(log);
}


void KgpgInterface::getOutput(KProcess *, char *data, int )
{
	message.append(TQString::fromUtf8(data));
}


void KgpgInterface::getCmdOutput(KProcess *p, char *data, int )
{
  gpgOutput.append(TQString::fromUtf8(data));
  log.append(data);

  int pos;
  while ((pos=gpgOutput.find("\n"))!=-1)
  {
	TQString required=gpgOutput.left(pos);
	gpgOutput.remove(0,pos+2);

	if (required.find("USERID_HINT",0,false)!=-1)
		updateIDs(required);

	if (required.find("ENC_TO")!=-1)
	{
		if (required.find("0000000000000000")!=-1)
			anonymous=true;
	}

	if (required.find("GET_")!=-1)
	{
		if ((required.find("passphrase.enter")!=-1))
		{
			if (userIDs.isEmpty())
			userIDs=i18n("[No user id found]");
			TQCString passphrase;
			TQString passdlgmessage;
			if (anonymous)
				passdlgmessage=i18n("<b>No user id found</b>. Trying all secret keys.<br>");
			if ((step<3) && (!anonymous))
				passdlgmessage=i18n("<b>Bad passphrase</b>. You have %1 tries left.<br>").tqarg(step);
			passdlgmessage+=i18n("Enter passphrase for <b>%1</b>").tqarg(userIDs);
			int code=KPasswordDialog::getPassword(passphrase,passdlgmessage);
			if (code!=TQDialog::Accepted)
			{
				p->deleteLater();
				emit processaborted(true);
				return;
			}
			passphrase.append("\n");
			p->writeStdin(passphrase,passphrase.length());
			userIDs=TQString();
			if (step>1) step--;
			else step=3;
		}
		else
		{
			p->writeStdin("quit",4);
			p->closeStdin();
		}
	}

	if (required.find("BEGIN_DECRYPTION")!=-1)
	{
		p->closeStdin();
		required=TQString();
	}

	if (required.find("END_DECRYPTION")!=-1) decfinished=true;
	if (required.find("DECRYPTION_OKAY")!=-1) decok=true;
	if (required.find("DECRYPTION_FAILED")!=-1) decok=false;
	if (required.find("BADMDC")!=-1) badmdc=true;
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////    Text signing


void KgpgInterface::KgpgSignText(TQString text,TQString userIDs, TQStringList Options)
{
        message=TQString();
	step=4;
	TQString txtprocess;
	TQTextCodec *codec =TQTextCodec::codecForLocale ();
	if (codec->canEncode(text)) txtprocess=text;
	else txtprocess=text.utf8();

        KProcIO *proc=new KProcIO(TQTextCodec::codecForLocale());
        *proc<<"gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--command-fd=0"<<"--status-fd=1"<<"--utf8-strings";

	for ( TQStringList::Iterator it = Options.begin(); it != Options.end(); ++it )
		if (!TQFile::encodeName(*it).isEmpty()) *proc<< TQString(TQFile::encodeName(*it));
        *proc<<"--clearsign"<<"-u"<<userIDs;

        /////////  when process ends, update dialog infos

        TQObject::connect(proc, TQT_SIGNAL(processExited(KProcess *)),this,TQT_SLOT(txtsignfin(KProcess *)));
        TQObject::connect(proc,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(txtsignprocess(KProcIO *)));

	//emit txtsigningstarted();

	proc->start(KProcess::NotifyOnExit,false);
	/*if (useAgent)
	{
	kdDebug(2100)<<"Using Agent+++++++++++++"<<endl;
	//KMessageBox::sorry(0,"using agent");
	proc->writeStdin(txtprocess,true);
	proc->closeWhenDone();
	}
	else*/
	message=txtprocess;
}


void KgpgInterface::txtsignfin(KProcess *)
{
        if (!message.isEmpty())
                emit txtSignOver(message);
        else
                emit txtSignOver(TQString());
}

void KgpgInterface::txtsignprocess(KProcIO *p)
{
        TQString required;
        while (p->readln(required,true)!=-1) {
//	kdDebug(2100)<<"SIGNING: "<<required<<endl;

	if (required.find("USERID_HINT",0,false)!=-1)
        updateIDs(required);

	if (required.find("GOOD_PASSPHRASE")!=-1)
	{
	p->writeStdin(message,true);
	message=TQString();
	p->closeWhenDone();
	}

	if ((required.find("passphrase.enter")!=-1))
            {
	      if (step>1) step--;
	      else step=3;
              if (userIDs.isEmpty())
                userIDs=i18n("[No user id found]");
              TQCString passphrase;
              TQString passdlgmessage;
              if (step<3)
              passdlgmessage=i18n("<b>Bad passphrase</b>. You have %1 tries left.<br>").tqarg(step);
              passdlgmessage+=i18n("Enter passphrase for <b>%1</b>").tqarg(userIDs);
              int code=KPasswordDialog::getPassword(passphrase,passdlgmessage);
	      if (code!=TQDialog::Accepted)
                {
                  p->deleteLater();
                  return;
                }
              p->writeStdin(passphrase,true);
            }
	    else
		if (!required.startsWith("[GNUPG:]")) message+=required+"\n";
        }
}


////////////////////////////////////////////////   decrypt file to text

void KgpgInterface::KgpgDecryptFileToText(KURL srcUrl,TQStringList Options)
{

  message=TQString();
  userIDs=TQString();
  step=3;
  anonymous=false;
decfinished=false;
decok=false;
badmdc=false;

  KProcess *proc=new KProcess();
  *proc<<"gpg"<<"--no-tty"<<"--utf8-strings"<<"--no-secmem-warning"<<"--command-fd=0"<<"--status-fd=2"<<"--no-batch"<<"-o"<<"-";
      	for ( TQStringList::Iterator it = Options.begin(); it != Options.end(); ++it ) {
       		if (!TQFile::encodeName(*it).isEmpty()) *proc<< TQString(TQFile::encodeName(*it));
    		}
      *proc<<"-d"<<TQString(TQFile::encodeName(srcUrl.path()));

  /////////  when process ends, update dialog infos

  connect(proc, TQT_SIGNAL(processExited(KProcess *)),this,TQT_SLOT(txtdecryptfin(KProcess *)));
  connect(proc, TQT_SIGNAL(receivedStdout(KProcess *, char *, int)),this, TQT_SLOT(getOutput(KProcess *, char *, int)));
  connect(proc, TQT_SIGNAL(receivedStderr(KProcess *, char *, int)),this, TQT_SLOT(getCmdOutput(KProcess *, char *, int)));
  proc->start(KProcess::NotifyOnExit,KProcess::All);
}


///////////////////////////////////////////////////////          verify text


void KgpgInterface::KgpgVerifyText(TQString text)
{

		TQTextCodec *codec =TQTextCodec::codecForLocale ();
		if (!codec->canEncode(text)) text=text.utf8();
		signmiss=false;
		signID=TQString();
		message=TQString();
		 KProcIO *verifyproc=new KProcIO(TQTextCodec::codecForLocale());
         *verifyproc<<"gpg"<<"--no-secmem-warning"<<"--status-fd=2"<<"--command-fd=0"<<"--utf8-strings"<<"--verify";
        	connect(verifyproc, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(slotverifyresult(KProcess *)));
        	connect(verifyproc, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(slotverifyread(KProcIO *)));
        	verifyproc->start(KProcess::NotifyOnExit,true);
		verifyproc->writeStdin (text);
		verifyproc->closeWhenDone();
}


void KgpgInterface::slotverifyresult(KProcess*)
{
if (signmiss) emit missingSignature(signID);
    else {
	if (signID.isEmpty()) signID=i18n("No signature found.");
	emit verifyOver(signID,message);
	}
//kdDebug(2100) << "GPG VERIFY OVER________"<<endl;
}

void KgpgInterface::slotverifyread(KProcIO *p)
{
TQString required;
  while (p->readln(required,true)!=-1)
    {
    message+=required+"\n";
    required=required.section("]",1,-1).stripWhiteSpace();
     if (required.startsWith("GOODSIG"))
     {
	     TQString userName=required.section(" ",2,-1).replace(TQRegExp("<"),"&lt;");
	     userName=checkForUtf8(userName);
     signID=i18n("<qt>Good signature from:<br><b>%1</b><br>Key ID: %2</qt>").tqarg(userName).tqarg("0x"+required.section(" ",1,1).right(8));
     }
     if (required.startsWith("BADSIG"))
     {
     signID=i18n("<qt><b>Bad signature</b> from:<br>%1<br>Key ID: %2<br><br><b>Text is corrupted.</b></qt>").tqarg(required.section(" ",2,-1).replace(TQRegExp("<"),"&lt;")).tqarg("0x"+required.section(" ",1,1).right(8));
     }
     if (required.startsWith("NO_PUBKEY"))
     {
     signID="0x"+required.section(" ",1,1).right(8);
     signmiss=true;
     }
     if (required.startsWith("UNEXPECTED") || required.startsWith("NODATA"))
	signID=i18n("No signature found.");
     if (required.startsWith("TRUST_UNDEFINED"))
     signID+=i18n("The signature is valid, but the key is untrusted");
     if (required.startsWith("TRUST_ULTIMATE"))
     signID+=i18n("The signature is valid, and the key is ultimately trusted");
     }
}


///////////////////////////////////////////////////////////////////////////////////////////////////   MD5

Md5Widget::Md5Widget(TQWidget *parent, const char *name,KURL url):KDialogBase( parent, name, true,i18n("MD5 Checksum"),Apply | Close)
{
        setButtonApply(i18n("Compare MD5 with Clipboard"));
        mdSum=TQString();
        TQFile f(url.path());
        f.open( IO_ReadOnly);
        KMD5 checkfile;
        checkfile.reset();
        checkfile.update(*TQT_TQIODEVICE(&f));
        mdSum=checkfile.hexDigest().data();
        f.close();
        TQWidget *page = new TQWidget(this);

        resize( 360, 150 );
        TQGridLayout *MyDialogLayout = new TQGridLayout( page, 1, 1, 5, 6, "MyDialogLayout");

        TQLabel *TextLabel1 = new TQLabel( page, "TextLabel1" );
        TextLabel1->setText(i18n("MD5 sum for <b>%1</b> is:").tqarg(url.fileName()));
        MyDialogLayout->addWidget( TextLabel1, 0, 0 );

        KLineEdit *KRestrictedLine1 = new KLineEdit(mdSum,page);
        KRestrictedLine1->setReadOnly(true);
        KRestrictedLine1->setPaletteBackgroundColor(TQColor(255,255,255));
        MyDialogLayout->addWidget( KRestrictedLine1, 1, 0 );


        TQHBoxLayout *Layout4 = new TQHBoxLayout( 0, 0, 6, "Layout4");

        KLed1=new KLed(TQColor(80,80,80),KLed::Off,KLed::Sunken,KLed::Circular,page,"KLed1");
        KLed1->off();
        KLed1->setSizePolicy( TQSizePolicy( (TQSizePolicy::SizeType)0, (TQSizePolicy::SizeType)0, 0, 0, KLed1->sizePolicy().hasHeightForWidth() ) );
        Layout4->addWidget( KLed1 );

        TextLabel1_2 = new TQLabel( page, "TextLabel1_2" );
        TextLabel1_2->setText(i18n( "<b>Unknown status</b>" ) );
        Layout4->addWidget( TextLabel1_2 );

        MyDialogLayout->addLayout( Layout4, 2, 0 );
        TQSpacerItem* spacer = new TQSpacerItem( 0, 0, TQSizePolicy::Minimum, TQSizePolicy::Expanding );
        MyDialogLayout->addItem( spacer, 3, 0 );

        page->show();
        page->resize(page->minimumSize());
        setMainWidget(page);


}

Md5Widget::~Md5Widget()
{}

void Md5Widget::slotApply()
{
        TQClipboard *cb = TQApplication::tqclipboard();
        TQString text;
        // Copy text from the clipboard (paste)
        text = cb->text(TQClipboard::Clipboard);
        if ( !text.isEmpty() ) {
                text=text.stripWhiteSpace();
                while (text.find(' ')!=-1)
                        text.remove(text.find(' '),1);
                if (text==mdSum) {
                        TextLabel1_2->setText(i18n("<b>Correct checksum</b>, file is ok."));
                        KLed1->setColor(TQColor(0,255,0));
                        KLed1->on();
                }//KMessageBox::sorry(0,"OK");
                else if (text.length()!=mdSum.length())
                        KMessageBox::sorry(0,i18n("Clipboard content is not a MD5 sum."));
                else {
                        TextLabel1_2->setText(i18n("<b>Wrong checksum, FILE CORRUPTED</b>"));
                        KLed1->setColor(TQColor(255,0,0));
                        KLed1->on();
                }
        }
}

/////////////////////////////////////////////////////////////////////////////////////////////   signatures


void KgpgInterface::KgpgSignFile(TQString keyID,KURL srcUrl,TQStringList Options)
{
        //////////////////////////////////////   create a detached signature for a chosen file
        message=TQString();
        step=3;
        /////////////       create gpg command
        KProcIO *proc=new KProcIO(TQTextCodec::codecForLocale());
        keyID=keyID.stripWhiteSpace();
        *proc<<"gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--utf8-strings"<<"--status-fd=2"<<"--command-fd=0"<<"-u"<<keyID.local8Bit();
		for ( TQStringList::Iterator it = Options.begin(); it != Options.end(); ++it )
       		if (!TQFile::encodeName(*it).isEmpty()) *proc<< TQString(TQFile::encodeName(*it));

	*proc<<"--output"<<TQString(TQFile::encodeName(srcUrl.path()+".sig"));
        *proc<<"--detach-sig"<<TQString(TQFile::encodeName(srcUrl.path()));

        TQObject::connect(proc, TQT_SIGNAL(processExited(KProcess *)),this,TQT_SLOT(signfin(KProcess *)));
        TQObject::connect(proc,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(readsignprocess(KProcIO *)));
        proc->start(KProcess::NotifyOnExit,true);
}



void KgpgInterface::signfin(KProcess *)
{
        if (message.find("SIG_CREATED")!=-1)
                KMessageBox::information(0,i18n("The signature file %1 was successfully created.").tqarg(file.fileName()));
        else if (message.find("BAD_PASSPHRASE")!=-1)
                KMessageBox::sorry(0,i18n("Bad passphrase, signature was not created."));
        else
                KMessageBox::sorry(0,message);
        emit signfinished();
}


void KgpgInterface::readsignprocess(KProcIO *p)
{
        TQString required;
        while (p->readln(required,true)!=-1) {
                if (required.find("USERID_HINT",0,false)!=-1)
		updateIDs(required);

                if (required.find("GET_")!=-1) {
                        if (required.find("openfile.overwrite.okay")!=-1)
                                p->writeStdin(TQString("Yes"));
                        else if ((required.find("passphrase.enter")!=-1)) {
                                if (userIDs.isEmpty())
                                        userIDs=i18n("[No user id found]");
                                TQCString passphrase;
                                TQString passdlgmessage;
                                if (step<3)
                                        passdlgmessage=i18n("<b>Bad passphrase</b>. you have %1 tries left.<br>").tqarg(step);
                                passdlgmessage+=i18n("Enter passphrase for <b>%1</b>").tqarg(userIDs);
                                int code=KPasswordDialog::getPassword(passphrase,passdlgmessage);
                                if (code!=TQDialog::Accepted) {
                                        p->deleteLater();
                                        emit signfinished();
                                        return;
                                }
                                p->writeStdin(passphrase,true);
                                userIDs=TQString();
				if (step>1) step--;
				else step=3;
                        } else {
                                p->writeStdin(TQString("quit"));
                                p->closeWhenDone();
                        }
                }
                message+=required+"\n";
        }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void KgpgInterface::KgpgVerifyFile(KURL sigUrl,KURL srcUrl)
{
        //////////////////////////////////////   verify signature for a chosen file
        message=TQString();
	signID=TQString();
	signmiss=false;
        /////////////       create gpg command
        KProcIO *proc=new KProcIO(TQTextCodec::codecForLocale());
        file=sigUrl;
        *proc<<"gpg"<<"--no-tty"<<"--utf8-strings"<<"--no-secmem-warning"<<"--status-fd=2"<<"--verify";
        if (!srcUrl.isEmpty())
                *proc<<TQString(TQFile::encodeName(srcUrl.path()));
        *proc<<TQString(TQFile::encodeName(sigUrl.path()));

        TQObject::connect(proc, TQT_SIGNAL(processExited(KProcess *)),this,TQT_SLOT(verifyfin(KProcess *)));
        TQObject::connect(proc,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(readprocess(KProcIO *)));
        proc->start(KProcess::NotifyOnExit,true);
}


void KgpgInterface::readprocess(KProcIO *p)
{
TQString required;
  while (p->readln(required,true)!=-1)
    {
    message+=required+"\n";
	if (required.find("GET_")!=-1) {
        p->writeStdin(TQString("quit"));
        p->closeWhenDone();
        }
    required=required.section("]",1,-1).stripWhiteSpace();
    if (required.startsWith("UNEXPECTED") || required.startsWith("NODATA"))
	signID=i18n("No signature found.");
    if (required.startsWith("GOODSIG"))
    {
    signID=i18n("<qt>Good signature from:<br><b>%1</b><br>Key ID: %2</qt>").tqarg(required.section(" ",2,-1).replace(TQRegExp("<"),"&lt;")).tqarg("0x"+required.section(" ",1,1).right(8));
    }
    if (required.startsWith("BADSIG"))
    {
    signID=i18n("<qt><b>BAD signature</b> from:<br> %1<br>Key id: %2<br><br>"
                             "<b>The file is corrupted!</b></qt>").tqarg(required.section(" ",2,-1).replace(TQRegExp("<"),"&lt;")).tqarg("0x"+required.section(" ",1,1).right(8));
    }
    if (required.startsWith("NO_PUBKEY"))
    {
    signmiss=true;
    signID="0x"+required.section(" ",1,1).right(8);
    }
    if (required.startsWith("TRUST_UNDEFINED"))
    	signID+=i18n("The signature is valid, but the key is untrusted");
    if (required.startsWith("TRUST_ULTIMATE"))
	signID+=i18n("The signature is valid, and the key is ultimately trusted");
    }
}


void KgpgInterface::verifyfin(KProcess *)
{
    if (!signmiss) {
        if (signID.isEmpty()) signID=i18n("No signature found.");
        (void) new KDetailedInfo(0,"verify_result",signID,message);
    }
    else {
    	if (KMessageBox::questionYesNo(0,i18n("<qt><b>Missing signature:</b><br>Key id: %1<br><br>"
                                                      "Do you want to import this key from a keyserver?</qt>").tqarg(signID),file.fileName(),TQString(), i18n("Import"), i18n("Do Not Import"))==KMessageBox::Yes)
    	emit verifyquerykey(signID);
    }
    emit verifyfinished();
}



////////////////////////////////////////////////////////////   sign a key

void KgpgInterface::KgpgSignKey(TQString keyID,TQString signKeyID,TQString signKeyMail,bool local,int checking)
{
        signKeyMail.replace(TQRegExp("<"),"&lt;");
        konsChecked=checking;
        konsLocal=local;
        konsSignKey=signKeyID;
        konsKeyID=keyID;
        errMessage=TQString();
        if (checkuid(keyID)>0)
	{
                openSignConsole();
		return;
	}

        signSuccess=0;
        step=1;
        output=TQString();
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<<"gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--utf8-strings"<<"--command-fd=0"<<"--status-fd=2"<<"-u"<<signKeyID;
        *conprocess<<"--edit-key"<<keyID;
	if (local) *conprocess<<"lsign";
	else *conprocess<<"sign";
        TQObject::connect(conprocess,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(sigprocess(KProcIO *)));
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(signover(KProcess *)));
        conprocess->start(KProcess::NotifyOnExit,true);
}

void KgpgInterface::sigprocess(KProcIO *p)
{
        TQString required=TQString();

        while (p->readln(required,true)!=-1)
        {

                output+=required+"\n";
                if (required.find("USERID_HINT",0,false)!=-1)
		updateIDs(required);

                if (signSuccess==4) {
                        if (required.find("GET_")!=-1)
                                p->writeStdin(TQString("quit"));
                        p->closeWhenDone();
                        return;
                }

                if ((required.find("GOOD_PASSPHRASE")!=-1)) {
                        signSuccess=3;
                        step=2;
                }

                if (required.find("sign_uid.expire")!=-1) {
                        p->writeStdin(TQString("Never"));
                        required=TQString();
                }
                if (required.find("sign_uid.class")!=-1) {
                        p->writeStdin(TQString::number(konsChecked));
                        required=TQString();
                }
                if (required.find("sign_uid.okay")!=-1) {
                        p->writeStdin(TQString("Y"));
                        required=TQString();
                }

		if (required.find("sign_all.okay")!=-1) {
                        p->writeStdin(TQString("Y"));
                        required=TQString();
                }

                if (required.find("passphrase.enter")!=-1) {
                        TQCString signpass;
                        int code=KPasswordDialog::getPassword(signpass,i18n("<qt>%1 Enter passphrase for <b>%2</b>:</qt>")
                                                              .tqarg(errMessage).tqarg(userIDs));
                        if (code!=TQDialog::Accepted) {
                                signSuccess=4;  /////  aborted by user mode
                                required=TQString();
                                p->writeStdin(TQString("quit"));
                                p->closeWhenDone();
                                return;
                        }
                        p->writeStdin(signpass,true);
                        required=TQString();
                        //               step=2;
                }
                if ((step==2) && (required.find("keyedit.prompt")!=-1)) {
                        p->writeStdin(TQString("save"));
                        required=TQString();
                }
                if (required.find("BAD_PASSPHRASE")!=-1) {
                        errMessage=i18n("<b>Bad passphrase</b>. Try again.</br>");
                        required=TQString();
                        signSuccess=2;  /////  bad passphrase
                }
                if (required.find("GET_")!=-1) /////// gpg asks for something unusal, turn to konsole mode
                {
                        if (signSuccess!=2)
                                signSuccess=1;  /////  switching to console mode
                        p->writeStdin(TQString("quit"));
                        p->closeWhenDone();

                }
        }
}


void KgpgInterface::signover(KProcess *)
{
        if (signSuccess>1)
                emit signatureFinished(signSuccess);  ////   signature successful or bad passphrase
        else {
                KDetailedConsole *q=new KDetailedConsole(0,"sign_error",i18n("<qt>Signing key <b>%1</b> with key <b>%2</b> failed.<br>"
                                    "Do you want to try signing the key in console mode?</qt>").tqarg(konsKeyID).tqarg(konsSignKey),output);
                if (q->exec()==TQDialog::Accepted)
                        openSignConsole();
                else
                        emit signatureFinished(0);
        }
}

void KgpgInterface::openSignConsole()
{
        KProcess conprocess;
	KConfig *config = KGlobal::config();
	config->setGroup("General");
	conprocess<< config->readPathEntry("TerminalApplication","konsole");
        conprocess<<"-e"<<"gpg";
        conprocess<<"--no-secmem-warning"<<"--expert"<<"-u"<<konsSignKey;
        if (!konsLocal)
                conprocess<<"--sign-key"<<konsKeyID;
        else
                conprocess<<"--lsign-key"<<konsKeyID;
        conprocess.start(KProcess::Block);
	emit signatureFinished(3);

}

////////////////////////////////////////////////////////////////////////////     delete signature


void KgpgInterface::KgpgDelSignature(TQString keyID,TQString signKeyID)
{
        if (checkuid(keyID)>0) {
                KMessageBox::sorry(0,i18n("This key has more than one user ID.\nEdit the key manually to delete signature."));
                return;
        }

        message=signKeyID.remove(0,2);
        deleteSuccess=false;
        step=0;

        FILE *fp;
        TQString encResult;
        char buffer[200];
        signb=0;
        sigsearch=0;

        TQString gpgcmd="gpg --no-tty --no-secmem-warning --with-colon --list-sigs "+keyID;
        fp = popen(TQFile::encodeName(gpgcmd), "r");
        while ( fgets( buffer, sizeof(buffer), fp)) {
                encResult=buffer;
                if (encResult.startsWith("sig")) {
                        if (encResult.find(message)!=-1)
                                break;
                        signb++;
                } else if (encResult.startsWith("rev"))
                        signb++;
        }
        pclose(fp);
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<<"gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--utf8-strings"<<"--command-fd=0"<<"--status-fd=2";
        *conprocess<<"--edit-key"<<keyID<<"uid 1"<<"delsig";
        TQObject::connect(conprocess,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(delsigprocess(KProcIO *)));
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(delsignover(KProcess *)));
        conprocess->start(KProcess::NotifyOnExit,true);
}


void KgpgInterface::delsigprocess(KProcIO *p)
{

        TQString required=TQString();
        while (p->readln(required,true)!=-1)
        {
                if (required.find("keyedit.delsig")!=-1){

                        if ((sigsearch==signb) && (step==0)) {
                                p->writeStdin(TQString("Y"));
                                step=1;
                        } else
                                p->writeStdin(TQString("n"));
                        sigsearch++;
                        required=TQString();
                }
                if ((step==1) && (required.find("keyedit.prompt")!=-1)) {
                        p->writeStdin(TQString("save"));
                        required=TQString();
                        deleteSuccess=true;
                }
                if (required.find("GET_LINE")!=-1) {
                        p->writeStdin(TQString("quit"));
                        p->closeWhenDone();
                        deleteSuccess=false;
                }
        }
}

void KgpgInterface::delsignover(KProcess *)
{
        emit delsigfinished(deleteSuccess);
}

/////////////////////////////////////////////////// check if a key has more than one id

int KgpgInterface::checkuid(TQString KeyID)
{
        FILE *fp;
        TQString encResult;
        char buffer[200];
        int  uidcnt=0;

        TQString gpgcmd="gpg --no-tty --no-secmem-warning --with-colon --list-sigs "+KeyID;
        //////////   encode with untrusted keys or armor if checked by user
        fp = popen(TQFile::encodeName(gpgcmd), "r");
        while (fgets(buffer, sizeof(buffer), fp)) {
                encResult=buffer;
                if (encResult.startsWith("uid"))
                        uidcnt++;
        }
        pclose(fp);
        return uidcnt;
}


///////////////////////////////////////////////////////////////    change key expiration


void KgpgInterface::KgpgKeyExpire(TQString keyID,TQDate date,bool unlimited)
{
        expSuccess=0;
        step=0;
        if (unlimited)
                expirationDelay=0;
        else
                expirationDelay=TQDate::currentDate().daysTo(date);
        output=TQString();
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<<"gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--command-fd=0"<<"--status-fd=2"<<"--utf8-strings";
        *conprocess<<"--edit-key"<<keyID<<"expire";
        TQObject::connect(conprocess,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(expprocess(KProcIO *)));
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(expover(KProcess *)));
        conprocess->start(KProcess::NotifyOnExit,KProcess::AllOutput);

}

void KgpgInterface::expprocess(KProcIO *p)
{
        TQString required=TQString();

        while (p->readln(required,true)!=-1) {
                output+=required+"\n";

                if (required.find("USERID_HINT",0,false)!=-1)
		updateIDs(required);

                if ((required.find("GOOD_PASSPHRASE")!=-1)) {
                        expSuccess=3;
                        step=2;
                }

                if (required.find("keygen.valid")!=-1) {
                        p->writeStdin(TQString::number(expirationDelay));
                        required=TQString();
                }

                if (required.find("passphrase.enter")!=-1) {
                        TQCString signpass;
                        int code=KPasswordDialog::getPassword(signpass,i18n("<qt>Enter passphrase for <b>%1</b>:</qt>").tqarg(userIDs));
                        if (code!=TQDialog::Accepted) {
                                expSuccess=3;  /////  aborted by user mode
                                p->writeStdin(TQString("quit"));
                                p->closeWhenDone();
                                return;
                        }
                        p->writeStdin(signpass,true);
                        required=TQString();
                        //              step=2;
                }
                if ((step==2) && (required.find("keyedit.prompt")!=-1)) {
                        p->writeStdin(TQString("save"));
			p->closeWhenDone();
                        required=TQString();
                }
		if ((step==2) && (required.find("keyedit.save.okay")!=-1)) {
                        p->writeStdin(TQString("YES"));
			p->closeWhenDone();
                        required=TQString();
                }
                if (required.find("BAD_PASSPHRASE")!=-1) {
                        p->writeStdin(TQString("quit"));
                        p->closeWhenDone();
                        expSuccess=2;  /////  bad passphrase
                }
                if ((required.find("GET_")!=-1) && (expSuccess!=2)) /////// gpg asks for something unusal, turn to konsole mode
                {
                        expSuccess=1;  /////  switching to console mode
                        p->writeStdin(TQString("quit"));
                        p->closeWhenDone();

                }
        }
}



void KgpgInterface::expover(KProcess *)
{
        if ((expSuccess==3) || (expSuccess==2))
                emit expirationFinished(expSuccess);  ////   signature successful or bad passphrase
        else {
                KDetailedConsole *q=new KDetailedConsole(0,"sign_error",i18n("<qt><b>Changing expiration failed.</b><br>"
                                    "Do you want to try changing the key expiration in console mode?</qt>"),output);
                if (q->exec()==TQDialog::Accepted)
                        KMessageBox::sorry(0,"work in progress...");
                //openSignConsole();
                else
                        emit expirationFinished(0);
        }
}


///////////////////////////////////////////////////////////////    change key trust


void KgpgInterface::KgpgTrustExpire(TQString keyID,int keyTrust)
{
	trustValue=keyTrust+1;
/*	Don't know=1; Do NOT trust=2; Marginally=3; Fully=4; Ultimately=5;   */

        output=TQString();
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<<"gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--command-fd=0"<<"--status-fd=2"<<"--utf8-strings";
        *conprocess<<"--edit-key"<<keyID<<"trust";
        TQObject::connect(conprocess,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(trustprocess(KProcIO *)));
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(trustover(KProcess *)));
        conprocess->start(KProcess::NotifyOnExit,true);

}

void KgpgInterface::trustprocess(KProcIO *p)
{
        TQString required=TQString();
        while (p->readln(required,true)!=-1) {
                output+=required+"\n";
                if (required.find("edit_ownertrust.set_ultimate.okay")!=-1) {
                        p->writeStdin(TQString("YES"));
                        required=TQString();
                }

                if (required.find("edit_ownertrust.value")!=-1) {
                        p->writeStdin(TQString::number(trustValue));
                        required=TQString();
                }

                if (required.find("keyedit.prompt")!=-1) {
                        p->writeStdin(TQString("save"));
			p->closeWhenDone();
                        required=TQString();
                }

                if (required.find("GET_")!=-1) /////// gpg asks for something unusal, turn to konsole mode
                {
                        expSuccess=1;  /////  switching to console mode
                        p->writeStdin(TQString("quit"));
                        p->closeWhenDone();

                }
        }
}



void KgpgInterface::trustover(KProcess *)
{
        emit trustfinished();
}


///////////////////////////////////////////////////////////////    change passphrase


void KgpgInterface::KgpgChangePass(TQString keyID)
{
        step=1;
        output=TQString();
        message=TQString();
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<<"gpg"<<"--no-secmem-warning"<<"--no-tty"<<"--no-use-agent"<<"--command-fd=0"<<"--status-fd=2"<<"--utf8-strings";
        *conprocess<<"--edit-key"<<keyID<<"passwd";
        TQObject::connect(conprocess,TQT_SIGNAL(readReady(KProcIO *)),this,TQT_SLOT(passprocess(KProcIO *)));
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(passover(KProcess *)));
        conprocess->start(KProcess::NotifyOnExit,KProcess::AllOutput);

}

void KgpgInterface::passprocess(KProcIO *p)
{
        TQString required=TQString();

        while (p->readln(required,true)!=-1) {
                output+=required+"\n";

                if (required.find("USERID_HINT",0,false)!=-1)
		updateIDs(required);

                if ((step>2) && (required.find("keyedit.prompt")!=-1)) {
			if (step==3)
			{
			emit passwordChanged();
                        p->writeStdin(TQString("save"));
			}
			else p->writeStdin(TQString("quit"));
                        required=TQString();
                }

                if ((required.find("GOOD_PASSPHRASE")!=-1) && (step==2))
                        step=3;

                if ((required.find("BAD_PASSPHRASE")!=-1) && (step==2)) {
                        step=1;
                        message=i18n("<b>Bad passphrase</b>. Try again<br>");
                }

                if ((required.find("passphrase.enter")!=-1)) {
                        if (userIDs.isEmpty())
                                userIDs=i18n("[No user id found]");
                        userIDs.replace(TQRegExp("<"),"&lt;");

                        if (step==1) {
                                TQCString passphrase;
                                int code=KPasswordDialog::getPassword(passphrase,i18n("<qt>%1 Enter passphrase for <b>%2</b></qt>")
                                                                      .tqarg(message).tqarg(userIDs));
                                if (code!=TQDialog::Accepted) {
                                        p->writeStdin(TQString("quit"));
                                        //				 p->closeWhenDone();
                                        emit processaborted(true);
                                        p->deleteLater();
                                        return;
                                }
                                p->writeStdin(passphrase,true);
                                step=2;
                        }

                        if (step==3) {
                                TQCString passphrase;
                                int code=KPasswordDialog::getNewPassword(passphrase,i18n("<qt>Enter new passphrase for <b>%1</b><br>If you forget this passphrase, all your encrypted files and messages will be lost !<br></qt>").tqarg(userIDs));
                                if (code!=TQDialog::Accepted) {
					step=4;
                                        p->writeStdin(TQString("quit"));
                                        p->writeStdin(TQString("quit"));
                                        p->closeWhenDone();
                                        emit processaborted(true);
                                        return;
                                }
                                p->writeStdin(passphrase,true);
                                userIDs=TQString();
                        }

                        required=TQString();
                }


                if (required.find("GET_")!=-1) /////// gpg asks for something unusal, turn to konsole mode
                {
                        p->writeStdin(TQString("quit"));
                        p->closeWhenDone();

                }
        }
}



void KgpgInterface::passover(KProcess *)
{
        //emit trustfinished();
}



//////////////////////////////////////////////////////////////    key export

TQString KgpgInterface::getKey(TQStringList IDs, bool attributes)
{
        keyString=TQString();
        KProcIO *proc=new KProcIO(TQTextCodec::codecForLocale());
        *proc<< "gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--utf8-strings";
        *proc<<"--export"<<"--armor";
        if (!attributes)
                *proc<<"--export-options"<<"no-include-attributes";

        for ( TQStringList::Iterator it = IDs.begin(); it != IDs.end(); ++it )
                *proc << *it;
        TQObject::connect(proc, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(slotReadKey(KProcIO *)));
        proc->start(KProcess::Block,false);
        return keyString;
}


void KgpgInterface::slotReadKey(KProcIO *p)
{
        TQString outp;
        while (p->readln(outp)!=-1)
                if (!outp.startsWith("gpg:")) keyString+=outp+"\n";
}


//////////////////////////////////////////////////////////////    key import

void KgpgInterface::importKeyURL(KURL url)
{
        /////////////      import a key

        if( KIO::NetAccess::download( url, tempKeyFile,0) ) {
                message=TQString();
                KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
                *conprocess<< "gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--status-fd=2"<<"--utf8-strings"<<"--import";
                *conprocess<<"--allow-secret-key-import";
                *conprocess<<tempKeyFile;
                TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(importURLover(KProcess *)));
                TQObject::connect(conprocess, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(importprocess(KProcIO *)));
                conprocess->start(KProcess::NotifyOnExit,true);
        }
}

void KgpgInterface::importKey(TQString keystr)
{
        /////////////      import a key
        message=TQString();
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<< "gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--status-fd=2"<<"--import";
        *conprocess<<"--allow-secret-key-import";
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(importover(KProcess *)));
        TQObject::connect(conprocess, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(importprocess(KProcIO *)));
        conprocess->start(KProcess::NotifyOnExit,true);
        conprocess->writeStdin(keystr, true);
        conprocess->closeWhenDone();
}

void KgpgInterface::importover(KProcess *)
{
TQStringList importedKeysIds;
TQStringList messageList;
TQString resultMessage;
bool secretImport=false;
kdDebug(2100)<<"Importing is over"<<endl;
        TQString parsedOutput=message;
        TQStringList importedKeys;

        while (parsedOutput.find("IMPORTED")!=-1) {
                parsedOutput.remove(0,parsedOutput.find("IMPORTED")+8);
		importedKeys<<parsedOutput.section("\n",0,0).stripWhiteSpace();
		importedKeysIds<<parsedOutput.stripWhiteSpace().section(' ',0,0);
        }


        if (message.find("IMPORT_RES")!=-1) {
                parsedOutput=message.section("IMPORT_RES",-1,-1).stripWhiteSpace();
		messageList=TQStringList::split(" ",parsedOutput,true);

                resultMessage=i18n("<qt>%n key processed.<br></qt>","<qt>%n keys processed.<br></qt>",messageList[0].toULong());
                if (messageList[4]!="0")
                        resultMessage+=i18n("<qt>One key unchanged.<br></qt>","<qt>%n keys unchanged.<br></qt>",messageList[4].toULong());
                if (messageList[7]!="0")
                        resultMessage+=i18n("<qt>One signature imported.<br></qt>","<qt>%n signatures imported.<br></qt>",messageList[7].toULong());
                if (messageList[1]!="0")
                        resultMessage+=i18n("<qt>One key without ID.<br></qt>","<qt>%n keys without ID.<br></qt>",messageList[1].toULong());
                if (messageList[3]!="0")
                        resultMessage+=i18n("<qt>One RSA key imported.<br></qt>","<qt>%n RSA keys imported.<br></qt>",messageList[3].toULong());
                if (messageList[5]!="0")
                        resultMessage+=i18n("<qt>One user ID imported.<br></qt>","<qt>%n user IDs imported.<br></qt>",messageList[5].toULong());
                if (messageList[6]!="0")
                        resultMessage+=i18n("<qt>One subkey imported.<br></qt>","<qt>%n subkeys imported.<br></qt>",messageList[6].toULong());
                if (messageList[8]!="0")
                        resultMessage+=i18n("<qt>One revocation certificate imported.<br></qt>","<qt>%n revocation certificates imported.<br></qt>",messageList[8].toULong());
                if (messageList[9]!="0")
			{
                        resultMessage+=i18n("<qt>One secret key processed.<br></qt>","<qt>%n secret keys processed.<br></qt>",messageList[9].toULong());
			secretImport=true;
			}
                if (messageList[10]!="0")
                        resultMessage+=i18n("<qt><b>One secret key imported.</b><br></qt>","<qt><b>%n secret keys imported.</b><br></qt>",messageList[10].toULong());
                if (messageList[11]!="0")
                        resultMessage+=i18n("<qt>One secret key unchanged.<br></qt>","<qt>%n secret keys unchanged.<br></qt>",messageList[11].toULong());
                if (messageList[12]!="0")
                        resultMessage+=i18n("<qt>One secret key not imported.<br></qt>","<qt>%n secret keys not imported.<br></qt>",messageList[12].toULong());
                if (messageList[2]!="0")
                        resultMessage+=i18n("<qt><b>One key imported:</b><br></qt>","<qt><b>%n keys imported:</b><br></qt>",messageList[2].toULong());

		if (secretImport) resultMessage+=i18n("<qt><br><b>You have imported a secret key.</b> <br>"
									"Please note that imported secret keys are not trusted by default.<br>"
									"To fully use this secret key for signing and encryption, you must edit the key (double click on it) and set its trust to Full or Ultimate.</qt>");
        } else
                resultMessage=i18n("No key imported... \nCheck detailed log for more infos");

	if (messageList[8]!="0") importedKeysIds="ALL";
	if ((messageList[9]!="0") && (importedKeysIds.isEmpty())) // orphaned secret key imported
	emit refreshOrphaned();
        emit importfinished(importedKeysIds);

	(void) new KDetailedInfo(0,"import_result",resultMessage,message,importedKeys);
}

void KgpgInterface::importURLover(KProcess *p)
{
        KIO::NetAccess::removeTempFile(tempKeyFile);
        importover(p);
        //KMessageBox::information(0,message);
        //emit importfinished();
}

void KgpgInterface::importprocess(KProcIO *p)
{
        TQString outp;
        while (p->readln(outp)!=-1) {
                if (outp.find("http-proxy")==-1)
                        message+=outp+"\n";
        }
}


///////////////////////////////////////////////////////////////////////////////////////   User ID's


void KgpgInterface::KgpgAddUid(TQString keyID,TQString name,TQString email,TQString comment)
{
uidName=name;
uidComment=comment;
uidEmail=email;
output=TQString();
addSuccess=true;

        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<< "gpg"<<"--no-tty"<<"--status-fd=2"<<"--command-fd=0"<<"--utf8-strings";
        *conprocess<<"--edit-key"<<keyID<<"adduid";
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(adduidover(KProcess *)));
        TQObject::connect(conprocess, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(adduidprocess(KProcIO *)));
        conprocess->start(KProcess::NotifyOnExit,true);
}

void KgpgInterface::adduidover(KProcess *)
{
if (addSuccess) emit addUidFinished();
else emit addUidError(output);
}

void KgpgInterface::adduidprocess(KProcIO *p)
{
        TQString required=TQString();
        while (p->readln(required,true)!=-1) {
                output+=required+"\n";
                if (required.find("USERID_HINT",0,false)!=-1)
		updateIDs(required);

                if (required.find("keygen.name")!=-1)  {
                        p->writeStdin(uidName);
                        required=TQString();
                }

		if (required.find("keygen.email")!=-1)  {
                        p->writeStdin(uidEmail);
                        required=TQString();
                }

		if (required.find("keygen.comment")!=-1)  {
                        p->writeStdin(uidComment);
                        required=TQString();
                }

                if (required.find("passphrase.enter")!=-1) {
                        TQCString delpass;
                        int code=KPasswordDialog::getPassword(delpass,i18n("<qt>Enter passphrase for <b>%1</b>:</qt>")
                                                              .tqarg(userIDs));
                        if (code!=TQDialog::Accepted) {
                                //addSuccess=false;
                                p->writeStdin(TQString("quit"));
                                p->closeWhenDone();
                                return;
                        }
                        p->writeStdin(delpass,true);
                        required=TQString();

                }

		if (required.find("keyedit.prompt")!=-1) {
		       p->writeStdin(TQString("save"));
                        required=TQString();
		}

		if ((required.find("GET_")!=-1)) /////// gpg asks for something unusal, turn to konsole mode
                {
                        kdDebug(2100)<<"unknown request"<<endl;
                        addSuccess=false;  /////  switching to console mode
                        p->writeStdin(TQString("quit"));
                        p->closeWhenDone();
                }
        }
}









/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  photo id's

void KgpgInterface::KgpgGetPhotoList(TQString keyID)
{
photoList.clear();
output=TQString();
photoCount=1;
userIDs=keyID;

        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<< "gpg"<<"--no-tty"<<"--status-fd=2"<<"--command-fd=0";
        *conprocess<<"--with-colon"<<"--list-keys"<<keyID;
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(photoreadover(KProcess *)));
        TQObject::connect(conprocess, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(photoreadprocess(KProcIO *)));
        conprocess->start(KProcess::NotifyOnExit,true);
}

void KgpgInterface::photoreadprocess(KProcIO *p)
{
        TQString required=TQString();
        while (p->readln(required,true)!=-1) {
                output+=required+"\n";
                if (required.startsWith("uat") || required.startsWith("uid")) photoCount++;
}
}


void KgpgInterface::photoreadover(KProcess *)
{
for (int i=1;i<photoCount+1;i++)
if (isPhotoId(i)) photoList+=TQString::number(i);

emit signalPhotoList(photoList);
}

bool KgpgInterface::isPhotoId(int uid)
{
KTempFile *kgpginfotmp=new KTempFile();
                kgpginfotmp->setAutoDelete(true);
                TQString pgpgOutput="cp %i "+kgpginfotmp->name();

KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<< "gpg"<<"--no-tty"<<"--status-fd=2"<<"--command-fd=0"<<"--utf8-strings";
        *conprocess<<"--photo-viewer"<<TQString(TQFile::encodeName(pgpgOutput))<<"--edit-key"<<userIDs<<"uid"<<TQString::number(uid)<<"showphoto";
        conprocess->start(KProcess::Block);
	if (kgpginfotmp->file()->size()>0)
	{
	kgpginfotmp->unlink();
	return true;
	}
	kgpginfotmp->unlink();
	return false;
}

void KgpgInterface::KgpgDeletePhoto(TQString keyID,TQString uid)
{
	delSuccess=true;
	output=TQString();
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<< "gpg"<<"--no-tty"<<"--status-fd=2"<<"--command-fd=0"<<"--utf8-strings";
        *conprocess<<"--edit-key"<<keyID<<"uid"<<uid<<"deluid";
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(delphotoover(KProcess *)));
        TQObject::connect(conprocess, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(delphotoprocess(KProcIO *)));
        conprocess->start(KProcess::NotifyOnExit,true);
}

void KgpgInterface::delphotoover(KProcess *)
{
if (delSuccess) emit delPhotoFinished();
else emit delPhotoError(output);
}

void KgpgInterface::delphotoprocess(KProcIO *p)
{
        TQString required=TQString();
        while (p->readln(required,true)!=-1) {
                output+=required+"\n";
                if (required.find("USERID_HINT",0,false)!=-1)
                                updateIDs(required);

                if (required.find("keyedit.remove.uid.okay")!=-1)  {
                        p->writeStdin(TQString("YES"));
                        required=TQString();
                }

                if (required.find("passphrase.enter")!=-1) {
                        TQCString delpass;
                        int code=KPasswordDialog::getPassword(delpass,i18n("<qt>Enter passphrase for <b>%1</b>:</qt>").tqarg(userIDs));
                        if (code!=TQDialog::Accepted) {
                                //deleteSuccess=false;
                                p->writeStdin(TQString("quit"));
                                p->closeWhenDone();
                                return;
                        }
                        p->writeStdin(delpass,true);
                        required=TQString();

                }

		if (required.find("keyedit.prompt")!=-1) {
		       p->writeStdin(TQString("save"));
                        required=TQString();
		}

		if ((required.find("GET_")!=-1)) /////// gpg asks for something unusal, turn to konsole mode
                {
                        kdDebug(2100)<<"unknown request"<<endl;
                        delSuccess=false;
                        p->writeStdin(TQString("quit"));
                        p->closeWhenDone();

                }
        }
}


void KgpgInterface::KgpgAddPhoto(TQString keyID,TQString imagePath)
{
photoUrl=imagePath;
output=TQString();
addSuccess=true;
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<< "gpg"<<"--no-tty"<<"--status-fd=2"<<"--command-fd=0"<<"--utf8-strings";
        *conprocess<<"--edit-key"<<keyID<<"addphoto";
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(addphotoover(KProcess *)));
        TQObject::connect(conprocess, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(addphotoprocess(KProcIO *)));
        conprocess->start(KProcess::NotifyOnExit,true);
}

void KgpgInterface::addphotoover(KProcess *)
{
if (addSuccess) emit addPhotoFinished();
else emit addPhotoError(output);
}

void KgpgInterface::addphotoprocess(KProcIO *p)
{
        TQString required=TQString();
        while (p->readln(required,true)!=-1) {
                output+=required+"\n";
                if (required.find("USERID_HINT",0,false)!=-1)
		updateIDs(required);

                if (required.find("photoid.jpeg.add")!=-1)  {
                        p->writeStdin(photoUrl);
                        required=TQString();
                }

		if (required.find("photoid.jpeg.size")!=-1)  {
			if (KMessageBox::questionYesNo(0,i18n("This image is very large. Use it anyway?"), TQString(), i18n("Use Anyway"), i18n("Do Not Use"))==KMessageBox::Yes)
                        p->writeStdin(TQString("Yes"));
			else
			{
			p->writeStdin(TQString("No"));
			p->writeStdin(TQString(""));
			p->writeStdin(TQString("quit"));
			}
                        required=TQString();
                }

                if (required.find("passphrase.enter")!=-1) {
                        TQCString delpass;
                        int code=KPasswordDialog::getPassword(delpass,i18n("<qt>Enter passphrase for <b>%1</b>:</qt>").tqarg(userIDs));
                        if (code!=TQDialog::Accepted) {
                                //deleteSuccess=false;
                                p->writeStdin(TQString("quit"));
                                p->closeWhenDone();
                                return;
                        }
                        p->writeStdin(delpass,true);
                        required=TQString();

                }

		if (required.find("keyedit.prompt")!=-1) {
		       p->writeStdin(TQString("save"));
                        required=TQString();
		}

		if ((required.find("GET_")!=-1)) /////// gpg asks for something unusal, turn to konsole mode
                {
                        kdDebug(2100)<<"unknown request"<<endl;
                        p->writeStdin(TQString("quit"));
			addSuccess=false;
                        p->closeWhenDone();

                }
        }
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  key revocation

void KgpgInterface::KgpgRevokeKey(TQString keyID,TQString revokeUrl,int reason,TQString description)
{
        revokeReason=reason;
        revokeSuccess=false;
        revokeDescription=description;
        certificateUrl=revokeUrl;
        output=TQString();
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<< "gpg"<<"--no-tty"<<"--status-fd=2"<<"--logger-fd=2"<<"--command-fd=0"<<"--utf8-strings";
        if (!revokeUrl.isEmpty())
                *conprocess<<"-o"<<revokeUrl;
        *conprocess<<"--gen-revoke"<<keyID;
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(revokeover(KProcess *)));
        TQObject::connect(conprocess, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(revokeprocess(KProcIO *)));
        conprocess->start(KProcess::NotifyOnExit,true);
}

void KgpgInterface::revokeover(KProcess *)
{
        if (!revokeSuccess)
                KMessageBox::detailedSorry(0,i18n("Creation of the revocation certificate failed..."),output);
        else {
                output=output.section("-----BEGIN",1);
                output.prepend("-----BEGIN");
                output=output.section("BLOCK-----",0);
                emit revokecertificate(output);
                if (!certificateUrl.isEmpty())
                        emit revokeurl(certificateUrl);
        }
}

void KgpgInterface::revokeprocess(KProcIO *p)
{
        TQString required=TQString();
        while (p->readln(required,true)!=-1) {
                output+=required+"\n";

                if (required.find("USERID_HINT",0,false)!=-1)
		updateIDs(required);

                if ((required.find("GOOD_PASSPHRASE")!=-1))
                        revokeSuccess=true;

                if ((required.find("gen_revoke.okay")!=-1) || (required.find("ask_revocation_reason.okay")!=-1) || (required.find("openfile.overwrite.okay")!=-1)) {
                        p->writeStdin(TQString("YES"));
                        required=TQString();
                }

                if (required.find("ask_revocation_reason.code")!=-1) {
                        p->writeStdin(TQString::number(revokeReason));
                        required=TQString();
                }

                if (required.find("passphrase.enter")!=-1) {
                        TQCString signpass;
                        int code=KPasswordDialog::getPassword(signpass,i18n("<qt>Enter passphrase for <b>%1</b>:</qt>").tqarg(userIDs));
                        if (code!=TQDialog::Accepted) {
                                expSuccess=3;  /////  aborted by user mode
                                p->writeStdin(TQString("quit"));
                                p->closeWhenDone();
                                return;
                        }
                        p->writeStdin(signpass,true);
                        required=TQString();

                }
                if (required.find("ask_revocation_reason.text")!=-1) {
                        //		kdDebug(2100)<<"description"<<endl;
                        p->writeStdin(revokeDescription);
                        revokeDescription=TQString();
                        required=TQString();
                }
                if ((required.find("GET_")!=-1)) /////// gpg asks for something unusal, turn to konsole mode
                {
                        kdDebug(2100)<<"unknown request"<<endl;
                        expSuccess=1;  /////  switching to console mode
                        p->writeStdin(TQString("quit"));
                        p->closeWhenDone();

                }
        }
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////   parsing of ./gnupg/options file

TQString KgpgInterface::getGpgSetting(TQString name,TQString configFile)
{
        name=name.stripWhiteSpace()+" ";
        TQFile qfile(TQFile::encodeName(configFile));
        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        if (result.stripWhiteSpace().startsWith(name)) {
                                result=result.stripWhiteSpace();
                                result.remove(0,name.length());
                                result=result.stripWhiteSpace();
                                return result.section(" ",0,0);
                        }
                        result=t.readLine();
                }
                qfile.close();
        }
        return TQString();
}

TQString KgpgInterface::getGpgMultiSetting(TQString name,TQString configFile)
{
// get GnuPG setting for item that can have multiple entries (eg. encrypt-to)

TQString parsedResult=TQString();

        name=name.stripWhiteSpace()+" ";
        TQFile qfile(TQFile::encodeName(configFile));
        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        if (result.stripWhiteSpace().startsWith(name)) {
                                result=result.stripWhiteSpace();
                                result.remove(0,name.length());
				if (parsedResult!=TQString())
				parsedResult+=" "+result.stripWhiteSpace();
				else
                                parsedResult+=result.stripWhiteSpace();
                                //return result.section(" ",0,0);
                        }
                        result=t.readLine();
                }
                qfile.close();
        }
        return parsedResult;
}

void KgpgInterface::delGpgGroup(TQString name, TQString configFile)
{
        TQString textToWrite;
        TQFile qfile(TQFile::encodeName(configFile));
        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        if (result.stripWhiteSpace().startsWith("group ")) {
                                TQString result2=result.stripWhiteSpace();
                                result2.remove(0,6);
                                result2=result2.stripWhiteSpace();
                                if (result2.startsWith(name) && (result2.remove(0,name.length()).stripWhiteSpace().startsWith("=")))
                                        result=TQString();
                        }
                       if (result!=TQString()) textToWrite+=result+"\n";
                        result=t.readLine();
                }
                qfile.close();
                if (qfile.open(IO_WriteOnly)) {
                        TQTextStream t( &qfile);
                        t << textToWrite;
                        qfile.close();
                }
        }
}

void KgpgInterface::setGpgGroupSetting(TQString name,TQStringList values, TQString configFile)
{
        TQString textToWrite;
        bool found=false;
        TQFile qfile(TQFile::encodeName(configFile));
        kdDebug(2100)<<"Changing group: "<<name<<endl;
        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        if (result.stripWhiteSpace().startsWith("group ")) {
                                TQString result2=result.stripWhiteSpace();
                                result2.remove(0,6);
                                result2=result2.stripWhiteSpace();
                                if (result2.startsWith(name) && (result2.remove(0,name.length()).stripWhiteSpace().startsWith("="))) {
//                                        kdDebug(2100)<<"Found group: "<<name<<endl;
                                        //kdDebug(2100)<<"New values: "<<values<<endl;
                                        result=TQString("group %1=%2").tqarg(name).tqarg(values.join(" "));
                                        found=true;
                                }
                        }
                        textToWrite+=result+"\n";
                        result=t.readLine();
                }
                qfile.close();
                if (!found)
                        textToWrite+="\n"+TQString("group %1=%2").tqarg(name).tqarg(values.join(" "));

                if (qfile.open(IO_WriteOnly)) {
                        TQTextStream t( &qfile);
                        t << textToWrite;
                        qfile.close();
                }
        }
}



TQStringList KgpgInterface::getGpgGroupSetting(TQString name,TQString configFile)
{

        TQFile qfile(TQFile::encodeName(configFile));
        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        result=result.stripWhiteSpace();
                        if (result.startsWith("group ")) {
                                kdDebug(2100)<<"Found 1 group"<<endl;
                                result.remove(0,6);
                                if (result.stripWhiteSpace().startsWith(name)) {
                                        kdDebug(2100)<<"Found group: "<<name<<endl;
                                        result=result.section('=',1);
                                        result=result.section('#',0,0);
                                        return TQStringList::split (" ",result);
                                }
                        }
                        result=t.readLine();
                }
                qfile.close();
        }
        return TQString();
}

TQStringList KgpgInterface::getGpgGroupNames(TQString configFile)
{
        TQStringList groups;
        TQFile qfile(TQFile::encodeName(configFile));
        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        result=result.stripWhiteSpace();
                        if (result.startsWith("group ")) {
                                result.remove(0,6);
                                groups<<result.section("=",0,0).stripWhiteSpace();
                        }
                        result=t.readLine();
                }
                qfile.close();
        }
        return groups;
}


bool KgpgInterface::getGpgBoolSetting(TQString name,TQString configFile)
{
        name=name;
        TQFile qfile(TQFile::encodeName(configFile));
        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        if (result.stripWhiteSpace().startsWith(name)) {
                                return true;
                        }
                        result=t.readLine();
                }
                qfile.close();
        }
        return false;
}

void KgpgInterface::setGpgSetting(TQString name,TQString value,TQString url)
{
        name=name+" ";
        TQString textToWrite;
        bool found=false;
        TQFile qfile(TQFile::encodeName(url));

        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        if (result.stripWhiteSpace().startsWith(name)) {
                                if (!value.isEmpty())
                                        result=name+" "+value;
                                else
                                        result=TQString();
                                found=true;
                        }
                        if (result!=TQString()) textToWrite+=result+"\n";
                        result=t.readLine();
                }
                qfile.close();
                if ((!found) && (!value.isEmpty()))
                        textToWrite+="\n"+name+" "+value;

                if (qfile.open(IO_WriteOnly)) {
                        TQTextStream t( &qfile);
                        t << textToWrite;
                        qfile.close();
                }
        }
}


void KgpgInterface::setGpgMultiSetting(TQString name,TQStringList values,TQString url)
{
        name=name+" ";
        TQString textToWrite;
        bool found=false;
        TQFile qfile(TQFile::encodeName(url));

        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        if (!result.stripWhiteSpace().startsWith(name))
				textToWrite+=result+"\n";
                        result=t.readLine();
                }
                qfile.close();

		while (!values.isEmpty())
		{
                        textToWrite+="\n"+name+" "+values.first();
			values.pop_front();
		}

                if (qfile.open(IO_WriteOnly)) {
                        TQTextStream t( &qfile);
                        t << textToWrite;
                        qfile.close();
                }
        }
}

void KgpgInterface::setGpgBoolSetting(TQString name,bool enable,TQString url)
{
        TQString textToWrite;
        bool found=false;
        TQFile qfile(TQFile::encodeName(url));

        if (qfile.open(IO_ReadOnly) && (qfile.exists())) {
                TQString result;
                TQTextStream t( &qfile );
                result=t.readLine();
                while (result!=NULL) {
                        if (result.stripWhiteSpace().startsWith(name)) {
                                if (enable)
                                        result=name;
                                else
                                        result=TQString();
                                found=true;
                        }
                       if (result!=TQString()) textToWrite+=result+"\n";
                        result=t.readLine();
                }
                qfile.close();
                if ((!found) && (enable))
                        textToWrite+=name;

                if (qfile.open(IO_WriteOnly)) {
                        TQTextStream t( &qfile);
                        t << textToWrite;
                        qfile.close();
                }
        }
}

TQString KgpgInterface::checkForUtf8bis(TQString txt)
{
    if (strchr (txt.ascii(), 0xc3) || (txt.find("\\x")!=-1))
	txt=checkForUtf8(txt);
    else {
	txt=checkForUtf8(txt);
	txt=TQString::fromUtf8(txt.ascii());
    }
    return txt;
}


TQString KgpgInterface::checkForUtf8(TQString txt)
{
        //    code borrowed from gpa
        const char *s;

        /* Make sure the encoding is UTF-8.
         * Test structure suggested by Werner Koch */
        if (txt.isEmpty())
                return TQString();

        for (s = txt.ascii(); *s && !(*s & 0x80); s++)
                ;
        if (*s && !strchr (txt.ascii(), 0xc3) && (txt.find("\\x")==-1))
                return txt;

        /* The string is not in UTF-8 */
        //if (strchr (txt.ascii(), 0xc3)) return (txt+" +++");
        if (txt.find("\\x")==-1)
                return TQString::fromUtf8(txt.ascii());
        //        if (!strchr (txt.ascii(), 0xc3) || (txt.find("\\x")!=-1)) {
        for ( int idx = 0 ; (idx = txt.find( "\\x", idx )) >= 0 ; ++idx ) {
                char str[2] = "x";
                str[0] = (char) TQString( txt.mid( idx + 2, 2 ) ).toShort( 0, 16 );
                txt.replace( idx, 4, str );
        }
        if (!strchr (txt.ascii(), 0xc3))
            return TQString::fromUtf8(txt.ascii());
        else
            return TQString::fromUtf8(TQString::fromUtf8(txt.ascii()).ascii());  // perform Utf8 twice, or some keys display badly

        return txt;
}


#include "kgpginterface.moc"
