/***************************************************************************
                          keyservers.h  -  description
                             -------------------
    begin                : Tue Nov 26 2002
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

#ifndef KEYSERVERS_H
#define KEYSERVERS_H

#include <kdialogbase.h>
#include "keyserver.h"

class TDEListViewItem;
class KSimpleConfig;
class KProcIO;
class TDEProcess;

class searchRes;

class keyServer : public KDialogBase
{
        Q_OBJECT
  
public:
        keyServer(TQWidget *parent=0, const char *name=0,bool modal=false,bool autoClose=false);
        ~keyServer();
	keyServerWidget *page;

public slots:
        void abortImport();
	void abortExport();
        void abortSearch();
        void transferKeyID();
        void slotsearchread(KProcIO *p);
        void slotOk();
        void syncCombobox();
        void slotImport();
        void slotSearch();
        void slotimportresult(TDEProcess*);
	void slotexportresult(TDEProcess*);
        void slotimportread(KProcIO *p);
        void slotprocread(KProcIO *p);
        void slotPreExport();
	void slotExport(TQStringList keyIds);
        void preimport();
        void slotsearchresult(TDEProcess *);
        void slotEnableProxyI(bool on);
        void slotEnableProxyE(bool on);
        void handleQuit();
    	void slotTextChanged( const TQString &text);

private:

        TQDialog *importpop;
        KSimpleConfig *config;
        uint keyNumbers;
        TQString readmessage;
        KProcIO *importproc,*exportproc;
	KProcIO *searchproc;
        searchRes *listpop;
        int count;
        bool cycle,autoCloseWindow;
        TDEListViewItem *kitem;
	KDialogBase *dialogServer;
	
signals:
	void importFinished(TQString);
};

#endif // KEYSERVERS_H

