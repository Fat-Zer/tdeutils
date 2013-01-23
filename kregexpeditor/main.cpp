/*
 *  Copyright (c) 2002-2003 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifdef TQT_ONLY
  #include "compat.h"
  #include <tqapplication.h>
#else
  #include <kaboutdata.h>
  #include <kapplication.h>
  #include <kcmdlineargs.h>
  #include <klocale.h>
  #include <kpushbutton.h>
#endif

#include "kregexpeditorgui.h"
#include <tqlayout.h>

int main( int argc, char* argv[] )
{
#ifdef TQT_ONLY
    TQApplication myapp( argc, argv );
#else
    TDEAboutData aboutData( "kregexpeditor", I18N_NOOP("RegExp Editor"),
                          "1.0", I18N_NOOP("Editor for Regular Expressions"),
			  TDEAboutData::License_GPL,
                          "(c) 2002-2003 Jesper K. Pedersen");
    TDECmdLineArgs::init(argc, argv, &aboutData);
    TDEApplication myapp;
#endif

    TQDialog* top = new TQDialog( 0 );
    TQVBoxLayout* lay = new TQVBoxLayout( top, 6 );

    KRegExpEditorGUI* iface = new KRegExpEditorGUI( top, "_editor", TQStringList() );
    iface->doSomething( TQString::fromLatin1("setAllowNonTQtSyntax"), (bool*) true );
    lay->addWidget( iface );

    TQHBoxLayout* lay2 = new TQHBoxLayout( lay, 6 );
    KPushButton* help = new KPushButton( KStdGuiItem::help(), top );
    KPushButton* quit = new KPushButton( KStdGuiItem::quit(), top );

    lay2->addWidget( help );
    lay2->addStretch(1);
    lay2->addWidget( quit );

    TQObject::connect( help, TQT_SIGNAL( clicked() ), iface, TQT_SLOT( showHelp() ) );
    TQObject::connect( quit, TQT_SIGNAL( clicked() ), tqApp, TQT_SLOT( quit() ) );

    top->show();
    TQObject::connect( tqApp, TQT_SIGNAL( lastWindowClosed() ), tqApp, TQT_SLOT( quit() ) );
    myapp.exec();
}

