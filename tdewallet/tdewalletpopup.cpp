/*
   Copyright (C) 2003 George Staikos <staikos@kde.org>

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


#include "tdewalletpopup.h"

#include <tdeaction.h>
#include <kdebug.h>
#include <kiconview.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <tdewallet.h>
#include <kstdguiitem.h>

TDEWalletPopup::TDEWalletPopup(const TQString& wallet, TQWidget *parent, const char *name)
: TDEPopupMenu(parent, name), _walletName(wallet) {
	insertTitle(wallet);
	TDEActionCollection *ac = new TDEActionCollection(this, "tdewallet context actions");
	TDEAction *act;

	act = new TDEAction(i18n("&New Wallet..."), 0, 0, TQT_TQOBJECT(this),
			TQT_SLOT(createWallet()), ac, "wallet_create");
	act->plug(this);

	act = new TDEAction(i18n("&Open..."), 0, Key_Return, TQT_TQOBJECT(this),
			TQT_SLOT(openWallet()), ac, "wallet_open");
	act->plug(this);

	act = new TDEAction(i18n("Change &Password..."), 0, 0, TQT_TQOBJECT(this),
			TQT_SLOT(changeWalletPassword()), ac, "wallet_password");
	act->plug(this);

	TQStringList ul = TDEWallet::Wallet::users(wallet);
	if (!ul.isEmpty()) {
		TDEPopupMenu *pm = new TDEPopupMenu(this, "Disconnect Apps");
		int id = 7000;
		for (TQStringList::Iterator it = ul.begin(); it != ul.end(); ++it) {
			_appMap[id] = *it;
			pm->insertItem(*it, this, TQT_SLOT(disconnectApp(int)), 0, id);
			pm->setItemParameter(id, id);
			id++;
		}

		insertItem(i18n("Disconnec&t"), pm);
	}

	act = KStdAction::close( TQT_TQOBJECT(this),
			TQT_SLOT(closeWallet()), ac, "wallet_close");
	// FIXME: let's track this inside the manager so we don't need a dcop
	//        roundtrip here.
	act->setEnabled(TDEWallet::Wallet::isOpen(wallet));
	act->plug(this);

	act = new TDEAction(i18n("&Delete"), 0, Key_Delete, TQT_TQOBJECT(this),
			TQT_SLOT(deleteWallet()), ac, "wallet_delete");
	act->plug(this);
}


TDEWalletPopup::~TDEWalletPopup() {
}


void TDEWalletPopup::openWallet() {
	emit walletOpened(_walletName);
}


void TDEWalletPopup::deleteWallet() {
        emit walletDeleted(_walletName);
}


void TDEWalletPopup::closeWallet() {
	emit walletClosed(_walletName);
}


void TDEWalletPopup::changeWalletPassword() {
	emit walletChangePassword(_walletName);
}


void TDEWalletPopup::createWallet() {
	emit walletCreated();
}


void TDEWalletPopup::disconnectApp(int id) {
	TDEWallet::Wallet::disconnectApplication(_walletName, _appMap[id].latin1());
}

#include "tdewalletpopup.moc"

