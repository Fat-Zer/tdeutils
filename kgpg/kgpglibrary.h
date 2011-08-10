/***************************************************************************
                          kgpglibrary.h  -  description
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


#ifndef KGPGLIBRARY_H
#define KGPGLIBRARY_H

#include <tqobject.h>
#include <kurl.h>
#include <kshortcut.h>
#include <kio/job.h> 

class KPassivePopup;
class KProgress;

class KgpgLibrary : public TQObject
{

        Q_OBJECT
  TQ_OBJECT

public:
        /**
         * Initialize the class
         */
        KgpgLibrary(TQWidget *parent=0,bool pgpExtension=false);
        ~KgpgLibrary();

        KURL::List urlselecteds;

public slots:
        void slotFileEnc(KURL::List urls=KURL(""),TQStringList opts=TQString(),TQStringList defaultKey=TQString(),KShortcut goDefaultKey=TQKeySequence(CTRL+TQt::Key_Home));
        void slotFileDec(KURL srcUrl,KURL destUrl,TQStringList customDecryptOption=TQStringList());
	void shredprocessenc(KURL::List filesToShred);

private slots:
	void startencode(TQStringList encryptKeys,TQStringList encryptOptions,bool shred,bool symetric);
        void fastencode(KURL &fileToCrypt,TQStringList selec,TQStringList encryptOptions,bool symetric);
//        void startencode(TQString &selec,TQString encryptOptions,bool shred,bool symetric);
	void slotShredResult( KIO::Job * job );
	void shredpreprocessenc(KURL fileToShred);
        void processenc(KURL);
        void processdecover();
        void processdecerror(TQString mssge);
        void processencerror(TQString mssge);
        void processpopup(TQString fileName);
        void processpopup2(TQString fileName);

private:
        TQString customDecrypt,tempFile,extension;
        KURL urlselected;
        KPassivePopup *pop;
	KProgress *shredProgressBar;
	bool popIsActive;
	TQWidget *panel;
	TQStringList _encryptKeys;
	TQStringList _encryptOptions;
	bool _shred;
	bool _symetric;

signals:
        void decryptionOver();
	void importOver(TQStringList);
	void systemMessage(TQString,bool reset=false);
};

#endif // KGPGLIBRARY_H

