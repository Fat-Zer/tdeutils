// -*- C++ -*-

//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld
//  email:  Christoph.Neerfeld@home.ivm.de or chris@kde.org
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <stdlib.h>
#include <kuniqueapplication.h>
#include <kcmdlineargs.h>
#include <kconfig.h>
#include <kaboutdata.h>

#include "KJotsMain.h"


static const char description[] = I18N_NOOP("KDE note taking utility");

static const char version[] = "v0.7";


int main( int argc, char **argv )
{
  KAboutData aboutData( "kjots", I18N_NOOP("KJots"),
    version, description, KAboutData::License_GPL,
    "(c) 1997-2002, Christoph Neerfeld");
  aboutData.addAuthor("Pradeepto K. Bhattacharya", I18N_NOOP("Current maintainer"), "pradeepto@kde.org");
  aboutData.addAuthor("Jaison Lee", 0, "lee.jaison@gmail.com");
  aboutData.addAuthor("Aaron J. Seigo", 0, "aseigo@kde.org");
  aboutData.addAuthor("Stanislav Kljuhhin", 0, "crz@starman.ee");
  aboutData.addAuthor("Christoph Neerfeld", I18N_NOOP("Original author"), "chris@kde.org");
  KCmdLineArgs::init(argc, argv, &aboutData);
  KUniqueApplication::addCmdLineOptions();

  if (!KUniqueApplication::start())
  {
     fprintf(stderr, "kjots is already running!\n");
     exit(0);
  }

  KUniqueApplication a;

  // backwards compatibility code to convert "old" user font settings
  // to the new config settings
  KConfig *config = a.config();
  config->setGroup("kjots");
  if (config->hasKey("EFontFamily")) {
    // read old font and create it
    QFont font( config->readEntry("EFontFamily"),
                config->readNumEntry("EFontSize", 12),
                config->readNumEntry("EFontWeight", 0),
                config->readNumEntry("EFontItalic", 0));
    // delete old entries
    config->deleteEntry("EFontFamily");
    config->deleteEntry("EFontSize");
    config->deleteEntry("EFontWeight");
    config->deleteEntry("EFontItalic");
    config->deleteEntry("EFontCharset");
    // write new "converted" entry
    config->writeEntry("Font", font);
  }

  KJotsMain *jots = new KJotsMain;
  if( a.isRestored() )
  {
    if( KJotsMain::canBeRestored(1) )
    {
      jots->restore(1);
    }
  }

  a.setMainWidget( (QWidget *) jots );
  jots->show();
  jots->resize(jots->size());
  return a.exec();
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
