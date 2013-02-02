/***************************************************************************
                          listkeys.cpp  -  description
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

////////////////////////////////////////////////////// code for the key management

#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <tqdir.h>
#include <tqfile.h>
#include <tqlayout.h>
#include <tqvariant.h>
#include <tqregexp.h>
#include <tqpainter.h>
#include <tqvbox.h>
#include <tqclipboard.h>
#include <tqkeysequence.h>
#include <tqtextcodec.h>
#include <kstatusbar.h>
#include <tqtimer.h>
#include <tqpaintdevicemetrics.h>
#include <tqtooltip.h>
#include <tqheader.h>
#include <ktempfile.h>
#include <kdebug.h>
#include <kprocess.h>
#include <kprocio.h>
#include <tqwidget.h>
#include <tdeaction.h>
#include <tqcheckbox.h>
#include <tqlabel.h>
#include <tqtoolbutton.h>
#include <tqradiobutton.h>
#include <tqpopupmenu.h>

#include <kurlrequester.h>
#include <tdeio/netaccess.h>
#include <kurl.h>
#include <tdefiledialog.h>
#include <tdeshortcut.h>
#include <tdestdaccel.h>
#include <klocale.h>
#include <ktip.h>
#include <krun.h>
#include <kprinter.h>
#include <kurldrag.h>
#include <twin.h>
#include <dcopclient.h>
#include <klineedit.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kapplication.h>
#include <kabc/stdaddressbook.h>
#include <kabc/addresseedialog.h>
#include <kdesktopfile.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <tqcombobox.h>
#include <tqtabwidget.h>
#include <kinputdialog.h>
#include <kpassdlg.h>
#include <kpassivepopup.h>
#include <kfinddialog.h>
#include <kfind.h>
#include <dcopref.h>

#include "newkey.h"
#include "kgpg.h"
#include "kgpgeditor.h"
#include "kgpgview.h"
#include "listkeys.h"
#include "keyexport.h"
#include "sourceselect.h"
#include "adduid.h"
#include "groupedit.h"
#include "kgpgrevokewidget.h"
#include "keyservers.h"
#include "keyserver.h"
#include "kgpginterface.h"
#include "kgpgsettings.h"
#include "keygener.h"
#include "kgpgoptions.h"
#include "keyinfowidget.h"

//////////////  TDEListviewItem special

class UpdateViewItem : public TDEListViewItem
{
public:
        UpdateViewItem(TQListView *parent, TQString name,TQString email, TQString tr, TQString val, TQString size, TQString creat, TQString id,bool isdefault,bool isexpired);
	UpdateViewItem(TQListViewItem *parent=0, TQString name=TQString(),TQString email=TQString(), TQString tr=TQString(), TQString val=TQString(), TQString size=TQString(), TQString creat=TQString(), TQString id=TQString());
        virtual void paintCell(TQPainter *p, const TQColorGroup &cg,int col, int width, int align);
        virtual int compare(  TQListViewItem * item, int c, bool ascending ) const;
	virtual TQString key( int column, bool ) const;
        bool def,exp;
};

UpdateViewItem::UpdateViewItem(TQListView *parent, TQString name,TQString email, TQString tr, TQString val, TQString size, TQString creat, TQString id,bool isdefault,bool isexpired)
                : TDEListViewItem(parent)
{
        def=isdefault;
        exp=isexpired;
        setText(0,name);
        setText(1,email);
        setText(2,tr);
        setText(3,val);
        setText(4,size);
        setText(5,creat);
        setText(6,id);
}

UpdateViewItem::UpdateViewItem(TQListViewItem *parent, TQString name,TQString email, TQString tr, TQString val, TQString size, TQString creat, TQString id)
                : TDEListViewItem(parent)
{
        setText(0,name);
        setText(1,email);
        setText(2,tr);
        setText(3,val);
        setText(4,size);
        setText(5,creat);
        setText(6,id);
}


void UpdateViewItem::paintCell(TQPainter *p, const TQColorGroup &cg,int column, int width, int alignment)
{
        TQColorGroup _cg( cg );
	if (depth()==0)
	{
	if ((def) && (column<2)) {
                TQFont font(p->font());
                font.setBold(true);
                p->setFont(font);
        }
	else if ((exp) && (column==3)) _cg.setColor( TQColorGroup::Text, TQt::red );
	}
	else
        if (column<2) {
                TQFont font(p->font());
                font.setItalic(true);
                p->setFont(font);
        }


        TDEListViewItem::paintCell(p,_cg, column, width, alignment);
}

#include <iostream>
using namespace std;

int UpdateViewItem :: compare(  TQListViewItem * item, int c, bool ascending ) const
{
        int rc = 0;
        if ((c==3) || (c==5)) {
                TQDate d = TDEGlobal::locale()->readDate(text(c));
                TQDate itemDate = TDEGlobal::locale()->readDate(item->text(c));
                bool itemDateValid = itemDate.isValid();
                if (d.isValid()) {
                        if (itemDateValid) {
                                if (d < itemDate)
                                        rc = -1;
                                else if (d > itemDate)
                                        rc = 1;
                        } else
                                rc = -1;
                } else if (itemDateValid)
                        rc = 1;
        return rc;
	}
	if (c==2)   /* sorting by pixmap */
        {
                const TQPixmap* pix = pixmap(c);
                const TQPixmap* itemPix = item->pixmap(c);
                int serial,itemSerial;
                if (!pix)
                        serial=0;
                else
                        serial=pix->serialNumber();
                if (!itemPix)
                        itemSerial=0;
                else
                        itemSerial=itemPix->serialNumber();
                if (serial<itemSerial)
                        rc=-1;
                else if (serial>itemSerial)
                        rc=1;
		return rc;
        }
	return TQListViewItem::compare(item,c,ascending);
}

TQString UpdateViewItem::key( int column, bool ) const
{
    return text( column ).lower();
}


////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////   Secret key selection dialog, used when user wants to sign a key
KgpgSelKey::KgpgSelKey(TQWidget *parent, const char *name,bool allowMultipleSelection, TQString preselected):
KDialogBase( parent, name, true,i18n("Private Key List"),Ok | Cancel)
{
        TQString keyname;
        page = new TQWidget(this);
        TQLabel *labeltxt;
        KIconLoader *loader = TDEGlobal::iconLoader();
        keyPair=loader->loadIcon("kgpg_key2",KIcon::Small,20);

        setMinimumSize(350,100);
        keysListpr = new TDEListView( page );
        keysListpr->setRootIsDecorated(true);
        keysListpr->addColumn( i18n( "Name" ));
	keysListpr->addColumn( i18n( "Email" ));
	keysListpr->addColumn( i18n( "ID" ));
        keysListpr->setShowSortIndicator(true);
        keysListpr->setFullWidth(true);
	keysListpr->setAllColumnsShowFocus(true);
	if (allowMultipleSelection) keysListpr->setSelectionMode(TQListView::Extended);

        labeltxt=new TQLabel(i18n("Choose secret key:"),page);
        vbox=new TQVBoxLayout(page);

        if (preselected==TQString()) preselected = KGpgSettings::defaultKey();

        FILE *fp,*fp2;
        TQString fullname,tst,tst2;
        char line[300];

        bool selectedok=false;
	bool warn=false;
	TDEListViewItem *item;

        fp = popen("gpg --no-tty --with-colons --list-secret-keys", "r");
        while ( fgets( line, sizeof(line), fp)) {
                tst=TQString::fromUtf8(line);
                if (tst.startsWith("sec")) {
                        TQStringList keyString=TQStringList::split(":",tst,true);
                        TQString val=keyString[6];
                        TQString id=TQString("0x"+keyString[4].right(8));
                        if (val.isEmpty())
                                val=i18n("Unlimited");
                        fullname=keyString[9];

                        fp2 = popen(TQFile::encodeName(TQString("gpg --no-tty --with-colons --list-key %1").arg(KShellProcess::quote(id))), "r");
                        bool dead=true;
                        while ( fgets( line, sizeof(line), fp2)) {
                                tst2=TQString::fromUtf8(line);
                                if (tst2.startsWith("pub")) {
                         const TQString trust2=tst2.section(':',1,1);
                        switch( trust2[0] ) {
                        case 'f':
				dead=false;
                                break;
                        case 'u':
				dead=false;
                                break;
			case '-':
				if (tst2.section(':',11,11).find('D')==-1) warn=true;
				break;
                        default:
                                break;
                        }
                                        if (tst2.section(':',11,11).find('D')!=-1)
                                                dead=true;
					break;
                                }
                        }
                        pclose(fp2);
                        if (!fullname.isEmpty() && (!dead)) {
			TQString keyMail,keyName;
			if (fullname.find("<")!=-1) {
                	keyMail=fullname.section('<',-1,-1);
                	keyMail.truncate(keyMail.length()-1);
                	keyName=fullname.section('<',0,0);
			} else {
                	keyMail=TQString();
			keyName=fullname;
        		}

        		keyName=KgpgInterface::checkForUtf8(keyName);


                                item=new TDEListViewItem(keysListpr,keyName,keyMail,id);
                                //TDEListViewItem *sub= new TDEListViewItem(item,i18n("ID: %1, trust: %2, expiration: %3").arg(id).arg(trust).arg(val));
				TDEListViewItem *sub= new TDEListViewItem(item,i18n("Expiration:"),val);
                                sub->setSelectable(false);
                                item->setPixmap(0,keyPair);
                                if (preselected.find(id,0,false)!=-1) {
                                        keysListpr->setSelected(item,true);
					keysListpr->setCurrentItem(item);
                                        selectedok=true;
                                }
                        }
                }
        }
        pclose(fp);

	if (warn)
	{
	KMessageBox::information(this,i18n("<qt><b>Some of your secret keys are untrusted.</b><br>Change their trust if you want to use them for signing.</qt>"),TQString(),"warnUntrusted");
	}
        TQObject::connect(keysListpr,TQT_SIGNAL(doubleClicked(TQListViewItem *,const TQPoint &,int)),TQT_TQOBJECT(this),TQT_SLOT(slotpreOk()));
        TQObject::connect(keysListpr,TQT_SIGNAL(clicked(TQListViewItem *)),TQT_TQOBJECT(this),TQT_SLOT(slotSelect(TQListViewItem *)));


        if (!selectedok)
	{
                keysListpr->setSelected(keysListpr->firstChild(),true);
		keysListpr->setCurrentItem(keysListpr->firstChild());
	}

        vbox->addWidget(labeltxt);
        vbox->addWidget(keysListpr);
        setMainWidget(page);
}


void KgpgSelKey::slotpreOk()
{
        if (keysListpr->currentItem()->depth()!=0)
                return;
        else
                slotOk();
}

void KgpgSelKey::slotOk()
{
        if (keysListpr->currentItem()==NULL)
                reject();
        else
                accept();
}

void KgpgSelKey::slotSelect(TQListViewItem *item)
{
        if (item==NULL)
                return;
        if (item->depth()!=0) {
                keysListpr->setSelected(item->parent(),true);
                keysListpr->setCurrentItem(item->parent());
        }
}


TQString KgpgSelKey::getkeyID()
{
        /////  emit selected key
        if (keysListpr->currentItem()==NULL)
                return(TQString());
	TQString result;
	TQPtrList< TQListViewItem > list = keysListpr->selectedItems(false);
	TQListViewItem *item;
    	for ( item = list.first(); item; item = list.next() )
	{
	result.append(item->text(2));
	if (item!=list.getLast()) result.append(" ");
	}
        return(result);
}

TQString KgpgSelKey::getkeyMail()
{
        TQString username;
        /////  emit selected key
        if (keysListpr->currentItem()==NULL)
                return(TQString());
        else {
                username=keysListpr->currentItem()->text(0);
                //username=username.section(' ',0,0);
                username=username.stripWhiteSpace();
                return(username);
        }
}



/////////////////////////////////////////////////////////////////////////////////////////////

KeyView::KeyView( TQWidget *parent, const char *name )
                : TDEListView( parent, name )
{
        KIconLoader *loader = TDEGlobal::iconLoader();

        pixkeyOrphan=loader->loadIcon("kgpg_key4",KIcon::Small,20);
        pixkeyGroup=loader->loadIcon("kgpg_key3",KIcon::Small,20);
        pixkeyPair=loader->loadIcon("kgpg_key2",KIcon::Small,20);
        pixkeySingle=loader->loadIcon("kgpg_key1",KIcon::Small,20);
        pixsignature=loader->loadIcon("signature",KIcon::Small,20);
        pixuserid=loader->loadIcon("kgpg_identity",KIcon::Small,20);
        pixuserphoto=loader->loadIcon("kgpg_photo",KIcon::Small,20);
        pixRevoke=loader->loadIcon("stop",KIcon::Small,20);
	TQPixmap blankFrame;
	blankFrame.load(locate("appdata", "pics/kgpg_blank.png"));

	trustunknown.load(locate("appdata", "pics/kgpg_fill.png"));
	trustunknown.fill(KGpgSettings::colorUnknown());
	bitBlt(&trustunknown,0,0,&blankFrame,0,0,50,15);

	trustbad.load(locate("appdata", "pics/kgpg_fill.png"));
	trustbad.fill(KGpgSettings::colorBad());//TQColor(172,0,0));
	bitBlt(&trustbad,0,0,&blankFrame,0,0,50,15);

	trustrevoked.load(locate("appdata", "pics/kgpg_fill.png"));
	trustrevoked.fill(KGpgSettings::colorRev());//TQColor(30,30,30));
	bitBlt(&trustrevoked,0,0,&blankFrame,0,0,50,15);

	trustgood.load(locate("appdata", "pics/kgpg_fill.png"));
	trustgood.fill(KGpgSettings::colorGood());//TQColor(144,255,0));
	bitBlt(&trustgood,0,0,&blankFrame,0,0,50,15);

        connect(this,TQT_SIGNAL(expanded (TQListViewItem *)),TQT_TQOBJECT(this),TQT_SLOT(expandKey(TQListViewItem *)));
        header()->setMovingEnabled(false);
        setAcceptDrops(true);
        setDragEnabled(true);
}



void  KeyView::droppedfile (KURL url)
{
        if (KMessageBox::questionYesNo(this,i18n("<p>Do you want to import file <b>%1</b> into your key ring?</p>").arg(url.path()), TQString(), i18n("Import"), i18n("Do Not Import"))!=KMessageBox::Yes)
                return;

        KgpgInterface *importKeyProcess=new KgpgInterface();
        importKeyProcess->importKeyURL(url);
        connect(importKeyProcess,TQT_SIGNAL(importfinished(TQStringList)),TQT_TQOBJECT(this),TQT_SLOT(slotReloadKeys(TQStringList)));
}

void KeyView::contentsDragMoveEvent(TQDragMoveEvent *e)
{
        e->accept (KURLDrag::canDecode(e));
}

void  KeyView::contentsDropEvent (TQDropEvent *o)
{
        KURL::List list;
        if ( KURLDrag::decode( o, list ) )
                droppedfile(list.first());
}

void  KeyView::startDrag()
{
        FILE *fp;
        char line[200]="";
        TQString keyid=currentItem()->text(6);
        if (!keyid.startsWith("0x"))
                return;
        TQString gpgcmd="gpg --display-charset=utf-8 --no-tty --export --armor "+KShellProcess::quote(keyid.local8Bit());

        TQString keytxt;
        fp=popen(TQFile::encodeName(gpgcmd),"r");
        while ( fgets( line, sizeof(line), fp))    /// read output
                if (!TQString(line).startsWith("gpg:"))
                        keytxt+=TQString::fromUtf8(line);
        pclose(fp);

        TQDragObject *d = new TQTextDrag( keytxt, this );
        d->dragCopy();
        // do NOT delete d.
}


mySearchLine::mySearchLine(TQWidget *parent, KeyView *listView, const char *name)
:TDEListViewSearchLine(parent,listView,name)
{
searchListView=listView;
setKeepParentsVisible(false);
}

mySearchLine::~ mySearchLine()
{}


bool mySearchLine::itemMatches(const TQListViewItem *item, const TQString & s) const
{
if (item->depth()!=0) return true;
else return TDEListViewSearchLine::itemMatches(item,s);
}



void mySearchLine::updateSearch(const TQString& s)
{
    TDEListViewSearchLine::updateSearch(s);
    if (searchListView->displayOnlySecret || !searchListView->displayDisabled)
    {
    int disabledSerial=searchListView->trustbad.serialNumber();
	TQListViewItem *item=searchListView->firstChild();
	while (item)
	{
	    if (item->isVisible() && !(item->text(6).isEmpty()))
	    {
		if (searchListView->displayOnlySecret && searchListView->secretList.find(item->text(6))==-1)
                    item->setVisible(false);
		if (!searchListView->displayDisabled && item->pixmap(2))
		    if (item->pixmap(2)->serialNumber()==disabledSerial)
                        item->setVisible(false);
            }
	 item=item->nextSibling();
	 }
    }
}

///////////////////////////////////////////////////////////////////////////////////////   main window for key management

listKeys::listKeys(TQWidget *parent, const char *name) : DCOPObject( "KeyInterface" ), TDEMainWindow(parent, name,0)
{
        //KWin::setType(TQt::WDestructiveClose);

        keysList2 = new KeyView(this);
        keysList2->photoKeysList=TQString();
        keysList2->groupNb=0;
	keyStatusBar=NULL;
        readOptions();

        if (showTipOfDay)
                installEventFilter(this);
        setCaption(i18n("Key Management"));

        (void) new TDEAction(i18n("&Open Editor"), "edit",0,TQT_TQOBJECT(this), TQT_SLOT(slotOpenEditor()),actionCollection(),"kgpg_editor");
        TDEAction *exportPublicKey = new TDEAction(i18n("E&xport Public Keys..."), "kgpg_export", TDEStdAccel::shortcut(TDEStdAccel::Copy),TQT_TQOBJECT(this), TQT_SLOT(slotexport()),actionCollection(),"key_export");
        TDEAction *deleteKey = new TDEAction(i18n("&Delete Keys"),"editdelete", TQt::Key_Delete,TQT_TQOBJECT(this), TQT_SLOT(confirmdeletekey()),actionCollection(),"key_delete");
        signKey = new TDEAction(i18n("&Sign Keys..."), "kgpg_sign", 0,TQT_TQOBJECT(this), TQT_SLOT(signkey()),actionCollection(),"key_sign");
        TDEAction *delSignKey = new TDEAction(i18n("Delete Sign&ature"),"editdelete", 0,TQT_TQOBJECT(this), TQT_SLOT(delsignkey()),actionCollection(),"key_delsign");
        TDEAction *infoKey = new TDEAction(i18n("&Edit Key"), "kgpg_info", TQt::Key_Return,TQT_TQOBJECT(this), TQT_SLOT(listsigns()),actionCollection(),"key_info");
        TDEAction *importKey = new TDEAction(i18n("&Import Key..."), "kgpg_import", TDEStdAccel::shortcut(TDEStdAccel::Paste),TQT_TQOBJECT(this), TQT_SLOT(slotPreImportKey()),actionCollection(),"key_import");
        TDEAction *setDefaultKey = new TDEAction(i18n("Set as De&fault Key"),0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotSetDefKey()),actionCollection(),"key_default");
        importSignatureKey = new TDEAction(i18n("Import Key From Keyserver"),"network", 0,TQT_TQOBJECT(this), TQT_SLOT(preimportsignkey()),actionCollection(),"key_importsign");
        importAllSignKeys = new TDEAction(i18n("Import &Missing Signatures From Keyserver"),"network", 0,TQT_TQOBJECT(this), TQT_SLOT(importallsignkey()),actionCollection(),"key_importallsign");
        refreshKey = new TDEAction(i18n("&Refresh Keys From Keyserver"),"reload", 0,TQT_TQOBJECT(this), TQT_SLOT(refreshKeyFromServer()),actionCollection(),"key_server_refresh");

	TDEAction *createGroup=new TDEAction(i18n("&Create Group with Selected Keys..."), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(createNewGroup()),actionCollection(),"create_group");
        TDEAction *delGroup= new TDEAction(i18n("&Delete Group"), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(deleteGroup()),actionCollection(),"delete_group");
        TDEAction *editCurrentGroup= new TDEAction(i18n("&Edit Group"), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(editGroup()),actionCollection(),"edit_group");

	TDEAction *newContact=new TDEAction(i18n("&Create New Contact in Address Book"), "kaddressbook", 0,TQT_TQOBJECT(this), TQT_SLOT(addToKAB()),actionCollection(),"add_kab");
        (void) new TDEAction(i18n("&Go to Default Key"), "gohome",TQKeySequence(CTRL+TQt::Key_Home) ,TQT_TQOBJECT(this), TQT_SLOT(slotGotoDefaultKey()),actionCollection(),"go_default_key");

        KStdAction::quit(TQT_TQOBJECT(this), TQT_SLOT(quitApp()), actionCollection());
        KStdAction::find(TQT_TQOBJECT(this), TQT_SLOT(findKey()), actionCollection());
        KStdAction::findNext(TQT_TQOBJECT(this), TQT_SLOT(findNextKey()), actionCollection());
        (void) new TDEAction(i18n("&Refresh List"), "reload", TDEStdAccel::reload(),TQT_TQOBJECT(this), TQT_SLOT(refreshkey()),actionCollection(),"key_refresh");
        TDEAction *openPhoto= new TDEAction(i18n("&Open Photo"), "image", 0,TQT_TQOBJECT(this), TQT_SLOT(slotShowPhoto()),actionCollection(),"key_photo");
        TDEAction *deletePhoto= new TDEAction(i18n("&Delete Photo"), "delete", 0,TQT_TQOBJECT(this), TQT_SLOT(slotDeletePhoto()),actionCollection(),"delete_photo");
        TDEAction *addPhoto= new TDEAction(i18n("&Add Photo"), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotAddPhoto()),actionCollection(),"add_photo");

        TDEAction *addUid= new TDEAction(i18n("&Add User Id"), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotAddUid()),actionCollection(),"add_uid");
        TDEAction *delUid= new TDEAction(i18n("&Delete User Id"), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotDelUid()),actionCollection(),"del_uid");

        TDEAction *editKey = new TDEAction(i18n("Edit Key in &Terminal"), "kgpg_term", TQKeySequence(ALT+TQt::Key_Return),TQT_TQOBJECT(this), TQT_SLOT(slotedit()),actionCollection(),"key_edit");
        TDEAction *exportSecretKey = new TDEAction(i18n("Export Secret Key..."), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotexportsec()),actionCollection(),"key_sexport");
        TDEAction *revokeKey = new TDEAction(i18n("Revoke Key..."), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(revokeWidget()),actionCollection(),"key_revoke");

        TDEAction *deleteKeyPair = new TDEAction(i18n("Delete Key Pair"), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(deleteseckey()),actionCollection(),"key_pdelete");
        TDEAction *generateKey = new TDEAction(i18n("&Generate Key Pair..."), "kgpg_gen", TDEStdAccel::shortcut(TDEStdAccel::New),TQT_TQOBJECT(this), TQT_SLOT(slotgenkey()),actionCollection(),"key_gener");

        TDEAction *regeneratePublic = new TDEAction(i18n("&Regenerate Public Key"), 0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotregenerate()),actionCollection(),"key_regener");

        (void) new TDEAction(i18n("&Key Server Dialog"), "network", 0,TQT_TQOBJECT(this), TQT_SLOT(showKeyServer()),actionCollection(),"key_server");
        KStdAction::preferences(TQT_TQOBJECT(this), TQT_SLOT(showOptions()), actionCollection(),"options_configure");
        (void) new TDEAction(i18n("Tip of the &Day"), "idea", 0,TQT_TQOBJECT(this), TQT_SLOT(slotTip()), actionCollection(),"help_tipofday");
        (void) new TDEAction(i18n("View GnuPG Manual"), "contents", 0,TQT_TQOBJECT(this), TQT_SLOT(slotManpage()),actionCollection(),"gpg_man");

        (void) new TDEToggleAction(i18n("&Show only Secret Keys"), "kgpg_show", 0,TQT_TQOBJECT(this), TQT_SLOT(slotToggleSecret()),actionCollection(),"show_secret");
        keysList2->displayOnlySecret=false;

	(void) new TDEToggleAction(i18n("&Hide Expired/Disabled Keys"),0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotToggleDisabled()),actionCollection(),"hide_disabled");
	keysList2->displayDisabled=true;

        sTrust=new TDEToggleAction(i18n("Trust"),0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotShowTrust()),actionCollection(),"show_trust");
        sSize=new TDEToggleAction(i18n("Size"),0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotShowSize()),actionCollection(),"show_size");
        sCreat=new TDEToggleAction(i18n("Creation"),0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotShowCreat()),actionCollection(),"show_creat");
        sExpi=new TDEToggleAction(i18n("Expiration"),0, 0,TQT_TQOBJECT(this), TQT_SLOT(slotShowExpi()),actionCollection(),"show_expi");


        photoProps = new TDESelectAction(i18n("&Photo ID's"),"kgpg_photo", actionCollection(), "photo_settings");
        connect(photoProps, TQT_SIGNAL(activated(int)), this, TQT_SLOT(slotSetPhotoSize(int)));

        // Keep the list in kgpg.kcfg in sync with this one!
        TQStringList list;
        list.append(i18n("Disable"));
        list.append(i18n("Small"));
        list.append(i18n("Medium"));
        list.append(i18n("Large"));
        photoProps->setItems(list);

        int pSize = KGpgSettings::photoProperties();
        photoProps->setCurrentItem( pSize );
        slotSetPhotoSize(pSize);

        keysList2->setRootIsDecorated(true);
        keysList2->addColumn( i18n( "Name" ),200);
        keysList2->addColumn( i18n( "Email" ),200);
        keysList2->addColumn( i18n( "Trust" ),60);
        keysList2->addColumn( i18n( "Expiration" ),100);
        keysList2->addColumn( i18n( "Size" ),100);
        keysList2->addColumn( i18n( "Creation" ),100);
        keysList2->addColumn( i18n( "Id" ),100);
        keysList2->setShowSortIndicator(true);
        keysList2->setAllColumnsShowFocus(true);
        keysList2->setFullWidth(true);
        keysList2->setAcceptDrops (true) ;
        keysList2->setSelectionModeExt(TDEListView::Extended);


        popup=new TQPopupMenu();
        exportPublicKey->plug(popup);
        deleteKey->plug(popup);
        signKey->plug(popup);
        infoKey->plug(popup);
        editKey->plug(popup);
        refreshKey->plug(popup);
        setDefaultKey->plug(popup);
        popup->insertSeparator();
        importAllSignKeys->plug(popup);

        popupsec=new TQPopupMenu();
        exportPublicKey->plug(popupsec);
        signKey->plug(popupsec);
        infoKey->plug(popupsec);
        editKey->plug(popupsec);
        refreshKey->plug(popupsec);
        setDefaultKey->plug(popupsec);
        popupsec->insertSeparator();
        importAllSignKeys->plug(popupsec);
        popupsec->insertSeparator();
        addPhoto->plug(popupsec);
        addUid->plug(popupsec);
        exportSecretKey->plug(popupsec);
        deleteKeyPair->plug(popupsec);
        revokeKey->plug(popupsec);

        popupgroup=new TQPopupMenu();
        editCurrentGroup->plug(popupgroup);
        delGroup->plug(popupgroup);

        popupout=new TQPopupMenu();
        importKey->plug(popupout);
        generateKey->plug(popupout);

        popupsig=new TQPopupMenu();
        importSignatureKey->plug(popupsig);
        delSignKey->plug(popupsig);

        popupphoto=new TQPopupMenu();
        openPhoto->plug(popupphoto);
        deletePhoto->plug(popupphoto);

        popupuid=new TQPopupMenu();
        delUid->plug(popupuid);

        popuporphan=new TQPopupMenu();
        regeneratePublic->plug(popuporphan);
        deleteKeyPair->plug(popuporphan);

	editCurrentGroup->setEnabled(false);
	delGroup->setEnabled(false);
	createGroup->setEnabled(false);
	infoKey->setEnabled(false);
	editKey->setEnabled(false);
	signKey->setEnabled(false);
	refreshKey->setEnabled(false);
	exportPublicKey->setEnabled(false);
	newContact->setEnabled(false);

        setCentralWidget(keysList2);
        keysList2->restoreLayout(TDEGlobal::config(), "KeyView");

        TQObject::connect(keysList2,TQT_SIGNAL(returnPressed(TQListViewItem *)),TQT_TQOBJECT(this),TQT_SLOT(listsigns()));
        TQObject::connect(keysList2,TQT_SIGNAL(doubleClicked(TQListViewItem *,const TQPoint &,int)),TQT_TQOBJECT(this),TQT_SLOT(listsigns()));
        TQObject::connect(keysList2,TQT_SIGNAL(selectionChanged ()),TQT_TQOBJECT(this),TQT_SLOT(checkList()));
        TQObject::connect(keysList2,TQT_SIGNAL(contextMenuRequested(TQListViewItem *,const TQPoint &,int)),
                         this,TQT_SLOT(slotmenu(TQListViewItem *,const TQPoint &,int)));
        TQObject::connect(keysList2,TQT_SIGNAL(destroyed()),TQT_TQOBJECT(this),TQT_SLOT(annule()));


        ///////////////    get all keys data
	keyStatusBar=statusBar();

	setupGUI(TDEMainWindow::Create | Save | ToolBar | StatusBar | Keys, "listkeys.rc");
        toolBar()->insertLineSeparator();

	TQToolButton *clearSearch = new TQToolButton(toolBar());
	clearSearch->setTextLabel(i18n("Clear Search"), true);
	clearSearch->setIconSet(SmallIconSet(TQApplication::reverseLayout() ? "clear_left"
                                            : "locationbar_erase"));
	(void) new TQLabel(i18n("Search: "),toolBar());
	listViewSearch = new mySearchLine(toolBar(),keysList2);
	connect(clearSearch, TQT_SIGNAL(pressed()), TQT_TQOBJECT(listViewSearch), TQT_SLOT(clear()));


	(void)new TDEAction(i18n("Filter Search"), TQt::Key_F6, TQT_TQOBJECT(listViewSearch), TQT_SLOT(setFocus()),actionCollection(), "search_focus");

        sTrust->setChecked(KGpgSettings::showTrust());
        sSize->setChecked(KGpgSettings::showSize());
        sCreat->setChecked(KGpgSettings::showCreat());
        sExpi->setChecked(KGpgSettings::showExpi());

        statusbarTimer = new TQTimer(this);

        keyStatusBar->insertItem("",0,1);
        keyStatusBar->insertFixedItem(i18n("00000 Keys, 000 Groups"),1,true);
        keyStatusBar->setItemAlignment(0, AlignLeft);
        keyStatusBar->changeItem("",1);
        TQObject::connect(keysList2,TQT_SIGNAL(statusMessage(TQString,int,bool)),TQT_TQOBJECT(this),TQT_SLOT(changeMessage(TQString,int,bool)));
        TQObject::connect(statusbarTimer,TQT_SIGNAL(timeout()),TQT_TQOBJECT(this),TQT_SLOT(statusBarTimeout()));

	s_kgpgEditor= new KgpgApp(parent, "editor",WType_Dialog,actionCollection()->action("go_default_key")->shortcut(),true);
        connect(s_kgpgEditor,TQT_SIGNAL(refreshImported(TQStringList)),keysList2,TQT_SLOT(slotReloadKeys(TQStringList)));
        connect(this,TQT_SIGNAL(fontChanged(TQFont)),s_kgpgEditor,TQT_SLOT(slotSetFont(TQFont)));
        connect(s_kgpgEditor->view->editor,TQT_SIGNAL(refreshImported(TQStringList)),keysList2,TQT_SLOT(slotReloadKeys(TQStringList)));
}


listKeys::~listKeys()
{}

void  listKeys::showKeyManager()
{
show();
}

void  listKeys::slotOpenEditor()
{
  KgpgApp *kgpgtxtedit = new KgpgApp(this, "editor",WType_TopLevel | WDestructiveClose,actionCollection()->action("go_default_key")->shortcut());
        connect(kgpgtxtedit,TQT_SIGNAL(refreshImported(TQStringList)),keysList2,TQT_SLOT(slotReloadKeys(TQStringList)));
	connect(kgpgtxtedit,TQT_SIGNAL(encryptFiles(KURL::List)),TQT_TQOBJECT(this),TQT_SIGNAL(encryptFiles(KURL::List)));
        connect(this,TQT_SIGNAL(fontChanged(TQFont)),kgpgtxtedit,TQT_SLOT(slotSetFont(TQFont)));
        connect(kgpgtxtedit->view->editor,TQT_SIGNAL(refreshImported(TQStringList)),keysList2,TQT_SLOT(slotReloadKeys(TQStringList)));
        kgpgtxtedit->show();
}

void listKeys::statusBarTimeout()
{
        keyStatusBar->changeItem("",0);
}

void listKeys::changeMessage(TQString msg, int nb, bool keep)
{
        statusbarTimer->stop();
        if ((nb==0) & (!keep))
                statusbarTimer->start(10000, true);
        keyStatusBar->changeItem(" "+msg+" ",nb);
}


void KeyView::slotRemoveColumn(int d)
{
        hideColumn(d);
        header()->setResizeEnabled(false,d);
        header()->setStretchEnabled(true,6);
}

void KeyView::slotAddColumn(int c)
{
        header()->setResizeEnabled(true,c);
        adjustColumn(c);
}

void listKeys::slotShowTrust()
{
        if (sTrust->isChecked())
                keysList2->slotAddColumn(2);
        else
                keysList2->slotRemoveColumn(2);
}

void listKeys::slotShowExpi()
{
        if (sExpi->isChecked())
                keysList2->slotAddColumn(3);
        else
                keysList2->slotRemoveColumn(3);
}

void listKeys::slotShowSize()
{
        if (sSize->isChecked())
                keysList2->slotAddColumn(4);
        else
                keysList2->slotRemoveColumn(4);
}

void listKeys::slotShowCreat()
{
        if (sCreat->isChecked())
                keysList2->slotAddColumn(5);
        else
                keysList2->slotRemoveColumn(5);
}


bool listKeys::eventFilter( TQObject *, TQEvent *e )
{
        if ((e->type() == TQEvent::Show) && (showTipOfDay)) {
                KTipDialog::showTip(this, TQString("kgpg/tips"), false);
                showTipOfDay=false;
        }
        return FALSE;
}


void listKeys::slotToggleSecret()
{
        TQListViewItem *item=keysList2->firstChild();
        if (!item)
                return;

        keysList2->displayOnlySecret=!keysList2->displayOnlySecret;
	listViewSearch->updateSearch(listViewSearch->text());
}

void listKeys::slotToggleDisabled()
{
       TQListViewItem *item=keysList2->firstChild();
        if (!item)
                return;

        keysList2->displayDisabled=!keysList2->displayDisabled;
	listViewSearch->updateSearch(listViewSearch->text());
}

void listKeys::slotGotoDefaultKey()
{
        TQListViewItem *myDefaulKey = keysList2->findItem(KGpgSettings::defaultKey(),6);
        keysList2->clearSelection();
        keysList2->setCurrentItem(myDefaulKey);
        keysList2->setSelected(myDefaulKey,true);
        keysList2->ensureItemVisible(myDefaulKey);
}



void listKeys::refreshKeyFromServer()
{
        if (keysList2->currentItem()==NULL)
                return;
        TQString keyIDS;
        keysList=keysList2->selectedItems();
        bool keyDepth=true;
        for ( uint i = 0; i < keysList.count(); ++i )
                if ( keysList.at(i) ) {
                        if ((keysList.at(i)->depth()!=0) || (keysList.at(i)->text(6).isEmpty()))
                                keyDepth=false;
                        else
                                keyIDS+=keysList.at(i)->text(6)+" ";
                }
        if (!keyDepth) {
                KMessageBox::sorry(this,i18n("You can only refresh primary keys. Please check your selection."));
                return;
        }
        kServer=new keyServer(0,"server_dialog",false);
        kServer->page->kLEimportid->setText(keyIDS);
        kServer->slotImport();
        connect( kServer, TQT_SIGNAL( importFinished(TQString) ) , this, TQT_SLOT(refreshFinished()));
}


void listKeys::refreshFinished()
{
        if (kServer)
                kServer=0L;

        for ( uint i = 0; i < keysList.count(); ++i )
                if (keysList.at(i))
                        keysList2->refreshcurrentkey(keysList.at(i));
}


void listKeys::slotDelUid()
{
        TQListViewItem *item=keysList2->currentItem();
        while (item->depth()>0)
                item=item->parent();

        TDEProcess *conprocess=new TDEProcess();
        TDEConfig *config = TDEGlobal::config();
        config->setGroup("General");
        *conprocess<< config->readPathEntry("TerminalApplication","konsole");
        *conprocess<<"-e"<<"gpg";
        *conprocess<<"--edit-key"<<item->text(6)<<"uid";
        conprocess->start(TDEProcess::Block);
        keysList2->refreshselfkey();
}


void listKeys::slotregenerate()
{
        FILE *fp;
        TQString tst;
        char line[300];
        TQString cmd="gpg --display-charset=utf-8 --no-secmem-warning --export-secret-key "+keysList2->currentItem()->text(6)+" | gpgsplit --no-split --secret-to-public | gpg --import";

        fp = popen(TQFile::encodeName(cmd), "r");
        while ( fgets( line, sizeof(line), fp)) {
                tst+=TQString::fromUtf8(line);
        }
        pclose(fp);
        TQString regID=keysList2->currentItem()->text(6);
        keysList2->takeItem(keysList2->currentItem());
        keysList2->refreshcurrentkey(regID);
}

void listKeys::slotAddUid()
{
        addUidWidget=new KDialogBase(KDialogBase::Swallow, i18n("Add New User Id"),  KDialogBase::Ok | KDialogBase::Cancel,KDialogBase::Ok,this,0,true);
        addUidWidget->enableButtonOK(false);
        AddUid *keyUid=new AddUid();
        addUidWidget->setMainWidget(keyUid);
        //keyUid->setMinimumSize(keyUid->sizeHint());
        keyUid->setMinimumWidth(300);
        connect(keyUid->kLineEdit1,TQT_SIGNAL(textChanged ( const TQString & )),TQT_TQOBJECT(this),TQT_SLOT(slotAddUidEnable(const TQString & )));
        if (addUidWidget->exec()!=TQDialog::Accepted)
                return;
        KgpgInterface *addUidProcess=new KgpgInterface();
        addUidProcess->KgpgAddUid(keysList2->currentItem()->text(6),keyUid->kLineEdit1->text(),keyUid->kLineEdit2->text(),keyUid->kLineEdit3->text());
        connect(addUidProcess,TQT_SIGNAL(addUidFinished()),keysList2,TQT_SLOT(refreshselfkey()));
        connect(addUidProcess,TQT_SIGNAL(addUidError(TQString)),TQT_TQOBJECT(this),TQT_SLOT(slotGpgError(TQString)));
}

void listKeys::slotAddUidEnable(const TQString & name)
{
        addUidWidget->enableButtonOK(name.length()>4);
}


void listKeys::slotAddPhoto()
{
        TQString mess=i18n("The image must be a JPEG file. Remember that the image is stored within your public key."
                          "If you use a very large picture, your key will become very large as well! Keeping the image "
                          "close to 240x288 is a good size to use.");

        if (KMessageBox::warningContinueCancel(this,mess)!=KMessageBox::Continue)
                return;

        TQString imagePath=KFileDialog::getOpenFileName (TQString(),"image/jpeg",this);
        if (imagePath.isEmpty())
                return;
        KgpgInterface *addPhotoProcess=new KgpgInterface();
        addPhotoProcess->KgpgAddPhoto(keysList2->currentItem()->text(6),imagePath);
        connect(addPhotoProcess,TQT_SIGNAL(addPhotoFinished()),TQT_TQOBJECT(this),TQT_SLOT(slotUpdatePhoto()));
        connect(addPhotoProcess,TQT_SIGNAL(addPhotoError(TQString)),TQT_TQOBJECT(this),TQT_SLOT(slotGpgError(TQString)));
}

void listKeys::slotGpgError(TQString errortxt)
{
        KMessageBox::detailedSorry(this,i18n("Something unexpected happened during the requested operation.\nPlease check details for full log output."),errortxt);
}


void listKeys::slotDeletePhoto()
{
        if (KMessageBox::warningContinueCancel(this,i18n("<qt>Are you sure you want to delete Photo id <b>%1</b><br>from key <b>%2 &lt;%3&gt;</b> ?</qt>").arg(keysList2->currentItem()->text(6)).arg(keysList2->currentItem()->parent()->text(0)).arg(keysList2->currentItem()->parent()->text(1)),i18n("Warning"),KGuiItem(i18n("Delete"),"editdelete"))!=KMessageBox::Continue)
                return;

        KgpgInterface *delPhotoProcess=new KgpgInterface();
        delPhotoProcess->KgpgDeletePhoto(keysList2->currentItem()->parent()->text(6),keysList2->currentItem()->text(6));
        connect(delPhotoProcess,TQT_SIGNAL(delPhotoFinished()),TQT_TQOBJECT(this),TQT_SLOT(slotUpdatePhoto()));
        connect(delPhotoProcess,TQT_SIGNAL(delPhotoError(TQString)),TQT_TQOBJECT(this),TQT_SLOT(slotGpgError(TQString)));
}

void listKeys::slotUpdatePhoto()
{
        keysList2->refreshselfkey();
}


void listKeys::slotSetPhotoSize(int size)
{
        switch( size) {
        case 1:
                showPhoto=true;
                keysList2->previewSize=22;
                break;
        case 2:
                showPhoto=true;
                keysList2->previewSize=42;
                break;
        case 3:
                showPhoto=true;
                keysList2->previewSize=65;
                break;
        default:
                showPhoto=false;
                break;
        }
        keysList2->displayPhoto=showPhoto;

        /////////////////////////////   refresh keys with photo id

        TQListViewItem *newdef = keysList2->firstChild();
        while (newdef) {
                //if ((keysList2->photoKeysList.find(newdef->text(6))!=-1) && (newdef->childCount ()>0))
                if (newdef->childCount ()>0) {
                        bool hasphoto=false;
                        TQListViewItem *newdefChild = newdef->firstChild();
                        while (newdefChild) {
                                if (newdefChild->text(0)==i18n("Photo id")) {
                                        hasphoto=true;
                                        break;
                                }
                                newdefChild = newdefChild->nextSibling();
                        }
                        if (hasphoto) {
                                while (newdef->firstChild())
                                        delete newdef->firstChild();
                                keysList2->expandKey(newdef);
                        }
                }
                newdef = newdef->nextSibling();
        }
}

void listKeys::findKey()
{
        KFindDialog fd(this,"find_dialog",0,"");
        if ( fd.exec() != TQDialog::Accepted )
                return;
        searchString=fd.pattern();
        searchOptions=fd.options();
        findFirstKey();
}

void listKeys::findFirstKey()
{
        if (searchString.isEmpty())
                return;
        bool foundItem=true;
        TQListViewItem *item=keysList2->firstChild();
        if (!item)
                return;
        TQString searchText=item->text(0)+" "+item->text(1)+" "+item->text(6);


        //
        KFind *m_find = new KFind(searchString, searchOptions,this);
        m_find->setData(searchText);
        while (m_find->find()==KFind::NoMatch) {
                if (!item->nextSibling()) {
                        foundItem=false;
                        break;
                } else {
                        item = item->nextSibling();
                        searchText=item->text(0)+" "+item->text(1)+" "+item->text(6);
                        m_find->setData(searchText);
                }
        }
        delete m_find;

        if (foundItem) {

                keysList2->clearSelection();
                keysList2->setCurrentItem(item);
                keysList2->setSelected(item,true);
                keysList2->ensureItemVisible(item);
        } else
                KMessageBox::sorry(this,i18n("<qt>Search string '<b>%1</b>' not found.").arg(searchString));
}

void listKeys::findNextKey()
{
				//kdDebug(2100)<<"find next"<<endl;
        if (searchString.isEmpty()) {
                findKey();
                return;
        }
        bool foundItem=true;
        TQListViewItem *item=keysList2->currentItem();
        if (!item)
                return;
        while(item->depth() > 0)
                item = item->parent();
        item=item->nextSibling();
        TQString searchText=item->text(0)+" "+item->text(1)+" "+item->text(6);
        //kdDebug(2100)<<"Next string:"<<searchText<<endl;
        //kdDebug(2100)<<"Search:"<<searchString<<endl;
        //kdDebug(2100)<<"OPts:"<<searchOptions<<endl;
        KFind *m_find = new KFind(searchString, searchOptions,this);
        m_find->setData(searchText);
        while (m_find->find()==KFind::NoMatch) {
                if (!item->nextSibling()) {
                        foundItem=false;
                        break;
                } else {
                        item = item->nextSibling();
                        searchText=item->text(0)+" "+item->text(1)+" "+item->text(6);
                        m_find->setData(searchText);
                        //kdDebug(2100)<<"Next string:"<<searchText<<endl;
                }
        }
        delete m_find;
        if (foundItem) {
                keysList2->clearSelection();
                keysList2->setCurrentItem(item);
                keysList2->setSelected(item,true);
                keysList2->ensureItemVisible(item);
        } else
                findFirstKey();
}




void listKeys::addToKAB()
{
        KABC::Key key;
	if (!keysList2->currentItem()) return;
        //TQString email=extractKeyMail(keysList2->currentItem()).stripWhiteSpace();
        TQString email=keysList2->currentItem()->text(1);

        KABC::AddressBook *ab = KABC::StdAddressBook::self();
        if ( !ab->load() ) {
                KMessageBox::sorry(this,i18n("Unable to contact the address book. Please check your installation."));
                return;
        }

	KABC::Addressee::List addresseeList = ab->findByEmail(email);
  	kapp->startServiceByDesktopName( "kaddressbook" );
  	DCOPRef call( "kaddressbook", "KAddressBookIface" );
  	if( !addresseeList.isEmpty() ) {
    	call.send( "showContactEditor(TQString)", addresseeList.first().uid() );
  	}
  	else {
    	call.send( "addEmail(TQString)", TQString (keysList2->currentItem()->text(0))+" <"+email+">" );
  	}
}

/*
void listKeys::allToKAB()
{
        KABC::Key key;
        TQString email;
        TQStringList keylist;
        KABC::Addressee a;

        KABC::AddressBook *ab = KABC::StdAddressBook::self();
        if ( !ab->load() ) {
                KMessageBox::sorry(this,i18n("Unable to contact the address book. Please check your installation."));
                return;
        }

        TQListViewItem * myChild = keysList2->firstChild();
        while( myChild ) {
                //email=extractKeyMail(myChild).stripWhiteSpace();
                email=myChild->text(1);
                KABC::Addressee::List addressees = ab->findByEmail( email );
                if (addressees.count()==1) {
                        a=addressees.first();
                        KgpgInterface *ks=new KgpgInterface();
                        key.setTextData(ks->getKey(myChild->text(6),true));
                        a.insertKey(key);
                        ab->insertAddressee(a);
                        keylist<<myChild->text(6)+": "+email;
                }
                //            doSomething( myChild );
                myChild = myChild->nextSibling();
        }
        KABC::StdAddressBook::save();
        if (!keylist.isEmpty())
                KMessageBox::informationList(this,i18n("The following keys were exported to the address book:"),keylist);
        else
                KMessageBox::sorry(this,i18n("No entry matching your keys were found in the address book."));
}
*/

void listKeys::slotManpage()
{
        kapp->startServiceByDesktopName("khelpcenter", TQString("man:/gpg"), 0, 0, 0, "", true);
}

void listKeys::slotTip()
{
        KTipDialog::showTip(this, TQString("kgpg/tips"), true);
}

void listKeys::closeEvent ( TQCloseEvent * e )
{
        //kapp->ref(); // prevent TDEMainWindow from closing the app
        //TDEMainWindow::closeEvent( e );
        e->accept();
        //	hide();
        //	e->ignore();
}

void listKeys::showKeyServer()
{
        keyServer *ks=new keyServer(this);
	connect(ks,TQT_SIGNAL( importFinished(TQString) ) , keysList2, TQT_SLOT(refreshcurrentkey(TQString)));
        ks->exec();
	if (ks)
                delete ks;
        refreshkey();
}


void listKeys::checkList()
{
        TQPtrList<TQListViewItem> exportList=keysList2->selectedItems();
        if (exportList.count()>1)
                {
		stateChanged("multi_selected");
		for ( uint i = 0; i < exportList.count(); ++i )
		{
		    if (exportList.at(i) && !(exportList.at(i)->isVisible()))
                        exportList.at(i)->setSelected(false);
		}
		}
        else {
                if (keysList2->currentItem()->text(6).isEmpty())
                        stateChanged("group_selected");
                else
		  stateChanged("single_selected");

        }
        int serial=keysList2->currentItem()->pixmap(0)->serialNumber();
        if (serial==keysList2->pixkeySingle.serialNumber()) {
                if (keysList2->currentItem()->depth()==0)
                        changeMessage(i18n("Public Key"),0);
                else
                        changeMessage(i18n("Sub Key"),0);
        } else if (serial==keysList2->pixkeyPair.serialNumber())
                changeMessage(i18n("Secret Key Pair"),0);
        else if (serial==keysList2->pixkeyGroup.serialNumber())
                changeMessage(i18n("Key Group"),0);
        else if (serial==keysList2->pixsignature.serialNumber())
                changeMessage(i18n("Signature"),0);
        else if (serial==keysList2->pixuserid.serialNumber())
                changeMessage(i18n("User ID"),0);
        else if (keysList2->currentItem()->text(0)==i18n("Photo id"))
                changeMessage(i18n("Photo ID"),0);
        else if (serial==keysList2->pixRevoke.serialNumber())
                changeMessage(i18n("Revocation Signature"),0);
        else if (serial==keysList2->pixkeyOrphan.serialNumber())
                changeMessage(i18n("Orphaned Secret Key"),0);

}

void listKeys::annule()
{
        /////////  close window
        close();
}

void listKeys::quitApp()
{
        /////////  close window
        exit(1);
}

void listKeys::readOptions()
{

        clipboardMode=TQClipboard::Clipboard;
        if (KGpgSettings::useMouseSelection() && (kapp->clipboard()->supportsSelection()))
                clipboardMode=TQClipboard::Selection;

        ///////  re-read groups in case the config file location was changed
        TQStringList groups=KgpgInterface::getGpgGroupNames(KGpgSettings::gpgConfigPath());
        KGpgSettings::setGroups(groups.join(","));
        keysList2->groupNb=groups.count();
        if (keyStatusBar)
                changeMessage(i18n("%1 Keys, %2 Groups").arg(keysList2->childCount()-keysList2->groupNb).arg(keysList2->groupNb),1);

        showTipOfDay= KGpgSettings::showTipOfDay();
}


void listKeys::showOptions()
{
        if (TDEConfigDialog::showDialog("settings"))
                return;
        kgpgOptions *optionsDialog=new kgpgOptions(this,"settings");
        connect(optionsDialog,TQT_SIGNAL(settingsUpdated()),TQT_TQOBJECT(this),TQT_SLOT(readAllOptions()));
        connect(optionsDialog,TQT_SIGNAL(homeChanged()),TQT_TQOBJECT(this),TQT_SLOT(refreshkey()));
	connect(optionsDialog,TQT_SIGNAL(reloadKeyList()),TQT_TQOBJECT(this),TQT_SLOT(refreshkey()));
	connect(optionsDialog,TQT_SIGNAL(refreshTrust(int,TQColor)),keysList2,TQT_SLOT(refreshTrust(int,TQColor)));
        connect(optionsDialog,TQT_SIGNAL(changeFont(TQFont)),TQT_TQOBJECT(this),TQT_SIGNAL(fontChanged(TQFont)));
	connect(optionsDialog,TQT_SIGNAL(installShredder()),TQT_TQOBJECT(this),TQT_SIGNAL(installShredder()));
        optionsDialog->exec();
	delete optionsDialog;
}

void listKeys::readAllOptions()
{
        readOptions();
        emit readAgainOptions();
}

void listKeys::slotSetDefKey()
{
        slotSetDefaultKey(keysList2->currentItem());
}

void listKeys::slotSetDefaultKey(TQString newID)
{
        TQListViewItem *newdef = keysList2->findItem(newID,6);
        if (newdef)
                slotSetDefaultKey(newdef);
}

void listKeys::slotSetDefaultKey(TQListViewItem *newdef)
{
        //kdDebug(2100)<<"------------------start ------------"<<endl;
        if ((!newdef) || (newdef->pixmap(2)==NULL))
                return;
        //kdDebug(2100)<<newdef->text(6)<<endl;
        //kdDebug(2100)<<KGpgSettings::defaultKey()<<endl;
        if (newdef->text(6)==KGpgSettings::defaultKey())
                return;
        if (newdef->pixmap(2)->serialNumber()!=keysList2->trustgood.serialNumber()) {
                KMessageBox::sorry(this,i18n("Sorry, this key is not valid for encryption or not trusted."));
                return;
        }

        TQListViewItem *olddef = keysList2->findItem(KGpgSettings::defaultKey(),6);

        KGpgSettings::setDefaultKey(newdef->text(6));
        KGpgSettings::writeConfig();
        if (olddef)
                keysList2->refreshcurrentkey(olddef);
        keysList2->refreshcurrentkey(newdef);
        keysList2->ensureItemVisible(keysList2->currentItem());
}



void listKeys::slotmenu(TQListViewItem *sel, const TQPoint &pos, int )
{
        ////////////  popup a different menu depending on which key is selected
        if (sel!=NULL) {
                if (keysList2->selectedItems().count()>1) {
                        TQPtrList<TQListViewItem> exportList=keysList2->selectedItems();
                        bool keyDepth=true;
                        for ( uint i = 0; i < exportList.count(); ++i )
                                if ( exportList.at(i) )
                                        if (exportList.at(i)->depth()!=0)
                                                keyDepth=false;
                        if (!keyDepth) {
                                signKey->setEnabled(false);
                                refreshKey->setEnabled(false);
                                popupout->exec(pos);
                                return;
                        } else {
                                signKey->setEnabled(true);
                                refreshKey->setEnabled(true);
                        }
                }

                if (sel->depth()!=0) {
                        //kdDebug(2100)<<sel->text(0)<<endl;
                        if ((sel->text(4)=="-") && (sel->text(6).startsWith("0x"))) {
                                if ((sel->text(2)=="-") || (sel->text(2)==i18n("Revoked"))) {
                                        if ((sel->text(0).startsWith("[")) && (sel->text(0).endsWith("]")))  ////// ugly hack to detect unknown keys
                                                importSignatureKey->setEnabled(true);
                                        else
                                                importSignatureKey->setEnabled(false);
                                        popupsig->exec(pos);
                                        return;
                                }
                        } else if (sel->text(0)==i18n("Photo id"))
                                popupphoto->exec(pos);
                        else if (sel->text(6)==("-"))
                                popupuid->exec(pos);
                } else {
                        keysList2->setSelected(sel,TRUE);
                        if (keysList2->currentItem()->text(6).isEmpty())
                                popupgroup->exec(pos);
                        else {
                                if ((keysList2->secretList.find(sel->text(6))!=-1) && (keysList2->selectedItems().count()==1))
                                        popupsec->exec(pos);
                                else
                                        if ((keysList2->orphanList.find(sel->text(6))!=-1) && (keysList2->selectedItems().count()==1))
                                                popuporphan->exec(pos);
                                        else
                                                popup->exec(pos);
                        }
                        return;
                }
        } else
                popupout->exec(pos);
}



void listKeys::slotrevoke(TQString keyID,TQString revokeUrl,int reason,TQString description)
{
        revKeyProcess=new KgpgInterface();
        revKeyProcess->KgpgRevokeKey(keyID,revokeUrl,reason,description);
}


void listKeys::revokeWidget()
{
        KDialogBase *keyRevokeWidget=new KDialogBase(KDialogBase::Swallow, i18n("Create Revocation Certificate"),  KDialogBase::Ok | KDialogBase::Cancel,KDialogBase::Ok,this,0,true);

        KgpgRevokeWidget *keyRevoke=new KgpgRevokeWidget();

        keyRevoke->keyID->setText(keysList2->currentItem()->text(0)+" ("+keysList2->currentItem()->text(1)+") "+i18n("ID: ")+keysList2->currentItem()->text(6));
        keyRevoke->kURLRequester1->setURL(TQDir::homeDirPath()+"/"+keysList2->currentItem()->text(1).section('@',0,0)+".revoke");
        keyRevoke->kURLRequester1->setMode(KFile::File);

        keyRevoke->setMinimumSize(keyRevoke->sizeHint());
        keyRevoke->show();
        keyRevokeWidget->setMainWidget(keyRevoke);

        if (keyRevokeWidget->exec()!=TQDialog::Accepted)
                return;
        if (keyRevoke->cbSave->isChecked()) {
                slotrevoke(keysList2->currentItem()->text(6),keyRevoke->kURLRequester1->url(),keyRevoke->comboBox1->currentItem(),keyRevoke->textDescription->text());
                if (keyRevoke->cbPrint->isChecked())
                        connect(revKeyProcess,TQT_SIGNAL(revokeurl(TQString)),TQT_TQOBJECT(this),TQT_SLOT(doFilePrint(TQString)));
                if (keyRevoke->cbImport->isChecked())
                        connect(revKeyProcess,TQT_SIGNAL(revokeurl(TQString)),TQT_TQOBJECT(this),TQT_SLOT(slotImportRevoke(TQString)));
        } else {
                slotrevoke(keysList2->currentItem()->text(6),TQString(),keyRevoke->comboBox1->currentItem(),keyRevoke->textDescription->text());
                if (keyRevoke->cbPrint->isChecked())
                        connect(revKeyProcess,TQT_SIGNAL(revokecertificate(TQString)),TQT_TQOBJECT(this),TQT_SLOT(doPrint(TQString)));
                if (keyRevoke->cbImport->isChecked())
                        connect(revKeyProcess,TQT_SIGNAL(revokecertificate(TQString)),TQT_TQOBJECT(this),TQT_SLOT(slotImportRevokeTxt(TQString)));
        }
}


void listKeys::slotImportRevoke(TQString url)
{
        KgpgInterface *importKeyProcess=new KgpgInterface();
        importKeyProcess->importKeyURL(KURL::fromPathOrURL( url ));
        connect(importKeyProcess,TQT_SIGNAL(importfinished(TQStringList)),keysList2,TQT_SLOT(refreshselfkey()));
}

void listKeys::slotImportRevokeTxt(TQString revokeText)
{
        KgpgInterface *importKeyProcess=new KgpgInterface();
        importKeyProcess->importKey(revokeText);
        connect(importKeyProcess,TQT_SIGNAL(importfinished(TQStringList)),keysList2,TQT_SLOT(refreshselfkey()));
}

void listKeys::slotexportsec()
{
        //////////////////////   export secret key
        TQString warn=i18n("Secret keys SHOULD NOT be saved in an unsafe place.\n"
                          "If someone else can access this file, encryption with this key will be compromised!\nContinue key export?");
        int result=KMessageBox::questionYesNo(this,warn,i18n("Warning"), i18n("Export"), i18n("Do Not Export"));
        if (result!=KMessageBox::Yes)
                return;

        TQString sname=keysList2->currentItem()->text(1).section('@',0,0);
        sname=sname.section('.',0,0);
        if (sname.isEmpty())
                sname=keysList2->currentItem()->text(0).section(' ',0,0);
        sname.append(".asc");
        sname.prepend(TQDir::homeDirPath()+"/");
        KURL url=KFileDialog::getSaveURL(sname,"*.asc|*.asc Files", this, i18n("Export PRIVATE KEY As"));

        if(!url.isEmpty()) {
                TQFile fgpg(url.path());
                if (fgpg.exists())
                        fgpg.remove();

                KProcIO *p=new KProcIO(TQTextCodec::codecForLocale());
                *p<<"gpg"<<"--no-tty"<<"--output"<<TQString(TQFile::encodeName(url.path()))<<"--armor"<<"--export-secret-keys"<<keysList2->currentItem()->text(6);
                p->start(TDEProcess::Block);

                if (fgpg.exists())
                        KMessageBox::information(this,i18n("Your PRIVATE key \"%1\" was successfully exported.\nDO NOT leave it in an insecure place.").arg(url.path()));
                else
                        KMessageBox::sorry(this,i18n("Your secret key could not be exported.\nCheck the key."));
        }

}


void listKeys::slotexport()
{
        /////////////////////  export key
        if (keysList2->currentItem()==NULL)
                return;
        if (keysList2->currentItem()->depth()!=0)
                return;


        TQPtrList<TQListViewItem> exportList=keysList2->selectedItems();
        if (exportList.count()==0)
                return;

        TQString sname;

        if (exportList.count()==1) {
                sname=keysList2->currentItem()->text(1).section('@',0,0);
                sname=sname.section('.',0,0);
                if (sname.isEmpty())
                        sname=keysList2->currentItem()->text(0).section(' ',0,0);
        } else
                sname="keyring";
        sname.append(".asc");
        sname.prepend(TQDir::homeDirPath()+"/");

        KDialogBase *dial=new KDialogBase( KDialogBase::Swallow, i18n("Public Key Export"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, this, "key_export",true);

        KeyExport *page=new KeyExport();
        dial->setMainWidget(page);
        page->newFilename->setURL(sname);
        page->newFilename->setCaption(i18n("Save File"));
        page->newFilename->setMode(KFile::File);
        page->show();

        if (dial->exec()==TQDialog::Accepted) {
                ////////////////////////// export to file
                TQString expname;
                bool exportAttr=page->exportAttributes->isChecked();
                if (page->checkServer->isChecked()) {
                        keyServer *expServer=new keyServer(0,"server_export",false);
                        expServer->page->exportAttributes->setChecked(exportAttr);
                        TQStringList exportKeysList;
                        for ( uint i = 0; i < exportList.count(); ++i )
                                if ( exportList.at(i) )
                                        exportKeysList << exportList.at(i)->text(6).stripWhiteSpace();
                        expServer->slotExport(exportKeysList);
                        return;
                }
                KProcIO *p=new KProcIO(TQTextCodec::codecForLocale());
                *p<<"gpg"<<"--no-tty";
                if (page->checkFile->isChecked()) {
                        expname=page->newFilename->url().stripWhiteSpace();
                        if (!expname.isEmpty()) {
                                TQFile fgpg(expname);
                                if (fgpg.exists())
                                        fgpg.remove();
                                *p<<"--output"<<TQString(TQFile::encodeName(expname))<<"--export"<<"--armor";
                                if (!exportAttr)
                                        *p<<"--export-options"<<"no-include-attributes";

                                for ( uint i = 0; i < exportList.count(); ++i )
                                        if ( exportList.at(i) )
                                                *p<<(exportList.at(i)->text(6)).stripWhiteSpace();


                                p->start(TDEProcess::Block);
                                if (fgpg.exists())
                                        KMessageBox::information(this,i18n("Your public key \"%1\" was successfully exported\n").arg(expname));
                                else
                                        KMessageBox::sorry(this,i18n("Your public key could not be exported\nCheck the key."));
                        }
                } else {

                        TQStringList tdelist;

                        for ( uint i = 0; i < exportList.count(); ++i )
                                if ( exportList.at(i) )
                                        tdelist.append(exportList.at(i)->text(6).stripWhiteSpace());

                        KgpgInterface *kexp=new KgpgInterface();

                        TQString result=kexp->getKey(tdelist,exportAttr);
                        if (page->checkClipboard->isChecked())
                                slotProcessExportClip(result);
                        //connect(kexp,TQT_SIGNAL(publicKeyString(TQString)),TQT_TQOBJECT(this),TQT_SLOT(slotProcessExportClip(TQString)));
                        else
                                slotProcessExportMail(result);
                        //connect(kexp,TQT_SIGNAL(publicKeyString(TQString)),TQT_TQOBJECT(this),TQT_SLOT(slotProcessExportMail(TQString)));

                }
        }
        delete dial;
}



void listKeys::slotProcessExportMail(TQString keys)
{
        ///   start default Mail application
        kapp->invokeMailer(TQString(), TQString(), TQString(), TQString(),
                           keys, //body
                           TQString(),
                           TQString()); // attachments
}

void listKeys::slotProcessExportClip(TQString keys)
{
        kapp->clipboard()->setText(keys,clipboardMode);
}


void listKeys::showKeyInfo(TQString keyID)
{
        KgpgKeyInfo *opts=new KgpgKeyInfo(this,"key_props",keyID);
        opts->show();
}


void listKeys::slotShowPhoto()
{
        TDETrader::OfferList offers = TDETrader::self()->query("image/jpeg", "Type == 'Application'");
        KService::Ptr ptr = offers.first();
        //KMessageBox::sorry(0,ptr->desktopEntryName());
        KProcIO *p=new KProcIO(TQTextCodec::codecForLocale());
        *p<<"gpg"<<"--no-tty"<<"--photo-viewer"<<TQString(TQFile::encodeName(ptr->desktopEntryName()+" %i"))<<"--edit-key"<<keysList2->currentItem()->parent()->text(6)<<"uid"<<keysList2->currentItem()->text(6)<<"showphoto"<<"quit";
        p->start(TDEProcess::DontCare,true);
}

void listKeys::listsigns()
{
        //kdDebug(2100)<<"Edit -------------------------------"<<endl;
        if (keysList2->currentItem()==NULL)
                return;
        if (keysList2->currentItem()->depth()!=0) {
                if (keysList2->currentItem()->text(0)==i18n("Photo id")) {
                        //////////////////////////    display photo
                        slotShowPhoto();
                }
                return;
        }

        if (keysList2->currentItem()->pixmap(0)->serialNumber()==keysList2->pixkeyOrphan.serialNumber()) {
                if (KMessageBox::questionYesNo(this,i18n("This key is an orphaned secret key (secret key without public key.) It is currently not usable.\n\n"
                                               "Would you like to regenerate the public key?"), TQString(), i18n("Generate"), i18n("Do Not Generate"))==KMessageBox::Yes)
                        slotregenerate();
                return;
        }

        /////////////   open a key info dialog (KgpgKeyInfo, see begining of this file)
        TQString key=keysList2->currentItem()->text(6);
        if (!key.isEmpty()) {
                KgpgKeyInfo *opts=new KgpgKeyInfo(this,"key_props",key);
                connect(opts,TQT_SIGNAL(keyNeedsRefresh()),keysList2,TQT_SLOT(refreshselfkey()));
                opts->exec();
        } else
                editGroup();
}

void listKeys::groupAdd()
{
        TQPtrList<TQListViewItem> addList=gEdit->availableKeys->selectedItems();
        for ( uint i = 0; i < addList.count(); ++i )
                if ( addList.at(i) ) {
                        gEdit->groupKeys->insertItem(addList.at(i));
                }
}

void listKeys::groupRemove()
{
        TQPtrList<TQListViewItem> remList=gEdit->groupKeys->selectedItems();
        for ( uint i = 0; i < remList.count(); ++i )
                if ( remList.at(i) ) {
                        gEdit->availableKeys->insertItem(remList.at(i));
                }
}

void listKeys::deleteGroup()
{
        if (!keysList2->currentItem() || !keysList2->currentItem()->text(6).isEmpty())
                return;

        int result=KMessageBox::warningContinueCancel(this,i18n("<qt>Are you sure you want to delete group <b>%1</b> ?</qt>").arg(keysList2->currentItem()->text(0)),i18n("Warning"),KGuiItem(i18n("Delete"),"editdelete"));
        if (result!=KMessageBox::Continue)
                return;
        KgpgInterface::delGpgGroup(keysList2->currentItem()->text(0), KGpgSettings::gpgConfigPath());
        TQListViewItem *item=keysList2->currentItem()->nextSibling();
        delete keysList2->currentItem();
        if (!item)
                item=keysList2->lastChild();
        keysList2->setCurrentItem(item);
        keysList2->setSelected(item,true);

        TQStringList groups=KgpgInterface::getGpgGroupNames(KGpgSettings::gpgConfigPath());
        KGpgSettings::setGroups(groups.join(","));
        keysList2->groupNb=groups.count();
        changeMessage(i18n("%1 Keys, %2 Groups").arg(keysList2->childCount()-keysList2->groupNb).arg(keysList2->groupNb),1);
}

void listKeys::groupChange()
{
        TQStringList selected;
        TQListViewItem *item=gEdit->groupKeys->firstChild();
        while (item) {
                selected+=item->text(2);
                item=item->nextSibling();
        }
        KgpgInterface::setGpgGroupSetting(keysList2->currentItem()->text(0),selected,KGpgSettings::gpgConfigPath());
}

void listKeys::createNewGroup()
{
        TQStringList badkeys,keysGroup;

        if (keysList2->selectedItems().count()>0) {
                TQPtrList<TQListViewItem> groupList=keysList2->selectedItems();
                bool keyDepth=true;
                for ( uint i = 0; i < groupList.count(); ++i )
                        if ( groupList.at(i) ) {
                                if (groupList.at(i)->depth()!=0)
                                        keyDepth=false;
                                else if (groupList.at(i)->text(6).isEmpty())
                                        keyDepth=false;
                                else if (groupList.at(i)->pixmap(2)) {
                                        if (groupList.at(i)->pixmap(2)->serialNumber()==keysList2->trustgood.serialNumber())
                                                keysGroup+=groupList.at(i)->text(6);
                                        else
                                                badkeys+=groupList.at(i)->text(0)+" ("+groupList.at(i)->text(1)+") "+groupList.at(i)->text(6);
                                }

                        }
                if (!keyDepth) {
                        KMessageBox::sorry(this,i18n("<qt>You cannot create a group containing signatures, subkeys or other groups.</qt>"));
                        return;
                }
                TQString groupName=KInputDialog::getText(i18n("Create New Group"),i18n("Enter new group name:"),TQString(),0,this);
                if (groupName.isEmpty())
                        return;
                if (!keysGroup.isEmpty()) {
                        if (!badkeys.isEmpty())
                                KMessageBox::informationList(this,i18n("Following keys are not valid or not trusted and will not be added to the group:"),badkeys);
                        KgpgInterface::setGpgGroupSetting(groupName,keysGroup,KGpgSettings::gpgConfigPath());
                        TQStringList groups=KgpgInterface::getGpgGroupNames(KGpgSettings::gpgConfigPath());
                        KGpgSettings::setGroups(groups.join(","));
                        keysList2->refreshgroups();
                        TQListViewItem *newgrp = keysList2->findItem(groupName,0);

                        keysList2->clearSelection();
                        keysList2->setCurrentItem(newgrp);
                        keysList2->setSelected(newgrp,true);
                        keysList2->ensureItemVisible(newgrp);
                        keysList2->groupNb=groups.count();
                        changeMessage(i18n("%1 Keys, %2 Groups").arg(keysList2->childCount()-keysList2->groupNb).arg(keysList2->groupNb),1);
                } else
                        KMessageBox::sorry(this,i18n("<qt>No valid or trusted key was selected. The group <b>%1</b> will not be created.</qt>").arg(groupName));
        }
}

void listKeys::groupInit(TQStringList keysGroup)
{
        kdDebug(2100)<<"preparing group"<<endl;
        TQStringList lostKeys;
        bool foundId;

        for ( TQStringList::Iterator it = keysGroup.begin(); it != keysGroup.end(); ++it ) {

                TQListViewItem *item=gEdit->availableKeys->firstChild();
                foundId=false;
                while (item) {
                        kdDebug(2100)<<"Searching in key: "<<item->text(0)<<endl;
                        if (TQString(*it).right(8).lower()==item->text(2).right(8).lower()) {
                                gEdit->groupKeys->insertItem(item);
                                foundId=true;
                                break;
                        }
                        item=item->nextSibling();
                }
                if (!foundId)
                        lostKeys+=TQString(*it);
        }
        if (!lostKeys.isEmpty())
                KMessageBox::informationList(this,i18n("Following keys are in the group but are not valid or not in your keyring. They will be removed from the group."),lostKeys);
}

void listKeys::editGroup()
{
  if (!keysList2->currentItem() || !keysList2->currentItem()->text(6).isEmpty())
                return;
        TQStringList keysGroup;
	//KDialogBase *dialogGroupEdit=new KDialogBase( this, "edit_group", true,i18n("Group Properties"),KDialogBase::Ok | KDialogBase::Cancel);
        KDialogBase *dialogGroupEdit=new KDialogBase(KDialogBase::Swallow, i18n("Group Properties"), KDialogBase::Ok | KDialogBase::Cancel,KDialogBase::Ok,this,0,true);

        gEdit=new groupEdit();
        gEdit->buttonAdd->setPixmap(TDEGlobal::iconLoader()->loadIcon("down",KIcon::Small,20));
        gEdit->buttonRemove->setPixmap(TDEGlobal::iconLoader()->loadIcon("up",KIcon::Small,20));

        connect(gEdit->buttonAdd,TQT_SIGNAL(clicked()),TQT_TQOBJECT(this),TQT_SLOT(groupAdd()));
        connect(gEdit->buttonRemove,TQT_SIGNAL(clicked()),TQT_TQOBJECT(this),TQT_SLOT(groupRemove()));
        //        connect(dialogGroupEdit->okClicked(),TQT_SIGNAL(clicked()),TQT_TQOBJECT(this),TQT_SLOT(groupChange()));
        connect(gEdit->availableKeys,TQT_SIGNAL(doubleClicked (TQListViewItem *, const TQPoint &, int)),TQT_TQOBJECT(this),TQT_SLOT(groupAdd()));
        connect(gEdit->groupKeys,TQT_SIGNAL(doubleClicked (TQListViewItem *, const TQPoint &, int)),TQT_TQOBJECT(this),TQT_SLOT(groupRemove()));
        TQListViewItem *item=keysList2->firstChild();
        if (item==NULL)
                return;
        if (item->pixmap(2)) {
                if (item->pixmap(2)->serialNumber()==keysList2->trustgood.serialNumber())
                        (void) new TDEListViewItem(gEdit->availableKeys,item->text(0),item->text(1),item->text(6));
        }
        while (item->nextSibling()) {
                item=item->nextSibling();
                if (item->pixmap(2)) {
                        if (item->pixmap(2)->serialNumber()==keysList2->trustgood.serialNumber())
                                (void) new TDEListViewItem(gEdit->availableKeys,item->text(0),item->text(1),item->text(6));
                }
        }
        keysGroup=KgpgInterface::getGpgGroupSetting(keysList2->currentItem()->text(0),KGpgSettings::gpgConfigPath());
        groupInit(keysGroup);
	dialogGroupEdit->setMainWidget(gEdit);
	gEdit->availableKeys->setColumnWidth(0,200);
	gEdit->availableKeys->setColumnWidth(1,200);
	gEdit->availableKeys->setColumnWidth(2,100);
	gEdit->availableKeys->setColumnWidthMode(0,TQListView::Manual);
	gEdit->availableKeys->setColumnWidthMode(1,TQListView::Manual);
	gEdit->availableKeys->setColumnWidthMode(2,TQListView::Manual);

	gEdit->groupKeys->setColumnWidth(0,200);
	gEdit->groupKeys->setColumnWidth(1,200);
	gEdit->groupKeys->setColumnWidth(2,100);
	gEdit->groupKeys->setColumnWidthMode(0,TQListView::Manual);
	gEdit->groupKeys->setColumnWidthMode(1,TQListView::Manual);
	gEdit->groupKeys->setColumnWidthMode(2,TQListView::Manual);

        gEdit->setMinimumSize(gEdit->sizeHint());
        gEdit->show();
        if (dialogGroupEdit->exec()==TQDialog::Accepted)
                groupChange();
        delete dialogGroupEdit;
}

void listKeys::signkey()
{
        ///////////////  sign a key
        if (keysList2->currentItem()==NULL)
                return;
        if (keysList2->currentItem()->depth()!=0)
                return;

        signList=keysList2->selectedItems();
        bool keyDepth=true;
        for ( uint i = 0; i < signList.count(); ++i )
                if ( signList.at(i) )
                        if (signList.at(i)->depth()!=0)
                                keyDepth=false;
        if (!keyDepth) {
                KMessageBox::sorry(this,i18n("You can only sign primary keys. Please check your selection."));
                return;
        }


        if (signList.count()==1) {
                FILE *pass;
                char line[200]="";
                TQString opt,fingervalue;
                TQString gpgcmd="gpg --no-tty --no-secmem-warning --with-colons --fingerprint "+KShellProcess::quote(keysList2->currentItem()->text(6));
                pass=popen(TQFile::encodeName(gpgcmd),"r");
                while ( fgets( line, sizeof(line), pass)) {
                        opt=TQString::fromUtf8(line);
                        if (opt.startsWith("fpr")) {
                                fingervalue=opt.section(':',9,9);
                                // format fingervalue in 4-digit groups
                                uint len = fingervalue.length();
                                if ((len > 0) && (len % 4 == 0))
                                        for (uint n = 0; 4*(n+1) < len; n++)
                                                fingervalue.insert(5*n+4, ' ');
                        }
                }
                pclose(pass);
                opt=	i18n("<qt>You are about to sign key:<br><br>%1<br>ID: %2<br>Fingerprint: <br><b>%3</b>.<br><br>"
                          "You should check the key fingerprint by phoning or meeting the key owner to be sure that someone "
                          "is not trying to intercept your communications</qt>").arg(keysList2->currentItem()->text(0)+" ("+keysList2->currentItem()->text(1)+")").arg(keysList2->currentItem()->text(6)).arg(fingervalue);

                if (KMessageBox::warningContinueCancel(this,opt)!=KMessageBox::Continue)
                        return;

        } else {
                TQStringList signKeyList;
                for ( uint i = 0; i < signList.count(); ++i )
                        if ( signList.at(i) )
                                signKeyList+=signList.at(i)->text(0)+" ("+signList.at(i)->text(1)+")"+": "+signList.at(i)->text(6);
                if (KMessageBox::warningContinueCancelList(this,i18n("<qt>You are about to sign the following keys in one pass.<br><b>If you have not carefully checked all fingerprints, the security of your communications may be compromised.</b></qt>"),signKeyList)!=KMessageBox::Continue)
                        return;
        }


        //////////////////  open a secret key selection dialog (KgpgSelKey, see begining of this file)
        KgpgSelKey *opts=new KgpgSelKey(this);

        TQLabel *signCheck = new TQLabel("<qt>"+i18n("How carefully have you checked that the key really "
                                            "belongs to the person with whom you wish to communicate:",
					    "How carefully have you checked that the %n keys really "
                                            "belong to the people with whom you wish to communicate:",signList.count()),opts->page);
        opts->vbox->addWidget(signCheck);
        TQComboBox *signTrust=new TQComboBox(opts->page);
        signTrust->insertItem(i18n("I Will Not Answer"));
        signTrust->insertItem(i18n("I Have Not Checked at All"));
        signTrust->insertItem(i18n("I Have Done Casual Checking"));
        signTrust->insertItem(i18n("I Have Done Very Careful Checking"));
        opts->vbox->addWidget(signTrust);

        TQCheckBox *localSign = new TQCheckBox(i18n("Local signature (cannot be exported)"),opts->page);
        opts->vbox->addWidget(localSign);

        TQCheckBox *terminalSign = new TQCheckBox(i18n("Do not sign all user id's (open terminal)"),opts->page);
        opts->vbox->addWidget(terminalSign);
        if (signList.count()!=1)
                terminalSign->setEnabled(false);

        opts->setMinimumHeight(300);

        if (opts->exec()!=TQDialog::Accepted) {
                delete opts;
                return;
        }

        globalkeyID=TQString(opts->getkeyID());
        globalkeyMail=TQString(opts->getkeyMail());
        globalisLocal=localSign->isChecked();
        globalChecked=signTrust->currentItem();
        keyCount=0;
        delete opts;
        globalCount=signList.count();
        if (!terminalSign->isChecked())
                signLoop();
        else {
                TDEProcess kp;

                TDEConfig *config = TDEGlobal::config();
                config->setGroup("General");
                kp<< config->readPathEntry("TerminalApplication","konsole");
                kp<<"-e"
                <<"gpg"
                <<"--no-secmem-warning"
                <<"-u"
                <<globalkeyID
                <<"--edit-key"
                <<signList.at(0)->text(6);
                if (globalisLocal)
                        kp<<"lsign";
                else
                        kp<<"sign";
                kp.start(TDEProcess::Block);
                keysList2->refreshcurrentkey(keysList2->currentItem());
        }
}

void listKeys::signLoop()
{
        if (keyCount<globalCount) {
                kdDebug(2100)<<"Sign process for key: "<<keyCount<<" on a total of "<<signList.count()<<endl;
                if ( signList.at(keyCount) ) {
                        KgpgInterface *signKeyProcess=new KgpgInterface();
			TQObject::connect(signKeyProcess,TQT_SIGNAL(signatureFinished(int)),TQT_TQOBJECT(this),TQT_SLOT(signatureResult(int)));
                        signKeyProcess->KgpgSignKey(signList.at(keyCount)->text(6),globalkeyID,globalkeyMail,globalisLocal,globalChecked);
                }
        }
}

void listKeys::signatureResult(int success)
{
        if (success==3)
                keysList2->refreshcurrentkey(signList.at(keyCount));

        else if (success==2)
                KMessageBox::sorry(this,i18n("<qt>Bad passphrase, key <b>%1</b> not signed.</qt>").arg(signList.at(keyCount)->text(0)+i18n(" (")+signList.at(keyCount)->text(1)+i18n(")")));

        keyCount++;
        signLoop();
}


void listKeys::importallsignkey()
{
        if (keysList2->currentItem()==NULL)
                return;
        if (! keysList2->currentItem()->firstChild()) {
                keysList2->currentItem()->setOpen(true);
                keysList2->currentItem()->setOpen(false);
        }
        TQString missingKeysList;
        TQListViewItem *current = keysList2->currentItem()->firstChild();
        while (current) {
                if ((current->text(0).startsWith("[")) && (current->text(0).endsWith("]")))   ////// ugly hack to detect unknown keys
                        missingKeysList+=current->text(6)+" ";
                current = current->nextSibling();
        }
        if (!missingKeysList.isEmpty())
                importsignkey(missingKeysList);
        else
                KMessageBox::information(this,i18n("All signatures for this key are already in your keyring"));
}


void listKeys::preimportsignkey()
{
        if (keysList2->currentItem()==NULL)
                return;
        else
                importsignkey(keysList2->currentItem()->text(6));
}

bool listKeys::importRemoteKey(TQString keyID)
{

        kServer=new keyServer(0,"server_dialog",false,true);
        kServer->page->kLEimportid->setText(keyID);
        kServer->page->Buttonimport->setDefault(true);
        kServer->page->tabWidget2->setTabEnabled(kServer->page->tabWidget2->page(1),false);
        kServer->show();
	kServer->raise();
        connect( kServer, TQT_SIGNAL( importFinished(TQString) ) , this, TQT_SLOT( dcopImportFinished()));

	return true;
}



void listKeys::dcopImportFinished()
{
        if (kServer)
                kServer=0L;
    TQByteArray params;
    TQDataStream stream(params, IO_WriteOnly);
   stream << true;
    kapp->dcopClient()->emitDCOPSignal("keyImported(bool)", params);
    refreshkey();
}

void listKeys::importsignkey(TQString importKeyId)
{
        ///////////////  sign a key
        kServer=new keyServer(0,"server_dialog",false);
        kServer->page->kLEimportid->setText(importKeyId);
        //kServer->Buttonimport->setDefault(true);
        kServer->slotImport();
        //kServer->show();
        connect( kServer, TQT_SIGNAL( importFinished(TQString) ) , this, TQT_SLOT( importfinished()));
}


void listKeys::importfinished()
{
        if (kServer)
                kServer=0L;
        refreshkey();
}


void listKeys::delsignkey()
{
        ///////////////  sign a key
        if (keysList2->currentItem()==NULL)
                return;
        if (keysList2->currentItem()->depth()>1) {
                KMessageBox::sorry(this,i18n("Edit key manually to delete this signature."));
                return;
        }

        TQString signID,parentKey,signMail,parentMail;

        //////////////////  open a key selection dialog (KgpgSelKey, see begining of this file)
        parentKey=keysList2->currentItem()->parent()->text(6);
        signID=keysList2->currentItem()->text(6);
        parentMail=keysList2->currentItem()->parent()->text(0)+" ("+keysList2->currentItem()->parent()->text(1)+")";
        signMail=keysList2->currentItem()->text(0)+" ("+keysList2->currentItem()->text(1)+")";

        if (parentKey==signID) {
                KMessageBox::sorry(this,i18n("Edit key manually to delete a self-signature."));
                return;
        }
        TQString ask=i18n("<qt>Are you sure you want to delete signature<br><b>%1</b> from key:<br><b>%2</b>?</qt>").arg(signMail).arg(parentMail);

        if (KMessageBox::questionYesNo(this,ask,TQString(),KStdGuiItem::del(),KStdGuiItem::cancel())!=KMessageBox::Yes)
                return;
        KgpgInterface *delSignKeyProcess=new KgpgInterface();
        delSignKeyProcess->KgpgDelSignature(parentKey,signID);
        connect(delSignKeyProcess,TQT_SIGNAL(delsigfinished(bool)),TQT_TQOBJECT(this),TQT_SLOT(delsignatureResult(bool)));
}

void listKeys::delsignatureResult(bool success)
{
        if (success) {
                TQListViewItem *top=keysList2->currentItem();
                while (top->depth()!=0)
                        top=top->parent();
                while (top->firstChild()!=0)
                        delete top->firstChild();
                keysList2->refreshcurrentkey(top);
        } else
                KMessageBox::sorry(this,i18n("Requested operation was unsuccessful, please edit the key manually."));
}

void listKeys::slotedit()
{
        if (!keysList2->currentItem())
                return;
        if (keysList2->currentItem()->depth()!=0)
                return;
        if (keysList2->currentItem()->text(6).isEmpty())
                return;

        TDEProcess kp;

        TDEConfig *config = TDEGlobal::config();
        config->setGroup("General");
        kp<< config->readPathEntry("TerminalApplication","konsole");
        kp<<"-e"
        <<"gpg"
        <<"--no-secmem-warning"
        <<"--utf8-strings"
        <<"--edit-key"
        <<keysList2->currentItem()->text(6)
        <<"help";
        kp.start(TDEProcess::Block);
        keysList2->refreshcurrentkey(keysList2->currentItem());
}


void listKeys::slotgenkey()
{
        //////////  generate key
        keyGenerate *genkey=new keyGenerate(this,0);
        if (genkey->exec()==TQDialog::Accepted) {
                if (!genkey->getmode())   ///  normal mode
                {
                        //// extract data
                        TQString ktype=genkey->getkeytype();
                        TQString ksize=genkey->getkeysize();
                        int kexp=genkey->getkeyexp();
                        TQString knumb=genkey->getkeynumb();
                        newKeyName=genkey->getkeyname();
                        newKeyMail=genkey->getkeymail();
                        TQString kcomment=genkey->getkeycomm();
                        delete genkey;

                        //genkey->delayedDestruct();
                        TQCString password;
                        bool goodpass=false;
                        while (!goodpass)
                        {
                                int code=KPasswordDialog::getNewPassword(password,i18n("<b>Enter passphrase for %1</b>:<br>Passphrase should include non alphanumeric characters and random sequences").arg(newKeyName+" <"+newKeyMail+">"));
                                if (code!=TQDialog::Accepted)
                                        return;
                                if (password.length()<5)
                                        KMessageBox::sorry(this,i18n("This passphrase is not secure enough.\nMinimum length= 5 characters"));
                                else
                                        goodpass=true;
                        }

			pop = new KPassivePopup((TQWidget *)parent(),"new_key");
                        pop->setTimeout(0);

                        TQWidget *wid=new TQWidget(pop);
                        TQVBoxLayout *vbox=new TQVBoxLayout(wid,3);

                        TQVBox *passiveBox=pop->standardView(i18n("Generating new key pair."),TQString(),TDEGlobal::iconLoader()->loadIcon("kgpg",KIcon::Desktop),wid);


                        TQMovie anim;
                        anim=TQMovie(locate("appdata", "pics/kgpg_anim.gif"));

                        TQLabel *tex=new TQLabel(wid);
                        TQLabel *tex2=new TQLabel(wid);
                        tex->setAlignment(AlignHCenter);
                        tex->setMovie(anim);
                        tex2->setText(i18n("\nPlease wait..."));
                        vbox->addWidget(passiveBox);
                        vbox->addWidget(tex);
                        vbox->addWidget(tex2);

                        pop->setView(wid);

                        pop->show();
                        changeMessage(i18n("Generating New Key..."),0,true);

                        TQRect qRect(TQApplication::desktop()->screenGeometry());
                        int iXpos=qRect.width()/2-pop->width()/2;
                        int iYpos=qRect.height()/2-pop->height()/2;
                        pop->move(iXpos,iYpos);
                        pop->setAutoDelete(false);
                        KProcIO *proc=new KProcIO(TQTextCodec::codecForLocale());
                        message=TQString();
                        //*proc<<"gpg"<<"--no-tty"<<"--no-secmem-warning"<<"--batch"<<"--passphrase-fd"<<res<<"--gen-key"<<"-a"<<"kgpg.tmp";
                        *proc<<"gpg"<<"--no-tty"<<"--status-fd=2"<<"--no-secmem-warning"<<"--batch"<<"--gen-key"<<"--utf8-strings";
                        /////////  when process ends, update dialog infos
                        TQObject::connect(proc, TQT_SIGNAL(processExited(TDEProcess *)),TQT_TQOBJECT(this), TQT_SLOT(genover(TDEProcess *)));
                        proc->start(TDEProcess::NotifyOnExit,true);

                        if (ktype=="RSA")
                                proc->writeStdin(TQString("Key-Type: 1"));
                        else
                        {
                                proc->writeStdin(TQString("Key-Type: DSA"));
                                proc->writeStdin(TQString("Subkey-Type: ELG-E"));
                                proc->writeStdin(TQString("Subkey-Length:%1").arg(ksize));
                        }
                        proc->writeStdin(TQString("Passphrase:%1").arg(password.data()));
                        proc->writeStdin(TQString("Key-Length:%1").arg(ksize));
                        proc->writeStdin(TQString("Name-Real:%1").arg(newKeyName));
                        if (!newKeyMail.isEmpty())
                                proc->writeStdin(TQString("Name-Email:%1").arg(newKeyMail));
                        if (!kcomment.isEmpty())
                                proc->writeStdin(TQString("Name-Comment:%1").arg(kcomment));
                        if (kexp==0)
                                proc->writeStdin(TQString("Expire-Date:0"));
                        if (kexp==1)
                                proc->writeStdin(TQString("Expire-Date:%1").arg(knumb));
                        if (kexp==2)
                                proc->writeStdin(TQString("Expire-Date:%1w").arg(knumb));

                        if (kexp==3)
                                proc->writeStdin(TQString("Expire-Date:%1m").arg(knumb));

                        if (kexp==4)
                                proc->writeStdin(TQString("Expire-Date:%1y").arg(knumb));
                        proc->writeStdin(TQString("%commit"));
                        TQObject::connect(proc,TQT_SIGNAL(readReady(KProcIO *)),TQT_TQOBJECT(this),TQT_SLOT(readgenprocess(KProcIO *)));
                        proc->closeWhenDone();
                } else  ////// start expert (=konsole) mode
                {
                        TDEProcess kp;

                        TDEConfig *config = TDEGlobal::config();
                        config->setGroup("General");
                        kp<< config->readPathEntry("TerminalApplication","konsole");
                        kp<<"-e"
                        <<"gpg"
                        <<"--gen-key";
                        kp.start(TDEProcess::Block);
                        refreshkey();
                }
        }
}

void listKeys::readgenprocess(KProcIO *p)
{
        TQString required;
        while (p->readln(required,true)!=-1) {
                if (required.find("KEY_CREATED")!=-1)
                        newkeyFinger=required.stripWhiteSpace().section(' ',-1);
                message+=required+"\n";
        }

        //  sample:   [GNUPG:] KEY_CREATED B 156A4305085A58C01E2988229282910254D1B368
}

void listKeys::genover(TDEProcess *)
{
        newkeyID=TQString();
        continueSearch=true;
        KProcIO *conprocess=new KProcIO(TQTextCodec::codecForLocale());
        *conprocess<< "gpg";
        *conprocess<<"--no-secmem-warning"<<"--with-colons"<<"--fingerprint"<<"--list-keys"<<newKeyName;
        TQObject::connect(conprocess,TQT_SIGNAL(readReady(KProcIO *)),TQT_TQOBJECT(this),TQT_SLOT(slotReadFingerProcess(KProcIO *)));
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(TDEProcess *)),TQT_TQOBJECT(this), TQT_SLOT(newKeyDone(TDEProcess *)));
        conprocess->start(TDEProcess::NotifyOnExit,true);
}


void listKeys::slotReadFingerProcess(KProcIO *p)
{
        TQString outp;
        while (p->readln(outp)!=-1) {
                if (outp.startsWith("pub") && (continueSearch)) {
                        newkeyID=outp.section(':',4,4).right(8).prepend("0x");

                }
                if (outp.startsWith("fpr")) {
                        if (newkeyFinger.lower()==outp.section(':',9,9).lower())
                                continueSearch=false;
                        //			kdDebug(2100)<<newkeyFinger<<" test:"<<outp.section(':',9,9)<<endl;
                }
        }
}


void listKeys::newKeyDone(TDEProcess *)
{
        changeMessage(i18n("Ready"),0);
        //        refreshkey();
        if (newkeyID.isEmpty()) {
                delete pop;
                KMessageBox::detailedSorry(this,i18n("Something unexpected happened during the key pair creation.\nPlease check details for full log output."),message);
                refreshkey();
                return;
        }
        keysList2->refreshcurrentkey(newkeyID);
        changeMessage(i18n("%1 Keys, %2 Groups").arg(keysList2->childCount()-keysList2->groupNb).arg(keysList2->groupNb),1);
        KDialogBase *keyCreated=new KDialogBase( this, "key_created", true,i18n("New Key Pair Created"), KDialogBase::Ok);
        newKey *page=new newKey(keyCreated);
        page->TLname->setText("<b>"+newKeyName+"</b>");
        page->TLemail->setText("<b>"+newKeyMail+"</b>");
	if (!newKeyMail.isEmpty())
	page->kURLRequester1->setURL(TQDir::homeDirPath()+"/"+newKeyMail.section("@",0,0)+".revoke");
	else
	page->kURLRequester1->setURL(TQDir::homeDirPath()+"/"+newKeyName.section(" ",0,0)+".revoke");
        page->TLid->setText("<b>"+newkeyID+"</b>");
        page->LEfinger->setText(newkeyFinger);
        page->CBdefault->setChecked(true);
        page->show();
        //page->resize(page->maximumSize());
        keyCreated->setMainWidget(page);
        delete pop;
        keyCreated->exec();

        TQListViewItem *newdef = keysList2->findItem(newkeyID,6);
        if (newdef)
                if (page->CBdefault->isChecked())
                        slotSetDefaultKey(newdef);
                else {
                        keysList2->clearSelection();
                        keysList2->setCurrentItem(newdef);
                        keysList2->setSelected(newdef,true);
                        keysList2->ensureItemVisible(newdef);
                }
        if (page->CBsave->isChecked()) {
                slotrevoke(newkeyID,page->kURLRequester1->url(),0,i18n("backup copy"));
                if (page->CBprint->isChecked())
                        connect(revKeyProcess,TQT_SIGNAL(revokeurl(TQString)),TQT_TQOBJECT(this),TQT_SLOT(doFilePrint(TQString)));
        } else if (page->CBprint->isChecked()) {
                slotrevoke(newkeyID,TQString(),0,i18n("backup copy"));
                connect(revKeyProcess,TQT_SIGNAL(revokecertificate(TQString)),TQT_TQOBJECT(this),TQT_SLOT(doPrint(TQString)));
        }
}

void listKeys::doFilePrint(TQString url)
{
        TQFile qfile(url);
        if (qfile.open(IO_ReadOnly)) {
                TQTextStream t( &qfile );
                doPrint(t.read());
        } else
                KMessageBox::sorry(this,i18n("<qt>Cannot open file <b>%1</b> for printing...</qt>").arg(url));
}

void listKeys::doPrint(TQString txt)
{
        KPrinter prt;
        //kdDebug(2100)<<"Printing..."<<endl;
        if (prt.setup(this)) {
                TQPainter painter(&prt);
                TQPaintDeviceMetrics metrics(painter.device());
                painter.drawText( 0, 0, metrics.width(), metrics.height(), AlignLeft|AlignTop|DontClip,txt );
        }
}

void listKeys::deleteseckey()
{
        //////////////////////// delete a key
        TQString res=keysList2->currentItem()->text(0)+" ("+keysList2->currentItem()->text(1)+")";
        int result=KMessageBox::warningContinueCancel(this,
                        i18n("<p>Delete <b>SECRET KEY</b> pair <b>%1</b>?</p>Deleting this key pair means you will never be able to decrypt files encrypted with this key again.").arg(res),
                        i18n("Warning"),
                        KGuiItem(i18n("Delete"),"editdelete"));
        if (result!=KMessageBox::Continue)
                return;

        TDEProcess *conprocess=new TDEProcess();
        TDEConfig *config = TDEGlobal::config();
        config->setGroup("General");
        *conprocess<< config->readPathEntry("TerminalApplication","konsole");
        *conprocess<<"-e"<<"gpg"
        <<"--no-secmem-warning"
        <<"--delete-secret-key"<<keysList2->currentItem()->text(6);
        TQObject::connect(conprocess, TQT_SIGNAL(processExited(TDEProcess *)),TQT_TQOBJECT(this), TQT_SLOT(reloadSecretKeys()));
        conprocess->start(TDEProcess::NotifyOnExit,TDEProcess::AllOutput);
}

void listKeys::reloadSecretKeys()
{
        FILE *fp;
        char line[300];
        keysList2->secretList=TQString();
        fp = popen("gpg --no-secmem-warning --no-tty --with-colons --list-secret-keys", "r");
        while ( fgets( line, sizeof(line), fp)) {
                TQString lineRead=TQString::fromUtf8(line);
                if (lineRead.startsWith("sec"))
                        keysList2->secretList+="0x"+lineRead.section(':',4,4).right(8)+",";
        }
        pclose(fp);
        deletekey();
}

void listKeys::confirmdeletekey()
{
        if (keysList2->currentItem()->depth()!=0) {
                if ((keysList2->currentItem()->depth()==1) && (keysList2->currentItem()->text(4)=="-") && (keysList2->currentItem()->text(6).startsWith("0x")))
                        delsignkey();
                return;
        }
        if (keysList2->currentItem()->text(6).isEmpty()) {
                deleteGroup();
                return;
        }
        if (((keysList2->secretList.find(keysList2->currentItem()->text(6))!=-1) || (keysList2->orphanList.find(keysList2->currentItem()->text(6))!=-1)) && (keysList2->selectedItems().count()==1))
                deleteseckey();
        else {
                TQStringList keysToDelete;
                TQString secList;
                TQPtrList<TQListViewItem> exportList=keysList2->selectedItems();
                bool secretKeyInside=false;
                for ( uint i = 0; i < exportList.count(); ++i )
                        if ( exportList.at(i) ) {
				if (keysList2->secretList.find(exportList.at(i)->text(6))!=-1) {
                                        secretKeyInside=true;
                                        secList+=exportList.at(i)->text(0)+" ("+exportList.at(i)->text(1)+")<br>";
                                        exportList.at(i)->setSelected(false);
                                } else
                                        keysToDelete+=exportList.at(i)->text(0)+" ("+exportList.at(i)->text(1)+")";
                        }

                if (secretKeyInside) {
                        int result=KMessageBox::warningContinueCancel(this,i18n("<qt>The following are secret key pairs:<br><b>%1</b>They will not be deleted.<br></qt>").arg(secList));
                        if (result!=KMessageBox::Continue)
                                return;
                }
                if (keysToDelete.isEmpty())
                        return;
		int result=KMessageBox::warningContinueCancelList(this,i18n("<qt><b>Delete the following public key?</b></qt>","<qt><b>Delete the following %n public keys?</b></qt>",keysToDelete.count()),keysToDelete,i18n("Warning"),KStdGuiItem::del());
                if (result!=KMessageBox::Continue)
                        return;
                else
                        deletekey();
        }
}

void listKeys::deletekey()
{
        TQPtrList<TQListViewItem> exportList=keysList2->selectedItems();
        if (exportList.count()==0)
                return;
        TDEProcess gp;
        gp << "gpg"
        << "--no-tty"
        << "--no-secmem-warning"
        << "--batch"
        << "--yes"
        << "--delete-key";
        for ( uint i = 0; i < exportList.count(); ++i )
                if ( exportList.at(i) )
                        gp<<(exportList.at(i)->text(6)).stripWhiteSpace();
        gp.start(TDEProcess::Block);

        for ( uint i = 0; i < exportList.count(); ++i )
                if ( exportList.at(i) )
                        keysList2->refreshcurrentkey(exportList.at(i));
        if (keysList2->currentItem()) {
                TQListViewItem * myChild = keysList2->currentItem();
                while(!myChild->isVisible()) {
                        myChild = myChild->nextSibling();
                        if (!myChild)
                                break;
                }
                if (!myChild) {
                        TQListViewItem * myChild = keysList2->firstChild();
                        while(!myChild->isVisible()) {
                                myChild = myChild->nextSibling();
                                if (!myChild)
                                        break;
                        }
                }
                if (myChild) {
                        myChild->setSelected(true);
                        keysList2->setCurrentItem(myChild);
                }
        }
	else stateChanged("empty_list");
        changeMessage(i18n("%1 Keys, %2 Groups").arg(keysList2->childCount()-keysList2->groupNb).arg(keysList2->groupNb),1);
}


void listKeys::slotPreImportKey()
{
        KDialogBase *dial=new KDialogBase( KDialogBase::Swallow, i18n("Key Import"), KDialogBase::Ok | KDialogBase::Cancel, KDialogBase::Ok, this, "key_import",true);

        SrcSelect *page=new SrcSelect();
        dial->setMainWidget(page);
        page->newFilename->setCaption(i18n("Open File"));
        page->newFilename->setMode(KFile::File);
        page->resize(page->minimumSize());
        dial->resize(dial->minimumSize());

        if (dial->exec()==TQDialog::Accepted) {
                if (page->checkFile->isChecked()) {
                        TQString impname=page->newFilename->url().stripWhiteSpace();
                        if (!impname.isEmpty()) {
                                changeMessage(i18n("Importing..."),0,true);
                                ////////////////////////// import from file
                                KgpgInterface *importKeyProcess=new KgpgInterface();
                                importKeyProcess->importKeyURL(KURL::fromPathOrURL( impname ));
                                connect(importKeyProcess,TQT_SIGNAL(importfinished(TQStringList)),keysList2,TQT_SLOT(slotReloadKeys(TQStringList)));
                                connect(importKeyProcess,TQT_SIGNAL(refreshOrphaned()),keysList2,TQT_SLOT(slotReloadOrphaned()));
                        }
                } else {
                        TQString keystr = kapp->clipboard()->text(clipboardMode);
                        if (!keystr.isEmpty()) {
                                changeMessage(i18n("Importing..."),0,true);
                                KgpgInterface *importKeyProcess=new KgpgInterface();
                                importKeyProcess->importKey(keystr);
                                connect(importKeyProcess,TQT_SIGNAL(importfinished(TQStringList)),keysList2,TQT_SLOT(slotReloadKeys(TQStringList)));
                                connect(importKeyProcess,TQT_SIGNAL(refreshOrphaned()),keysList2,TQT_SLOT(slotReloadOrphaned()));
                        }
                }
        }
        delete dial;
}


void KeyView::expandGroup(TQListViewItem *item)
{

        TQStringList keysGroup=KgpgInterface::getGpgGroupSetting(item->text(0),KGpgSettings::gpgConfigPath());
        kdDebug(2100)<<keysGroup<<endl;
        for ( TQStringList::Iterator it = keysGroup.begin(); it != keysGroup.end(); ++it ) {
                UpdateViewItem *item2=new UpdateViewItem(item,TQString(*it),TQString(),TQString(),TQString(),TQString(),TQString(),TQString());
                item2->setPixmap(0,pixkeyGroup);
                item2->setExpandable(false);
        }
}

TQPixmap KeyView::slotGetPhoto(TQString photoId,bool mini)
{
        KTempFile *phototmp=new KTempFile();
        TQString popt="cp %i "+phototmp->name();
        KProcIO *p=new KProcIO(TQTextCodec::codecForLocale());
        *p<<"gpg"<<"--show-photos"<<"--photo-viewer"<<TQString(TQFile::encodeName(popt))<<"--list-keys"<<photoId;
        p->start(TDEProcess::Block);

        TQPixmap pixmap;

        pixmap.load(phototmp->name());
        TQImage dup=pixmap.convertToImage();
        TQPixmap dup2;
        if (!mini)
                dup2.convertFromImage(dup.scale(previewSize+5,previewSize,TQ_ScaleMin));
        else
                dup2.convertFromImage(dup.scale(22,22,TQ_ScaleMin));
        phototmp->unlink();
        delete phototmp;
        return dup2;
}

void KeyView::expandKey(TQListViewItem *item)
{

        if (item->childCount()!=0)
                return;   // key has already been expanded
        FILE *fp;
        TQString cycle;
        TQStringList tst;
        char tmpline[300];
        UpdateViewItem *itemsub=NULL;
        UpdateViewItem *itemuid=NULL;
        UpdateViewItem *itemsig=NULL;
        UpdateViewItem *itemrev=NULL;
        TQPixmap keyPhotoId;
        int uidNumber=2;
        bool dropFirstUid=false;

        kdDebug(2100)<<"Expanding Key: "<<item->text(6)<<endl;

        cycle="pub";
        bool noID=false;
        fp = popen(TQFile::encodeName(TQString("gpg --no-secmem-warning --no-tty --with-colons --list-sigs "+item->text(6))), "r");

        while ( fgets( tmpline, sizeof(tmpline), fp)) {
                TQString line = TQString::fromUtf8( tmpline );
                tst=TQStringList::split(":",line,true);
                if ((tst[0]=="pub") && (tst[9].isEmpty())) /// Primary User Id is separated from public key
                        uidNumber=1;
                if (tst[0]=="uid" || tst[0]=="uat") {
                        if (dropFirstUid) {
                                dropFirstUid=false;
                        } else {
                                gpgKey uidKey=extractKey(line);

                                if (tst[0]=="uat") {
                                        kdDebug(2100)<<"Found photo at uid "<<uidNumber<<endl;
                                        itemuid= new UpdateViewItem(item,i18n("Photo id"),TQString(),TQString(),"-","-","-",TQString::number(uidNumber));
                                        if (displayPhoto) {
                                                kgpgphototmp=new KTempFile();
                                                kgpgphototmp->setAutoDelete(true);
                                                TQString pgpgOutput="cp %i "+kgpgphototmp->name();
                                                KProcIO *p=new KProcIO(TQTextCodec::codecForLocale());
                                                *p<<"gpg"<<"--no-tty"<<"--photo-viewer"<<TQString(TQFile::encodeName(pgpgOutput));
                                                *p<<"--edit-key"<<item->text(6)<<"uid"<<TQString::number(uidNumber)<<"showphoto"<<"quit";
                                                p->start(TDEProcess::Block);
                                                TQPixmap pixmap;
                                                pixmap.load(kgpgphototmp->name());
                                                TQImage dup=pixmap.convertToImage();
                                                TQPixmap dup2;
                                                dup2.convertFromImage(dup.scale(previewSize+5,previewSize,TQ_ScaleMin));
                                                itemuid->setPixmap(0,dup2);
                                                delete kgpgphototmp;
                                                //itemuid->setPixmap(0,keyPhotoId);
                                        } else
                                                itemuid->setPixmap(0,pixuserphoto);
                                        itemuid->setPixmap(2,uidKey.trustpic);
                                        cycle="uid";
                                } else {
                                        kdDebug(2100)<<"Found uid at "<<uidNumber<<endl;
                                        itemuid= new UpdateViewItem(item,uidKey.gpgkeyname,uidKey.gpgkeymail,TQString(),"-","-","-","-");
                                        itemuid->setPixmap(2,uidKey.trustpic);
                                        if (noID) {
                                                item->setText(0,uidKey.gpgkeyname);
                                                item->setText(1,uidKey.gpgkeymail);
                                                noID=false;
                                        }
                                        itemuid->setPixmap(0,pixuserid);
                                        cycle="uid";
                                }
                        }
                        uidNumber++;
                } else
                        if (tst[0]=="rev") {
                                gpgKey revKey=extractKey(line);
                                if (cycle=="uid" || cycle=="uat")
                                        itemrev= new UpdateViewItem(itemuid,revKey.gpgkeyname,revKey.gpgkeymail+i18n(" [Revocation signature]"),"-","-","-",revKey.gpgkeycreation,revKey.gpgkeyid);
                                else if (cycle=="pub") { //////////////public key revoked
                                        itemrev= new UpdateViewItem(item,revKey.gpgkeyname,revKey.gpgkeymail+i18n(" [Revocation signature]"),"-","-","-",revKey.gpgkeycreation,revKey.gpgkeyid);
                                        dropFirstUid=true;
                                } else if (cycle=="sub")
                                        itemrev= new UpdateViewItem(itemsub,revKey.gpgkeyname,revKey.gpgkeymail+i18n(" [Revocation signature]"),"-","-","-",revKey.gpgkeycreation,revKey.gpgkeyid);
                                itemrev->setPixmap(0,pixRevoke);
                        } else


                                if (tst[0]=="sig") {
                                        gpgKey sigKey=extractKey(line);

                                        if (tst[10].endsWith("l"))
                                                sigKey.gpgkeymail+=i18n(" [local]");

                                        if (cycle=="pub")
                                                itemsig= new UpdateViewItem(item,sigKey.gpgkeyname,sigKey.gpgkeymail,"-",sigKey.gpgkeyexpiration,"-",sigKey.gpgkeycreation,sigKey.gpgkeyid);
                                        if (cycle=="sub")
                                                itemsig= new UpdateViewItem(itemsub,sigKey.gpgkeyname,sigKey.gpgkeymail,"-",sigKey.gpgkeyexpiration,"-",sigKey.gpgkeycreation,sigKey.gpgkeyid);
                                        if (cycle=="uid")
                                                itemsig= new UpdateViewItem(itemuid,sigKey.gpgkeyname,sigKey.gpgkeymail,"-",sigKey.gpgkeyexpiration,"-",sigKey.gpgkeycreation,sigKey.gpgkeyid);

                                        itemsig->setPixmap(0,pixsignature);
                                } else
                                        if (tst[0]=="sub") {
                                                gpgKey subKey=extractKey(line);
                                                itemsub= new UpdateViewItem(item,i18n("%1 subkey").arg(subKey.gpgkeyalgo),TQString(),TQString(),subKey.gpgkeyexpiration,subKey.gpgkeysize,subKey.gpgkeycreation,subKey.gpgkeyid);
                                                itemsub->setPixmap(0,pixkeySingle);
                                                itemsub->setPixmap(2,subKey.trustpic);
                                                cycle="sub";

                                        }
        }
        pclose(fp);
}


void listKeys::refreshkey()
{
        keysList2->refreshkeylist();
	listViewSearch->updateSearch(listViewSearch->text());
}

void KeyView::refreshkeylist()
{
        emit statusMessage(i18n("Loading Keys..."),0,true);
        kapp->processEvents();
        ////////   update display of keys in main management window
        kdDebug(2100)<<"Refreshing key list"<<endl;
        TQString tst;
        char line[300];
        UpdateViewItem *item=NULL;
        bool noID=false;
        bool emptyList=true;
        TQString openKeys;

        // get current position.
        TQListViewItem *current = currentItem();
        if(current != NULL) {
                while(current->depth() > 0) {
                        current = current->parent();
                }
                takeItem(current);
        }

        // refill
        clear();
        FILE *fp2,*fp;
        TQStringList issec;
        secretList=TQString();
        orphanList=TQString();
        fp2 = popen("gpg --no-secmem-warning --no-tty --with-colons --list-secret-keys", "r");
        while ( fgets( line, sizeof(line), fp2)) {
                TQString lineRead=TQString::fromUtf8(line);
                kdDebug(2100) << k_funcinfo << "Read one secret key line: " << lineRead << endl;
                if (lineRead.startsWith("sec"))
                        issec<<lineRead.section(':',4,4).right(8);
        }
        pclose(fp2);

        TQString defaultKey = KGpgSettings::defaultKey();
        fp = popen("gpg --no-secmem-warning --no-tty --with-colons --list-keys", "r");
        while ( fgets( line, sizeof(line), fp)) {
                tst=TQString::fromUtf8(line);
                kdDebug(2100) << k_funcinfo << "Read one public key line: " << tst << endl;
                if (tst.startsWith("pub")) {
                        emptyList=false;
                        noID=false;
                        gpgKey pubKey=extractKey(tst);

                        bool isbold=false;
                        bool isexpired=false;
                        if (pubKey.gpgkeyid==defaultKey)
                                isbold=true;
                        if (pubKey.gpgkeytrust==i18n("Expired"))
                                isexpired=true;
                        if (pubKey.gpgkeyname.isEmpty())
                                noID=true;

                        item=new UpdateViewItem(this,pubKey.gpgkeyname,pubKey.gpgkeymail,TQString(),pubKey.gpgkeyexpiration,pubKey.gpgkeysize,pubKey.gpgkeycreation,pubKey.gpgkeyid,isbold,isexpired);

                        item->setPixmap(2,pubKey.trustpic);
                        item->setExpandable(true);


                        TQStringList::Iterator ite;
                        ite=issec.find(pubKey.gpgkeyid.right(8));
                        if (ite!=issec.end()) {
                                item->setPixmap(0,pixkeyPair);
                                secretList+=pubKey.gpgkeyid;
                                issec.remove(*ite);
                        } else item->setPixmap(0,pixkeySingle);

                        if (openKeys.find(pubKey.gpgkeyid)!=-1)
                                item->setOpen(true);
                }

        }
        pclose(fp);
        if (!issec.isEmpty())
                insertOrphanedKeys(issec);
        if (emptyList) {
                kdDebug(2100)<<"No key found"<<endl;
                emit statusMessage(i18n("Ready"),0);
                return;
        }
        kdDebug(2100)<<"Checking Groups"<<endl;
        TQStringList groups=KgpgInterface::getGpgGroupNames(KGpgSettings::gpgConfigPath());
        groupNb=groups.count();
        for ( TQStringList::Iterator it = groups.begin(); it != groups.end(); ++it )
                if (!TQString(*it).isEmpty()) {
                        item=new UpdateViewItem(this,TQString(*it),TQString(),TQString(),TQString(),TQString(),TQString(),TQString(),false,false);
                        item->setPixmap(0,pixkeyGroup);
                        item->setExpandable(false);
                }
        kdDebug(2100)<<"Finished Groups"<<endl;

        TQListViewItem *newPos=0L;
        if(current != NULL) {
                // select previous selected
                if (!current->text(6).isEmpty())
                        newPos = findItem(current->text(6), 6);
                else
                        newPos = findItem(current->text(0), 0);
                delete current;
        }

        if (newPos != 0L) {
                setCurrentItem(newPos);
                setSelected(newPos, true);
                ensureItemVisible(newPos);
        } else {
                setCurrentItem(firstChild());
                setSelected(firstChild(),true);
        }

        emit statusMessage(i18n("%1 Keys, %2 Groups").arg(childCount()-groupNb).arg(groupNb),1);
        emit statusMessage(i18n("Ready"),0);
        kdDebug(2100)<<"Refresh Finished"<<endl;
}

void KeyView::insertOrphan(TQString currentID)
{
        FILE *fp;
        char line[300];
        UpdateViewItem *item=NULL;
        bool keyFound=false;
        fp = popen("gpg --no-secmem-warning --no-tty --with-colons --list-secret-keys", "r");
        while ( fgets( line, sizeof(line), fp)) {
                TQString lineRead=TQString::fromUtf8(line);
                if ((lineRead.startsWith("sec")) && (lineRead.section(':',4,4).right(8))==currentID.right(8)) {
                        gpgKey orphanedKey=extractKey(lineRead);
                        keyFound=true;
                        bool isbold=false;
                        bool isexpired=false;
                        //           if (orphanedKey.gpgkeyid==defaultKey)
                        //               isbold=true;
                        if (orphanedKey.gpgkeytrust==i18n("Expired"))
                                isexpired=true;
                        //          if (orphanedKey.gpgkeyname.isEmpty())
                        //              noID=true;

                        item=new UpdateViewItem(this,orphanedKey.gpgkeyname,orphanedKey.gpgkeymail,TQString(),orphanedKey.gpgkeyexpiration,orphanedKey.gpgkeysize,orphanedKey.gpgkeycreation,orphanedKey.gpgkeyid,isbold,isexpired);
                        item->setPixmap(0,pixkeyOrphan);
                }
        }
        pclose(fp);
        if (!keyFound) {
                orphanList.remove(currentID);
                setSelected(currentItem(),true);
                return;
        }
        clearSelection();
        setCurrentItem(item);
        setSelected(item,true);
}

void KeyView::insertOrphanedKeys(TQStringList orphans)
{
        FILE *fp;
        char line[300];
        fp = popen("gpg --no-secmem-warning --no-tty --with-colons --list-secret-keys", "r");
        while ( fgets( line, sizeof(line), fp)) {
                TQString lineRead=TQString::fromUtf8(line);
                if ((lineRead.startsWith("sec")) && (orphans.find(lineRead.section(':',4,4).right(8))!=orphans.end())) {
                        gpgKey orphanedKey=extractKey(lineRead);

                        bool isbold=false;
                        bool isexpired=false;
                        //           if (orphanedKey.gpgkeyid==defaultKey)
                        //               isbold=true;
                        if (orphanedKey.gpgkeytrust==i18n("Expired"))
                                isexpired=true;
                        //          if (orphanedKey.gpgkeyname.isEmpty())
                        //              noID=true;
                        orphanList+=orphanedKey.gpgkeyid+",";
                        UpdateViewItem *item=new UpdateViewItem(this,orphanedKey.gpgkeyname,orphanedKey.gpgkeymail,TQString(),orphanedKey.gpgkeyexpiration,orphanedKey.gpgkeysize,orphanedKey.gpgkeycreation,orphanedKey.gpgkeyid,isbold,isexpired);
                        item->setPixmap(0,pixkeyOrphan);
                }
        }
        pclose(fp);
}

void KeyView::refreshgroups()
{
        TQListViewItem *item=firstChild();
        while (item) {
                if (item->text(6).isEmpty()) {
                        TQListViewItem *item2=item->nextSibling();
                        delete item;
                        item=item2;
                } else
                        item=item->nextSibling();
        }

        TQStringList groups=KgpgInterface::getGpgGroupNames(KGpgSettings::gpgConfigPath());
        groupNb=groups.count();
        for ( TQStringList::Iterator it = groups.begin(); it != groups.end(); ++it )
                if (!TQString(*it).isEmpty()) {
                        item=new UpdateViewItem(this,TQString(*it),TQString(),TQString(),TQString(),TQString(),TQString(),TQString(),false,false);
                        item->setPixmap(0,pixkeyGroup);
                        item->setExpandable(false);
                }
        emit statusMessage(i18n("%1 Keys, %2 Groups").arg(childCount()-groupNb).arg(groupNb),1);
        emit statusMessage(i18n("Ready"),0);
}

void KeyView::refreshselfkey()
{
        kdDebug(2100)<<"Refreshing key"<<endl;
        if (currentItem()->depth()==0)
                refreshcurrentkey(currentItem());
        else
                refreshcurrentkey(currentItem()->parent());
}

void KeyView::slotReloadKeys(TQStringList keyIDs)
{
        if (keyIDs.isEmpty())
                return;
	if (keyIDs.first()=="ALL")
	{
	refreshkeylist();
	return;
	}
        for ( TQStringList::Iterator it = keyIDs.begin(); it != keyIDs.end(); ++it ) {
                refreshcurrentkey(*it);
        }
        kdDebug(2100)<<"Refreshing key:--------"<<TQString((keyIDs.last()).right(8).prepend("0x"))<<endl;
        ensureItemVisible(this->findItem((keyIDs.last()).right(8).prepend("0x"),6));
        emit statusMessage(i18n("%1 Keys, %2 Groups").arg(childCount()-groupNb).arg(groupNb),1);
        emit statusMessage(i18n("Ready"),0);
}

void KeyView::slotReloadOrphaned()
{
        TQStringList issec;
        FILE *fp,*fp2;
        char line[300];

        fp2 = popen("gpg --no-secmem-warning --no-tty --with-colons --list-secret-keys", "r");
        while ( fgets( line, sizeof(line), fp2)) {
                TQString lineRead=TQString::fromUtf8(line);
                if (lineRead.startsWith("sec"))
                        issec<<"0x"+lineRead.section(':',4,4).right(8);
        }
        pclose(fp2);

        fp = popen("gpg --no-secmem-warning --no-tty --with-colons --list-keys", "r");
        while ( fgets( line, sizeof(line), fp)) {
                TQString lineRead=TQString::fromUtf8(line);
                if (lineRead.startsWith("pub"))
                        issec.remove("0x"+lineRead.section(':',4,4).right(8));
        }
        pclose(fp);

        TQStringList::Iterator it;

        for ( it = issec.begin(); it != issec.end(); ++it ) {
                if (findItem(*it,6)==0) {
                        insertOrphan(*it);
                        orphanList+=*it+",";
                }
        }
        setSelected(findItem(*it,6),true);
        emit statusMessage(i18n("%1 Keys, %2 Groups").arg(childCount()-groupNb).arg(groupNb),1);
        emit statusMessage(i18n("Ready"),0);
}

void KeyView::refreshcurrentkey(TQString currentID)
{
	if (currentID.isNull()) return;
        UpdateViewItem *item=NULL;
        TQString issec=TQString();
        FILE *fp,*fp2;
        char line[300];

        fp2 = popen("gpg --no-secmem-warning --no-tty --with-colons --list-secret-keys", "r");
        while ( fgets( line, sizeof(line), fp2)) {
                TQString lineRead=TQString::fromUtf8(line);
                if (lineRead.startsWith("sec"))
                        issec+=lineRead.section(':',4,4);
        }
        pclose(fp2);

        TQString defaultKey = KGpgSettings::defaultKey();

        TQString tst;
        bool keyFound=false;
        TQString cmd="gpg --no-secmem-warning --no-tty --with-colons --list-keys "+currentID;
        fp = popen(TQFile::encodeName(cmd), "r");
        while ( fgets( line, sizeof(line), fp)) {
                tst=TQString::fromUtf8(line);
                if (tst.startsWith("pub")) {
                        gpgKey pubKey=extractKey(tst);
                        keyFound=true;
                        bool isbold=false;
                        bool isexpired=false;
                        if (pubKey.gpgkeyid==defaultKey)
                                isbold=true;
                        if (pubKey.gpgkeytrust==i18n("Expired"))
                                isexpired=true;
                        item=new UpdateViewItem(this,pubKey.gpgkeyname,pubKey.gpgkeymail,TQString(),pubKey.gpgkeyexpiration,pubKey.gpgkeysize,pubKey.gpgkeycreation,pubKey.gpgkeyid,isbold,isexpired);
                        item->setPixmap(2,pubKey.trustpic);
			item->setVisible(true);
                        item->setExpandable(true);
                        if (issec.find(pubKey.gpgkeyid.right(8),0,FALSE)!=-1) {
                                item->setPixmap(0,pixkeyPair);
                                secretList+=pubKey.gpgkeyid;
                        } else {
                                item->setPixmap(0,pixkeySingle);
                        }
                }
        }
        pclose(fp);

        if (!keyFound) {
                if (orphanList.find(currentID)==-1)
                        orphanList+=currentID+",";
                insertOrphan(currentID);
                return;
        }
        if (orphanList.find(currentID)!=-1)
                orphanList.remove(currentID);

        clearSelection();
        setCurrentItem(item);

}

void KeyView::refreshcurrentkey(TQListViewItem *current)
{
        if (!current)
                return;
        bool keyIsOpen=false;
        TQString keyUpdate=current->text(6);
        if (keyUpdate.isEmpty())
                return;
        if (current->isOpen())
                keyIsOpen=true;
        delete current;
        refreshcurrentkey(keyUpdate);
        if (currentItem())
                if (currentItem()->text(6)==keyUpdate)
                        currentItem()->setOpen(keyIsOpen);
}


void KeyView::refreshTrust(int color,TQColor newColor)
{
if (!newColor.isValid()) return;
TQPixmap blankFrame,newtrust;
int trustFinger=0;
blankFrame.load(locate("appdata", "pics/kgpg_blank.png"));
newtrust.load(locate("appdata", "pics/kgpg_fill.png"));
newtrust.fill(newColor);
bitBlt(&newtrust,0,0,&blankFrame,0,0,50,15);
switch (color)
{
case GoodColor:
trustFinger=trustgood.serialNumber();
trustgood=newtrust;
break;
case BadColor:
trustFinger=trustbad.serialNumber();
trustbad=newtrust;
break;
case UnknownColor:
trustFinger=trustunknown.serialNumber();
trustunknown=newtrust;
break;
case RevColor:
trustFinger=trustrevoked.serialNumber();
trustrevoked=newtrust;
break;
}
TQListViewItem *item=firstChild();
                while (item) {
			if (item->pixmap(2))
			{
                        if (item->pixmap(2)->serialNumber()==trustFinger) item->setPixmap(2,newtrust);
			}
			item=item->nextSibling();
                }
}

gpgKey KeyView::extractKey(TQString keyColon)
{
        TQStringList keyString=TQStringList::split(":",keyColon,true);
        gpgKey ret;

        ret.gpgkeysize=keyString[2];
        ret.gpgkeycreation=keyString[5];
        if(!ret.gpgkeycreation.isEmpty()) {
                TQDate date = TQDate::fromString(ret.gpgkeycreation, Qt::ISODate);
                ret.gpgkeycreation=TDEGlobal::locale()->formatDate(date, true);
        }
        TQString tid=keyString[4];
        ret.gpgkeyid=TQString("0x"+tid.right(8));
        ret.gpgkeyexpiration=keyString[6];
        if (ret.gpgkeyexpiration.isEmpty())
                ret.gpgkeyexpiration=i18n("Unlimited");
        else {
                TQDate date = TQDate::fromString(ret.gpgkeyexpiration, Qt::ISODate);
                ret.gpgkeyexpiration=TDEGlobal::locale()->formatDate(date, true);
        }
        TQString fullname=keyString[9];
        if (fullname.find("<")!=-1) {
                ret.gpgkeymail=fullname.section('<',-1,-1);
                ret.gpgkeymail.truncate(ret.gpgkeymail.length()-1);
                ret.gpgkeyname=fullname.section('<',0,0);
                //ret.gpgkeyname=ret.gpgkeyname.section('(',0,0);
        } else {
                ret.gpgkeymail=TQString();
		ret.gpgkeyname=fullname;
                //ret.gpgkeyname=fullname.section('(',0,0);
        }

        //ret.gpgkeyname=KgpgInterface::checkForUtf8(ret.gpgkeyname); // FIXME lukas

        TQString algo=keyString[3];
        if (!algo.isEmpty())
                switch( algo.toInt() ) {
                case  1:
                        algo=i18n("RSA");
                        break;
                case 16:
                case 20:
                        algo=i18n("ElGamal");
                        break;
                case 17:
                        algo=i18n("DSA");
                        break;
                default:
                        algo=TQString("#" + algo);
                        break;
                }
        ret.gpgkeyalgo=algo;

        const TQString trust=keyString[1];
        switch( trust[0] ) {
        case 'o':
                ret.gpgkeytrust=i18n("Unknown");
                ret.trustpic=trustunknown;
                break;
        case 'i':
                ret.gpgkeytrust=i18n("Invalid");
                ret.trustpic=trustbad;
                break;
        case 'd':
                ret.gpgkeytrust=i18n("Disabled");
                ret.trustpic=trustbad;
                break;
        case 'r':
                ret.gpgkeytrust=i18n("Revoked");
                ret.trustpic=trustrevoked;
                break;
        case 'e':
                ret.gpgkeytrust=i18n("Expired");
                ret.trustpic=trustbad;
                break;
        case 'q':
                ret.gpgkeytrust=i18n("Undefined");
                ret.trustpic=trustunknown;
                break;
        case 'n':
                ret.gpgkeytrust=i18n("None");
                ret.trustpic=trustunknown;
                break;
        case 'm':
                ret.gpgkeytrust=i18n("Marginal");
                ret.trustpic=trustbad;
                break;
        case 'f':
                ret.gpgkeytrust=i18n("Full");
                ret.trustpic=trustgood;
                break;
        case 'u':
                ret.gpgkeytrust=i18n("Ultimate");
                ret.trustpic=trustgood;
                break;
        default:
                ret.gpgkeytrust=i18n("?");
                ret.trustpic=trustunknown;
                break;
        }
        if (keyString[11].find('D')!=-1) {
                ret.gpgkeytrust=i18n("Disabled");
                ret.trustpic=trustbad;
        }

        return ret;
}

#include "listkeys.moc"
