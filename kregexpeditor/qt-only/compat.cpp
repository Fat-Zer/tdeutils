/*
 *  Copyright (c) 2002-2004 Jesper K. Pedersen <blackie@kde.org>
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

#include "compat.h"

TQString i18n( const TQString& a) {
    return TQObject::tr(a);
}

TQString i18n( const TQString& a, const TQString& b) {
    return TQObject::tr(b,a);
}

KDialogBase::KDialogBase( int /*dialogFace*/, const TQString &caption, int buttonMask,
                          ButtonCode defaultButton, TQWidget *parent, const char *name,
                          bool modal )
    :TQDialog( parent, name, modal )
{
    init( buttonMask, defaultButton, caption );
}

KDialogBase::KDialogBase( TQWidget* parent, const char* name, bool modal,
                          const TQString& caption, int buttonMask )
        : TQDialog( parent, name, modal )
{
    init( buttonMask, Ok, caption );
}

void KDialogBase::init( int buttonMask, ButtonCode /*defaultButton*/, const TQString& caption )
{
    setCaption( caption );
    _layout = new TQVBoxLayout( this, 6 );
    TQHBoxLayout* buts = new TQHBoxLayout( _layout, 6 );
    TQPushButton* but;
    if ( buttonMask & Help ) {
        but = new TQPushButton( tr("Help"), this );
        buts->addWidget( but );
        connect( but, TQT_SIGNAL( clicked() ), this, TQT_SIGNAL( helpClicked() ) );
    }
    buts->addStretch(1);
    if ( buttonMask & Ok ) {
        but = new TQPushButton( tr("OK"), this );
        buts->addWidget( but );
        connect( but, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotOk() ) );
    }
    if ( buttonMask & Cancel ) {
        but = new TQPushButton( tr("Cancel"), this );
        buts->addWidget( but );
        connect( but, TQT_SIGNAL( clicked() ), this, TQT_SLOT( slotCancel() ) );
    }
}

void KDialogBase::setMainWidget( TQWidget* top )
{
    top->reparent( this, 0, TQPoint(0,0) );
    _layout->insertWidget( 0, top );
}

TQFrame* KDialogBase::plainPage()
{
    TQFrame* frame = new TQFrame( this );
    setMainWidget( frame );
    return frame;
}

void KDialogBase::slotOk()
{
    accept();
    emit okClicked();
    emit finished();
}

void KDialogBase::slotCancel()
{
    reject();
    emit cancelClicked();
    emit finished();
}

int KMessageBox::warningYesNo(TQWidget *parent, const TQString &text, const TQString &caption )
{
    int code = warning( parent, caption, text, tr("No"), tr("Yes") );
    if ( code == 0 )
        return Yes;
    else
        return No;
}

int KMessageBox::information( TQWidget* parent, const TQString& text, const TQString& caption,
                              const TQString& /*dontShowAgainName*/ )
{
    return TQMessageBox::information( parent, caption, text );
}

int KMessageBox::sorry( TQWidget* parent, const TQString& text, const TQString& caption )
{
    return TQMessageBox::information( parent, caption, text );
}


