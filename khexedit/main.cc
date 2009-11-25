/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999-2000 Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


#include <stdlib.h>

#include <kapplication.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>

#include "toplevel.h"
#include "version.h" // Contains khexedit name string and version string


static const char description[] =
	I18N_NOOP("KDE hex editor");

static const char version[] = APP_VERSION_STRING;

static KCmdLineOptions option[] =
{
   { "offset <offset>", I18N_NOOP("Jump to 'offset'"), 0 },
   { "+[file(s)]", I18N_NOOP("File(s) to open"), 0 },
   KCmdLineLastOption
};

static uint parseDecimalOrHexadecimal( char *buf );

int main( int argc, char **argv )
{
  KAboutData aboutData("khexedit", I18N_NOOP("KHexEdit"),
    version, description, KAboutData::License_GPL_V2,
    "(c) 1999-2000, Espen Sand");
  aboutData.addAuthor("Espen Sand",0, "espensa@online.no",
		      "http://home.online.no/~espensa/khexedit/" );
  aboutData.addCredit("",I18N_NOOP("\n"
    "This program uses modified code and techniques from other KDE programs,\n"
    "specifically kwrite, kiconedit and ksysv. Credit goes to authors\n"
    "and maintainers.\n"
    "\n"
    "Leon Lessing, leon@lrlabs.com, has made parts of the bit swapping\n"
    "functionality.\n"
    "\n"
    "Craig Graham, c_graham@hinge.mistral.co.uk, has made parts of\n"
    "the bit stream functionality of the conversion field.\n"
    "\n"
    "Dima Rogozin, dima@mercury.co.il, has extended the string dialog\n"
    "list capabilities.\n"
    "\n"
    "Edward Livingston-Blade, sbcs@bigfoot.com, has given me very good\n"
    "reports which removed some nasty bugs.\n"));
  KCmdLineArgs::init( argc, argv, &aboutData );
  KCmdLineArgs::addCmdLineOptions( option );

  KApplication app;

  if( app.isRestored() != 0 )
  {
    RESTORE( KHexEdit );
  }
  else
  {
    KHexEdit *hexEdit = new KHexEdit;
    if( hexEdit == 0 )
    {
      std::cerr << "Unable to start - Memory exhausted" << std::endl;
      return( 1 );
    }

    hexEdit->show();

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    if (args->isSet("offset"))
    {
        QCString offsetStr = args->getOption("offset");
        uint _offset = parseDecimalOrHexadecimal(offsetStr.data() );
        hexEdit->setStartupOffset( _offset );
    }

    for(int i = 0; i < args->count(); i++)
      hexEdit->addStartupFile( args->url(i).url() );

    args->clear();
  }

  int result = app.exec();
  return( result );
}



static uint parseDecimalOrHexadecimal( char *buf )
{
  if( buf == 0 ) { return( 0 ); }
  
  long int value;
  char *end = 0;
	    
  value = strtol( buf, &end, 10 );
  if( *end != 0 )
  {
    value = strtol( buf, &end, 16 );
    if( *end != 0 )
    {
      value = 0;
    }
  }
  if( value < 0 ) { value = 0; }
  return( value );
}

