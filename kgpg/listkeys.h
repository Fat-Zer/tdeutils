/***************************************************************************
                          listkeys.h  -  description
                             -------------------
    begin                : Thu Jul 4 2002
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

#ifndef LISTKEYS_H
#define LISTKEYS_H

#include <kdialogbase.h>
#include <klistview.h>
#include <kmainwindow.h>
#include <klistviewsearchline.h>

#include <kactionclasses.h> 
#include <tqclipboard.h>

#include "dcopiface.h"

#include <tqptrlist.h>
#include <tqstringlist.h>
#include <kurl.h>

#include <tqcheckbox.h>
#include <kmainwindow.h>

class TQPushButton;
class TQPopupMenu;
class TQLabel;
class TQCheckbox;
class KStatusBar;
class KPassivePopup;
class KProcess;
class KProcIO;
class TQEvent;
class KTempFile;
class KgpgApp;
class keyServer;
class groupEdit;
class KgpgInterface;
class KSelectAction;

struct gpgKey
{
        TQString gpgkeymail;
        TQString gpgkeyname;
        TQString gpgkeyid;
        TQString gpgkeytrust;
        TQString gpgkeyvalidity;
        TQString gpgkeysize;
        TQString gpgkeycreation;
        TQString gpgkeyexpiration;
        TQString gpgkeyalgo;
        TQPixmap trustpic;
};

class KgpgSelKey : public KDialogBase
{
        Q_OBJECT
  TQ_OBJECT

public:
        KgpgSelKey( TQWidget *parent = 0, const char *name = 0,bool allowMultipleSelection=false, TQString preselected=TQString());
        KListView *keysListpr;
        TQPixmap keyPair;
        TQCheckBox *local;
        TQVBoxLayout *vbox;
        TQWidget *page;
private slots:
        void slotOk();
        void slotpreOk();
        void slotSelect(TQListViewItem *item);

public slots:
        TQString getkeyID();
        TQString getkeyMail();
};



class KeyView : public KListView
{
        Q_OBJECT
  TQ_OBJECT
        friend class listKeys;
public:
        KeyView( TQWidget *parent = 0, const char *name = 0);
        bool displayPhoto,displayOnlySecret,displayDisabled;
        int previewSize;
	TQString secretList;
	TQPixmap trustbad;
	
private:

        TQString orphanList;
        TQString photoKeysList;
        TQPixmap pixkeyPair, pixkeySingle, pixkeyGroup, pixsignature, pixuserid, pixuserphoto;
	TQPixmap trustunknown, trustrevoked, trustgood, pixRevoke, pixkeyOrphan;
        TQListViewItem *itemToOpen;
        KTempFile *kgpgphototmp;
        int groupNb;

public slots:
        void slotRemoveColumn(int d);
        void slotAddColumn(int c);

private slots:
	void refreshTrust(int color,TQColor newColor);
        void  droppedfile (KURL);
        void refreshkeylist();
        gpgKey extractKey(TQString keyColon);
        void expandKey(TQListViewItem *item);
        void expandGroup(TQListViewItem *item);
        void refreshcurrentkey(TQListViewItem *current);
        void refreshcurrentkey(TQString currentID);
        void refreshselfkey();
        void refreshgroups();
        void insertOrphanedKeys(TQStringList orpans);
        void insertOrphan(TQString currentID);
        TQPixmap slotGetPhoto(TQString photoId,bool mini=false);
        void slotReloadKeys(TQStringList keyIDs);
        void slotReloadOrphaned();

signals:
        void statusMessage(TQString,int,bool keep=false);

protected:
        virtual void startDrag();
        virtual void contentsDragMoveEvent(TQDragMoveEvent *e);
        virtual void  contentsDropEvent (TQDropEvent*);
};

class mySearchLine: public KListViewSearchLine
{
    Q_OBJECT
  TQ_OBJECT
public:
    mySearchLine(TQWidget *parent = 0, KeyView *listView = 0, const char *name = 0);
    virtual ~mySearchLine();
private:
 KeyView *searchListView;    
    
public slots:
virtual void updateSearch(const TQString &s = TQString());
protected:
virtual bool itemMatches(const TQListViewItem *item, const TQString & s)  const;
};


class listKeys : public KMainWindow, virtual public KeyInterface
{
        friend class KeyView;
        Q_OBJECT
  TQ_OBJECT

public:
        listKeys(TQWidget *parent=0, const char *name=0);
        ~listKeys();
        TQLabel *keyPhoto;
        KeyView *keysList2;
        TQPopupMenu *popup,*popupsec,*popupout,*popupsig,*popupgroup,*popupphoto,*popupuid,*popuporphan;
        TQString message;
        TQStringList keynames;
        KPassivePopup *pop;
        KToggleAction *sTrust,*sCreat,*sExpi,*sSize;
        KSelectAction *photoProps;
        KStatusBar *keyStatusBar;
	KgpgApp *s_kgpgEditor;

private:
        TQPushButton *bouton1,*bouton2,*bouton0;
        TQString tempKeyFile,newKeyMail,newKeyName,newkeyFinger,newkeyID;
	KListViewSearchLine* listViewSearch;	
        bool continueSearch;
        bool showPhoto;
        keyServer *kServer;
        KTempFile *kgpgtmp;
        KAction *importSignatureKey,*importAllSignKeys,*signKey,*refreshKey;
        TQPtrList<TQListViewItem> signList,keysList;
        uint globalCount,keyCount;
        int globalChecked;
        bool globalisLocal,showTipOfDay;
        TQString globalkeyMail,globalkeyID,searchString;
        long searchOptions;
        groupEdit *gEdit;
        KgpgInterface *revKeyProcess;
        KDialogBase *addUidWidget;
        TQClipboard::Mode clipboardMode;
        TQTimer *statusbarTimer;


protected:
        void closeEvent( TQCloseEvent * e );
        bool eventFilter( TQObject *, TQEvent *e );

public slots:
        void slotgenkey();
        void refreshkey();
        void readAllOptions();
        void showKeyInfo(TQString keyID);
        void findKey();
        void findFirstKey();
        void findNextKey();
        void slotSetDefaultKey(TQString newID);

private slots:
        void quitApp();
        void  slotOpenEditor();
        void changeMessage(TQString,int, bool keep=false);
        void statusBarTimeout();
        void slotShowTrust();
        void slotShowSize();
        void slotShowCreat();
        void slotShowExpi();
        void slotToggleSecret();
	void slotToggleDisabled();
        void slotGotoDefaultKey();
        void slotDelUid();
        void slotAddUid();
        void slotAddUidEnable(const TQString & name);
        void slotGpgError(TQString errortxt);
        void slotUpdatePhoto();
        void slotDeletePhoto();
        void slotAddPhoto();
        void slotSetPhotoSize(int size);
        void slotShowPhoto();
        void readgenprocess(KProcIO *p);
        void newKeyDone(KProcess *);
        void slotrevoke(TQString keyID,TQString revokeUrl,int reason,TQString description);
        void revokeWidget();
        void doFilePrint(TQString url);
        void doPrint(TQString txt);
        void checkList();
        void signLoop();
        void slotManpage();
        void slotTip();
        void showKeyServer();
	void showKeyManager();
        void slotReadFingerProcess(KProcIO *p);
        void slotProcessExportMail(TQString keys);
        void slotProcessExportClip(TQString keys);
        void readOptions();
        void genover(KProcess *p);
        void showOptions();
        void slotSetDefKey();
        void slotSetDefaultKey(TQListViewItem *newdef);
        void annule();
        void confirmdeletekey();
        void deletekey();
        void deleteseckey();
        void signkey();
        void delsignkey();
        void preimportsignkey();
        bool importRemoteKey(TQString keyID);
        void importsignkey(TQString importKeyId);
        void importallsignkey();
        void importfinished();
        void signatureResult(int success);
        void delsignatureResult(bool);
        void listsigns();
        void slotexport();
        void slotexportsec();
        void slotmenu(TQListViewItem *,const TQPoint &,int);
        void slotPreImportKey();
        void slotedit();
        void addToKAB();
        //	void allToKAB();
        void editGroup();
        void groupAdd();
        void groupRemove();
        void groupInit(TQStringList keysGroup);
        void groupChange();
        void createNewGroup();
        void deleteGroup();
        void slotImportRevoke(TQString url);
        void slotImportRevokeTxt(TQString revokeText);
        void refreshKeyFromServer();
        void refreshFinished();
        void slotregenerate();
        void reloadSecretKeys();
	void dcopImportFinished();
	
signals:
        void readAgainOptions();
        void certificate(TQString);
        void closeAsked();
        void fontChanged(TQFont);
	void encryptFiles(KURL::List);
	void installShredder();

};


#endif // LISTKEYS_H

