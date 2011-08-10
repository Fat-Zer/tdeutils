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
#else
  #include <kdialogbase.h>
  #include "characterswidget.moc"
#endif

#include "characterswidget.h"
#include "textrangeregexp.h"
#include "regexp.h"
#include "charselector.h"
#include "myfontmetrics.h"
#include "regexpconverter.h"

#include <tqpainter.h>
#include <tqgrid.h>
#include <iostream>
#include <tqhgroupbox.h>
#include <tqcursor.h>
#include <tqcheckbox.h>

CharacterEdits* CharactersWidget::_configWindow = 0;

CharactersWidget::CharactersWidget(RegExpEditorWindow* editorWindow, TQWidget *parent,
                                   const char *name)
    : RegExpWidget(editorWindow, parent, name)
{
    _regexp = new TextRangeRegExp( false /* not used */);
}

CharactersWidget::CharactersWidget( TextRangeRegExp* regexp, RegExpEditorWindow* editorWindow,
                                    TQWidget* parent, const char* name )
    : RegExpWidget( editorWindow, parent, name )
{
    _regexp = dynamic_cast<TextRangeRegExp*>( regexp->clone() );
    Q_ASSERT( _regexp );
}

CharactersWidget::~CharactersWidget()
{
    delete _regexp;
}


TQSize CharactersWidget::tqsizeHint() const
{
    TQFontMetrics metrics = fontMetrics();
    _textSize = HackCalculateFontSize(metrics, title());
    //  _textSize = metrics.size(0, title());

    TQSize headerSize = TQSize(_textSize.width() + 4 * bdSize,
                             _textSize.height());

    _contentSize = HackCalculateFontSize(metrics,text());
    //  _contentSize = metrics.size(0, text());

    return TQSize(TQMAX(headerSize.width(), bdSize + _contentSize.width() + bdSize+ 2*pw),
                 headerSize.height() + bdSize + _contentSize.height() + bdSize + 2*pw);
}

void CharactersWidget::paintEvent(TQPaintEvent *e)
{
    TQSize mySize = tqsizeHint();

    TQPainter painter(this);
    drawPossibleSelection( painter, mySize );

    int center = _textSize.height()/2;
    int offset = 0;

    // draw the horizontal line and the label
    painter.drawLine(pw,center, bdSize,center);
    offset += pw + 2*bdSize;

    painter.drawText(offset, 0, _textSize.width(), _textSize.height(), 0, title());
    offset += _textSize.width() + bdSize;

    painter.drawLine(offset, center, mySize.width(), center);

    // Draw the rest of the lines
    int y = mySize.width() - pw;
    int x = mySize.height() -pw;
    painter.drawLine(0,center,0,mySize.height());
    painter.drawLine(y,center,y,mySize.height());
    painter.drawLine(0,x,y,x);

    // Draw the text
    painter.drawText(bdSize, bdSize+_textSize.height(), _contentSize.width(),
                     _contentSize.height(), 0, text());

    RegExpWidget::paintEvent(e);
}

RegExp* CharactersWidget::regExp() const
{
	RegExp* r = _regexp->clone();
    r->setSelected( isSelected() );
    return r;
}

TQString CharactersWidget::text() const
{
    TQString res = TQString::tqfromLatin1("");

    if (_regexp->wordChar())
        res += i18n("- A word character\n");

    if (_regexp->nonWordChar())
        res += i18n("- A non-word character\n");

    if (_regexp->digit())
        res += i18n("- A digit character\n");

    if (_regexp->nonDigit())
        res += i18n("- A non-digit character\n");

    if ( _regexp->space() )
        res += i18n("- A space character\n");

    if ( _regexp->nonSpace() )
        res += i18n("- A non-space character\n");

    // Single characters
    TQStringList chars = _regexp->chars();
    if ( !chars.isEmpty() ) {
        TQString str = chars.join( TQString::fromLocal8Bit(", ") );
        res += TQString::fromLocal8Bit("- ") + str + TQString::fromLocal8Bit("\n");
    }

    // Ranges characters
    TQPtrList<StringPair> range = _regexp->range();
    for ( TQPtrListIterator<StringPair> it( range ); *it; ++it ) {
        StringPair* elm = static_cast<StringPair*>(*it);
        if (elm) {
            TQString fromText = elm->first();
            TQString toText = elm->second();

            res += TQString::fromLocal8Bit("- ") + i18n("from ") + fromText + i18n(" to ") + toText + TQString::fromLocal8Bit("\n");
        }
    }
    return res.left(res.length()-1);
}


TQString CharactersWidget::title() const
{
    if (_regexp->negate())
        return i18n("Any Character Except");
    else
        return i18n("One of Following Characters");
}


RegExpWidget* CharactersWidget::findWidgetToEdit( TQPoint globalPos )
{
    if ( TQRect(mapToGlobal(TQPoint(0,0)), size()).contains( globalPos ) )
        return this;
    else
        return 0;
}

int CharactersWidget::edit()
{
    if ( _configWindow == 0 ) {
        TQApplication::setOverrideCursor( WaitCursor );
        // No parent here, as this window should continue to exists.
        _configWindow = new CharacterEdits( 0, "CharactersWidget::_configWindow" );
        TQApplication::restoreOverrideCursor();
    }

    _configWindow->move(TQCursor::pos() - TQPoint(_configWindow->tqsizeHint().width()/2,
                                                _configWindow->tqsizeHint().height()/2));
    int ret = _configWindow->exec(_regexp );
    if ( ret == TQDialog::Accepted ) {
        _editorWindow->updateContent( 0 );
        update();
    }
    return ret;
}

void CharacterEdits::addCharacter( TQString txt )
{
    KMultiFormListBoxEntryList list = _single->elements();
    for ( TQPtrListIterator<KMultiFormListBoxEntry> it(list); *it; ++it ) {
        SingleEntry* entry = dynamic_cast<SingleEntry*>( *it );
        if ( entry && entry->isEmpty() ) {
            entry->setText( txt );
            return;
        }
    }

    SingleEntry* entry = new SingleEntry( _single );
    entry->setText( txt );
    _single->append( entry );
}

void CharacterEdits::addRange( TQString from, TQString to )
{
    KMultiFormListBoxEntryList list = _range->elements();
    for ( TQPtrListIterator<KMultiFormListBoxEntry> it(list); *it; ++it ) {
        RangeEntry* entry = dynamic_cast<RangeEntry*>( *it );
        if ( entry && entry->isEmpty() ) {
            entry->setFrom( from );
            entry->setTo( to );
            return;
        }
    }

    RangeEntry* entry = new RangeEntry( _range );
    entry->setFrom( from );
    entry->setTo( to );
    _range->append( entry );
}

int CharacterEdits::exec( TextRangeRegExp* regexp )
{
    _regexp = regexp;
    negate->setChecked( regexp->negate() );
    digit->setChecked( regexp->digit() );
    _nonDigit->setChecked( regexp->nonDigit() );
    space->setChecked( regexp->space() );
    _nonSpace->setChecked( regexp->nonSpace() );
    wordChar->setChecked( regexp->wordChar() );
    _nonWordChar->setChecked( regexp->nonWordChar() );

    bool enabled = (RegExpConverter::current()->features() & RegExpConverter::CharacterRangeNonItems);
    _nonWordChar->setEnabled( enabled );
    _nonDigit->setEnabled( enabled );
    _nonSpace->setEnabled( enabled );

    // Characters

    KMultiFormListBoxEntryList list1 = _single->elements();
    for ( TQPtrListIterator<KMultiFormListBoxEntry> it(list1); *it; ++it ) {
        SingleEntry* entry = dynamic_cast<SingleEntry*>( *it );
        if (entry)
            entry->setText( TQString::fromLocal8Bit("") );
    }
    TQStringList list2 = regexp->chars();
    for ( TQStringList::Iterator it2( list2.begin() ); ! (*it2).isNull(); ++it2 ) {
        addCharacter( *it2 );
    }

    // Ranges
    KMultiFormListBoxEntryList list3 = _range->elements();
    for ( TQPtrListIterator<KMultiFormListBoxEntry> it3(list3); *it3; ++it3 ) {
        RangeEntry* entry = dynamic_cast<RangeEntry*>( *it3 );
        if (entry) {
            entry->setFrom( TQString::fromLocal8Bit("") );
            entry->setTo( TQString::fromLocal8Bit("") );
        }
    }

    TQPtrList<StringPair> ranges = regexp->range();
    for ( TQPtrListIterator<StringPair> it4(ranges); *it4; ++it4 ) {
        TQString from = (*it4)->first();
        TQString to = (*it4)->second();
        addRange(from,to);
    }

    int res = KDialogBase::exec();
    _regexp = 0;
    return res;
}


CharacterEdits::CharacterEdits( TQWidget *parent, const char *name)
  : KDialogBase( parent, name == 0 ? "CharacterEdits" : name, true,
                 i18n("Specify Characters"),
                 KDialogBase::Ok | KDialogBase::Cancel)
{
    TQWidget* top = new TQWidget( this );
    TQVBoxLayout *topLayout = new TQVBoxLayout(top, 6);
    setMainWidget( top );

    negate = new TQCheckBox(i18n("Do not match the characters specified here"),
                           top);
    topLayout->addWidget( negate );


    // The predefined box
    TQHGroupBox* predefined = new TQHGroupBox(i18n("Predefined Character Ranges"),top);
    topLayout->addWidget(predefined);
    TQGrid* grid = new TQGrid(3, predefined );

    wordChar = new TQCheckBox(i18n("A word character"),grid);
    digit = new TQCheckBox(i18n("A digit character"),grid);
    space = new TQCheckBox(i18n("A space character"), grid);

    _nonWordChar = new TQCheckBox(i18n("A non-word character"),grid);
    _nonDigit = new TQCheckBox(i18n("A non-digit character"),grid);
    _nonSpace = new TQCheckBox(i18n("A non-space character"), grid);

    // Single characters
    TQVGroupBox* singleBox = new TQVGroupBox(i18n("Single Characters"), top );
    topLayout->addWidget( singleBox );
    _single = new KMultiFormListBox(new SingleFactory(), KMultiFormListBox::MultiVisible,
                                    singleBox);
    _single->addElement(); _single->addElement(); _single->addElement();

    TQWidget* moreW = new TQWidget( singleBox );
    TQHBoxLayout* moreLay = new TQHBoxLayout( moreW );
    TQPushButton* more = new TQPushButton( i18n("More Entries"), moreW );
    moreLay->addWidget( more );
    moreLay->addStretch( 1 );

    connect(more,TQT_SIGNAL(clicked()), _single, TQT_SLOT(addElement()));

    // Ranges
    TQVGroupBox* rangeBox = new TQVGroupBox(i18n("Character Ranges"), top );
    topLayout->addWidget( rangeBox );

    _range = new KMultiFormListBox(new RangeFactory(), KMultiFormListBox::MultiVisible,
                                   rangeBox);
    _range->addElement(); _range->addElement(); _range->addElement();

    moreW = new TQWidget( rangeBox );
    moreLay = new TQHBoxLayout( moreW );
    more = new TQPushButton( i18n("More Entries"), moreW );
    moreLay->addWidget( more );
    moreLay->addStretch( 1 );
    connect(more,TQT_SIGNAL(clicked()), _range, TQT_SLOT(addElement()));
    // Buttons
    connect(this, TQT_SIGNAL(okClicked()), this, TQT_SLOT(slotOK()));
}

void CharacterEdits::slotOK()
{
    _regexp->setNegate( negate->isChecked() );

    _regexp->setWordChar( wordChar->isChecked() );
    _regexp->setNonWordChar( _nonWordChar->isChecked() );

    _regexp->setDigit( digit->isChecked() );
    _regexp->setNonDigit( _nonDigit->isChecked() );

    _regexp->setSpace( space->isChecked() );
    _regexp->setNonSpace( _nonSpace->isChecked() );

	// single characters
    _regexp->clearChars();
    KMultiFormListBoxEntryList list = _single->elements();
    for ( TQPtrListIterator<KMultiFormListBoxEntry> it( list ); *it; ++it ) {
        SingleEntry* entry = dynamic_cast<SingleEntry*>(*it);
        if ( entry && !entry->isEmpty() ) {
            _regexp->addCharacter( entry->text() );
        }
    }

    // Ranges
    _regexp->clearRange();
    list = _range->elements();
    for ( TQPtrListIterator<KMultiFormListBoxEntry> it2( list ); *it2; ++it2 ) {
        RangeEntry* entry = dynamic_cast<RangeEntry*>(*it2);
        if ( entry && !entry->isEmpty() ) {
            _regexp->addRange( entry->fromText(), entry->toText() );
        }
    }
}


SingleEntry::SingleEntry(TQWidget* parent, const char* name )
    :KMultiFormListBoxEntry( parent, name )
{
    TQHBoxLayout* tqlayout = new TQHBoxLayout( this, 3, 6 );
    _selector = new CharSelector( this );
    tqlayout->addWidget( _selector );
    tqlayout->addStretch(1);
}

TQString SingleEntry::text() const
{
    return _selector->text();
}

void SingleEntry::setText( TQString text )
{
    _selector->setText( text );
}

bool SingleEntry::isEmpty() const
{
    return _selector->isEmpty();
}


RangeEntry::RangeEntry(TQWidget* parent, const char* name )
    :KMultiFormListBoxEntry( parent, name )
{
    TQHBoxLayout* tqlayout = new TQHBoxLayout( this, 3, 6 );

    TQLabel* label = new TQLabel(i18n("From:"), this );
    _from = new CharSelector( this );
    tqlayout->addWidget( label );
    tqlayout->addWidget( _from );

    tqlayout->addStretch(1);

    label = new TQLabel(i18n("end of range","To:"), this );
    _to = new CharSelector( this );
    tqlayout->addWidget( label );
    tqlayout->addWidget( _to );
}

TQString RangeEntry::fromText() const
{
    return _from->text();
}

TQString RangeEntry::toText() const
{
    return _to->text();
}

void RangeEntry::setFrom( TQString text )
{
    _from->setText( text );
}

void RangeEntry::setTo( TQString text )
{
    _to->setText( text );
}

bool RangeEntry::isEmpty() const
{
    return _from->isEmpty() || _to->isEmpty();
}

