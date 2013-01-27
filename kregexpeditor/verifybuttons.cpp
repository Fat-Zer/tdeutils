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
#include <tqfiledialog.h>
#include "images.h"
#else
#include <klocale.h>
#include "verifybuttons.moc"
#include <kstandarddirs.h>
#include <tdefiledialog.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#endif

#include "verifybuttons.h"
#include <tqtooltip.h>
#include <tqlayout.h>
#include <tqwhatsthis.h>
#include "qtregexpconverter.h"
#include "emacsregexpconverter.h"
#include <tqtoolbutton.h>
#include "util.h"
#include <tqpopupmenu.h>
#include <tqaction.h>

VerifyButtons::VerifyButtons( TQWidget* parent, const char* name )
    :TQDockWindow( TQDockWindow::InDock, parent, name ), _configMenu( 0 )
{
    TQBoxLayout* layout = boxLayout();

    _verify =  new TQToolButton(this);
    TQIconSet icon = Util::getSystemIconSet( TQString::fromLatin1("spellcheck"));
    _verify->setIconSet( icon );
    TQToolTip::add( _verify, i18n( "Verify regular expression" ) );
    TQWhatsThis::add( _verify, i18n("Shows what part of the regular expression is being matched in the <i>verifier window</i>."
                                   "(The window below the graphical editor window)."));
    layout->addWidget( _verify );
    connect( _verify, TQT_SIGNAL( clicked() ), this, TQT_SIGNAL( verify() ) );

    TQToolButton* button = new TQToolButton(this);
    button->setPixmap( Util::getSystemIcon( TQString::fromLatin1("fileopen")) );
    layout->addWidget( button );
    connect(button, TQT_SIGNAL(clicked()), this, TQT_SLOT(loadText()));
    TQToolTip::add( button, i18n("Load text in the verifier window") );

    button = new TQToolButton(this);
    button->setPixmap( Util::getSystemIcon( TQString::fromLatin1("package_settings")) );
    layout->addWidget( button );
    connect(button, TQT_SIGNAL(clicked()), this, TQT_SLOT(configure()));
    TQToolTip::add( button, i18n("Settings") );

    // It is currently not possible to ask for the paragraph being highlighted, thefore scrolling to next/prev match
    // do not work. Enable this when they work.
    // _first = new TQToolButton( TQString::fromLatin1("<<"), this);
    // layout->addWidget( _first );
    // connect(_first, TQT_SIGNAL(clicked()), this, TQT_SIGNAL( gotoFirst()));
    // _first->setFixedWidth( 25 );
    //
    // _prev = new TQToolButton(TQString::fromLatin1("<"), this);
    // layout->addWidget( _prev );
    // connect(_prev, TQT_SIGNAL(clicked()), this, TQT_SIGNAL( gotoPrev()));
    // _prev->setFixedWidth( 20 );
    //
    // _next = new TQToolButton(TQString::fromLatin1(">"), this);
    // layout->addWidget( _next );
    // connect(_next, TQT_SIGNAL(clicked()), this, TQT_SIGNAL( gotoNext()));
    // _next->setFixedWidth( 20 );
    //
    // _last = new TQToolButton(TQString::fromLatin1(">>"), this);
    // layout->addWidget( _last );
    // connect(_last, TQT_SIGNAL(clicked()), this, TQT_SIGNAL( gotoLast()));
    // _last->setFixedWidth( 25 );

    // Same as above
//  TQLabel* label = new TQLabel( i18n("Matches: "), this );
//  layout->addWidget( label );
//  _matches = new TQLabel(i18n("-"), this );
//  layout->addWidget( _matches );
//  TQString txt = i18n( "Shows number of times regular expression matches the text in the verifier window");
//  TQToolTip::add( label, txt );
//  TQToolTip::add( _matches, txt );

    _verify->setEnabled( false );

    // -------------------------------------------------- RegExp Converters

    // TQt
    RegExpConverter* converter = new TQtRegExpConverter();
    _converters.append( tqMakePair( converter, static_cast<TQAction*>( 0 ) ) );
    TQString qtConverterName = converter->name();

    // Emacs
    converter = new EmacsRegExpConverter();
    _converters.append( tqMakePair( converter, static_cast<TQAction*>( 0 )  ) );


    // -------------------------------------------------- Initialize the config menu
    _configMenu = new TQPopupMenu( this, "config menu" );

    // Auto Verify
    TQAction* autoVerify = new TQAction( i18n("Verify on the Fly"), 0, this );
    autoVerify->setToggleAction( true );
    autoVerify->setOn( true );
    connect( autoVerify, TQT_SIGNAL( toggled( bool ) ), this, TQT_SLOT( updateVerifyButton( bool ) ) );
    connect( autoVerify, TQT_SIGNAL( toggled( bool ) ), this, TQT_SIGNAL( autoVerify( bool ) ) );
    autoVerify->addTo( _configMenu );
    autoVerify->setToolTip( i18n( "Toggle on-the-fly verification of regular expression" ) );
    autoVerify->setWhatsThis( i18n( "Enabling this option will make the verifier update for each edit. "
                                    "If the verify window contains much text, or if the regular expression is either "
                                    "complex or matches a lot of time, this may be very slow."));

    // RegExp Languages
    TQPopupMenu* languages = new TQPopupMenu( _configMenu );
    _languageId = _configMenu->insertItem( i18n("RegExp Language"), languages );

    TQActionGroup* grp = new TQActionGroup( this );
    for( TQValueList< TQPair<RegExpConverter*,TQAction*> >::Iterator it = _converters.begin(); it != _converters.end(); ++it ) {
        TQString name = (*it).first->name();
        TQAction* action = new TQAction( name, 0, this );
        action->setToggleAction( true );
        grp->add( action );
        (*it).second = action;
    }
    grp->addTo( languages );
    connect( grp, TQT_SIGNAL( selected( TQAction* ) ), this, TQT_SLOT( slotChangeSyntax( TQAction* ) ) );
    _configMenu->setItemEnabled( _languageId, false );

    // Select the TQt converter by default
    setSyntax( qtConverterName );
}



void VerifyButtons::updateVerifyButton( bool b )
{
    _verify->setEnabled( !b );
}

void VerifyButtons::loadText()
{
    TQString fileName = KFileDialog::getOpenFileName(TQString(), TQString(), this);
    if ( !fileName.isNull() ) {
        emit loadVerifyText( fileName );
    }
}

// TQt anchors do not work for <pre>...</pre>, thefore scrolling to next/prev match
// do not work. Enable this when they work.
// void VerifyButtons::enableBackwardButtons( bool b )
// {
//     _first->setEnabled( b );
//     _prev->setEnabled( b );
// }
//
// void VerifyButtons::enableForwardButtons( bool b )
// {
//     _next->setEnabled( b );
//     _last->setEnabled( b );
// }

void VerifyButtons::setMatchCount( int /*count*/ )
{
// currently this is not possible due to limitation in TQSyntaxHighlighter
/*
  if ( count == -1 )
  _matches->setText( TQString::fromLatin1("-") );
  else
  _matches->setText( TQString::number( count ) );
*/
}

void VerifyButtons::slotChangeSyntax( TQAction* action )
{
    emit changeSyntax( action->menuText()  );
}

RegExpConverter* VerifyButtons::setSyntax( const TQString& which)
{
    for( TQValueList< TQPair<RegExpConverter*, TQAction*> >::Iterator it = _converters.begin(); it != _converters.end(); ++it ) {
        TQString name = (*it).first->name();
        if ( name == which ) {
            (*it).second->setOn( true );
            return (*it).first;
        }
    }
    tqWarning( "No such converter: '%s'", which.latin1() );
    return 0;
}

void VerifyButtons::configure()
{
    _configMenu->exec( TQCursor::pos() );
}

void VerifyButtons::setAllowNonTQtSyntax( bool b )
{
    _configMenu->setItemEnabled( _languageId, b );
}
