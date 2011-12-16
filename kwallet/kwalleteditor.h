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

#ifndef KWALLETEDITOR_H
#define KWALLETEDITOR_H

#include "walletwidget.h"
#include <kwallet.h>
#include <kmainwindow.h>
#include <tqstringlist.h>

class KAction;
class TQIconViewItem;
class TQListViewItem;
class TQCheckBox;
class KWalletFolderIconView;
class KWalletEntryList;
class KWMapEditor;

class KWalletEditor : public KMainWindow {
	Q_OBJECT
  TQ_OBJECT

	public:
		KWalletEditor(const TQString& wallet, bool isPath, TQWidget *parent = 0, const char* name = 0);
		virtual ~KWalletEditor();

		bool isOpen() const { return _w != 0L; }

		void setNewWallet(bool newWallet);

	public slots:
		void walletClosed();
		void createFolder();
		void deleteFolder();

	private slots:
		void tqlayout();
		void updateFolderList(bool checkEntries = false);
		void entrySelectionChanged(TQListViewItem *item);
		void listItemRenamed(TQListViewItem *, int, const TQString&);
		void listContextMenuRequested(TQListViewItem *item, const TQPoint& pos, int col);
		void updateEntries(const TQString& folder);

		void newEntry();
		void renameEntry();
		void deleteEntry();
		void entryEditted();
		void restoreEntry();
		void saveEntry();

		void changePassword();

		void walletOpened(bool success);
		void hidePasswordContents();
		void showPasswordContents();
		void showHideMapEditorValue(bool show);

		void saveAs();
		void exportXML();
		void importXML();
		void importWallet();

		void copyPassword();

	signals:
		void enableWalletActions(bool enable);
		void enableFolderActions(bool enable);
		void enableContextFolderActions(bool enable);
		void editorClosed(KMainWindow*);

	public:
		TQString _walletName;

	private:
		void createActions();
		bool _nonLocal;
		KWallet::Wallet *_w;
		WalletWidget *_ww;
		KWalletEntryList *_entryList;
		bool _walletIsOpen;
		KAction *_newFolderAction, *_deleteFolderAction;
		KAction *_passwordAction, *_exportAction, *_saveAsAction, *_mergeAction, *_importAction;
		KAction *_copyPassAction;
		TQLabel*_details;
		TQString _currentFolder;
		TQMap<TQString,TQString> _currentMap; // save memory by storing
						   // only the most recent map.
		KWMapEditor *_mapEditor;
		TQCheckBox *_mapEditorShowHide;
		bool _newWallet;
};

#endif
