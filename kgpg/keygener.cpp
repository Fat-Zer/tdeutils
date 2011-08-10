/***************************************************************************
                          keygen.cpp  -  description
                             -------------------
    begin                : Mon Jul 8 2002
    copyright            : (C) 2002 by Jean-Baptiste Mardelle
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

///////////////////////////////////////////////             code for new key generation


#include <tqwhatsthis.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqvbox.h>
#include <kcombobox.h>
#include <klineedit.h>
#include <tqcheckbox.h>
#include <tqbuttongroup.h>
#include <tqhbuttongroup.h>
#include <tqvbuttongroup.h>
#include <kmessagebox.h>
#include <klocale.h>

#include "keygener.h"

///////////////////////   main window
keyGenerate::keyGenerate(TQWidget *tqparent, const char *name):KDialogBase( tqparent, name, true,i18n("Key Generation"),Apply | Ok | Cancel)
{
        expert=false;
        setButtonApply(i18n("Expert Mode"));

        TQWidget *page = new TQWidget(this);
        TQVBoxLayout *vbox=new TQVBoxLayout(page);

        TQVButtonGroup *bgroup1=new TQVButtonGroup(i18n("Generate Key Pair"),page);

        (void) new TQLabel(i18n("Name:"),bgroup1);
        kname=new KLineEdit("",bgroup1);
        kname->setFocus();
        (void) new TQLabel(i18n("Email:"),bgroup1);
        mail=new KLineEdit("",bgroup1);

        (void) new TQLabel(i18n("Comment (optional):"),bgroup1);
        comment=new KLineEdit("",bgroup1);

        (void) new TQLabel(i18n("Expiration:"),bgroup1);
        TQHButtonGroup *bgroup=new  TQHButtonGroup(bgroup1);
        numb=new KLineEdit("0",bgroup);
        numb->setMaxLength(4);
        numb->setDisabled(true);
        keyexp = new KComboBox(bgroup);
        keyexp->insertItem(i18n("Never"),0);
        keyexp->insertItem(i18n("Days"),1);
        keyexp->insertItem(i18n("Weeks"),2);
        keyexp->insertItem(i18n("Months"),3);
        keyexp->insertItem(i18n("Years"),4);
        keyexp->setMinimumSize(keyexp->tqsizeHint());
        connect(keyexp,TQT_SIGNAL(activated(int)),this,TQT_SLOT(activateexp(int)));

        (void) new TQLabel(i18n("Key size:"),bgroup1);
        keysize = new KComboBox(bgroup1);
        keysize->insertItem("768");
        keysize->insertItem("1024");
        keysize->insertItem("2048");
        keysize->insertItem("4096");
        keysize->setCurrentItem("1024");
        keysize->setMinimumSize(keysize->tqsizeHint());

        (void) new TQLabel(i18n("Algorithm:"),bgroup1);
        keykind = new KComboBox(bgroup1);
        keykind->insertItem("DSA & ElGamal");
        keykind->insertItem("RSA");
        keykind->setMinimumSize(keykind->tqsizeHint());

        vbox->addWidget(bgroup1);
        page->show();
        page->resize(page->tqmaximumSize());
        setMainWidget(page);
}

void keyGenerate::slotOk()
{
        if (TQString(kname->text()).stripWhiteSpace().isEmpty()) {
                KMessageBox::sorry(this,i18n("You must give a name."));
                return;
        }
        TQString vmail=mail->text();
	if (vmail.isEmpty())
	{
	if (KMessageBox::warningContinueCancel(this,i18n("You are about to create a key with no email address"))!=KMessageBox::Continue) return;
        }
	else if ((vmail.find(" ")!=-1) || (vmail.find(".")==-1) || (vmail.find("@")==-1)) {
                KMessageBox::sorry(this,i18n("Email address not valid"));
                return;
        }
        accept();
}

void keyGenerate::slotApply()
{
        expert=true;
        accept();
}

void keyGenerate::activateexp(int state)
{
        if (state==0)
                numb->setDisabled(true);
        else
                numb->setDisabled(false);
}

bool keyGenerate::getmode()
{
        return(expert);
}


TQString keyGenerate::getkeytype()
{
        return(keykind->currentText());
}

TQString keyGenerate::getkeysize()
{
        return(keysize->currentText());
}

int keyGenerate::getkeyexp()
{
        return(keyexp->currentItem());
}

TQString keyGenerate::getkeynumb()
{
        if (numb->text()!=NULL)
                return(numb->text());
        else
                return ("");
}

TQString keyGenerate::getkeyname()
{
        if (kname->text()!=NULL)
                return(kname->text());
        else
                return ("");
}

TQString keyGenerate::getkeymail()
{
        if (mail->text()!=NULL)
                return(mail->text());
        else
                return ("");
}

TQString keyGenerate::getkeycomm()
{
        if (comment->text()!=NULL)
                return(comment->text());
        else
                return ("");
}

#include "keygener.moc"
