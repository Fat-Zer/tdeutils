/***************************************************************************
                          keyinfowidget.cpp  -  description
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
 *   (at your gpgOutpution) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <klocale.h>
#include <kprocess.h>
#include <tqdatetime.h>
#include <tqfile.h>
#include <kprocio.h>
#include <kpassivepopup.h>
#include <kaction.h>
#include <tqregexp.h>
#include <ktempfile.h>
#include <tqimage.h>
#include <kdatepicker.h>
#include <tqpushbutton.h>
#include <kcombobox.h>
#include <tqlabel.h>
#include <kiconloader.h>
#include <tqcheckbox.h>
#include <layout.h>
#include <kactivelabel.h>
#include <klineedit.h>
#include <ktrader.h>
#include <kservice.h>
#include <kmessagebox.h>

#include "keyinfowidget.h"
#include "keyproperties.h"
#include "kgpgsettings.h"
#include "kgpginterface.h"

KgpgKeyInfo::KgpgKeyInfo(TQWidget *parent, const char *name,TQString sigkey):KDialogBase( Swallow, i18n("Key Properties"), Close, Close, parent, name,true)
{

        FILE *pass;
        char line[200]="";
        TQString gpgOutput,fullID;
        hasPhoto=false;
	bool isSecret=false;
	keyWasChanged=false;

	prop=new KeyProperties();
	setMainWidget(prop);
	TQString gpgcmd="gpg --no-tty --no-secmem-warning --with-colon --list-secret-key "+KShellProcess::quote(sigkey);
        pass=popen(TQFile::encodeName(gpgcmd),"r");
        while ( fgets( line, sizeof(line), pass)) {
                gpgOutput=line;
                if (gpgOutput.startsWith("sec")) isSecret=true;
               }
	 pclose(pass);

        if (!isSecret) {
                prop->changeExp->hide();
                prop->changePass->hide();
		        }
	loadKey(sigkey);

        if (hasPhoto) {
	KgpgInterface *photoProcess=new KgpgInterface();
        photoProcess->KgpgGetPhotoList(displayedKeyID);
	connect(photoProcess,TQT_SIGNAL(signalPhotoList(TQStringList)),this,TQT_SLOT(slotSetMainPhoto(TQStringList)));
        }
	else
	prop->comboId->setEnabled(false);

	connect(prop->changeExp,TQT_SIGNAL(clicked()),this,TQT_SLOT(slotChangeExp()));
        connect(this,TQT_SIGNAL(closeClicked()),this,TQT_SLOT(slotPreOk()));
        connect(prop->changePass,TQT_SIGNAL(clicked()),this,TQT_SLOT(slotChangePass()));
	connect(prop->comboId,TQT_SIGNAL(activated (const TQString &)),this,TQT_SLOT(reloadMainPhoto(const TQString &)));
	connect(prop->cbDisabled,TQT_SIGNAL(toggled(bool)),this,TQT_SLOT(slotDisableKey(bool)));
	connect(prop->kCOwnerTrust,TQT_SIGNAL(activated (int)),this,TQT_SLOT(slotChangeTrust(int)));
	connect(this,TQT_SIGNAL(changeMainPhoto(const TQPixmap&)),this,TQT_SLOT(slotSetPhoto(const TQPixmap&)));

	//prop->setMinimumSize(prop->sizeHint());
}

void KgpgKeyInfo::slotDisableKey(bool isOn)
{
KProcess kp;

	kp<<"gpg"
        <<"--no-tty"
	<<"--edit-key"
        <<displayedKeyID;
if (isOn) kp<<"disable";
else kp<<"enable";
	kp<<"save";
        kp.start(KProcess::Block);
loadKey(displayedKeyID);
keyWasChanged=true;
}

void KgpgKeyInfo::loadKey(TQString Keyid)
{
TQColor trustColor;
TQString fingervalue=TQString();
FILE *pass;
char line[200]="";
TQString gpgOutput,fullID;

TQString gpgcmd="gpg --no-tty --no-secmem-warning --with-colon --with-fingerprint --list-key "+KShellProcess::quote(Keyid);

        pass=popen(TQFile::encodeName(gpgcmd),"r");
        while ( fgets( line, sizeof(line), pass)) {
                gpgOutput=TQString::fromUtf8(line);
                if (gpgOutput.startsWith("uat"))
                        hasPhoto=true;
                if (gpgOutput.startsWith("pub")) {
                        TQString algo=gpgOutput.section(':',3,3);
                        switch( algo.toInt() ) {
                        case  1:
                                algo="RSA";
                                break;
                        case 16:
                        case 20:
                                algo="ElGamal";
                                break;
                        case 17:
                                algo="DSA";
                                break;
                        default:
                                algo=TQString("#" + algo);
                                break;
                        }
			prop->tLAlgo->setText(algo);

                        const TQString trust=gpgOutput.section(':',1,1);
                        TQString tr;
                        switch( trust[0] ) {
                        case 'o':
                                tr= i18n("Unknown");
				trustColor=KGpgSettings::colorUnknown();
                                break;
                        case 'i':
                                tr= i18n("Invalid");
				trustColor=KGpgSettings::colorBad();
                                break;
                        case 'd':
                                tr=i18n("Disabled");
				trustColor=KGpgSettings::colorBad();
				prop->cbDisabled->setChecked(true);
                                break;
                        case 'r':
                                tr=i18n("Revoked");
				trustColor=KGpgSettings::colorRev();
                                break;
                        case 'e':
                                tr=i18n("Expired");
				trustColor=KGpgSettings::colorBad();
                                break;
                        case 'q':
                                tr=i18n("Undefined");
				trustColor=KGpgSettings::colorUnknown();
                                break;
                        case 'n':
                                tr=i18n("None");
				trustColor=KGpgSettings::colorUnknown();
                                break;
                        case 'm':
                                tr=i18n("Marginal");
				trustColor=KGpgSettings::colorBad();
                                break;
                        case 'f':
                                tr=i18n("Full");
				trustColor=KGpgSettings::colorGood();
                                break;
                        case 'u':
                                tr=i18n("Ultimate");
				trustColor=KGpgSettings::colorGood();
                                break;
                        default:
                                tr="?";
				trustColor=KGpgSettings::colorUnknown();
                                break;
                        }

			if (gpgOutput.section(':',11,11).find("D",0,true)!=-1)  // disabled key
			{
				tr=i18n("Disabled");
				trustColor=KGpgSettings::colorBad();
				prop->cbDisabled->setChecked(true);
			}

                        prop->kLTrust->setText(tr);
                        prop->pixmapTrust->setPaletteBackgroundColor(trustColor);

			fullID=gpgOutput.section(':',4,4);
                        displayedKeyID=fullID.right(8);
                        prop->tLID->setText(fullID);

                        TQString fullname=gpgOutput.section(':',9,9);

                        TQDate date = TQDate::fromString(gpgOutput.section(':',5,5), Qt::ISODate);
                        prop->tLCreation->setText(KGlobal::locale()->formatDate(date));

			if (gpgOutput.section(':',6,6).isEmpty()) expirationDate=i18n("Unlimited");
			else
			{
			date = TQDate::fromString(gpgOutput.section(':',6,6), Qt::ISODate);
			expirationDate=KGlobal::locale()->formatDate(date);
			}
                        prop->tLExpiration->setText(expirationDate);

                        prop->tLLength->setText(gpgOutput.section(':',2,2));

                        const TQString otrust=gpgOutput.section(':',8,8);
			int ownerTrust=0;

			/*	Don't know=1; Do NOT trust=2; Marginally=3; Fully=4; Ultimately=5;   */

                        switch( otrust[0] ) {
                        case 'f':
                                ownerTrust=3;
                                break;
                        case 'u':
                                ownerTrust=4;
                                break;
                        case 'm':
                                ownerTrust=2;
                                break;
                        case 'n':
                                ownerTrust=1;
                                break;
                        default:
                                ownerTrust=0;
                                break;
                        }
                        prop->kCOwnerTrust->setCurrentItem(ownerTrust);

                        if (fullname.find("<")!=-1) {
                                TQString kmail=fullname;
				if (fullname.find(")")!=-1)
				kmail=kmail.section(')',1);
				kmail=kmail.section('<',1);
				kmail.truncate(kmail.length()-1);
				if (kmail.find("<")!=-1) ////////  several email addresses in the same key
				{
				kmail=kmail.replace(">",";");
				kmail.remove("<");
				}
				prop->tLMail->setText("<qt><a href=mailto:"+kmail+">"+kmail+"</a></qt>");
                        } else
                                prop->tLMail->setText(i18n("none"));

                        TQString kname=fullname.section('<',0,0);
                        if (fullname.find("(")!=-1) {
                                kname=kname.section('(',0,0);
                                TQString comment=fullname.section('(',1,1);
                                comment=comment.section(')',0,0);
                                prop->tLComment->setText(KgpgInterface::checkForUtf8(comment));
                        } else
                                prop->tLComment->setText(i18n("none"));

			prop->tLName->setText("<qt><b>"+KgpgInterface::checkForUtf8(kname).replace(TQRegExp("<"),"&lt;")+"</b></qt>");

                }
		if (gpgOutput.startsWith("fpr") && (fingervalue.isNull())) {
                        fingervalue=gpgOutput.section(':',9,9);
                        // format fingervalue in 4-digit groups
                        uint len = fingervalue.length();
                        if ((len > 0) && (len % 4 == 0))
                                for (uint n = 0; 4*(n+1) < len; n++)
                                        fingervalue.insert(5*n+4, ' ');
					prop->lEFinger->setText(fingervalue);
                }
        }
        pclose(pass);
}

void KgpgKeyInfo::slotSetMainPhoto(TQStringList list)
{
prop->comboId->insertStringList(list);
reloadMainPhoto(prop->comboId->currentText());
}

void KgpgKeyInfo::reloadMainPhoto(const TQString &uid)
{

                kgpginfotmp=new KTempFile();
                kgpginfotmp->setAutoDelete(true);
                TQString pgpgOutput="cp %i "+kgpginfotmp->name();
                KProcIO *p=new KProcIO();
                *p<<"gpg"<<"--no-tty"<<"--show-photos"<<"--photo-viewer"<<TQString(TQFile::encodeName(pgpgOutput));
		*p<<"--edit-key"<<displayedKeyID<<"uid"<<uid<<"showphoto";
		TQObject::connect(p, TQT_SIGNAL(readReady(KProcIO *)),this, TQT_SLOT(finishphotoreadprocess(KProcIO *)));
                TQObject::connect(p, TQT_SIGNAL(processExited(KProcess *)),this, TQT_SLOT(slotMainImageRead(KProcess *)));
                p->start(KProcess::NotifyOnExit,true);

}


void KgpgKeyInfo::slotMainImageRead(KProcess *p)
{
	p->deleteLater();
	TQPixmap pixmap;
        pixmap.load(kgpginfotmp->name());
	emit changeMainPhoto(pixmap);
        kgpginfotmp->unlink();
}


KgpgKeyInfo::~KgpgKeyInfo()
{
}

void KgpgKeyInfo::slotSetPhoto(const TQPixmap &pix)
{
TQImage dup=pix.convertToImage();
TQPixmap dup2;
dup2.convertFromImage(dup.scale(prop->pLPhoto->width(),prop->pLPhoto->height(),TQ_ScaleMin));
prop->pLPhoto->setPixmap(dup2);
}




void KgpgKeyInfo::finishphotoreadprocess(KProcIO *p)
{
        TQString required=TQString();
        while (p->readln(required,true)!=-1)
		if (required.find("keyedit.prompt")!=-1) {
                        p->writeStdin(TQString("quit"));
			p->closeWhenDone();

                }
}


void KgpgKeyInfo::openPhoto()
{
			 KTrader::OfferList offers = KTrader::self()->query("image/jpeg", "Type == 'Application'");
 			KService::Ptr ptr = offers.first();
 			//KMessageBox::sorry(0,ptr->desktopEntryName());
                        KProcIO *p=new KProcIO();
                        *p<<"gpg"<<"--show-photos"<<"--photo-viewer"<<TQString(TQFile::encodeName(ptr->desktopEntryName()+" %i"))<<"--list-keys"<<displayedKeyID;
                        p->start(KProcess::DontCare,true);
}

void KgpgKeyInfo::slotChangeExp()
{
chdate=new KDialogBase( this, "choose_date", true,i18n("Choose New Expiration"),KDialogBase::Ok | KDialogBase::Cancel);
TQWidget *page = new TQWidget(chdate);
kb= new TQCheckBox(i18n("Unlimited"),page );

if (prop->tLExpiration->text()==i18n("Unlimited"))
{
kdt= new KDatePicker( page );
kb->setChecked(true);
kdt->setEnabled(false);
}
else
kdt= new KDatePicker(page,KGlobal::locale()->readDate(prop->tLExpiration->text()));
TQVBoxLayout *vbox=new TQVBoxLayout(page,3);
vbox->addWidget(kdt);
vbox->addWidget(kb);
connect(kb,TQT_SIGNAL(toggled(bool)),this,TQT_SLOT(slotEnableDate(bool)));
connect(chdate,TQT_SIGNAL(okClicked()),this,TQT_SLOT(slotChangeDate()));
connect(kdt,TQT_SIGNAL(dateChanged(TQDate)),this,TQT_SLOT(slotCheckDate(TQDate)));
connect(kdt,TQT_SIGNAL(dateEntered(TQDate)),this,TQT_SLOT(slotCheckDate(TQDate)));

chdate->setMainWidget(page);
chdate->show();
}

void KgpgKeyInfo::slotCheckDate(TQDate date)
{
chdate->enableButtonOK(date>=TQDate::currentDate ());
}

void KgpgKeyInfo::slotChangeDate()
{
KgpgInterface *KeyExpirationProcess=new KgpgInterface();
		if (kb->isChecked())
                KeyExpirationProcess->KgpgKeyExpire(displayedKeyID,TQDate::currentDate(),true);
		else
		KeyExpirationProcess->KgpgKeyExpire(displayedKeyID,kdt->date(),false);
                connect(KeyExpirationProcess,TQT_SIGNAL(expirationFinished(int)),this,TQT_SLOT(slotInfoExpirationChanged(int)));
}

void KgpgKeyInfo::slotEnableDate(bool isOn)
{
if (isOn)
{
kdt->setEnabled(false);
chdate->enableButtonOK(true);
}
else
{
kdt->setEnabled(true);
chdate->enableButtonOK(kdt->date()>=TQDate::currentDate ());
}
}

void KgpgKeyInfo::slotinfoimgread(KProcess *)
{
	TQPixmap pixmap;
        pixmap.load(kgpginfotmp->name());
	emit signalPhotoId(pixmap);
        kgpginfotmp->unlink();
}

void KgpgKeyInfo::slotChangePass()
{
        KgpgInterface *ChangeKeyPassProcess=new KgpgInterface();
        ChangeKeyPassProcess->KgpgChangePass(displayedKeyID);
	connect(ChangeKeyPassProcess,TQT_SIGNAL(passwordChanged()),this,TQT_SLOT(slotInfoPasswordChanged()));
}

void KgpgKeyInfo::slotChangeTrust(int newTrust)
{
        KgpgInterface *KeyTrustProcess=new KgpgInterface();
                KeyTrustProcess->KgpgTrustExpire(displayedKeyID,newTrust);
                connect(KeyTrustProcess,TQT_SIGNAL(trustfinished()),this,TQT_SLOT(slotInfoTrustChanged()));
}


void KgpgKeyInfo::slotInfoPasswordChanged()
{
KPassivePopup::message(i18n("Passphrase for the key was changed"),TQString(),KGlobal::iconLoader()->loadIcon("kgpg",KIcon::Desktop),this);
}

void KgpgKeyInfo::slotInfoTrustChanged()
{
keyWasChanged=true;
loadKey(displayedKeyID);
//KPassivePopup::message(i18n("Owner trust of the key was changed"),TQString(),KGlobal::iconLoader()->loadIcon("kgpg",KIcon::Desktop),this,0,600);
}

void KgpgKeyInfo::slotInfoExpirationChanged(int res)
{
TQString infoMessage,infoText;
if (res==3)
{
keyWasChanged=true;
if (kb->isChecked()) prop->tLExpiration->setText(i18n("Unlimited"));
else prop->tLExpiration->setText(KGlobal::locale()->formatDate(kdt->date()));
}
if (res==2) {
infoMessage=i18n("Could not change expiration");infoText=i18n("Bad passphrase");
KPassivePopup::message(infoMessage,infoText,KGlobal::iconLoader()->loadIcon("kgpg",KIcon::Desktop),this);
}
}


void KgpgKeyInfo::slotPreOk()
{
if (keyWasChanged) emit keyNeedsRefresh();
accept();
}

#include "keyinfowidget.moc"
