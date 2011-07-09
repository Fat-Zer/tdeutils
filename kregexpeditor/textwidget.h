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
#ifndef __textwidget
#define __textwidget

#include "regexpwidget.h"
class SelectableLineEdit;
class TextRegExp;

/**
   RegExp widget representing text.

   @internal
*/
class TextWidget : public RegExpWidget
{
    Q_OBJECT
  TQ_OBJECT

public:
    TextWidget(RegExpEditorWindow* editorWindow, TQWidget *tqparent,
               const char *name = 0);
    TextWidget( TextRegExp* regexp,  RegExpEditorWindow* editorWindow,
                TQWidget* tqparent, const char* name = 0);
    virtual TQSize tqsizeHint() const;
	virtual RegExp* regExp() const;
    virtual RegExpType type() const { return TEXT; }
    virtual void updateAll();
    virtual void selectWidget( bool );

protected:
    void init( const TQString& text );
    virtual void paintEvent( TQPaintEvent *e );
    virtual bool updateSelection( bool parentSelected );
    virtual void clearSelection();
    virtual bool eventFilter( TQObject*, TQEvent* );

protected slots:
    void slotUpdate();

private:
    TQString text;
    SelectableLineEdit *_edit;
    TQSize textSize, boxSize, editSize;
};



#endif // __textwidget
