/*
   Copyright (C) 2003-2005 George Staikos <staikos@kde.org>

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

#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <kuniqueapplication.h>

#include <tqfile.h>
#include <tqfileinfo.h>

#include "tdewalletmanager.h"


class MyApp : public KUniqueApplication {
	public:
		MyApp() : KUniqueApplication() { ref(); }
		virtual ~MyApp() {}

		virtual int newInstance() { return 0; }
};

int main(int argc, char **argv) {
	static TDECmdLineOptions options[] = {
		{"show", I18N_NOOP("Show window on startup"), 0},
		{"tdewalletd", I18N_NOOP("For use by tdewalletd only"), 0},
		{"+name", I18N_NOOP("A wallet name"), 0},
		TDECmdLineLastOption
	};

	TDEAboutData about("tdewalletmanager", I18N_NOOP("TDE Wallet Manager"), "1.1",
		I18N_NOOP("TDE Wallet Management Tool"),
		TDEAboutData::License_GPL,
		I18N_NOOP("(c) 2003,2004 George Staikos"), 0,
		"http://www.kde.org/");

	about.addAuthor("George Staikos", I18N_NOOP("Primary author and maintainer"), "staikos@kde.org");
	about.addAuthor("Isaac Clerencia", I18N_NOOP("Developer"), "isaac@warp.es");

	TDECmdLineArgs::init(argc, argv, &about);
	TDECmdLineArgs::addCmdLineOptions(options);

	if (!KUniqueApplication::start()) {
		return 0;
	}

	MyApp a;

	KWalletManager wm;
	wm.setCaption(i18n("TDE Wallet Manager"));

	a.setMainWidget(&wm);

	TDEGlobal::dirs()->addResourceType("tdewallet", "share/apps/tdewallet");

	TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

	if (args->isSet("show")) {
		wm.show();
	}

	if (args->isSet("tdewalletd")) {
		wm.tdewalletdLaunch();
	}

	for (int i = 0; i < args->count(); ++i) {
		TQString fn = TQFileInfo(args->arg(i)).absFilePath();
		KMimeType::Ptr ptr;
		if (TQFile::exists(fn) &&
			(ptr = KMimeType::findByFileContent(fn)) &&
			ptr->is("application/x-kde-wallet")) {
			wm.openWalletFile(fn);
		} else {
			wm.openWallet(args->arg(i));
		}
	}
	args->clear();
	return a.exec();
}

