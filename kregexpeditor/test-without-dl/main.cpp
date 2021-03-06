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
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tqfile.h>
#include "../kregexpeditorgui.h"
class ShootABug :public TQObject
{
public:
  virtual bool eventFilter( TQObject* recv, TQEvent* event )
  {
    if ( event->type() == TQEvent::MouseButtonPress &&
         dynamic_cast<TQMouseEvent*>(event)->state() == TQt::ControlButton ) {
      // Ctrl + left mouse click.

      tqDebug("----------------------------------------------------");
      tqDebug((TQString("Widget name : ") + TQString( recv->name() )).latin1() );
      tqDebug((TQString("Widget class: ") + TQString( recv->className() )).latin1() );
      tqDebug("\nObject info:");
      recv->dumpObjectInfo();
      tqDebug("\nObject tree:");
      recv->dumpObjectTree();
      tqDebug("----------------------------------------------------");
      return false;
    }
    return false;
  }
};

int main( int argc, char* argv[] )
{
  TDECmdLineArgs::init(argc, argv, "RegExp Example","","");
  TDEApplication myapp( argc, argv );

  tqApp->installEventFilter( new ShootABug() );

  KRegExpEditorGUIDialog* iface = new KRegExpEditorGUIDialog( 0, "_editor", TQStringList() );
  iface->setRegExp( TQString::fromLatin1( "#include" ) );
  iface->doSomething( "setMinimal", (void*) false );
  iface->doSomething( "setSyntax", (void*) new TQString( TQString::fromLatin1( "Emacs" ) ) );
  iface->doSomething( "setShowSyntaxCombo", (bool*) true );

  TQFile file("/packages/kde-src/tdeutils/kregexpeditor/test/main.cpp");
  file.open(IO_ReadOnly);
  TQTextStream stream( &file);
  TQString txt = stream.read();
  iface->setMatchText( txt );

  iface->exec();
}

