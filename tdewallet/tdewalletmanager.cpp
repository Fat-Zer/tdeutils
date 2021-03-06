/*
   Copyright (C) 2003,2004 George Staikos <staikos@kde.org>

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


#include "tdewalletmanager.h"
#include "tdewalletpopup.h"
#include "tdewalleteditor.h"
#include "allyourbase.h"

#include <dcopclient.h>
#include <dcopref.h>
#include <tdeaction.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <kiconview.h>
#include <kinputdialog.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>
#include <kstdaction.h>
#include <ksystemtray.h>
#include <tdewallet.h>

#include <tqaccel.h>
#include <tqguardedptr.h>
#include <tqptrstack.h>
#include <tqregexp.h>
#include <tqtimer.h>
#include <tqtooltip.h>

TDEWalletManager::TDEWalletManager(TQWidget *parent, const char *name, WFlags f)
: TDEMainWindow(parent, name, f), DCOPObject("TDEWalletManager") {
	TDEGlobal::dirs()->addResourceType("tdewallet", "share/apps/tdewallet");
	_tdewalletdLaunch = false;
	TQAccel *accel = new TQAccel(this, "tdewalletmanager");

	TDEApplication::dcopClient()->setQtBridgeEnabled(false);
	_shuttingDown = false;
	TDEConfig cfg("tdewalletrc"); // not sure why this setting isn't in tdewalletmanagerrc...
	TDEConfigGroup walletConfigGroup(&cfg, "Wallet");
	_dcopRef = 0L;
	if (walletConfigGroup.readBoolEntry("Launch Manager", true)) {
		_tray = new KSystemTray(this, "tdewalletmanager tray");
		_tray->setPixmap(loadSystemTrayIcon("wallet_closed"));
		TQToolTip::add(_tray, i18n("TDE Wallet: No wallets open."));
		connect(_tray, TQT_SIGNAL(quitSelected()), TQT_SLOT(shuttingDown()));
		TQStringList wl = TDEWallet::Wallet::walletList();
		bool isOpen = false;
		for (TQStringList::Iterator it = wl.begin(); it != wl.end(); ++it) {
			if (TDEWallet::Wallet::isOpen(*it)) {
				_tray->setPixmap(loadSystemTrayIcon("wallet_open"));
				TQToolTip::remove(_tray);
				TQToolTip::add(_tray, i18n("TDE Wallet: A wallet is open."));
				isOpen = true;
				break;
			}
		}
		if (!isOpen && kapp->isRestored()) {
			delete _tray;
			_tray = 0L;
			TQTimer::singleShot( 0, kapp, TQT_SLOT( quit()));
			return;
		}
	} else {
		_tray = 0L;
	}

	_iconView = new TDEWalletIconView(this, "tdewalletmanager icon view");
	connect(_iconView, TQT_SIGNAL(executed(TQIconViewItem*)), TQT_TQOBJECT(this), TQT_SLOT(openWallet(TQIconViewItem*)));
	connect(_iconView, TQT_SIGNAL(contextMenuRequested(TQIconViewItem*, const TQPoint&)), TQT_TQOBJECT(this), TQT_SLOT(contextMenu(TQIconViewItem*, const TQPoint&)));

	updateWalletDisplay();
	setCentralWidget(_iconView);
	_iconView->setMinimumSize(320, 200);

	_dcopRef = new DCOPRef("kded", "tdewalletd");
	_dcopRef->dcopClient()->setNotifications(true);
	connect(_dcopRef->dcopClient(),
		TQT_SIGNAL(applicationRemoved(const TQCString&)),
		this,
		TQT_SLOT(possiblyRescan(const TQCString&)));
	connect(_dcopRef->dcopClient(),
		TQT_SIGNAL(applicationRegistered(const TQCString&)),
		this,
		TQT_SLOT(possiblyRescan(const TQCString&)));

	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "allWalletsClosed()", "allWalletsClosed()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletClosed(TQString)", "updateWalletDisplay()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletOpened(TQString)", "aWalletWasOpened()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletDeleted(TQString)", "updateWalletDisplay()", false);
	connectDCOPSignal(_dcopRef->app(), _dcopRef->obj(), "walletListDirty()", "updateWalletDisplay()", false);

	// FIXME: slight race - a wallet can open, then we get launched, but the
	//        wallet closes before we are done opening.  We will then stay
	//        open.  Must check that a wallet is still open here.

	new TDEAction(i18n("&New Wallet..."), "tdewalletmanager", 0, TQT_TQOBJECT(this),
			TQT_SLOT(createWallet()), actionCollection(),
			"wallet_create");
	TDEAction *act = new TDEAction(i18n("Configure &Wallet..."), "configure",
			0, TQT_TQOBJECT(this), TQT_SLOT(setupWallet()), actionCollection(),
			"wallet_settings");
	if (_tray) {
		act->plug(_tray->contextMenu());
	}
	act = new TDEAction(i18n("Close &All Wallets"), 0, 0, TQT_TQOBJECT(this),
			TQT_SLOT(closeAllWallets()), actionCollection(),
			"close_all_wallets");
	if (_tray) {
		act->plug(_tray->contextMenu());
	}
	KStdAction::quit(TQT_TQOBJECT(this), TQT_SLOT(shuttingDown()), actionCollection());
	KStdAction::keyBindings(guiFactory(), TQT_SLOT(configureShortcuts()),
actionCollection());

	createGUI("tdewalletmanager.rc");
	accel->connectItem(accel->insertItem(Key_Return), TQT_TQOBJECT(this), TQT_SLOT(openWallet()));
	accel->connectItem(accel->insertItem(Key_Delete), TQT_TQOBJECT(this), TQT_SLOT(deleteWallet()));

	if (_tray) {
		_tray->show();
	} else {
		show();
	}

	kapp->setName("tdewallet"); // hack to fix docs
}


TDEWalletManager::~TDEWalletManager() {
	_tray = 0L;
	delete _dcopRef;
	_dcopRef = 0L;
}


void TDEWalletManager::tdewalletdLaunch() {
	_tdewalletdLaunch = true;
}


bool TDEWalletManager::queryClose() {
	if (!_shuttingDown && !kapp->sessionSaving()) {
		if (!_tray) {
			kapp->quit();
		} else {
			hide();
		}
		return false;
	}
	return true;
}


void TDEWalletManager::aWalletWasOpened() {
	if (_tray) {
		_tray->setPixmap(loadSystemTrayIcon("wallet_open"));
		TQToolTip::remove(_tray);
		TQToolTip::add(_tray, i18n("TDE Wallet: A wallet is open."));
	}
	updateWalletDisplay();
}


void TDEWalletManager::updateWalletDisplay() {
TQStringList wl = TDEWallet::Wallet::walletList();
TQPtrStack<TQIconViewItem> trash;

	for (TQIconViewItem *item = _iconView->firstItem(); item; item = item->nextItem()) {
		if (!wl.contains(item->text())) {
			trash.push(item);
		}
	}

	trash.setAutoDelete(true);
	trash.clear();

	for (TQStringList::Iterator i = wl.begin(); i != wl.end(); ++i) {
		if (!_iconView->findItem(*i)) {
			// FIXME: if TDEWallet::Wallet::isOpen(*i) then show
			//        a different icon!
			new TDEWalletItem(_iconView, *i);
		} else {
			// FIXME: See if icon needs to be updated
		}
	}
}


void TDEWalletManager::contextMenu(TQIconViewItem *item, const TQPoint& pos) {
	if (item) {
		TQGuardedPtr<TDEWalletPopup> popupMenu = new TDEWalletPopup(item->text(), this);
		connect(popupMenu, TQT_SIGNAL(walletOpened(const TQString&)), TQT_TQOBJECT(this), TQT_SLOT(openWallet(const TQString&)));
		connect(popupMenu, TQT_SIGNAL(walletClosed(const TQString&)), TQT_TQOBJECT(this), TQT_SLOT(closeWallet(const TQString&)));
		connect(popupMenu, TQT_SIGNAL(walletDeleted(const TQString&)), TQT_TQOBJECT(this), TQT_SLOT(deleteWallet(const TQString&)));
		connect(popupMenu, TQT_SIGNAL(walletChangePassword(const TQString&)), TQT_TQOBJECT(this), TQT_SLOT(changeWalletPassword(const TQString&)));
		connect(popupMenu, TQT_SIGNAL(walletCreated()), TQT_TQOBJECT(this), TQT_SLOT(createWallet()));
		popupMenu->exec(pos);
		delete popupMenu;
	}
}


void TDEWalletManager::deleteWallet(const TQString& walletName) {
	int rc = KMessageBox::warningContinueCancel(this, i18n("Are you sure you wish to delete the wallet '%1'?").arg(walletName),"",KStdGuiItem::del());
	if (rc != KMessageBox::Continue) {
		return;
	}
	rc = TDEWallet::Wallet::deleteWallet(walletName);
	if (rc != 0) {
		KMessageBox::sorry(this, i18n("Unable to delete the wallet. Error code was %1.").arg(rc));
	}
	updateWalletDisplay();
}


void TDEWalletManager::closeWallet(const TQString& walletName) {
	int rc = TDEWallet::Wallet::closeWallet(walletName, false);
	if (rc != 0) {
		rc = KMessageBox::warningYesNo(this, i18n("Unable to close wallet cleanly. It is probably in use by other applications. Do you wish to force it closed?"), TQString(), i18n("Force Closure"), i18n("Do Not Force"));
		if (rc == KMessageBox::Yes) {
			rc = TDEWallet::Wallet::closeWallet(walletName, true);
			if (rc != 0) {
				KMessageBox::sorry(this, i18n("Unable to force the wallet closed. Error code was %1.").arg(rc));
			}
		}
	}

	updateWalletDisplay();
}


void TDEWalletManager::changeWalletPassword(const TQString& walletName) {
	TDEWallet::Wallet::changePassword(walletName);
}


void TDEWalletManager::openWalletFile(const TQString& path) {
	TDEWalletEditor *we = new TDEWalletEditor(path, true, this, "Wallet Editor");
	if (we->isOpen()) {
		connect(we, TQT_SIGNAL(editorClosed(TDEMainWindow*)),
			this, TQT_SLOT(editorClosed(TDEMainWindow*)));
		we->show();
	} else {
		KMessageBox::sorry(this, i18n("Error opening wallet %1.").arg(path));
		delete we;
	}
}


void TDEWalletManager::openWallet() {
	TQIconViewItem *item = _iconView->currentItem();
	openWallet(item);
}

void TDEWalletManager::deleteWallet() {
	TQIconViewItem *item = _iconView->currentItem();
	if (item) {
		deleteWallet(item->text());
	}
}


void TDEWalletManager::openWallet(const TQString& walletName) {
	openWallet(walletName, false);
}


void TDEWalletManager::openWallet(const TQString& walletName, bool newWallet) {
	// Don't allow a wallet to open in two windows
	for (TDEMainWindow *w = _windows.first(); w; w = _windows.next()) {
		TDEWalletEditor *e = static_cast<TDEWalletEditor*>(w);
		if (e->isOpen() && e->_walletName == walletName) {
			w->raise();
			return;
		}
	}

	TDEWalletEditor *we = new TDEWalletEditor(walletName, false, this, "Wallet Editor");
	we->setNewWallet(newWallet);
	if (we->isOpen()) {
		connect(we, TQT_SIGNAL(editorClosed(TDEMainWindow*)),
			this, TQT_SLOT(editorClosed(TDEMainWindow*)));
		we->show();
		_windows.append(we);
	} else if (!newWallet) {
		KMessageBox::sorry(this, i18n("Error opening wallet %1.").arg(walletName));
		delete we;
	}
}


void TDEWalletManager::openWallet(TQIconViewItem *item) {
	if (item) {
		openWallet(item->text());
	}
}


void TDEWalletManager::allWalletsClosed() {
	if (_tray) {
		_tray->setPixmap(loadSystemTrayIcon("wallet_closed"));
		TQToolTip::remove(_tray);
		TQToolTip::add(_tray, i18n("TDE Wallet: No wallets open."));
	}
	possiblyQuit();
}


void TDEWalletManager::possiblyQuit() {
	TDEConfig cfg("tdewalletrc");
	cfg.setGroup("Wallet");
	if (_windows.isEmpty() &&
			!isVisible() &&
			!cfg.readBoolEntry("Leave Manager Open", false) &&
			_tdewalletdLaunch) {
		kapp->quit();
	}
}


void TDEWalletManager::editorClosed(TDEMainWindow* e) {
	_windows.remove(e);
}


void TDEWalletManager::possiblyRescan(const TQCString& app) {
	if (app == "kded") {
		updateWalletDisplay();
	}
}


void TDEWalletManager::createWallet() {
	TQString n;
	bool ok;
	// FIXME: support international names
	TQRegExp regexp("^[A-Za-z0-9]+[A-Za-z0-9_\\s\\-]*$");
	TQString txt = i18n("Please choose a name for the new wallet:");

	if (!TDEWallet::Wallet::isEnabled()) {
		// FIXME: KMessageBox::warningYesNo(this, i1_8n("TDEWallet is not enabled.  Do you want to enable it?"), TQString(), i18n("Enable"), i18n("Keep Disabled"));
		return;
	}

	do {
		n = KInputDialog::getText(i18n("New Wallet"),
				txt,
				TQString(),
				&ok,
				this);

		if (!ok) {
			return;
		}

		if (_iconView->findItem(n)) {
			int rc = KMessageBox::questionYesNo(this, i18n("Sorry, that wallet already exists. Try a new name?"), TQString(), i18n("Try New"), i18n("Do Not Try"));
			if (rc == KMessageBox::Yes) {
				continue;
			}
			n = TQString();
		} else if (regexp.exactMatch(n)) {
			break;
		} else {
			txt = i18n("Please choose a name that contains only alphanumeric characters:");
		}
	} while (true);

	// Small race here - the wallet could be created on us already.
	if (!n.isEmpty()) {
		openWallet(n, true);
	}
}


void TDEWalletManager::shuttingDown() {
	_shuttingDown = true;
	kapp->quit();
}


void TDEWalletManager::setupWallet() {
	TDEApplication::startServiceByDesktopName("tdewallet_config");
}


void TDEWalletManager::closeAllWallets() {
	_dcopRef->call("closeAllWallets");
}


TQPixmap TDEWalletManager::loadSystemTrayIcon(const TQString &icon) {
#if KDE_IS_VERSION(3, 1, 90)
	return KSystemTray::loadIcon(icon);
#else
	TDEConfig *appCfg = kapp->config();
	TDEConfigGroupSaver configSaver(appCfg, "System Tray");
	int iconWidth = appCfg->readNumEntry("systrayIconWidth", 22);
	return kapp->iconLoader()->loadIcon( icon, TDEIcon::Panel, iconWidth );
#endif
}


#include "tdewalletmanager.moc"
