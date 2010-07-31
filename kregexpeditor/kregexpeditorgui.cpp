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

#ifdef QT_ONLY
  #include "compat.h"
#else
  #include <klocale.h>
  #include <kgenericfactory.h>
  #include <kapplication.h>
  #include "kregexpeditorgui.moc"
#endif

#include "kregexpeditorgui.h"
// #include <unistd.h> // DO I need this?
#include <stdio.h>
#include "kregexpeditorprivate.h"
#include <tqlayout.h>

const TQString KRegExpEditorGUI::version = TQString::fromLocal8Bit("1.0");


KRegExpEditorGUI::KRegExpEditorGUI(TQWidget *parent, const char *name,
	                           const TQStringList & )
  : TQWidget( parent, name)
{
  TQHBoxLayout* layout = new TQHBoxLayout( this, 6 );
  _editor = new KRegExpEditorPrivate( this, "_editor" );
  layout->addWidget( _editor );
  connect( _editor, TQT_SIGNAL( canUndo(bool) ), this, TQT_SIGNAL( canUndo(bool) ) );
  connect( _editor, TQT_SIGNAL( canRedo(bool) ), this, TQT_SIGNAL( canRedo(bool) ) );
  connect( _editor, TQT_SIGNAL( changes(bool) ), this, TQT_SIGNAL( changes(bool) ) );
}

TQString KRegExpEditorGUI::regExp() const
{
  return _editor->regexp();
}

void KRegExpEditorGUI::redo()
{
  _editor->slotRedo();
}

void KRegExpEditorGUI::undo()
{
  _editor->slotUndo();
}

void KRegExpEditorGUI::setRegExp( const TQString &regexp )
{
  _editor->slotSetRegexp( regexp );
}

KRegExpEditorGUIDialog::KRegExpEditorGUIDialog( TQWidget *parent,
                                                const char *name,
                                                const TQStringList & )
  : KDialogBase( KDialogBase::Plain, i18n("Regular Expression Editor"),
                 KDialogBase::Ok | KDialogBase::Cancel | KDialogBase::Help, KDialogBase::Ok,
                 parent, name ? name : "KRegExpDialog" )
{
  TQFrame* frame = plainPage();
  TQVBoxLayout* layout = new TQVBoxLayout( frame, 6 );
  layout->setAutoAdd( true );
  _editor = new KRegExpEditorGUI( frame );

  connect( _editor, TQT_SIGNAL( canUndo(bool) ), this, TQT_SIGNAL( canUndo(bool) ) );
  connect( _editor, TQT_SIGNAL( canRedo(bool) ), this, TQT_SIGNAL( canRedo(bool) ) );
  connect( _editor, TQT_SIGNAL( changes(bool) ), this, TQT_SIGNAL( changes(bool) ) );
  resize( 640, 400 );

  setHelp( TQString::null, TQString::fromLocal8Bit( "KRegExpEditor" ) );
#ifdef QT_ONLY
  connect( this, TQT_SIGNAL( helpClicked() ), _editor, TQT_SLOT( showHelp() ) );
#endif
}


TQString KRegExpEditorGUIDialog::regExp() const
{
    return _editor->regExp();
}

void KRegExpEditorGUIDialog::setRegExp( const TQString &regexp )
{
    _editor->setRegExp( regexp );
}

void KRegExpEditorGUIDialog::redo()
{
  _editor->redo();
}

void KRegExpEditorGUIDialog::undo()
{
  _editor->undo();
}

void KRegExpEditorGUIDialog::doSomething( TQString method, void* arguments )
{
    _editor->doSomething( method, arguments );
}

void KRegExpEditorGUI::doSomething( TQString method, void* arguments )
{
    if ( method == TQString::fromLatin1( "setCaseSensitive" ) ) {
        _editor->setCaseSensitive( (bool) arguments );
    }
    else if ( method == TQString::fromLatin1("setMinimal") ) {
        _editor->setMinimal( (bool) arguments );
    }
    else if ( method == TQString::fromLatin1("setSyntax") ) {
        _editor->setSyntax( *((TQString*) arguments) );
    }
    else if ( method == TQString::fromLatin1("setAllowNonQtSyntax") ) {
        _editor->setAllowNonQtSyntax( (bool) arguments );
    }
    else {
        qFatal( "%s", tr("Method '%1' is not valid!").arg(method).latin1() );
    }
}

void KRegExpEditorGUIDialog::setMatchText( const TQString& txt )
{
    _editor->setMatchText( txt );
}

void KRegExpEditorGUI::setMatchText( const TQString& txt )
{
    _editor->setMatchText( txt );
}


void KRegExpEditorGUI::showHelp()
{
#ifdef QT_ONLY
    _editor->showHelp();
#else
    kapp->invokeHelp( TQString::null, TQString::fromLocal8Bit( "KRegExpEditor" ) );
#endif
}

#ifndef QT_ONLY
typedef K_TYPELIST_2( KRegExpEditorGUI, KRegExpEditorGUIDialog ) Products;
K_EXPORT_COMPONENT_FACTORY( libkregexpeditorgui,
                            KGenericFactory<Products>( "kregexpeditor" ) )
#endif
