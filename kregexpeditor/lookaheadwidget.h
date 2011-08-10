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
#ifndef __lookaheadwidget
#define __lookaheadwidget

#include "singlecontainerwidget.h"
class LookAheadRegExp;

class LookAheadWidget :public SingleContainerWidget
{
    Q_OBJECT
  TQ_OBJECT
public:
    LookAheadWidget( RegExpEditorWindow* editorWindow, RegExpType tp, TQWidget* parent, const char* name = 0 );
    LookAheadWidget( LookAheadRegExp* regexp, RegExpEditorWindow* editorWindow, RegExpType tp,
                     TQWidget* parent, const char* name = 0);

 	virtual RegExp* regExp() const;
 	virtual RegExpType type() const { return _tp; }
    virtual TQSize tqsizeHint() const;
    virtual RegExpWidget* findWidgetToEdit( TQPoint globalPos );


protected:
    void init();
    virtual void paintEvent( TQPaintEvent *e );


private:
    RegExpType _tp;
    TQString _text;

    mutable TQSize _textSize;
    mutable TQSize _childSize;
};

#endif // __lookaheadwidget
