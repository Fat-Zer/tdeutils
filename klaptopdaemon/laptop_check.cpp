/*
 * laptop_check.cpp
 * Copyright (C) 2003 Paul Campbell <paul@taniwha.com>
 *
 * quick check application - runs from kcminit - makes the check
 * 	to see if kded should be told to run the laptop daemon at
 * 	book time
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "daemon_state.h"
#include <stdlib.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <klocale.h>
static const char description[] =
        I18N_NOOP("KDE laptop daemon starter");

static const char version[] = "v0.0.1";

extern void wake_laptop_daemon();

int
main(int argc, char **argv)
{
    KAboutData aboutData( "klaptop_check", I18N_NOOP("KLaptop"),
		            version, description, KAboutData::License_GPL,
			            "(c) 2003, Paul Campbell");
    aboutData.addAuthor("Paul Campbell",0, "paul@taniwha.com");
    KCmdLineArgs::init( argc, argv, &aboutData );

    KApplication a;
    daemon_state s;
    s.load();
    if (s.need_to_run()) 
	wake_laptop_daemon();
    return(0);
}
