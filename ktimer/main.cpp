/*
 * (c) 2001 Stefan Schimanski <schimmi@kde.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>

#include "ktimer.h"

static const char description[] =
        I18N_NOOP("TDE Timer");

static const char version[] = "v0.1";

int main( int argc, char **argv )
{
    TDEAboutData aboutData( "ktimer", I18N_NOOP("KTimer"),
                          version, description, TDEAboutData::License_GPL,
                          "(c) 2001, Stefan Schimanski");
    aboutData.addAuthor("Stefan Schimanski",0, "schimmi@kde.org");
    TDECmdLineArgs::init( argc, argv, &aboutData );

    TDEApplication app;

    KTimerPref *timer = new KTimerPref;
    app.setMainWidget( timer );
    app.setTopWidget( timer );

    return app.exec();
}
