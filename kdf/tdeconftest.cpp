/*
**
** Copyright (C) 1999 by Michael Kropfberger
**
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
** MA 02110-1301, USA.
*/

/*
** Bug reports and questions can be sent to kde-devel@kde.org
*/


/* compile with
* gcc  -I$TQTDIR/include -I$TDEDIR/include   -L/opt/kde/lib -L/usr/X11R6/lib -lkfm -ltdeui -ltdecore -lqt -lX11 -lXext -fno-rtti tdeconftest.cpp                
*/


#include <iostream>
#include <tqstring.h>
#include <tqdict.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>

static const char description[] =
	I18N_NOOP("A test application");

static const char version[] = "v0.0.1";

main(int argc, char ** argv)
{
 TDECmdLineArgs::init(argc, argv, "test", description, version);
 
 TDEApplication app;
 TDEConfig * cfg = kapp->config();

 TQDict<char> dict;

 dict.insert("Blah", "Arse");
 dict.insert("Blah", "Smack");
 dict.insert("Blah", "Monkey");

 TQDictIterator<char> it(dict);

 TQString key = "TestConfigItem";

 for (; it.current(); ++it) {

  cerr << "Before saving: " << endl;
  cerr << "key : \"" << key << "\"" << endl;
  cerr << "val : \"" << it.current() << "\"" << endl;

  debug("got back [%s]",cfg->writeEntry(key, it.current()));
//  debug("got back [%s]",s.data());

  cerr << "After saving: " << endl;
  cerr << "key : \"" << key << "\"" << endl;
  cerr << "val : \"" << it.current() << "\"" << endl;

  cerr << endl;
 }
}
