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


#include "kbetterthankdialogbase.h"
#include "tdewalleteditor.h"
#include "kwmapeditor.h"
#include "allyourbase.h"

#include <dcopclient.h>
#include <dcopref.h>
#include <tdeaction.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <kdialog.h>
#include <tdefiledialog.h>
#include <kiconview.h>
#include <kinputdialog.h>
#include <tdeio/netaccess.h>
#include <tdelistviewsearchline.h>
#include <tdelocale.h>
#include <kmdcodec.h>
#include <tdemessagebox.h>
#include <tdepopupmenu.h>
#include <ksqueezedtextlabel.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <kstringhandler.h>
#include <tdetempfile.h>

#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqclipboard.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqlistview.h>
#include <tqptrstack.h>
#include <tqpushbutton.h>
#include <tqstylesheet.h>
#include <tqtextedit.h>
#include <tqtimer.h>
#include <tqwidgetstack.h>

#include <assert.h>
#include <stdlib.h>

TDEWalletEditor::TDEWalletEditor(const TQString& wallet, bool isPath, TQWidget *parent, const char *name)
: TDEMainWindow(parent, name), _walletName(wallet), _nonLocal(isPath) {
	_newWallet = false;
	_ww = new WalletWidget(this, "Wallet Widget");
	_copyPassAction = KStdAction::copy(TQT_TQOBJECT(this), TQT_SLOT(copyPassword()), actionCollection());

	TQVBoxLayout *box = new TQVBoxLayout(_ww->_entryListFrame);
	box->setSpacing( KDialog::spacingHint() );
	box->setMargin( KDialog::marginHint() );
	_entryList = new TDEWalletEntryList(_ww->_entryListFrame, "Wallet Entry List");
	box->addWidget(new TDEListViewSearchLineWidget(_entryList, _ww->_entryListFrame));
	box->addWidget(_entryList);

	_ww->_entryStack->setEnabled(true);

	box = new TQVBoxLayout(_ww->_entryStack->widget(2));
	_mapEditorShowHide = new TQCheckBox(i18n("&Show values"), _ww->_entryStack->widget(2));
	connect(_mapEditorShowHide, TQT_SIGNAL(toggled(bool)), this, TQT_SLOT(showHideMapEditorValue(bool)));
	_mapEditor = new KWMapEditor(_currentMap, _ww->_entryStack->widget(2));
	box->addWidget(_mapEditorShowHide);
	box->addWidget(_mapEditor);

	setCentralWidget(_ww);

	resize(600, 400);

	connect(_entryList, TQT_SIGNAL(selectionChanged(TQListViewItem*)),
		this, TQT_SLOT(entrySelectionChanged(TQListViewItem*)));
	connect(_entryList,
		TQT_SIGNAL(contextMenuRequested(TQListViewItem*,const TQPoint&,int)),
		this,
		TQT_SLOT(listContextMenuRequested(TQListViewItem*,const TQPoint&,int)));
	connect(_entryList,
		TQT_SIGNAL(itemRenamed(TQListViewItem*, int, const TQString&)),
		this,
		TQT_SLOT(listItemRenamed(TQListViewItem*, int, const TQString&)));

	connect(_ww->_passwordValue, TQT_SIGNAL(textChanged()),
		this, TQT_SLOT(entryEditted()));
	connect(_mapEditor, TQT_SIGNAL(dirty()),
		this, TQT_SLOT(entryEditted()));

	connect(_ww->_undoChanges, TQT_SIGNAL(clicked()),
		this, TQT_SLOT(restoreEntry()));
	connect(_ww->_saveChanges, TQT_SIGNAL(clicked()),
		this, TQT_SLOT(saveEntry()));

	connect(_ww->_showContents, TQT_SIGNAL(clicked()),
		this, TQT_SLOT(showPasswordContents()));
	connect(_ww->_hideContents, TQT_SIGNAL(clicked()),
		this, TQT_SLOT(hidePasswordContents()));

	_walletIsOpen = false;

	_w = TDEWallet::Wallet::openWallet(wallet, winId(), isPath ? TDEWallet::Wallet::Path : TDEWallet::Wallet::Asynchronous);
	if (_w) {
		connect(_w, TQT_SIGNAL(walletOpened(bool)), this, TQT_SLOT(walletOpened(bool)));
		connect(_w, TQT_SIGNAL(walletClosed()), this, TQT_SLOT(walletClosed()));
		connect(_w, TQT_SIGNAL(folderUpdated(const TQString&)), this, TQT_SLOT(updateEntries(const TQString&)));
		connect(_w, TQT_SIGNAL(folderListUpdated()), this, TQT_SLOT(updateFolderList()));
		updateFolderList();
	} else {
		kdDebug(2300) << "Wallet open failed!" << endl;
	}

	createActions();
	createGUI("tdewalleteditor.rc");
	delete toolBar();

	setCaption(wallet);

	TQTimer::singleShot(0, this, TQT_SLOT(layout()));
}

TDEWalletEditor::~TDEWalletEditor() {
	emit editorClosed(this);
	delete _newFolderAction;
	_newFolderAction = 0L;
	delete _deleteFolderAction;
	_deleteFolderAction = 0L;
	delete _w;
	_w = 0L;
	if (_nonLocal) {
		TDEWallet::Wallet::closeWallet(_walletName, true);
	}
}

void TDEWalletEditor::layout() {
	TQValueList<int> sz = _ww->_splitter->sizes();
	int sum = sz[0] + sz[1];
	sz[0] = sum/2;
	sz[1] = sum/2;
	_ww->_splitter->setSizes(sz);
}

void TDEWalletEditor::createActions() {
	_newFolderAction = new TDEAction(i18n("&New Folder..."), "folder_new",
			0, TQT_TQOBJECT(this), TQT_SLOT(createFolder()), actionCollection(),
			"create_folder");
	connect(this, TQT_SIGNAL(enableFolderActions(bool)),
		_newFolderAction, TQT_SLOT(setEnabled(bool)));

	_deleteFolderAction = new TDEAction(i18n("&Delete Folder"), 0, 0,
			TQT_TQOBJECT(this), TQT_SLOT(deleteFolder()), actionCollection(),
			"delete_folder");
	connect(this, TQT_SIGNAL(enableContextFolderActions(bool)),
		_deleteFolderAction, TQT_SLOT(setEnabled(bool)));
	connect(this, TQT_SIGNAL(enableFolderActions(bool)),
		_deleteFolderAction, TQT_SLOT(setEnabled(bool)));

	_passwordAction = new TDEAction(i18n("Change &Password..."), 0, 0, TQT_TQOBJECT(this),
			TQT_SLOT(changePassword()), actionCollection(),
			"change_password");
	connect(this, TQT_SIGNAL(enableWalletActions(bool)),
		_passwordAction, TQT_SLOT(setEnabled(bool)));

	_mergeAction = new TDEAction(i18n("&Merge Wallet..."), 0, 0, TQT_TQOBJECT(this),
			TQT_SLOT(importWallet()), actionCollection(),
			"merge");
	connect(this, TQT_SIGNAL(enableWalletActions(bool)),
		_mergeAction, TQT_SLOT(setEnabled(bool)));

	_importAction = new TDEAction(i18n("&Import XML..."), 0, 0, TQT_TQOBJECT(this),
			TQT_SLOT(importXML()), actionCollection(),
			"import");
	connect(this, TQT_SIGNAL(enableWalletActions(bool)),
		_importAction, TQT_SLOT(setEnabled(bool)));

	_exportAction = new TDEAction(i18n("&Export..."), 0, 0, TQT_TQOBJECT(this),
			TQT_SLOT(exportXML()), actionCollection(),
			"export");
	connect(this, TQT_SIGNAL(enableWalletActions(bool)),
		_exportAction, TQT_SLOT(setEnabled(bool)));

	_saveAsAction = KStdAction::saveAs(TQT_TQOBJECT(this), TQT_SLOT(saveAs()), actionCollection());
	connect(this, TQT_SIGNAL(enableWalletActions(bool)),
		_saveAsAction, TQT_SLOT(setEnabled(bool)));

	KStdAction::quit(TQT_TQOBJECT(this), TQT_SLOT(close()), actionCollection());
	KStdAction::keyBindings(guiFactory(), TQT_SLOT(configureShortcuts()),
actionCollection());
	emit enableWalletActions(false);
	emit enableFolderActions(false);
	emit enableContextFolderActions(false);
}


void TDEWalletEditor::walletClosed() {
	delete _w;
	_walletIsOpen = false;
	_w = 0L;
	_ww->setEnabled(false);
	emit enableWalletActions(false);
	emit enableFolderActions(false);
	KMessageBox::sorry(this, i18n("This wallet was forced closed.  You must reopen it to continue working with it."));
	deleteLater();
}


void TDEWalletEditor::updateFolderList(bool checkEntries) {
	TQStringList fl = _w->folderList();
	TQPtrStack<TQListViewItem> trash;

	for (TQListViewItem *i = _entryList->firstChild(); i; i = i->nextSibling()) {
		TDEWalletFolderItem *fi = dynamic_cast<TDEWalletFolderItem *>(i);
		if (!fi) {
			continue;
		}
		if (!fl.contains(fi->name())) {
			trash.push(i);
		}
	}

	trash.setAutoDelete(true);
	trash.clear();

	for (TQStringList::Iterator i = fl.begin(); i != fl.end(); ++i) {
		if (_entryList->existsFolder(*i)) {
			if (checkEntries) {
				updateEntries(*i);
			}
			continue;
		}

		_w->setFolder(*i);
		TQStringList entries = _w->entryList();
		TDEWalletFolderItem *item = new TDEWalletFolderItem(_w,_entryList,
			*i, entries.count());

		TDEWalletContainerItem *pi = new TDEWalletContainerItem(item, i18n("Passwords"),TDEWallet::Wallet::Password);
		TDEWalletContainerItem *mi = new TDEWalletContainerItem(item, i18n("Maps"),TDEWallet::Wallet::Map);
		TDEWalletContainerItem *bi = new TDEWalletContainerItem(item, i18n("Binary Data"),TDEWallet::Wallet::Stream);
		TDEWalletContainerItem *ui = new TDEWalletContainerItem(item, i18n("Unknown"),TDEWallet::Wallet::Unknown);

		for (TQStringList::Iterator j = entries.begin(); j != entries.end(); ++j) {
			switch (_w->entryType(*j)) {
				case TDEWallet::Wallet::Password:
					new TDEWalletEntryItem(_w, pi, *j);
					break;
				case TDEWallet::Wallet::Stream:
					new TDEWalletEntryItem(_w, bi, *j);
					break;
				case TDEWallet::Wallet::Map:
					new TDEWalletEntryItem(_w, mi, *j);
					break;
				case TDEWallet::Wallet::Unknown:
				default:
					new TQListViewItem(ui, *j);
					break;
			}
		}
		_entryList->setEnabled(true);
	}

	//check if the current folder has been removed
	if (!fl.contains(_currentFolder)) {
		_currentFolder = "";
		_ww->_entryTitle->clear();
		_ww->_iconTitle->clear();
	}
}

void TDEWalletEditor::deleteFolder() {
	if (_w) {
		TQListViewItem *i = _entryList->currentItem();
		if (i) {
			TDEWalletFolderItem *fi = dynamic_cast<TDEWalletFolderItem *>(i);
			if (!fi) {
				return;
			}

			int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the folder '%1' from the wallet?").arg(fi->name()),"",KStdGuiItem::del());
			if (rc == KMessageBox::Continue) {
				bool rc = _w->removeFolder(fi->name());
				if (!rc) {
					KMessageBox::sorry(this, i18n("Error deleting folder."));
					return;
				}
				_currentFolder = "";
				_ww->_entryTitle->clear();
				_ww->_iconTitle->clear();
				updateFolderList();
			}
		}
	}
}


void TDEWalletEditor::createFolder() {
	if (_w) {
		TQString n;
		bool ok;

		do {
			n = KInputDialog::getText(i18n("New Folder"),
					i18n("Please choose a name for the new folder:"),
					TQString(),
					&ok,
					this);

			if (!ok) {
				return;
			}

			if (_entryList->existsFolder(n)) {
				int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that folder name is in use. Try again?"), TQString(), i18n("Try Again"), i18n("Do Not Try"));
				if (rc == KMessageBox::Yes) {
					continue;
				}
				n = TQString();
			}
			break;
		} while (true);

		_w->createFolder(n);
		updateFolderList();
	}
}


void TDEWalletEditor::saveEntry() {
	int rc = 1;
	TQListViewItem *item = _entryList->currentItem();
	_ww->_saveChanges->setEnabled(false);
	_ww->_undoChanges->setEnabled(false);

	if (item && _w && item->parent()) {
		TDEWalletContainerItem *ci = dynamic_cast<TDEWalletContainerItem*>(item->parent());
		if (ci) {
			if (ci->type() == TDEWallet::Wallet::Password) {
				rc = _w->writePassword(item->text(0), _ww->_passwordValue->text());
			} else if (ci->type() == TDEWallet::Wallet::Map) {
				_mapEditor->saveMap();
				rc = _w->writeMap(item->text(0), _currentMap);
			} else {
				return;
			}

			if (rc == 0) {
				return;
			}
		}
	}

	KMessageBox::sorry(this, i18n("Error saving entry. Error code: %1").arg(rc));
}


void TDEWalletEditor::restoreEntry() {
	entrySelectionChanged(_entryList->currentItem());
}


void TDEWalletEditor::entryEditted() {
	_ww->_saveChanges->setEnabled(true);
	_ww->_undoChanges->setEnabled(true);
}


void TDEWalletEditor::entrySelectionChanged(TQListViewItem *item) {
	TDEWalletContainerItem *ci = 0L;
	TDEWalletFolderItem *fi = 0L;

	switch (item->rtti()) {
		case TDEWalletEntryItemClass:
			ci = dynamic_cast<TDEWalletContainerItem*>(item->parent());
			if (!ci) {
				return;
			}
			fi = dynamic_cast<TDEWalletFolderItem*>(ci->parent());
			if (!fi) {
				return;
			}
			_w->setFolder(fi->name());
			_deleteFolderAction->setEnabled(false);
			if (ci->type() == TDEWallet::Wallet::Password) {
				TQString pass;
				if (_w->readPassword(item->text(0), pass) == 0) {
					_ww->_entryStack->raiseWidget(int(4));
					_ww->_entryName->setText(i18n("Password: %1")
							.arg(item->text(0)));
					_ww->_passwordValue->setText(pass);
					_ww->_saveChanges->setEnabled(false);
					_ww->_undoChanges->setEnabled(false);
				}
			} else if (ci->type() == TDEWallet::Wallet::Map) {
				_ww->_entryStack->raiseWidget(int(2));
				_mapEditorShowHide->setChecked(false);
				showHideMapEditorValue(false);
				if (_w->readMap(item->text(0), _currentMap) == 0) {
					_mapEditor->reload();
					_ww->_entryName->setText(i18n("Name-Value Map: %1").arg(item->text(0)));
					_ww->_saveChanges->setEnabled(false);
					_ww->_undoChanges->setEnabled(false);
				}
			} else if (ci->type() == TDEWallet::Wallet::Stream) {
				_ww->_entryStack->raiseWidget(int(3));
				TQByteArray ba;
				if (_w->readEntry(item->text(0), ba) == 0) {
					_ww->_entryName->setText(i18n("Binary Data: %1")
							.arg(item->text(0)));
					_ww->_saveChanges->setEnabled(false);
					_ww->_undoChanges->setEnabled(false);
				}
			}
			break;

		case TDEWalletContainerItemClass:
			fi = dynamic_cast<TDEWalletFolderItem*>(item->parent());
			if (!fi) {
				return;
			}
			_w->setFolder(fi->name());
			_deleteFolderAction->setEnabled(false);
			_ww->_entryName->clear();
			_ww->_entryStack->raiseWidget(int(0));
			break;

		case TDEWalletFolderItemClass:
			fi = dynamic_cast<TDEWalletFolderItem*>(item);
			if (!fi) {
				return;
			}
			_w->setFolder(fi->name());
			_deleteFolderAction->setEnabled(true);
			_ww->_entryName->clear();
			_ww->_entryStack->raiseWidget(int(0));
			break;
	}	

	if (fi) {
		_currentFolder = fi->name();
		_ww->_entryTitle->setText(TQString("<font size=\"+1\">%1</font>").arg(fi->text(0)));
		_ww->_iconTitle->setPixmap(fi->getFolderIcon(TDEIcon::Toolbar));
	}
}

void TDEWalletEditor::updateEntries(const TQString& folder) {
	TQPtrStack<TQListViewItem> trash;

	_w->setFolder(folder);
	TQStringList entries = _w->entryList();

	TDEWalletFolderItem *fi = _entryList->getFolder(folder);

	if (!fi) {
		return;
	}

	TDEWalletContainerItem *pi = fi->getContainer(TDEWallet::Wallet::Password);
	TDEWalletContainerItem *mi = fi->getContainer(TDEWallet::Wallet::Map);
	TDEWalletContainerItem *bi = fi->getContainer(TDEWallet::Wallet::Stream);
	TDEWalletContainerItem *ui = fi->getContainer(TDEWallet::Wallet::Unknown);

	// Remove deleted entries
	for (TQListViewItem *i = pi->firstChild(); i; i = i->nextSibling()) {
		if (!entries.contains(i->text(0))) {
			if (i == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(i);
		}
	}

	for (TQListViewItem *i = mi->firstChild(); i; i = i->nextSibling()) {
		if (!entries.contains(i->text(0))) {
			if (i == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(i);
		}
	}

	for (TQListViewItem *i = bi->firstChild(); i; i = i->nextSibling()) {
		if (!entries.contains(i->text(0))) {
			if (i == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(i);
		}
	}

	for (TQListViewItem *i = ui->firstChild(); i; i = i->nextSibling()) {
		if (!entries.contains(i->text(0))) {
			if (i == _entryList->currentItem()) {
				entrySelectionChanged(0L);
			}
			trash.push(i);
		}
	}

	trash.setAutoDelete(true);
	trash.clear();

	// Add new entries
	for (TQStringList::Iterator i = entries.begin(); i != entries.end(); ++i) {
		if (fi->contains(*i)){
			continue;
		}

		switch (_w->entryType(*i)) {
			case TDEWallet::Wallet::Password:
				new TDEWalletEntryItem(_w, pi, *i);
				break;
			case TDEWallet::Wallet::Stream:
				new TDEWalletEntryItem(_w, bi, *i);
				break;
			case TDEWallet::Wallet::Map:
				new TDEWalletEntryItem(_w, mi, *i);
				break;
			case TDEWallet::Wallet::Unknown:
			default:
				new TQListViewItem(ui, *i);
				break;
		}
	}
	fi->refresh();
	if (fi->name() == _currentFolder) {
		_ww->_entryTitle->setText(TQString("<font size=\"+1\">%1</font>").arg(fi->text(0)));
	}
	if (!_entryList->selectedItem()) {
		_ww->_entryName->clear();
		_ww->_entryStack->raiseWidget(int(0));
	}
}

void TDEWalletEditor::listContextMenuRequested(TQListViewItem *item, const TQPoint& pos, int col) {
	Q_UNUSED(col)

	if (!_walletIsOpen) {
		return;
	}

	TDEWalletListItemClasses menuClass = TDEWalletUnknownClass;
	TDEWalletContainerItem *ci = 0L;

	if (item) {
		if (item->rtti() == TDEWalletEntryItemClass) {
			ci = dynamic_cast<TDEWalletContainerItem *>(item->parent());
			if (!ci) {
				return;
			}
		} else if (item->rtti() == TDEWalletContainerItemClass) {
			ci = dynamic_cast<TDEWalletContainerItem *>(item);
			if (!ci) {
				return;
			}
		}

		if (ci && ci->type() == TDEWallet::Wallet::Unknown) {
			return;
		}
		menuClass = static_cast<TDEWalletListItemClasses>(item->rtti());
	}

	TDEPopupMenu *m = new TDEPopupMenu(this);
	if (item) {
		TQString title = item->text(0);
		// I think 200 pixels is wide enough for a title
		title = KStringHandler::cPixelSqueeze(title, m->fontMetrics(), 200);
		m->insertTitle(title);
		switch (menuClass) {
			case TDEWalletEntryItemClass:
				m->insertItem(i18n("&New..." ), this, TQT_SLOT(newEntry()), Key_Insert);
				m->insertItem(i18n( "&Rename" ), this, TQT_SLOT(renameEntry()), Key_F2);
				m->insertItem(i18n( "&Delete" ), this, TQT_SLOT(deleteEntry()), Key_Delete);
				if (ci && ci->type() == TDEWallet::Wallet::Password) {
					m->insertSeparator();
					_copyPassAction->plug(m);
				}
				break;

			case TDEWalletContainerItemClass:
				m->insertItem(i18n( "&New..." ), this, TQT_SLOT(newEntry()), Key_Insert);
				break;

			case TDEWalletFolderItemClass:
				_newFolderAction->plug(m);
				_deleteFolderAction->plug(m);
				break;
			default:
				abort();
		}
	} else {
		_newFolderAction->plug(m);
	}
	m->popup(pos);
}


void TDEWalletEditor::copyPassword() {
	TQListViewItem *item = _entryList->selectedItem();
	if (_w && item) {
		TQString pass;
		if (_w->readPassword(item->text(0), pass) == 0) {
			TQApplication::clipboard()->setText(pass);
		}
	}
}


void TDEWalletEditor::newEntry() {
	TQListViewItem *item = _entryList->selectedItem();
	TQString n;
	bool ok;

	TQListViewItem *p;
	TDEWalletFolderItem *fi;

	//set the folder where we're trying to create the new entry
	if (_w && item) {
		p = item;
		if (p->rtti() == TDEWalletEntryItemClass) {
			p = item->parent();
		}
		fi = dynamic_cast<TDEWalletFolderItem *>(p->parent());
		if (!fi) {
			return;
		}
		_w->setFolder(fi->name());
	} else {
		return;
	}

	do {
		n = KInputDialog::getText(i18n("New Entry"),
				i18n("Please choose a name for the new entry:"),
				TQString(),
				&ok,
				this);

		if (!ok) {
			return;
		}

		// FIXME: prohibits the use of the subheadings
		if (fi->contains(n)) {
			int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that entry already exists. Try again?"), TQString(), i18n("Try Again"), i18n("Do Not Try"));
			if (rc == KMessageBox::Yes) {
				continue;
			}
			n = TQString();
		}
		break;
	} while (true);

	if (_w && item && !n.isEmpty()) {
		TQListViewItem *p = item;
		if (p->rtti() == TDEWalletEntryItemClass) {
			p = item->parent();
		}

		TDEWalletFolderItem *fi = dynamic_cast<TDEWalletFolderItem *>(p->parent());
		if (!fi) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to add the new entry"));
			return;
		}
		_w->setFolder(fi->name());

		TDEWalletEntryItem *ni = new TDEWalletEntryItem(_w, p, n);
		_entryList->setSelected(ni,true);
		_entryList->ensureItemVisible(ni);

		TDEWalletContainerItem *ci = dynamic_cast<TDEWalletContainerItem*>(p);
		if (!ci) {
			KMessageBox::error(this, i18n("An unexpected error occurred trying to add the new entry"));
			return;
		}
		if (ci->type() == TDEWallet::Wallet::Password) {
			_w->writePassword(n, TQString());
		} else if (ci->type() == TDEWallet::Wallet::Map) {
			_w->writeMap(n, TQMap<TQString,TQString>());
		} else if (ci->type() == TDEWallet::Wallet::Stream) {
			_w->writeEntry(n, TQByteArray());
		} else {
			abort();
		}
		fi->refresh();
		_ww->_entryTitle->setText(TQString("<font size=\"+1\">%1</font>").arg(fi->text(0)));
	}
}


void TDEWalletEditor::renameEntry() {
	TQListViewItem *item = _entryList->selectedItem();
	if (_w && item) {
		item->startRename(0);
	}
}


// Only supports renaming of TDEWalletEntryItem derived classes.
void TDEWalletEditor::listItemRenamed(TQListViewItem* item, int, const TQString& t) {
	if (item) {
		TDEWalletEntryItem *i = dynamic_cast<TDEWalletEntryItem*>(item);
		if (!i) {
			return;
		}

		if (!_w || t.isEmpty()) {
			i->setText(0, i->oldName());
			return;
		}

		if (_w->renameEntry(i->oldName(), t) == 0) {
			i->clearOldName();
			TDEWalletContainerItem *ci = dynamic_cast<TDEWalletContainerItem*>(item->parent());
			if (!ci) {
				KMessageBox::error(this, i18n("An unexpected error occurred trying to rename the entry"));
				return;
			}
			if (ci->type() == TDEWallet::Wallet::Password) {
				_ww->_entryName->setText(i18n("Password: %1").arg(item->text(0)));
			} else if (ci->type() == TDEWallet::Wallet::Map) {
				_ww->_entryName->setText(i18n("Name-Value Map: %1").arg(item->text(0)));
			} else if (ci->type() == TDEWallet::Wallet::Stream) {
				_ww->_entryName->setText(i18n("Binary Data: %1").arg(item->text(0)));
			}
		} else {
			i->setText(0, i->oldName());
		}
	}
}


void TDEWalletEditor::deleteEntry() {
	TQListViewItem *item = _entryList->selectedItem();
	if (_w && item) {
		int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the item '%1'?").arg(item->text(0)),"",KStdGuiItem::del());
		if (rc == KMessageBox::Continue) {
			TDEWalletFolderItem *fi = dynamic_cast<TDEWalletFolderItem *>(item->parent()->parent());
			if (!fi) {
				KMessageBox::error(this, i18n("An unexpected error occurred trying to delete the entry"));
				return;
			}
			_w->removeEntry(item->text(0));
			delete item;
			entrySelectionChanged(_entryList->currentItem());
			fi->refresh();
			_ww->_entryTitle->setText(TQString("<font size=\"+1\">%1</font>").arg(fi->text(0)));
		}
	}
}

void TDEWalletEditor::changePassword() {
	TDEWallet::Wallet::changePassword(_walletName);
}


void TDEWalletEditor::walletOpened(bool success) {
	if (success) {
		emit enableFolderActions(true);
		emit enableContextFolderActions(false);
		emit enableWalletActions(true);
		updateFolderList();
		show();
		_entryList->setWallet(_w);
		_walletIsOpen = true;
	} else {
		if (!_newWallet) {
			KMessageBox::sorry(this, i18n("Unable to open the requested wallet."));
		}
		close();
	}
}


void TDEWalletEditor::hidePasswordContents() {
	_ww->_entryStack->raiseWidget(int(4));
}


void TDEWalletEditor::showPasswordContents() {
	_ww->_entryStack->raiseWidget(int(1));
}


void TDEWalletEditor::showHideMapEditorValue(bool show) {
	if (show) {
		_mapEditor->showColumn(2);
	} else {
		_mapEditor->hideColumn(2);
	}
}


enum MergePlan { Prompt = 0, Always = 1, Never = 2, Yes = 3, No = 4 };

void TDEWalletEditor::importWallet() {
	KURL url = KFileDialog::getOpenURL(TQString(), "*.kwl", this);
	if (url.isEmpty()) {
		return;
	}

	TQString tmpFile;
	if (!TDEIO::NetAccess::download(url, tmpFile, this)) {
		KMessageBox::sorry(this, i18n("Unable to access wallet '<b>%1</b>'.").arg(url.prettyURL()));
		return;
	}

	TDEWallet::Wallet *w = TDEWallet::Wallet::openWallet(tmpFile, winId(), TDEWallet::Wallet::Path);
	if (w && w->isOpen()) {
		MergePlan mp = Prompt;
		TQStringList fl = w->folderList();
		for (TQStringList::ConstIterator f = fl.constBegin(); f != fl.constEnd(); ++f) {
			if (!w->setFolder(*f)) {
				continue;
			}

			if (!_w->hasFolder(*f)) {
				_w->createFolder(*f);
			}

			_w->setFolder(*f);
			
			TQMap<TQString, TQMap<TQString, TQString> > map;
			int rc;
			rc = w->readMapList("*", map);
			if (rc == 0) {
				TQMap<TQString, TQMap<TQString, TQString> >::ConstIterator me;
				for (me = map.constBegin(); me != map.constEnd(); ++me) {
					bool hasEntry = _w->hasEntry(me.key());
					if (hasEntry && mp == Prompt) {
						KBetterThanKDialogBase *bd;
						bd = new KBetterThanKDialogBase(this);
						bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?").arg(TQStyleSheet::escape(*f)).arg(TQStyleSheet::escape(me.key())));
						mp = (MergePlan)bd->exec();
						delete bd;
						bool ok = false;
						if (mp == Always || mp == Yes) {
							ok = true;
						}
						if (mp == Yes || mp == No) {
						       	// reset mp
							mp = Prompt;
						}
						if (!ok) {
							continue;
						}
					} else if (hasEntry && mp == Never) {
						continue;
					}
					_w->writeMap(me.key(), me.data());
				}
			}

			TQMap<TQString, TQString> pwd;
			rc = w->readPasswordList("*", pwd);
			if (rc == 0) {
				TQMap<TQString, TQString>::ConstIterator pe;
				for (pe = pwd.constBegin(); pe != pwd.constEnd(); ++pe) {
					bool hasEntry = _w->hasEntry(pe.key());
					if (hasEntry && mp == Prompt) {
						KBetterThanKDialogBase *bd;
						bd = new KBetterThanKDialogBase(this);
						bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?").arg(TQStyleSheet::escape(*f)).arg(TQStyleSheet::escape(pe.key())));
						mp = (MergePlan)bd->exec();
						delete bd;
						bool ok = false;
						if (mp == Always || mp == Yes) {
							ok = true;
						}
						if (mp == Yes || mp == No) {
						       	// reset mp
							mp = Prompt;
						}
						if (!ok) {
							continue;
						}
					} else if (hasEntry && mp == Never) {
						continue;
					}
					_w->writePassword(pe.key(), pe.data());
				}
			}

			TQMap<TQString, TQByteArray> ent;
			rc = w->readEntryList("*", ent);
			if (rc == 0) {
				TQMap<TQString, TQByteArray>::ConstIterator ee;
				for (ee = ent.constBegin(); ee != ent.constEnd(); ++ee) {
					bool hasEntry = _w->hasEntry(ee.key());
					if (hasEntry && mp == Prompt) {
						KBetterThanKDialogBase *bd;
						bd = new KBetterThanKDialogBase(this);
						bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?").arg(TQStyleSheet::escape(*f)).arg(TQStyleSheet::escape(ee.key())));
						mp = (MergePlan)bd->exec();
						delete bd;
						bool ok = false;
						if (mp == Always || mp == Yes) {
							ok = true;
						}
						if (mp == Yes || mp == No) {
						       	// reset mp
							mp = Prompt;
						}
						if (!ok) {
							continue;
						}
					} else if (hasEntry && mp == Never) {
						continue;
					}
					_w->writeEntry(ee.key(), ee.data());
				}
			}
		}
	}

	delete w;

	TDEIO::NetAccess::removeTempFile(tmpFile);
	updateFolderList(true);
	restoreEntry();
}


void TDEWalletEditor::importXML() {
	KURL url = KFileDialog::getOpenURL(TQString(), "*.xml", this);
	if (url.isEmpty()) {
		return;
	}

	TQString tmpFile;
	if (!TDEIO::NetAccess::download(url, tmpFile, this)) {
		KMessageBox::sorry(this, i18n("Unable to access XML file '<b>%1</b>'.").arg(url.prettyURL()));
		return;
	}

	TQFile qf(tmpFile);
	if (!qf.open(IO_ReadOnly)) {
		KMessageBox::sorry(this, i18n("Error opening XML file '<b>%1</b>' for input.").arg(url.prettyURL()));
		TDEIO::NetAccess::removeTempFile(tmpFile);
		return;
	}

	TQDomDocument doc(tmpFile);
	if (!doc.setContent(&qf)) {
		KMessageBox::sorry(this, i18n("Error reading XML file '<b>%1</b>' for input.").arg(url.prettyURL()));
		TDEIO::NetAccess::removeTempFile(tmpFile);
		return;
	}

	TQDomElement top = doc.documentElement();
	if (top.tagName().lower() != "wallet") {
		KMessageBox::sorry(this, i18n("Error: XML file does not contain a wallet."));
		TDEIO::NetAccess::removeTempFile(tmpFile);
		return;
	}

	TQDomNode n = top.firstChild();
	MergePlan mp = Prompt;
	while (!n.isNull()) {
		TQDomElement e = n.toElement();
		if (e.tagName().lower() != "folder") {
			n = n.nextSibling();
			continue;
		}

		TQString fname = e.attribute("name");
		if (fname.isEmpty()) {
			n = n.nextSibling();
			continue;
		}
		if (!_w->hasFolder(fname)) {
			_w->createFolder(fname);
		}
		_w->setFolder(fname);
		TQDomNode enode = e.firstChild();
		while (!enode.isNull()) {
			e = enode.toElement();
			TQString type = e.tagName().lower();
			TQString ename = e.attribute("name");
			bool hasEntry = _w->hasEntry(ename);
			if (hasEntry && mp == Prompt) {
				KBetterThanKDialogBase *bd;
				bd = new KBetterThanKDialogBase(this);
				bd->setLabel(i18n("Folder '<b>%1</b>' already contains an entry '<b>%2</b>'.  Do you wish to replace it?").arg(TQStyleSheet::escape(fname)).arg(TQStyleSheet::escape(ename)));
				mp = (MergePlan)bd->exec();
				delete bd;
				bool ok = false;
				if (mp == Always || mp == Yes) {
					ok = true;
				}
				if (mp == Yes || mp == No) { // reset mp
					mp = Prompt;
				}
				if (!ok) {
					enode = enode.nextSibling();
					continue;
				}
			} else if (hasEntry && mp == Never) {
				enode = enode.nextSibling();
				continue;
			}

			if (type == "password") {
				_w->writePassword(ename, e.text());
			} else if (type == "stream") {
				_w->writeEntry(ename, KCodecs::base64Decode(TQCString(e.text().latin1())));
			} else if (type == "map") {
				TQMap<TQString,TQString> map;
				TQDomNode mapNode = e.firstChild();
				while (!mapNode.isNull()) {
					TQDomElement mape = mapNode.toElement();
					if (mape.tagName().lower() == "mapentry") {
						map[mape.attribute("name")] = mape.text();
					}
					mapNode = mapNode.nextSibling();
				}
				_w->writeMap(ename, map);
			}
			enode = enode.nextSibling();
		}
		n = n.nextSibling();
	}

	TDEIO::NetAccess::removeTempFile(tmpFile);
	updateFolderList(true);
	restoreEntry();
}


void TDEWalletEditor::exportXML() {
	KTempFile tf;
	tf.setAutoDelete(true);
	TQTextStream& ts(*tf.textStream());
	TQStringList fl = _w->folderList();

	ts << "<wallet name=\"" << _walletName << "\">" << endl;
	for (TQStringList::Iterator i = fl.begin(); i != fl.end(); ++i) {
		ts << "  <folder name=\"" << *i << "\">" << endl;
		_w->setFolder(*i);
		TQStringList entries = _w->entryList();
		for (TQStringList::Iterator j = entries.begin(); j != entries.end(); ++j) {
			switch (_w->entryType(*j)) {
				case TDEWallet::Wallet::Password:
					{
						TQString pass;
						if (_w->readPassword(*j, pass) == 0) {
							ts << "    <password name=\"" << TQStyleSheet::escape(*j) << "\">";
							ts << TQStyleSheet::escape(pass);
							ts << "</password>" << endl;
						}
						break;
					}
				case TDEWallet::Wallet::Stream:
					{
						TQByteArray ba;
						if (_w->readEntry(*j, ba) == 0) {
							ts << "    <stream name=\"" << TQStyleSheet::escape(*j) << "\">";
							ts << KCodecs::base64Encode(ba);

							ts << "</stream>" << endl;
						}
						break;
					}
				case TDEWallet::Wallet::Map:
					{
						TQMap<TQString,TQString> map;
						if (_w->readMap(*j, map) == 0) {
							ts << "    <map name=\"" << TQStyleSheet::escape(*j) << "\">" << endl;
							for (TQMap<TQString,TQString>::ConstIterator k = map.begin(); k != map.end(); ++k) {
								ts << "      <mapentry name=\"" << TQStyleSheet::escape(k.key()) << "\">" << TQStyleSheet::escape(k.data()) << "</mapentry>" << endl;
							}
							ts << "    </map>" << endl;
						}
						break;
					}
				case TDEWallet::Wallet::Unknown:
				default:
					break;
			}
		}
		ts << "  </folder>" << endl;
	}

	ts << "</wallet>" << endl;
	tf.close();

	KURL url = KFileDialog::getSaveURL(TQString(), "*.xml", this);

	if (!url.isEmpty()) {
		bool ok = true;
		if (TDEIO::NetAccess::exists(url, false, this)) {
			int rc = KMessageBox::warningContinueCancel(this, i18n("The file '%1' already exists. Would you like to overwrite this file?").arg(url.prettyURL()), i18n("Overwrite"));
			if (rc == KMessageBox::Cancel) {
				ok = false;
			}
		}
		if (ok) {
			KURL tfURL; tfURL.setPath(tf.name());
			TDEIO::NetAccess::file_copy(tfURL, url, 0600, true, false, this);
		}
	}
}


void TDEWalletEditor::setNewWallet(bool x) {
	_newWallet = x;
}


void TDEWalletEditor::saveAs() {
	KURL url = KFileDialog::getSaveURL(TQString(), "*.kwl", this);
	if (!url.isEmpty()) {
		// Sync() tdewalletd
		if (_nonLocal) {
			TDEIO::NetAccess::file_copy(KURL(_walletName), url, 0600, false, false, this);
		} else {
			TQString path = TDEGlobal::dirs()->saveLocation("tdewallet") + "/" + _walletName + ".kwl";
			KURL destURL; destURL.setPath(path);
			TDEIO::NetAccess::file_copy(destURL, url, 0600, false, false, this);
		}
	}
}


#include "tdewalleteditor.moc"

