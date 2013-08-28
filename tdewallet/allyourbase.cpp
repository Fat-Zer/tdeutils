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

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdeio/netaccess.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kurldrag.h>
#include <tdewallet.h>

#include <tqdragobject.h>
#include <tqfile.h>
#include <tqptrlist.h>

/****************
 *  TDEWalletFolderItem - ListView items to represent tdewallet folders
 */
TDEWalletFolderItem::TDEWalletFolderItem(TDEWallet::Wallet *w, TQListView* parent, const TQString &name, int entries)
: TDEListViewItem(parent),_wallet(w),_name(name),_entries(entries) {
	setText(0, TQString("%1 (%2)").arg(_name).arg(_entries));
	setRenameEnabled(0, false);
	setDragEnabled(true);
	setDropEnabled(true);

	TQPixmap pix = getFolderIcon(TDEIcon::Small);
	
	setPixmap(0,pix);
}

TQPixmap TDEWalletFolderItem::getFolderIcon(TDEIcon::Group group){
	TDEIconLoader *loader = TDEGlobal::instance()->iconLoader();
	TQPixmap pix = loader->loadIcon( _name, group, 0,
			TDEIcon::DefaultState, 0, true );
	if (pix.isNull())
		pix = loader->loadIcon( _name.lower(), group, 0,
			TDEIcon::DefaultState, 0, true);
	if (pix.isNull())
		pix = loader->loadIcon( "folder_red", group, 0,
			TDEIcon::DefaultState, 0, true);
	return pix;
}

void TDEWalletFolderItem::refresh() {
	TQString saveFolder = _wallet->currentFolder();
	_wallet->setFolder(_name);
	setText(0, TQString("%1 (%2)").arg(_name).arg(_wallet->entryList().count()));
	_wallet->setFolder(saveFolder);
}

TDEWalletContainerItem* TDEWalletFolderItem::getContainer(TDEWallet::Wallet::EntryType type) {
	for (TQListViewItem *i = firstChild(); i; i = i->nextSibling()) {
		TDEWalletContainerItem *ci = dynamic_cast<TDEWalletContainerItem *>(i);
		if (!ci) {
			continue;
		}
		if (ci->type() == type) {
			return ci;
		}
	}
	return 0;
}

bool TDEWalletFolderItem::contains(const TQString& key) {
	return (getItem(key) != 0);
}

TQListViewItem* TDEWalletFolderItem::getItem(const TQString& key) {
	for (TQListViewItem *i = firstChild(); i; i = i->nextSibling()) {
		TDEWalletContainerItem *ci = dynamic_cast<TDEWalletContainerItem *>(i);
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

bool TDEWalletFolderItem::acceptDrop(const TQMimeSource *mime) const {
	return mime->provides("application/x-tdewallet-entry") ||
		mime->provides("text/uri-list");
}

int TDEWalletFolderItem::rtti() const {
	return TDEWalletFolderItemClass;
}

TQString TDEWalletFolderItem::name() const {
	return _name;
}

TDEWalletFolderItem::~TDEWalletFolderItem() {
}

/****************
 *  TDEWalletContainerItem - ListView items to represent tdewallet containers, i.e.
 *  passwords, maps, ...
 */
TDEWalletContainerItem::TDEWalletContainerItem(TQListViewItem* parent, const TQString &name, TDEWallet::Wallet::EntryType type)
: TDEListViewItem(parent, name), _type(type) {
	setRenameEnabled(0, false);
	setDragEnabled(true);
}

TDEWalletContainerItem::~TDEWalletContainerItem() {
}

int TDEWalletContainerItem::rtti() const {
	return TDEWalletContainerItemClass;
}

TDEWallet::Wallet::EntryType TDEWalletContainerItem::type() {
	return _type;
}

bool TDEWalletContainerItem::contains(const TQString& key) {
	return getItem(key) != 0;
}

TQListViewItem *TDEWalletContainerItem::getItem(const TQString& key) {
	for (TQListViewItem *i = firstChild(); i; i = i->nextSibling()) {
		if (i->text(0) == key) {
			return i;
		}
	}
	return 0;
}

/****************
 *  TDEWalletEntryItem - ListView items to represent tdewallet entries
 */
TDEWalletEntryItem::TDEWalletEntryItem(TDEWallet::Wallet *w, TQListViewItem* parent, const TQString& ename)
: TDEListViewItem(parent, ename), _wallet(w), _oldName(ename) {
	setRenameEnabled(0, true);
	setDragEnabled(true);
}

int TDEWalletEntryItem::rtti() const {
	return TDEWalletEntryItemClass;
}

TDEWalletEntryItem::~TDEWalletEntryItem() {
}

/****************
 * TDEWalletItem - IconView items to represent wallets
 */
TDEWalletItem::TDEWalletItem(TQIconView *parent, const TQString& walletName)
: TQIconViewItem(parent, walletName, DesktopIcon("tdewalletmanager")) {
}

TDEWalletItem::~TDEWalletItem() {
}

bool TDEWalletItem::acceptDrop(const TQMimeSource *mime) const {
	return mime->provides("application/x-tdewallet-folder") ||
		mime->provides("text/uri-list");
}

static bool decodeEntry(TDEWallet::Wallet *_wallet, TQDataStream& ds) {
	TQ_UINT32 magic;
	ds >> magic;
	if (magic != KWALLETENTRYMAGIC) {
		kdDebug() << "bad magic" << endl;
		return false;
	}
	TQString name;
	TQByteArray value;
	TDEWallet::Wallet::EntryType et;
	ds >> name;
	if (_wallet->hasEntry(name)) {
		int rc = KMessageBox::warningContinueCancel(0L, i18n("An entry by the name '%1' already exists. Would you like to continue?").arg(name));
		if (rc == KMessageBox::Cancel) {
			return false;
		}
	}
	long l;
	ds >> l;
	et = TDEWallet::Wallet::EntryType(l);
	ds >> value;
	_wallet->writeEntry(name, value, et);
	return true;
}

static bool decodeFolder(TDEWallet::Wallet *_wallet, TQDataStream& ds) {
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
		TDEWallet::Wallet::EntryType et;
		ds >> name;
		long l;
		ds >> l;
		et = TDEWallet::Wallet::EntryType(l);
		ds >> value;
		_wallet->writeEntry(name, value, et);
	}
	return true;
}

void TDEWalletItem::dropped(TQDropEvent *e, const TQValueList<TQIconDragItem>& lst) {
	Q_UNUSED(lst);
	if (e->provides("application/x-tdewallet-folder") || 
			e->provides("text/uri-list")) {

		// FIXME: don't allow the drop if the wallet name is the same

		TDEWallet::Wallet *_wallet = TDEWallet::Wallet::openWallet(text());
		if (!_wallet) {
			e->ignore();
			return;
		}

		TQString saveFolder = _wallet->currentFolder();

		TQFile file;
		TQDataStream *ds = 0L;

		if (e->provides("application/x-tdewallet-folder")) {
			TQByteArray edata = e->encodedData("application/x-tdewallet-folder");
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
			if (TDEIO::NetAccess::download(u, tmpFile, 0L)) {
				file.setName(tmpFile);
				file.open(IO_ReadOnly);
				ds = new TQDataStream(&file);
				TDEIO::NetAccess::removeTempFile(tmpFile);
			} else {
				KMessageBox::error(iconView(), TDEIO::NetAccess::lastErrorString());
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
			!strcmp(e->source()->parent()->className(), "TDEWalletEntryList") &&
			!(state & TQt::ControlButton)) {
		
			TDEWalletEntryList *el =
				dynamic_cast<TDEWalletEntryList*>(e->source()->parent());
			if (el) {
				TDEWalletFolderItem *fi = 
					dynamic_cast<TDEWalletFolderItem*>(el->selectedItem());
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
 *  TDEWalletEntryDrag - Stores data for wallet entry drags
 */
class TDEWalletEntryDrag : public TQStoredDrag {
	public:
		TDEWalletEntryDrag(TQWidget *dragSource, const char *name = 0L)
			: TQStoredDrag("application/x-tdewallet-entry", dragSource, name) {
		}

		virtual ~TDEWalletEntryDrag() {}
};

/****************
 *  TDEWalletFolderDrag - Stores data for wallet folder drags
 */
class TDEWalletFolderDrag : public TQStoredDrag {
	public:
		TDEWalletFolderDrag(TQWidget *dragSource, const char *name = 0L)
			: TQStoredDrag("application/x-tdewallet-folder", dragSource, name) {
		}

		virtual ~TDEWalletFolderDrag() {}
};

/****************
 *  TDEWalletEntryList - A listview to store wallet entries
 */
TDEWalletEntryList::TDEWalletEntryList(TQWidget *parent, const char *name)
: TDEListView(parent, name) {
	addColumn(i18n("Folders"));
	setRootIsDecorated(true);
	setDefaultRenameAction(Reject);
	setAcceptDrops(true);
	setItemsMovable(false);
	setDropVisualizer(false);
	viewport()->setAcceptDrops(true);
}

TDEWalletEntryList::~TDEWalletEntryList() {
}

bool TDEWalletEntryList::acceptDrag(TQDropEvent* e) const {
	TQListViewItem *i = itemAt(contentsToViewport(e->pos()));
	if (i) {
		if (e->provides("application/x-tdewallet-entry") ||
				e->provides("text/uri-list")) {
			return true;
		}
	}
	if ((e->provides("application/x-tdewallet-folder") &&
			e->source() != viewport()) || 
			e->provides("text/uri-list")) {
		return true;
	}
	return false;
}

//returns true if the item has been dropped successfully
void TDEWalletEntryList::itemDropped(TQDropEvent *e, TQListViewItem *item) {
	bool ok = true;
	bool isEntry;
	TQFile file;
	TQDataStream *ds;

	TDEWalletEntryList *el = 0L;
	TQListViewItem *sel = 0L;

	//detect if we are dragging from tdewallet itself
	if (e->source() && e->source()->parent() &&
		!strcmp(e->source()->parent()->className(), "TDEWalletEntryList")) {

		el = dynamic_cast<TDEWalletEntryList*>(e->source()->parent());
		if (!el) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to drop the item"));
		} else
			sel = el->selectedItem();
	}

	if (e->provides("application/x-tdewallet-entry")) {
		//do nothing if we are in the same folder
		if (sel && sel->parent()->parent() == 
				TDEWalletEntryList::getItemFolder(item)) {
			e->ignore();
			return;
		}
		isEntry = true;
		TQByteArray data = e->encodedData("application/x-tdewallet-entry");
		if (data.isEmpty()) {
			e->ignore();
			return;
		}
		ds = new TQDataStream(data, IO_ReadOnly);
	} else if (e->provides("application/x-tdewallet-folder")) {
		//do nothing if we are in the same wallet
		if (this == el) {
			e->ignore();
			return;
		}
		isEntry = false;
		TQByteArray data = e->encodedData("application/x-tdewallet-folder");
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
		if (TDEIO::NetAccess::download(u, tmpFile, 0L)) {
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
			TDEIO::NetAccess::removeTempFile(tmpFile);
		} else {
			KMessageBox::error(this, TDEIO::NetAccess::lastErrorString());
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
		TDEWalletFolderItem *fi = TDEWalletEntryList::getItemFolder(item);
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
		//from tdewalletmanager and Control is not pressed
		if (ok && el && !(state & TQt::ControlButton) && sel) {
			el->_wallet->removeEntry(sel->text(0));
			delete sel;
		}
		e->accept();
	} else {
		ok = decodeFolder(_wallet, *ds);
		delete ds;
		//delete source if we were moving, i.e., we are dragging
		//from tdewalletmanager and Control is not pressed
		if (ok && el && !(state & TQt::ControlButton) && sel) {
			TDEWalletFolderItem *fi = dynamic_cast<TDEWalletFolderItem *>(sel);
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

void TDEWalletEntryList::setWallet(TDEWallet::Wallet *w) {
	_wallet = w;
}

bool TDEWalletEntryList::existsFolder(const TQString& name) {
	for (TQListViewItem *vi = firstChild(); vi; vi = vi->nextSibling()) {
		TDEWalletFolderItem *fi = dynamic_cast<TDEWalletFolderItem *>(vi);
		if (!fi) {
			continue;
		}
		if (name == fi->name()) {
			return true;
		}
	}
	return false;
}

void TDEWalletEntryList::contentsDropEvent(TQDropEvent *e) {
	TQListViewItem *i = itemAt(contentsToViewport(e->pos()));
	itemDropped(e, i);
}

void TDEWalletEntryList::contentsDragEnterEvent(TQDragEnterEvent *e) {
	if (e->provides("application/x-tdewallet-entry") ||
		e->provides("application/x-tdewallet-folder") ||
		e->provides("application/uri-list")) {
		e->accept();
	} else {
		e->ignore();
	}
}

TDEWalletFolderItem* TDEWalletEntryList::getFolder(const TQString& name) {
	for (TQListViewItem *vi = firstChild(); vi; vi = vi->nextSibling()) {
		TDEWalletFolderItem *fi = dynamic_cast<TDEWalletFolderItem *>(vi);
		if (!fi) {
			continue;
		}
		if (name == fi->name()) {
			return fi;
		}
	}
	return 0;
}

TDEWalletFolderItem *TDEWalletEntryList::getItemFolder(TQListViewItem *item) {
	switch (item->rtti()) {
		case TDEWalletFolderItemClass:
			return dynamic_cast<TDEWalletFolderItem *>(item);
		case TDEWalletContainerItemClass:
			return dynamic_cast<TDEWalletFolderItem *>(item->parent());
		case TDEWalletEntryItemClass:
			return dynamic_cast<TDEWalletFolderItem *>(item->parent()->parent());
	}
	return 0;
}

/****************
 *  TDEWalletIconDrag - Stores the data for wallet drags
 */
class TDEWalletIconDrag : public TQIconDrag {
	public:
		TDEWalletIconDrag(TQWidget *dragSource, const char *name = 0L)
			: TQIconDrag(dragSource, name) {
		}

		virtual ~TDEWalletIconDrag() {}

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
*  *  TDEWalletIconView - An iconview to store wallets
*   */
TDEWalletIconView::TDEWalletIconView(TQWidget *parent, const char *name)
: TDEIconView(parent, name) {
	TDEGlobal::dirs()->addResourceType("tdewallet", "share/apps/tdewallet");
	connect(this, TQT_SIGNAL(dropped(TQDropEvent*, const TQValueList<TQIconDragItem>&)), TQT_SLOT(slotDropped(TQDropEvent*, const TQValueList<TQIconDragItem>&)));
}

TDEWalletIconView::~TDEWalletIconView() {
}

void TDEWalletIconView::slotDropped(TQDropEvent *e, const TQValueList<TQIconDragItem>& /*lst*/) {
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

	TQString dest = TDEGlobal::dirs()->saveLocation("tdewallet") + u.fileName();
	if (TQFile::exists(dest)) {
		KMessageBox::sorry(viewport(), i18n("That wallet file already exists.  You cannot overwrite wallets."));
		e->ignore();
		return;
	}

	// FIXME: verify that it is a real wallet file first
	TDEIO::NetAccess::file_copy(u, KURL::fromPathOrURL(dest));
	e->accept();
}

void TDEWalletIconView::contentsMousePressEvent(TQMouseEvent *e) {
	_mousePos = e->pos();
	if (!findItem(_mousePos)) {
		clearSelection();
	}
	TDEIconView::contentsMousePressEvent( e );
}

TQDragObject *TDEWalletIconView::dragObject() {
	TDEWalletIconDrag* id = new TDEWalletIconDrag(viewport(), "TDEWallet Drag");
	TQString path = "file:" + TDEGlobal::dirs()->saveLocation("tdewallet");
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

TQDragObject *TDEWalletEntryList::dragObject() {
	TQListViewItem *i = currentItem();

	TQStoredDrag *sd = 0L;  

	if (i->rtti() == TDEWalletEntryItemClass) {
		TDEWalletEntryItem *ei = dynamic_cast<TDEWalletEntryItem*>(i);
		if (!ei) {
			return 0L;
		}
		sd = new TDEWalletEntryDrag(viewport(), "TDEWallet Entry Drag");
		TQByteArray a;
		TQDataStream ds(a, IO_WriteOnly);
		ds << KWALLETENTRYMAGIC;
		ds << *ei;
		sd->setEncodedData(a);
	} else if (i->rtti() == TDEWalletFolderItemClass) {
		TDEWalletFolderItem *fi = dynamic_cast<TDEWalletFolderItem*>(i);
		if (!fi) {
			return 0L;
		}
		sd = new TDEWalletFolderDrag(viewport(), "TDEWallet Folder Drag");
		TQByteArray a;
		TQDataStream ds(a, IO_WriteOnly);

		ds << KWALLETFOLDERMAGIC;
		ds << *fi;
		sd->setEncodedData(a);
	}
	return sd;
}

#include "allyourbase.moc"
