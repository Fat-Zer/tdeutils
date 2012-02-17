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
#ifndef VERIFYBUTTONS_H
#define VERIFYBUTTONS_H

#include <tqdockwindow.h>
#include "regexpconverter.h"
class TQToolButton;
class TQLabel;
class TQAction;
class TQPopupMenu;

class VerifyButtons :public TQDockWindow
{
    Q_OBJECT
  

public:
    VerifyButtons( TQWidget* parent, const char* name );
    RegExpConverter* setSyntax( const TQString& );
    void setAllowNonTQtSyntax( bool );

signals:
    void verify();
    void autoVerify( bool );
    void loadVerifyText( const TQString& );

    // TQt anchors do not work for <pre>...</pre>, thefore scrolling to next/prev match
    // do not work. Enable this when they work.
    // void gotoFirst();
    // void gotoPrev();
    // void gotoNext();
    // void gotoLast();

    void changeSyntax( const TQString& );

public slots:
    //     void enableForwardButtons( bool );
    //     void enableBackwardButtons( bool );
    void setMatchCount( int );

protected slots:
    void updateVerifyButton( bool );
    void loadText();
    void slotChangeSyntax( TQAction* action );
    void configure();

private:
    TQToolButton* _verify;
    TQLabel* _matches;
    TQPopupMenu* _configMenu;
    int _languageId;

    // TQt anchors do not work for <pre>...</pre>, thefore scrolling to next/prev match
    // do not work. Enable this when they work.
    // TQToolButton* _first;
    // TQToolButton* _prev;
    // TQToolButton* _next;
    // TQToolButton* _last;

    TQValueList< TQPair<RegExpConverter*,TQAction*> > _converters;
};


#endif // VERIFYBUTTONS_H
