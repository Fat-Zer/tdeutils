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

#ifndef KWALLETMANAGER_H
#define KWALLETMANAGER_H

#include <kmainwindow.h>
#include <dcopobject.h>
#include <tqptrlist.h>

class KSystemTray;
class KWalletIconView;
class TQIconViewItem;
class DCOPRef;


class KWalletManager : public KMainWindow, public DCOPObject {
	Q_OBJECT
//	TQ_OBJECT
	K_DCOP

	public:
		KWalletManager(TQWidget *tqparent = 0, const char* name = 0, WFlags f = 0);
		virtual ~KWalletManager();

                TQPixmap loadSystemTrayIcon(const TQString &icon);

		void kwalletdLaunch();

	public slots:
		void createWallet();
		void deleteWallet(const TQString& walletName);
		void closeWallet(const TQString& walletName);
		void changeWalletPassword(const TQString& walletName);
		void openWallet(const TQString& walletName);
		void openWallet(const TQString& walletName, bool newWallet);
		void openWalletFile(const TQString& path);
		void openWallet(TQIconViewItem *item);
		void contextMenu(TQIconViewItem *item, const TQPoint& pos);

	protected:
		virtual bool queryClose();

	private:
	k_dcop:
		ASYNC allWalletsClosed();
		ASYNC updateWalletDisplay();
		ASYNC aWalletWasOpened();

	private slots:
		void shuttingDown();
		void possiblyQuit();
		void editorClosed(KMainWindow* e);
		void possiblyRescan(const TQCString& app);
		void setupWallet();
		void openWallet();
		void deleteWallet();
		void closeAllWallets();

	private:
		KSystemTray *_tray;
		bool _shuttingDown;
		KWalletIconView *_iconView;
		DCOPRef *_dcopRef;
		TQPtrList<KMainWindow> _windows;
		bool _kwalletdLaunch;
};

#endif
