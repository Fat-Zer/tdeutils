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
  #include "images.h"
#else
  #include "auxbuttons.moc"
  #include <kiconloader.h>
  #include <klocale.h>
#endif

#include "auxbuttons.h"
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include <tqtooltip.h>
#include <tqtoolbutton.h>
#include "util.h"

AuxButtons::AuxButtons( TQWidget* tqparent, const char* name = 0)
  :TQDockWindow( TQDockWindow::InDock, tqparent, name)
{
  TQBoxLayout* tqlayout = boxLayout();

  _undo = new TQToolButton( this );
  _undo->setIconSet( Util::getSystemIconSet(TQString::tqfromLatin1("undo") ) );
  tqlayout->addWidget( _undo );
  connect( _undo, TQT_SIGNAL(clicked()), this, TQT_SIGNAL(undo()) );
  TQToolTip::add( _undo, i18n( "Undo" ) );

  _redo = new TQToolButton( this );
  _redo->setIconSet( Util::getSystemIconSet(TQString::tqfromLatin1("redo") ) );
  tqlayout->addWidget( _redo );
  connect( _redo, TQT_SIGNAL(clicked()), this, TQT_SIGNAL(redo()) );
  TQToolTip::add( _redo, i18n( "Redo" ) );

  _cut = new TQToolButton( this );
  _cut->setIconSet( Util::getSystemIconSet(TQString::tqfromLatin1("editcut") ) );
  tqlayout->addWidget( _cut );
  connect( _cut, TQT_SIGNAL(clicked()), this, TQT_SIGNAL(cut()) );
  TQToolTip::add( _cut, i18n( "Cut" ) );

  _copy = new TQToolButton( this );
  _copy->setIconSet( Util::getSystemIconSet(TQString::tqfromLatin1("editcopy") ) );
  tqlayout->addWidget( _copy );
  connect( _copy, TQT_SIGNAL(clicked()), this, TQT_SIGNAL(copy()) );
  TQToolTip::add( _copy, i18n( "Copy" ) );

  _paste = new TQToolButton( this );
  _paste->setIconSet( Util::getSystemIconSet(TQString::tqfromLatin1("editpaste")) );
  tqlayout->addWidget( _paste );
  connect( _paste, TQT_SIGNAL(clicked()), this, TQT_SIGNAL(paste()) );
  TQToolTip::add( _paste, i18n( "Paste" ) );

  _save = new TQToolButton( this );
  _save->setIconSet( Util::getSystemIconSet(TQString::tqfromLatin1("filesave")) );
  tqlayout->addWidget( _save );
  connect( _save, TQT_SIGNAL(clicked()), this, TQT_SIGNAL(save()) );
  TQToolTip::add( _save, i18n( "Save" ) );


  TQToolButton* button = new TQToolButton(this);
  button->setPixmap( Util::getSystemIcon( TQString::tqfromLatin1("contexthelp") ) );
  tqlayout->addWidget( button );
  connect(button, TQT_SIGNAL(clicked()), this, TQT_SLOT(slotEnterWhatsThis()));

  _undo->setEnabled( false );
  _redo->setEnabled( false );

}

void AuxButtons::slotEnterWhatsThis()
{
  TQWhatsThis::enterWhatsThisMode ();
}

void AuxButtons::slotCanUndo( bool b )
{
  _undo->setEnabled( b );
}

void AuxButtons::slotCanRedo( bool b )
{
  _redo->setEnabled( b );
}

void AuxButtons::slotCanCut( bool b )
{
  _cut->setEnabled( b );
}

void AuxButtons::slotCanCopy( bool b )
{
  _copy->setEnabled( b );
}

void AuxButtons::slotCanPaste( bool b )
{
  _paste->setEnabled( b );
}

void AuxButtons::slotCanSave( bool b )
{
  _save->setEnabled( b );
}

