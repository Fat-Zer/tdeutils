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

#include "qtregexpconverter.h"
#include "qtregexphighlighter.h"
#include "regexp.h"
#include "textregexp.h"
#include "altnregexp.h"
#include "concregexp.h"
#include "lookaheadregexp.h"
#include "textrangeregexp.h"
#include "compoundregexp.h"
#include "dotregexp.h"
#include "positionregexp.h"
#include "repeatregexp.h"

extern RegExp* parseQtRegExp( TQString str, bool *ok );
extern RegExp* parseDataQtRegExp();

bool QtRegExpConverter::canParse()
{
    return true;
}


RegExp* QtRegExpConverter::parse( const TQString& txt, bool* ok )
{
    return parseQtRegExp( txt, ok );
}

TQString QtRegExpConverter::toString( AltnRegExp* regexp, bool markSelection )
{
    TQString res;

	bool first = true;
    RegExpList list = regexp->children();
    for ( RegExpListIt it(list); *it; ++it ) {
		if ( !first ) {
			res += TQString::fromLatin1( "|" );
		}
		first = false;
        if ( markSelection && !regexp->isSelected() && (*it)->isSelected() ) {
            res += TQString::fromLatin1("(") + toStr( *it, markSelection ) + TQString::fromLatin1(")");
        }
        else {
            res += toStr( *it, markSelection );
        }
	}
	return res;
}

TQString QtRegExpConverter::toString( ConcRegExp* regexp, bool markSelection )
{
    TQString res;
    bool childSelected = false;

    RegExpList list = regexp->children();
	for ( RegExpListIt it(list); *it; ++it ) {
        TQString startPar = TQString::fromLocal8Bit("");
        TQString endPar = TQString::fromLocal8Bit("");
        if ( (*it)->precedence() < regexp->precedence() ) {
            if ( markSelection )
                startPar = TQString::fromLocal8Bit("(?:");
            else
                startPar = TQString::fromLatin1( "(" );
            endPar = TQString::fromLatin1( ")" );
        }

        // Note these two have different tests! They are activated in each their iteration of the loop.
        if ( markSelection && !childSelected && !regexp->isSelected() && (*it)->isSelected() ) {
            res += TQString::fromLatin1("(");
            childSelected = true;
        }

        if ( markSelection && childSelected && !regexp->isSelected() && !(*it)->isSelected() ) {
            res += TQString::fromLatin1(")");
            childSelected= false;
        }

		res += startPar + toStr( *it, markSelection ) + endPar;
	}
    if ( markSelection && childSelected && !regexp->isSelected() ) {
        res += TQString::fromLatin1(")");
    }
	return res;
}

TQString QtRegExpConverter::toString( LookAheadRegExp* regexp, bool markSelection )
{
    if ( regexp->lookAheadType() == LookAheadRegExp::POSITIVE )
        return TQString::fromLatin1( "(?=" ) + toStr( regexp->child(), markSelection ) + TQString::fromLocal8Bit( ")" );
    else
        return TQString::fromLatin1( "(?!" ) + toStr( regexp->child(), markSelection ) + TQString::fromLocal8Bit( ")" );
}

TQString QtRegExpConverter::toString( TextRangeRegExp* regexp, bool /*markSelection*/ )
{
	TQString txt;

	bool foundCarrot = false;
	bool foundDash = false;
	bool foundParenthesis = false;

	// Now print the rest of the single characters, but keep "^" as the very
	// last element of the characters.
    TQStringList chars = regexp->chars();
	for (unsigned int i = 0; i< chars.count(); i++) {
		if ( *chars.at(i) == TQChar( ']' ) ) {
			foundParenthesis = true;
		}
		else if ( *chars.at(i) == TQChar( '-' ) ) {
			foundDash = true;
		}
		else if ( *chars.at(i) == TQChar( '^' ) ) {
			foundCarrot = true;
		}
		else {
			txt.append( *chars.at(i) );
		}
	}

	// Now insert the ranges.
    TQPtrList<StringPair> ranges = regexp->range();
    for ( TQPtrListIterator<StringPair> it(ranges); *it; ++it ) {
		txt.append((*it)->first()+ TQString::fromLatin1("-")+ (*it)->second());
	}

	// Ok, its time to build each part of the regexp, here comes the rule:
	// if a ']' is one of the characters, then it must be the first one in the
	// list (after then opening '[' and eventually negation '^')
	// Next if a '-' is one of the characters, then it must come
	// finally if '^' is one of the characters, then it must not be the first
	// one!

	TQString res = TQString::fromLatin1("[");

	if ( regexp->negate() )
		res.append(TQString::fromLatin1("^"));


	// a ']' must be the first character in teh range.
	if ( foundParenthesis ) {
		res.append(TQString::fromLatin1("]"));
	}

	// a '-' must be the first character ( only coming after a ']')
	if ( foundDash ) {
		res.append(TQString::fromLatin1("-"));
	}

	res += txt;

	// Insert \s,\S,\d,\D,\w, and \W
    if ( regexp->digit() )
        res += TQString::fromLocal8Bit("\\d");
    if ( regexp->nonDigit() )
        res += TQString::fromLocal8Bit("\\D");
    if ( regexp->space() )
        res += TQString::fromLocal8Bit("\\s");
    if ( regexp->nonSpace() )
        res += TQString::fromLocal8Bit("\\S");
    if ( regexp->wordChar() )
        res += TQString::fromLocal8Bit("\\w");
    if ( regexp->nonWordChar() )
        res += TQString::fromLocal8Bit("\\W");


	if ( foundCarrot ) {
		res.append( TQChar( '^' ) );
	}

	res.append(TQString::fromLatin1("]"));

	return res;
}

TQString QtRegExpConverter::toString( CompoundRegExp* regexp, bool markSelection )
{
    if ( markSelection && !regexp->isSelected() && regexp->child()->isSelected() )
        return TQString::fromLatin1( "(" ) + toStr( regexp->child(), markSelection ) + TQString::fromLatin1( ")" );
    else
        return  toStr( regexp->child(), markSelection );
}

TQString QtRegExpConverter::toString( DotRegExp* /*regexp*/, bool /*markSelection*/ )
{
    return TQString::fromLatin1( "." );
}

TQString QtRegExpConverter::toString( PositionRegExp* regexp, bool /*markSelection*/ )
{
	switch (regexp->position()) {
	case PositionRegExp::BEGLINE:
		return TQString::fromLatin1("^");
	case PositionRegExp::ENDLINE:
		return TQString::fromLatin1("$");
	case PositionRegExp::WORDBOUNDARY:
        return TQString::fromLatin1("\\b");
	case PositionRegExp::NONWORDBOUNDARY:
        return TQString::fromLatin1("\\B");
    }
	Q_ASSERT( false );
	return TQString::fromLatin1("");
}

TQString QtRegExpConverter::toString( RepeatRegExp* regexp, bool markSelection )
{
    RegExp* child = regexp->child();
    TQString cText = toStr( child, markSelection );
    TQString startPar;
    TQString endPar;

    if ( markSelection ) {
        if ( !regexp->isSelected() && child->isSelected()) {
            startPar = TQString::fromLatin1( "(" );
            endPar = TQString::fromLatin1( ")" );
        }
        else if ( child->precedence() < regexp->precedence() ) {
            startPar = TQString::fromLatin1( "(?:" );
            endPar = TQString::fromLatin1( ")" );
        }
    }
    else if ( child->precedence() < regexp->precedence() ) {
        startPar = TQString::fromLatin1( "(" );
        endPar = TQString::fromLatin1( ")" );
    }

    if ( regexp->min() == 0 && regexp->max() == -1) {
        return startPar + cText +endPar + TQString::fromLocal8Bit("*");
    }
    else if ( regexp->min() == 0 && regexp->max() == 1 ) {
        return startPar + cText + endPar + TQString::fromLocal8Bit("?");
    }
    else if ( regexp->min() == 1 && regexp->max() == -1 ) {
        return startPar + cText + endPar + TQString::fromLocal8Bit("+");
    }
    else if ( regexp->max() == -1 ) {
        return startPar + cText + endPar + TQString::fromLocal8Bit("{") +
            TQString::number( regexp->min() ) + TQString::fromLocal8Bit(",") +
            TQString::fromLocal8Bit("}");
    }
    else {
        return startPar + cText + endPar + TQString::fromLocal8Bit("{") +
            TQString::number( regexp->min() ) + TQString::fromLocal8Bit(",") +
            TQString::number( regexp->max() ) + TQString::fromLocal8Bit("}");
    }
}

TQString QtRegExpConverter::toString( TextRegExp* regexp, bool /*markSelection*/ )
{
    TQValueList<TQChar> list;
	list << TQChar('$')
         << TQChar('^')
         << TQChar('.')
         << TQChar('*')
         << TQChar('+')
         << TQChar('?')
         << TQChar('[')
         << TQChar(']')
         << TQChar('\\')
         << TQChar('{')
         << TQChar('}')
         << TQChar('(')
         << TQChar(')')
         << TQChar('|');

	TQString res = escape( regexp->text(), list, TQChar('\\') );
	return res;
}

TQString QtRegExpConverter::name()
{
    return TQString::fromLatin1( "Qt" );
}

int QtRegExpConverter::features()
{
    return WordBoundary | NonWordBoundary | PosLookAhead | NegLookAhead | CharacterRangeNonItems | ExtRange;
}

RegexpHighlighter* QtRegExpConverter::highlighter( TQTextEdit* edit )
{
    return new QtRegexpHighlighter( edit );
}
