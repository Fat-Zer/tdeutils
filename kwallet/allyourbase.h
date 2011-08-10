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

#ifndef ALLYOURBASE_H
#define ALLYOURBASE_H

#include <kiconview.h>
#include <klistview.h>
#include <kwallet.h>
#include <kiconloader.h>
#include <kicontheme.h>

#define KWALLETENTRYMAGIC ((TQ_UINT32) 0x6B776C65)
#define KWALLETFOLDERMAGIC ((TQ_UINT32) 0x6B776C66)

enum KWalletListItemClasses {
	KWalletFolderItemClass = 1000,
	KWalletContainerItemClass,
	KWalletEntryItemClass,
	KWalletUnknownClass = 2000
};

class KWalletEntryItem : public KListViewItem {
	public:
		KWalletEntryItem(KWallet::Wallet *w, TQListViewItem* tqparent, const TQString& ename);
		virtual ~KWalletEntryItem();

		const TQString& oldName() { return _oldName; }
		TQString currentName() { return text(0); }

		void clearOldName() { _oldName = text(0); }
		virtual int rtti() const;

	public:
		KWallet::Wallet *_wallet;

	private:
		TQString _oldName;
};

class KWalletContainerItem : public KListViewItem {
	public:
		KWalletContainerItem(TQListViewItem* tqparent, const TQString& name,
		    KWallet::Wallet::EntryType type);
		virtual ~KWalletContainerItem();

	public:
		virtual int rtti() const;
		KWallet::Wallet::EntryType type();
		bool contains(const TQString& itemKey);
		TQListViewItem* getItem(const TQString& itemKey);

	private:
		KWallet::Wallet::EntryType _type;
};

class KWalletFolderItem : public KListViewItem {
	public:
		KWalletFolderItem(KWallet::Wallet *w, TQListView* tqparent, 
			const TQString& name, int entries);
		virtual ~KWalletFolderItem();

		virtual bool acceptDrop(const TQMimeSource *mime) const;
		virtual int rtti() const;

		TQString name() const;
		void refresh();
		KWalletContainerItem* getContainer(KWallet::Wallet::EntryType type);
		TQPixmap getFolderIcon(KIcon::Group group);
		bool contains(const TQString& itemKey);
		TQListViewItem* getItem(const TQString& itemKey);

	public:
		KWallet::Wallet *_wallet;

	private:
		TQString _name;
		int _entries;
};

class KWalletEntryList : public KListView {
	Q_OBJECT
  TQ_OBJECT
	public:
		KWalletEntryList(TQWidget *tqparent, const char *name = 0L);
		virtual ~KWalletEntryList();

		bool existsFolder(const TQString& name);
		KWalletFolderItem* getFolder(const TQString& name);
		void contentsDropEvent(TQDropEvent *e);
		void contentsDragEnterEvent(TQDragEnterEvent *e);
		void setWallet(KWallet::Wallet *w);

	protected:
		void itemDropped(TQDropEvent *e, TQListViewItem *item);
		virtual TQDragObject *dragObject();
		virtual bool acceptDrag (TQDropEvent* event) const;

	private:
		static KWalletFolderItem *getItemFolder(TQListViewItem *item);
	
	public:
		KWallet::Wallet *_wallet;
};

class KWalletItem : public TQIconViewItem {
	public:
		KWalletItem(TQIconView *tqparent, const TQString& walletName);
		virtual ~KWalletItem();

		virtual bool acceptDrop(const TQMimeSource *mime) const;

	protected:
		virtual void dropped(TQDropEvent *e, const TQValueList<TQIconDragItem>& lst); 
};


class KWalletIconView : public KIconView {
	Q_OBJECT
  TQ_OBJECT
	public:
		KWalletIconView(TQWidget *tqparent, const char *name = 0L);
		virtual ~KWalletIconView();

	protected slots:
		virtual void slotDropped(TQDropEvent *e, const TQValueList<TQIconDragItem>& lst);

	protected:
		virtual TQDragObject *dragObject();
		virtual void contentsMousePressEvent(TQMouseEvent *e);
		TQPoint _mousePos;
};


inline TQDataStream& operator<<(TQDataStream& str, const KWalletEntryItem& w) {
	TQString name = w.text(0);
	str << name;
	KWallet::Wallet::EntryType et = w._wallet->entryType(name);
	str << long(et);
	TQByteArray a;
	w._wallet->readEntry(name, a);
	str << a;
	return str;
}

inline TQDataStream& operator<<(TQDataStream& str, const KWalletFolderItem& w) {
	TQString oldFolder = w._wallet->currentFolder();
	str << w.name();
	w._wallet->setFolder(w.name());
	TQStringList entries = w._wallet->entryList();
	for (TQStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
		str << *it;
		KWallet::Wallet::EntryType et = w._wallet->entryType(*it);
		str << long(et);
		TQByteArray a;
		w._wallet->readEntry(*it, a);
		str << a;
	}
	w._wallet->setFolder(oldFolder);
	return str;
}

#endif
