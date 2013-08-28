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
#include <tdelistview.h>
#include <tdewallet.h>
#include <kiconloader.h>
#include <kicontheme.h>

#define KWALLETENTRYMAGIC ((TQ_UINT32) 0x6B776C65)
#define KWALLETFOLDERMAGIC ((TQ_UINT32) 0x6B776C66)

enum TDEWalletListItemClasses {
	TDEWalletFolderItemClass = 1000,
	TDEWalletContainerItemClass,
	TDEWalletEntryItemClass,
	TDEWalletUnknownClass = 2000
};

class TDEWalletEntryItem : public TDEListViewItem {
	public:
		TDEWalletEntryItem(TDEWallet::Wallet *w, TQListViewItem* parent, const TQString& ename);
		virtual ~TDEWalletEntryItem();

		const TQString& oldName() { return _oldName; }
		TQString currentName() { return text(0); }

		void clearOldName() { _oldName = text(0); }
		virtual int rtti() const;

	public:
		TDEWallet::Wallet *_wallet;

	private:
		TQString _oldName;
};

class TDEWalletContainerItem : public TDEListViewItem {
	public:
		TDEWalletContainerItem(TQListViewItem* parent, const TQString& name,
		    TDEWallet::Wallet::EntryType type);
		virtual ~TDEWalletContainerItem();

	public:
		virtual int rtti() const;
		TDEWallet::Wallet::EntryType type();
		bool contains(const TQString& itemKey);
		TQListViewItem* getItem(const TQString& itemKey);

	private:
		TDEWallet::Wallet::EntryType _type;
};

class TDEWalletFolderItem : public TDEListViewItem {
	public:
		TDEWalletFolderItem(TDEWallet::Wallet *w, TQListView* parent, 
			const TQString& name, int entries);
		virtual ~TDEWalletFolderItem();

		virtual bool acceptDrop(const TQMimeSource *mime) const;
		virtual int rtti() const;

		TQString name() const;
		void refresh();
		TDEWalletContainerItem* getContainer(TDEWallet::Wallet::EntryType type);
		TQPixmap getFolderIcon(TDEIcon::Group group);
		bool contains(const TQString& itemKey);
		TQListViewItem* getItem(const TQString& itemKey);

	public:
		TDEWallet::Wallet *_wallet;

	private:
		TQString _name;
		int _entries;
};

class TDEWalletEntryList : public TDEListView {
	Q_OBJECT
  
	public:
		TDEWalletEntryList(TQWidget *parent, const char *name = 0L);
		virtual ~TDEWalletEntryList();

		bool existsFolder(const TQString& name);
		TDEWalletFolderItem* getFolder(const TQString& name);
		void contentsDropEvent(TQDropEvent *e);
		void contentsDragEnterEvent(TQDragEnterEvent *e);
		void setWallet(TDEWallet::Wallet *w);

	protected:
		void itemDropped(TQDropEvent *e, TQListViewItem *item);
		virtual TQDragObject *dragObject();
		virtual bool acceptDrag (TQDropEvent* event) const;

	private:
		static TDEWalletFolderItem *getItemFolder(TQListViewItem *item);
	
	public:
		TDEWallet::Wallet *_wallet;
};

class TDEWalletItem : public TQIconViewItem {
	public:
		TDEWalletItem(TQIconView *parent, const TQString& walletName);
		virtual ~TDEWalletItem();

		virtual bool acceptDrop(const TQMimeSource *mime) const;

	protected:
		virtual void dropped(TQDropEvent *e, const TQValueList<TQIconDragItem>& lst); 
};


class TDEWalletIconView : public TDEIconView {
	Q_OBJECT
  
	public:
		TDEWalletIconView(TQWidget *parent, const char *name = 0L);
		virtual ~TDEWalletIconView();

	protected slots:
		virtual void slotDropped(TQDropEvent *e, const TQValueList<TQIconDragItem>& lst);

	protected:
		virtual TQDragObject *dragObject();
		virtual void contentsMousePressEvent(TQMouseEvent *e);
		TQPoint _mousePos;
};


inline TQDataStream& operator<<(TQDataStream& str, const TDEWalletEntryItem& w) {
	TQString name = w.text(0);
	str << name;
	TDEWallet::Wallet::EntryType et = w._wallet->entryType(name);
	str << long(et);
	TQByteArray a;
	w._wallet->readEntry(name, a);
	str << a;
	return str;
}

inline TQDataStream& operator<<(TQDataStream& str, const TDEWalletFolderItem& w) {
	TQString oldFolder = w._wallet->currentFolder();
	str << w.name();
	w._wallet->setFolder(w.name());
	TQStringList entries = w._wallet->entryList();
	for (TQStringList::Iterator it = entries.begin(); it != entries.end(); ++it) {
		str << *it;
		TDEWallet::Wallet::EntryType et = w._wallet->entryType(*it);
		str << long(et);
		TQByteArray a;
		w._wallet->readEntry(*it, a);
		str << a;
	}
	w._wallet->setFolder(oldFolder);
	return str;
}

#endif
