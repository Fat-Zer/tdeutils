/***************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : Mon Jul 8 2002
    copyright          : (C) 2002 by Jean-Baptiste Mardelle
    email                : bj@altern.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <tqstring.h>
#include <tdemessagebox.h>


#include <kuniqueapplication.h>
#include <dcopclient.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <tdelocale.h>
#include "kgpg.h"

static const char description[] =
        I18N_NOOP("Kgpg - simple gui for gpg\n\nKgpg was designed to make gpg very easy to use.\nI tried to make it as secure as possible.\nHope you enjoy it.");

static const char version[] = "1.2.2";

static TDECmdLineOptions options[] =
        {
                { "e", I18N_NOOP("Encrypt file"), 0 },
                { "k", I18N_NOOP("Open key manager"), 0 },
                { "s", I18N_NOOP("Show encrypted file"), 0 },
                { "S", I18N_NOOP("Sign file"), 0 },
                { "V", I18N_NOOP("Verify signature"), 0 },
                { "X", I18N_NOOP("Shred file"), 0 },
                { "+[File]", I18N_NOOP("File to open"), 0 },
                TDECmdLineLastOption
                // INSERT YOUR COMMANDLINE OPTIONS HERE
        };

int main(int argc, char *argv[])
{

        TDEAboutData about("kgpg", I18N_NOOP("KGpg"), version, description,
                         TDEAboutData::License_GPL, "(C) 2003 Jean-Baptiste Mardelle");
        about.addAuthor( "Jean-Baptiste Mardelle", 0, "bj@altern.org" );
        TDECmdLineArgs::init(argc, argv, &about);
        TDECmdLineArgs::addCmdLineOptions(options);
        KUniqueApplication::addCmdLineOptions();


        //KMessageBox::sorry(0,"nombre: "+TQString::number(i));

        if (!KUniqueApplication::start())
                return 0;

        //KgpgAppletApp *app;
        TDEApplication *app;
        app=new KgpgAppletApp;
        return app->exec();

        /*

        KUniqueApplication app;

           // register ourselves as a dcop client
         //app.dcopClient()->registerAs(app.name(), false);


         kgpgapplet widget;// = new kgpgapplet("kgpg");
         widget.show();
         return app.exec();*/
}
