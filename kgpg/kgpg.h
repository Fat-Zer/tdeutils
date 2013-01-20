/***************************************************************************
                          kgpg.h  -  description
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

#ifndef KGPGAPPLET_H
#define KGPGAPPLET_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <kaction.h>
#include <ksystemtray.h>
#include <kuniqueapplication.h>
#include <kurl.h>
#include <kshortcut.h>

#include <tqlabel.h>
#include <tqstringlist.h>
#include <tqclipboard.h>


class TQPopupMenu;
class KTempFile;
class KAboutData;
class KPassivePopup;
class KgpgWizard;
class popupPublic;

class MyView : public TQLabel
{

        Q_OBJECT
  

public:
        MyView( TQWidget *parent = 0, const char *name = 0);
        ~MyView();

        KURL droppedUrl;
        KURL::List droppedUrls;
        KTempFile *kgpgfoldertmp;
	KShortcut goDefaultKey;
	TQClipboard::Mode clipboardMode;

private:
        TQPopupMenu *droppopup,*udroppopup;
        KAboutData   *_aboutData;
        TQStringList customDecrypt;
        KgpgWizard *wiz;
        KPassivePopup *pop;
        KTempFile *kgpgFolderExtract;
	int compressionScheme,openTasks;
	popupPublic *dialogue;

public slots:
	void busyMessage(TQString mssge,bool reset=false);
        void  encryptDroppedFile();
        void  decryptDroppedFile();
        void  slotVerifyFile();
        void  signDroppedFile();
        void  showDroppedFile ();
        void  clipDecrypt();
	void  clipSign(bool openEditor=true);
        void  clipEncrypt();
        void  shredDroppedFile();
        void encryptDroppedFolder();
        void startFolderEncode(TQStringList selec,TQStringList encryptOptions,bool ,bool symetric);
        void  slotFolderFinished(KURL);
        void  slotFolderFinishedError(TQString errmsge);
	void  encryptFiles(KURL::List urls);
	void installShred();

private slots:
        void  slotWizardClose();
        void  startWizard();
	void  slotWizardChange();
        void  slotSaveOptionsPath();
        void  slotGenKey();
        void importSignature(TQString ID);
        void slotSetClip(TQString newtxt);
	void slotPassiveClip();
        void encryptClipboard(TQStringList selec,TQStringList encryptOptions,bool,bool symmetric);
        void help();
        void about();
        void firstRun();
        void readOptions();
        void  droppedfile (KURL::List);
        void  droppedtext (TQString inputText, bool allowEncrypt=true);
        void  unArchive();
        void slotSetCompression(int cp);
	void  decryptNextFile();

protected:
        virtual void dragEnterEvent(TQDragEnterEvent *);
        virtual void  dropEvent (TQDropEvent*);

protected slots:


signals:
	void setFont(TQFont);
        void readAgain2();
	void createNewKey();
	void updateDefault(TQString);
	void importedKeys(TQStringList);
};

class kgpgapplet : public KSystemTray//KUniqueApplication
{
        Q_OBJECT
  

public:
        kgpgapplet( TQWidget *parent = 0, const char *name = 0);
        /** destructor */
        ~kgpgapplet();
        MyView *w;

private:
        KSystemTray *kgpgapp;
	KAction *KgpgEncryptClipboard, *KgpgDecryptClipboard, *KgpgSignClipboard;

private slots:
	void slotOpenKeyManager();
	void slotOpenServerDialog();
	void showOptions();
	void checkMenu();
};

class TDECmdLineArgs;

class KgpgAppletApp : public KUniqueApplication
{
        Q_OBJECT
  
        friend class kgpgapplet;
public:
        KgpgAppletApp();
        ~KgpgAppletApp();
        int newInstance ();
        KURL::List urlList;
        bool running;
	KShortcut goHome;

protected:
        TDECmdLineArgs *args;
private:
        kgpgapplet *kgpg_applet;
        class listKeys *s_keyManager;

private slots:
        void slotHandleQuit();
	void wizardOver(TQString defaultKeyId);
};

#endif // KGPGAPPLET_H

