/***************************************************************************
                          detailledconsole.cpp  -  description
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


#include <tqlayout.h>
#include <tqlabel.h>
#include <tqvgroupbox.h>
#include <tqtextedit.h>

#include <kmessagebox.h>
#include <klocale.h>
#include <klistbox.h>
#include <kglobal.h>

#include "kgpgoptions.h"
#include "detailedconsole.h"


KDetailedConsole::KDetailedConsole(TQWidget *parent, const char *name,const TQString &boxLabel,const TQString &errormessage)
    : KDialogBase(parent,name,true,i18n("Sorry"),KDialogBase::Details|KDialogBase::Ok|KDialogBase::Cancel, KDialogBase::Ok)
{
        TQWidget *page = new TQWidget( this );
        setMainWidget(page);
        TQVBoxLayout *vbox=new TQVBoxLayout(page,0, spacingHint() );

        TQLabel *lab1=new TQLabel(page);
        lab1->setText(boxLabel);

        TQVGroupBox *detailsGroup = new TQVGroupBox( i18n("Details"), page);
        (void) new TQLabel(errormessage,detailsGroup);
        //labdetails->setMinimumSize(labdetails->tqsizeHint());
        setDetailsWidget(detailsGroup);
        vbox->addWidget(lab1);

}

KDetailedConsole::~KDetailedConsole()
{}

KDetailedInfo::KDetailedInfo(TQWidget *parent, const char *name , const TQString &boxLabel,const TQString &errormessage,TQStringList keysList)
    : KDialogBase(Swallow, i18n("Info"),KDialogBase::Details|KDialogBase::Ok, KDialogBase::Ok,parent,name,true)
{
	bool checkboxResult;
	KMessageBox::createKMessageBox(this, TQMessageBox::Information,
                       boxLabel, keysList,TQString(),&checkboxResult, 0,errormessage);
}

KDetailedInfo::~KDetailedInfo()
{}

//#include "detailedconsole.moc"
