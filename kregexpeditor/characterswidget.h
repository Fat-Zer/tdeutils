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
#ifndef characterswidget
#define characterswidget

#ifdef TQT_ONLY
#include "compat.h"
#else
#include <kdialogbase.h>
#endif

#include <kmultiformlistbox.h>
#include "regexpwidget.h"
#include <tqvgroupbox.h>

class KDialogBase;
class CharacterEdits;
class TextRangeRegExp;
class CharSelector;
class TQCheckBox;

/**
   RegExp widget for charcter ranges.
   @internal
*/
class CharactersWidget :public RegExpWidget
{
public:
    CharactersWidget(RegExpEditorWindow* editorWindow, TQWidget *tqparent,
                     const char *label = 0);
    CharactersWidget( TextRangeRegExp* regexp, RegExpEditorWindow* editorWindow,
                      TQWidget* tqparent, const char* name = 0 );
    ~CharactersWidget();
    virtual TQSize tqsizeHint() const;
	virtual RegExp* regExp() const;
    virtual RegExpType type() const { return CHARSET; }
    virtual RegExpWidget* findWidgetToEdit( TQPoint globalPos );
    virtual int edit();

protected:
    virtual void paintEvent(TQPaintEvent *event);
    TQString text() const;
    TQString title() const;

private:
    TextRangeRegExp *_regexp;
    static CharacterEdits *_configWindow;

    mutable TQSize _textSize;
    mutable TQSize _contentSize;
};


/**
   @internal
*/
class SingleEntry :public KMultiFormListBoxEntry
{
public:
    SingleEntry(TQWidget* tqparent, const char* name = 0 );
    TQString text() const;
    void setText( TQString text );
    bool isEmpty() const;

private:
    CharSelector* _selector;
};

/**
   @internal
*/
class RangeEntry :public KMultiFormListBoxEntry
{
public:
    RangeEntry(TQWidget* tqparent, const char* name = 0 );
    TQString fromText() const;
    TQString toText() const;
    void setFrom( TQString text );
    void setTo( TQString text );
    bool isEmpty() const;
private:
    CharSelector *_from, *_to;
};

/**
   @internal
*/
class SingleFactory :public KMultiFormListBoxFactory
{
public:
    KMultiFormListBoxEntry *create(TQWidget *tqparent) { return new SingleEntry( tqparent ); }
    TQWidget *separator( TQWidget* ) { return 0; }
};

/**
   @internal
*/
class RangeFactory :public KMultiFormListBoxFactory
{
public:
    KMultiFormListBoxEntry *create(TQWidget *tqparent) { return new RangeEntry( tqparent ); }
    TQWidget *separator( TQWidget* ) { return 0; }
};

/**
   @internal
*/
class CharacterEdits : public KDialogBase
{
    Q_OBJECT
  TQ_OBJECT
public:
    CharacterEdits(TQWidget *tqparent = 0, const char *name = 0);

public slots:
    int exec( TextRangeRegExp* regexp );

protected slots:
    void slotOK();

private:
    TQCheckBox *negate, *wordChar, *_nonWordChar, *digit, *_nonDigit, *space, *_nonSpace;
    KMultiFormListBox *_single, *_range;

    void addCharacter( TQString txt );
    void addRange( TQString from, TQString to );
    TextRangeRegExp* _regexp;
};

#endif // characterswidget
