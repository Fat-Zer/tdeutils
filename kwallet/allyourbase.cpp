/*
   Copyright (C) 2003-2005 George Staikos <staikos@kde.org>
   Copyright (C) 2005 Isaac Clerencia <isaac@warp.es>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "allyourbase.h"

#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kio/netaccess.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kurldrag.h>
#include <kwallet.h>

#include <tqdragobject.h>
#include <tqfile.h>
#include <tqptrlist.h>

/****************
 *  KWalletFolderItem - ListView items to represent kwallet folders
 */
KWalletFolderItem::KWalletFolderItem(KWallet::Wallet *w, TQListView* parent, const TQString &name, int entries)
: KListViewItem(parent),_wallet(w),_name(name),_entries(entries) {
	setText(0, TQString("%1 (%2)").arg(_name).arg(_entries));
	setRenameEnabled(0, false);
	setDragEnabled(true);
	setDropEnabled(true);

	TQPixmap pix = getFolderIcon(KIcon::Small);
	
	setPixmap(0,pix);
}

TQPixmap KWalletFolderItem::getFolderIcon(KIcon::Group group){
	KIconLoader *loader = KGlobal::instance()->iconLoader();
	TQPixmap pix = loader->loadIcon( _name, group, 0,
			KIcon::DefaultState, 0, true );
	if (pix.isNull())
		pix = loader->loadIcon( _name.lower(), group, 0,
			KIcon::DefaultState, 0, true);
	if (pix.isNull())
		pix = loader->loadIcon( "folder_red", group, 0,
			KIcon::DefaultState, 0, true);
	return pix;
}

void KWalletFolderItem::refresh() {
	TQString saveFolder = _wallet->currentFolder();
	_wallet->setFolder(_name);
	setText(0, TQString("%1 (%2)").arg(_name).arg(_wallet->entryList().count()));
	_wallet->setFolder(saveFolder);
}

KWalletContainerItem* KWalletFolderItem::getContainer(KWallet::Wallet::EntryType type) {
	for (TQListViewItem *i = firstChild(); i; i = i->nextSibling()) {
		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem *>(i);
		if (!ci) {
			continue;
		}
		if (ci->type() == type) {
			return ci;
		}
	}
	return 0;
}

bool KWalletFolderItem::contains(const TQString& key) {
	return (getItem(key) != 0);
}

TQListViewItem* KWalletFolderItem::getItem(const TQString& key) {
	for (TQListViewItem *i = firstChild(); i; i = i->nextSibling()) {
		KWalletContainerItem *ci = dynamic_cast<KWalletContainerItem *>(i);
		if (!ci) {
			continue;
		}
		TQListViewItem *tmp = ci->getItem(key);
		if (tmp) {
			return tmp;
		}
	}
	return 0;
}

bool KWalletFolderItem::acceptDrop(const TQMimeSource *mime) const {
	return mime->provides("application/x-kwallet-entry") ||
		mime->provides("text/uri-list");
}

int KWalletFolderItem::rtti() const {
	return KWalletFolderItemClass;
}

TQString KWalletFolderItem::name() const {
	return _name;
}

KWalletFolderItem::~KWalletFolderItem() {
}

/****************
 *  KWalletContainerItem - ListView items to represent kwallet containers, i.e.
 *  passwords, maps, ...
 */
KWalletContainerItem::KWalletContainerItem(TQListViewItem* parent, const TQString &name, KWallet::Wallet::EntryType type)
: KListViewItem(parent, name), _type(type) {
	setRenameEnabled(0, false);
	setDragEnabled(true);
}

KWalletContainerItem::~KWalletContainerItem() {
}

int KWalletContainerItem::rtti() const {
	return KWalletContainerItemClass;
}

KWallet::Wallet::EntryType KWalletContainerItem::type() {
	return _type;
}

bool KWalletContainerItem::contains(const TQString& key) {
	return getItem(key) != 0;
}

TQListViewItem *KWalletContainerItem::getItem(const TQString& key) {
	for (TQListViewItem *i = firstChild(); i; i = i->nextSibling()) {
		if (i->text(0) == key) {
			return i;
		}
	}
	return 0;
}

/****************
 *  KWalletEntryItem - ListView items to represent kwallet entries
 */
KWalletEntryItem::KWalletEntryItem(KWallet::Wallet *w, TQListViewItem* parent, const TQString& ename)
: KListViewItem(parent, ename), _wallet(w), _oldName(ename) {
	setRenameEnabled(0, true);
	setDragEnabled(true);
}

int KWalletEntryItem::rtti() const {
	return KWalletEntryItemClass;
}

KWalletEntryItem::~KWalletEntryItem() {
}

/****************
 * KWalletItem - IconView items to represent wallets
 */
KWalletItem::KWalletItem(TQIconView *parent, const TQString& walletName)
: TQIconViewItem(parent, walletName, DesktopIcon("kwalletmanager")) {
}

KWalletItem::~KWalletItem() {
}

bool KWalletItem::acceptDrop(const TQMimeSource *mime) const {
	return mime->provides("application/x-kwallet-folder") ||
		mime->provides("text/uri-list");
}

static bool decodeEntry(KWallet::Wallet *_wallet, TQDataStream& ds) {
	TQ_UINT32 magic;
	ds >> magic;
	if (magic != KWALLETENTRYMAGIC) {
		kdDebug() << "bad magic" << endl;
		return false;
	}
	TQString name;
	TQByteArray value;
	KWallet::Wallet::EntryType et;
	ds >> name;
	if (_wallet->hasEntry(name)) {
		int rc = KMessageBox::warningContinueCancel(0L, i18n("An entry by the name '%1' already exists. Would you like to continue?").arg(name));
		if (rc == KMessageBox::Cancel) {
			return false;
		}
	}
	long l;
	ds >> l;
	et = KWallet::Wallet::EntryType(l);
	ds >> value;
	_wallet->writeEntry(name, value, et);
	return true;
}

static bool decodeFolder(KWallet::Wallet *_wallet, TQDataStream& ds) {
	TQ_UINT32 magic;
	ds >> magic;
	if (magic != KWALLETFOLDERMAGIC) {
		kdDebug() << "bad magic" << endl;
		return false;
	}
	TQString folder;
	ds >> folder;
	if (_wallet->hasFolder(folder)) {
		int rc = KMessageBox::warningYesNoCancel(0L, i18n("A folder by the name '%1' already exists.  What would you like to do?").arg(folder), TQString(), KStdGuiItem::cont(), i18n("Replace"));
		if (rc == KMessageBox::Cancel) {
			return false;
		}
		if (rc == KMessageBox::No) {
			_wallet->removeFolder(folder);
			_wallet->createFolder(folder);
		}
	} else {
		_wallet->createFolder(folder);
	}

	_wallet->setFolder(folder);
	while (!ds.atEnd()) {
		TQString name;
		TQByteArray value;
		KWallet::Wallet::EntryType et;
		ds >> name;
		long l;
		ds >> l;
		et = KWallet::Wallet::EntryType(l);
		ds >> value;
		_wallet->writeEntry(name, value, et);
	}
	return true;
}

void KWalletItem::dropped(TQDropEvent *e, const TQValueList<TQIconDragItem>& lst) {
	Q_UNUSED(lst);
	if (e->provides("application/x-kwallet-folder") || 
			e->provides("text/uri-list")) {

		// FIXME: don't allow the drop if the wallet name is the same

		KWallet::Wallet *_wallet = KWallet::Wallet::openWallet(text());
		if (!_wallet) {
			e->ignore();
			return;
		}

		TQString saveFolder = _wallet->currentFolder();

		TQFile file;
		TQDataStream *ds = 0L;

		if (e->provides("application/x-kwallet-folder")) {
			TQByteArray edata = e->encodedData("application/x-kwallet-folder");
			if (!edata.isEmpty()) {
				ds = new TQDataStream(edata, IO_ReadOnly);
			}
		} else { // text/uri-list
			TQStrList urls;
			TQUriDrag::decode(e, urls);

			if (urls.isEmpty()) {
				e->ignore();
				return;
			}

			KURL u(urls.first());
			if (u.fileName().isEmpty()) {
				e->ignore();
				return;
			}
			TQString tmpFile;
			if (KIO::NetAccess::download(u, tmpFile, 0L)) {
				file.setName(tmpFile);
				file.open(IO_ReadOnly);
				ds = new TQDataStream(&file);
				KIO::NetAccess::removeTempFile(tmpFile);
			} else {
				KMessageBox::error(iconView(), KIO::NetAccess::lastErrorString());
			}
		}
		if (ds) {
			decodeFolder(_wallet, *ds);
			delete ds;
		}
		_wallet->setFolder(saveFolder);
		delete _wallet;

		//delete the folder from the source if we were moving
		TQt::ButtonState state = kapp->keyboardMouseState();
		if (e->source() && e->source()->parent() &&
			!strcmp(e->source()->parent()->className(), "KWalletEntryList") &&
			!(state & TQt::ControlButton)) {
		
			KWalletEntryList *el =
				dynamic_cast<KWalletEntryList*>(e->source()->parent());
			if (el) {
				KWalletFolderItem *fi = 
					dynamic_cast<KWalletFolderItem*>(el->selectedItem());
				if (fi) {
					el->_wallet->removeFolder(fi->name());	
				}
			}
		}
		e->accept();
	} else {
		e->ignore();
		return;
	}
}

/****************
 *  KWalletEntryDrag - Stores data for wallet entry drags
 */
class KWalletEntryDrag : public TQStoredDrag {
	public:
		KWalletEntryDrag(TQWidget *dragSource, const char *name = 0L)
			: TQStoredDrag("application/x-kwallet-entry", dragSource, name) {
		}

		virtual ~KWalletEntryDrag() {}
};

/****************
 *  KWalletFolderDrag - Stores data for wallet folder drags
 */
class KWalletFolderDrag : public TQStoredDrag {
	public:
		KWalletFolderDrag(TQWidget *dragSource, const char *name = 0L)
			: TQStoredDrag("application/x-kwallet-folder", dragSource, name) {
		}

		virtual ~KWalletFolderDrag() {}
};

/****************
 *  KWalletEntryList - A listview to store wallet entries
 */
KWalletEntryList::KWalletEntryList(TQWidget *parent, const char *name)
: KListView(parent, name) {
	addColumn(i18n("Folders"));
	setRootIsDecorated(true);
	setDefaultRenameAction(Reject);
	setAcceptDrops(true);
	setItemsMovable(false);
	setDropVisualizer(false);
	viewport()->setAcceptDrops(true);
}

KWalletEntryList::~KWalletEntryList() {
}

bool KWalletEntryList::acceptDrag(TQDropEvent* e) const {
	TQListViewItem *i = itemAt(contentsToViewport(e->pos()));
	if (i) {
		if (e->provides("application/x-kwallet-entry") ||
				e->provides("text/uri-list")) {
			return true;
		}
	}
	if ((e->provides("application/x-kwallet-folder") &&
			e->source() != viewport()) || 
			e->provides("text/uri-list")) {
		return true;
	}
	return false;
}

//returns true if the item has been dropped successfully
void KWalletEntryList::itemDropped(TQDropEvent *e, TQListViewItem *item) {
	bool ok = true;
	bool isEntry;
	TQFile file;
	TQDataStream *ds;

	KWalletEntryList *el = 0L;
	TQListViewItem *sel = 0L;

	//detect if we are dragging from kwallet itself
	if (e->source() && e->source()->parent() &&
		!strcmp(e->source()->parent()->className(), "KWalletEntryList")) {

		el = dynamic_cast<KWalletEntryList*>(e->source()->parent());
		if (!el) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to drop the item"));
		} else
			sel = el->selectedItem();
	}

	if (e->provides("application/x-kwallet-entry")) {
		//do nothing if we are in the same folder
		if (sel && sel->parent()->parent() == 
				KWalletEntryList::getItemFolder(item)) {
			e->ignore();
			return;
		}
		isEntry = true;
		TQByteArray data = e->encodedData("application/x-kwallet-entry");
		if (data.isEmpty()) {
			e->ignore();
			return;
		}
		ds = new TQDataStream(data, IO_ReadOnly);
	} else if (e->provides("application/x-kwallet-folder")) {
		//do nothing if we are in the same wallet
		if (this == el) {
			e->ignore();
			return;
		}
		isEntry = false;
		TQByteArray data = e->encodedData("application/x-kwallet-folder");
		if (data.isEmpty()) {
			e->ignore();
			return;
		}
		ds = new TQDataStream(data, IO_ReadOnly);
	} else if (e->provides("text/uri-list")) {
		TQStrList urls;
		TQUriDrag::decode(e, urls);
		if (urls.isEmpty()) {
			e->ignore();
			return;
		}
		KURL u(urls.first());
		if (u.fileName().isEmpty()) {
			e->ignore();
			return;
		}
		TQString tmpFile;
		if (KIO::NetAccess::download(u, tmpFile, 0L)) {
			file.setName(tmpFile);
			file.open(IO_ReadOnly);
			ds = new TQDataStream(&file);
			//check magic to discover mime type
			TQ_UINT32 magic;
			(*ds) >> magic;
			if (magic == KWALLETENTRYMAGIC) {
				isEntry = true;
			} else if (magic == KWALLETFOLDERMAGIC) {
				isEntry = false;
			} else {
				kdDebug() << "bad magic" << endl;
				e->ignore();
				return;
			}
			delete ds;
			//set the file back to the beginning
			file.reset();
			ds = new TQDataStream(&file);
			KIO::NetAccess::removeTempFile(tmpFile);
		} else {
			KMessageBox::error(this, KIO::NetAccess::lastErrorString());
			return;
		}
	} else {
		e->ignore();
		return;
	}
	TQt::ButtonState state = kapp->keyboardMouseState();
	if (isEntry) {
		if (!item) {
			e->ignore();
			return;
		}
		KWalletFolderItem *fi = KWalletEntryList::getItemFolder(item);
		if (!fi) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to drop the entry"));
			delete(ds);
			e->accept();
			return;
		}
		TQString saveFolder = _wallet->currentFolder();
		_wallet->setFolder(fi->name());
		ok = decodeEntry(_wallet, *ds);
		_wallet->setFolder(saveFolder);
		fi->refresh();
		delete(ds);
		//delete source if we were moving, i.e., we are dragging
		//from kwalletmanager and Control is not pressed
		if (ok && el && !(state & TQt::ControlButton) && sel) {
			el->_wallet->removeEntry(sel->text(0));
			delete sel;
		}
		e->accept();
	} else {
		ok = decodeFolder(_wallet, *ds);
		delete ds;
		//delete source if we were moving, i.e., we are dragging
		//from kwalletmanager and Control is not pressed
		if (ok && el && !(state & TQt::ControlButton) && sel) {
			KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(sel);
			if (fi) {
				el->_wallet->removeFolder(fi->name());
				delete sel;
			} else {
				KMessageBox::error(this, i18n("An unexpected error occurred trying to delete the original folder, but the folder has been copied successfully"));
			}
		}
		e->accept();
	}
}

void KWalletEntryList::setWallet(KWallet::Wallet *w) {
	_wallet = w;
}

bool KWalletEntryList::existsFolder(const TQString& name) {
	for (TQListViewItem *vi = firstChild(); vi; vi = vi->nextSibling()) {
		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(vi);
		if (!fi) {
			continue;
		}
		if (name == fi->name()) {
			return true;
		}
	}
	return false;
}

void KWalletEntryList::contentsDropEvent(TQDropEvent *e) {
	TQListViewItem *i = itemAt(contentsToViewport(e->pos()));
	itemDropped(e, i);
}

void KWalletEntryList::contentsDragEnterEvent(TQDragEnterEvent *e) {
	if (e->provides("application/x-kwallet-entry") ||
		e->provides("application/x-kwallet-folder") ||
		e->provides("application/uri-list")) {
		e->accept();
	} else {
		e->ignore();
	}
}

KWalletFolderItem* KWalletEntryList::getFolder(const TQString& name) {
	for (TQListViewItem *vi = firstChild(); vi; vi = vi->nextSibling()) {
		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem *>(vi);
		if (!fi) {
			continue;
		}
		if (name == fi->name()) {
			return fi;
		}
	}
	return 0;
}

KWalletFolderItem *KWalletEntryList::getItemFolder(TQListViewItem *item) {
	switch (item->rtti()) {
		case KWalletFolderItemClass:
			return dynamic_cast<KWalletFolderItem *>(item);
		case KWalletContainerItemClass:
			return dynamic_cast<KWalletFolderItem *>(item->parent());
		case KWalletEntryItemClass:
			return dynamic_cast<KWalletFolderItem *>(item->parent()->parent());
	}
	return 0;
}

/****************
 *  KWalletIconDrag - Stores the data for wallet drags
 */
class KWalletIconDrag : public TQIconDrag {
	public:
		KWalletIconDrag(TQWidget *dragSource, const char *name = 0L)
			: TQIconDrag(dragSource, name) {
		}

		virtual ~KWalletIconDrag() {}

		virtual const char *format(int i = 0) const {
			if (i == 0) {
				return "application/x-qiconlist";
			} else if (i == 1) {
				return "text/uri-list";
			}
			return 0L;
		}

		TQByteArray encodedData(const char *mime) const {
			TQByteArray a;
			TQCString mimetype(mime);
			if (mimetype == "application/x-qiconlist") {
				return TQIconDrag::encodedData(mime);
			} else if (mimetype == "text/uri-list") {
				TQCString s = _urls.join("\r\n").latin1();
				if (_urls.count() > 0) {
					s.append("\r\n");
				}
				a.resize(s.length() + 1);
				memcpy(a.data(), s.data(), s.length() + 1);
			}
			return a;
		}

		void append(const TQIconDragItem &item, const TQRect &pr,
				const TQRect &tr, const TQString &url) {
			TQIconDrag::append(item, pr, tr);
			_urls.append(url);
		}

	private:
		TQStringList _urls;
};

/****************
*  *  KWalletIconView - An iconview to store wallets
*   */
KWalletIconView::KWalletIconView(TQWidget *parent, const char *name)
: KIconView(parent, name) {
	KGlobal::dirs()->addResourceType("kwallet", "share/apps/kwallet");
	connect(this, TQT_SIGNAL(dropped(TQDropEvent*, const TQValueList<TQIconDragItem>&)), TQT_SLOT(slotDropped(TQDropEvent*, const TQValueList<TQIconDragItem>&)));
}

KWalletIconView::~KWalletIconView() {
}

void KWalletIconView::slotDropped(TQDropEvent *e, const TQValueList<TQIconDragItem>& /*lst*/) {
	if (e->source() == viewport()) {
		e->ignore();
		return;
	}

	if (!e->provides("text/uri-list")) {
		e->ignore();
		return;
	}

	TQByteArray edata = e->encodedData("text/uri-list");
	TQCString urls = edata.data();

	TQStringList ul = TQStringList::split("\r\n", urls);
	if (ul.isEmpty() || ul.first().isEmpty()) {
		e->ignore();
		return;
	}

	KURL u(ul.first());

	if (u.fileName().isEmpty()) {
		e->ignore();
		return;
	}

	TQString dest = KGlobal::dirs()->saveLocation("kwallet") + u.fileName();
	if (TQFile::exists(dest)) {
		KMessageBox::sorry(viewport(), i18n("That wallet file already exists.  You cannot overwrite wallets."));
		e->ignore();
		return;
	}

	// FIXME: verify that it is a real wallet file first
	KIO::NetAccess::file_copy(u, KURL::fromPathOrURL(dest));
	e->accept();
}

void KWalletIconView::contentsMousePressEvent(TQMouseEvent *e) {
	_mousePos = e->pos();
	if (!findItem(_mousePos)) {
		clearSelection();
	}
	KIconView::contentsMousePressEvent( e );
}

TQDragObject *KWalletIconView::dragObject() {
	KWalletIconDrag* id = new KWalletIconDrag(viewport(), "KWallet Drag");
	TQString path = "file:" + KGlobal::dirs()->saveLocation("kwallet");
	TQPoint pos = _mousePos;
	for (TQIconViewItem *item = firstItem(); item; item = item->nextItem()) {
		if (item->isSelected()) {
			TQString url = path + item->text() + ".kwl";
			TQIconDragItem idi;
			idi.setData(url.local8Bit());
			id->append(idi,
			TQRect(item->pixmapRect(false).topLeft() - pos,
			item->pixmapRect(false).size()),
			TQRect(item->textRect(false).topLeft() - pos,
			item->textRect(false).size()),
			url);
		}
	}

	id->setPixmap(*currentItem()->pixmap(),
	pos - currentItem()->pixmapRect(false).topLeft());

	return id;
}

TQDragObject *KWalletEntryList::dragObject() {
	TQListViewItem *i = currentItem();

	TQStoredDrag *sd = 0L;  

	if (i->rtti() == KWalletEntryItemClass) {
		KWalletEntryItem *ei = dynamic_cast<KWalletEntryItem*>(i);
		if (!ei) {
			return 0L;
		}
		sd = new KWalletEntryDrag(viewport(), "KWallet Entry Drag");
		TQByteArray a;
		TQDataStream ds(a, IO_WriteOnly);
		ds << KWALLETENTRYMAGIC;
		ds << *ei;
		sd->setEncodedData(a);
	} else if (i->rtti() == KWalletFolderItemClass) {
		KWalletFolderItem *fi = dynamic_cast<KWalletFolderItem*>(i);
		if (!fi) {
			return 0L;
		}
		sd = new KWalletFolderDrag(viewport(), "KWallet Folder Drag");
		TQByteArray a;
		TQDataStream ds(a, IO_WriteOnly);

		ds << KWALLETFOLDERMAGIC;
		ds << *fi;
		sd->setEncodedData(a);
	}
	return sd;
}

#include "allyourbase.moc"
