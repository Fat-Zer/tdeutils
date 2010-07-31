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

#include "emacsregexpconverter.h"
#include "regexp.h"
#include "altnregexp.h"
#include "concregexp.h"
#include "lookaheadregexp.h"
#include "textrangeregexp.h"
#include "textregexp.h"
#include "compoundregexp.h"
#include "dotregexp.h"
#include "positionregexp.h"
#include "repeatregexp.h"

#include <kmessagebox.h>
#include <klocale.h>
bool EmacsRegExpConverter::canParse()
{
    return false;
}

TQString EmacsRegExpConverter::toString( AltnRegExp* regexp, bool markSelection )
{
    TQString res;

	bool first = true;
    RegExpList list = regexp->children();
    for ( RegExpListIt it(list); *it; ++it ) {
		if ( !first ) {
			res += TQString::fromLatin1("\\|");
		}
		first = false;
        res += toStr( *it, markSelection );
	}
	return res;

}

TQString EmacsRegExpConverter::toString( ConcRegExp* regexp, bool markSelection )
{
	TQString res;

    RegExpList list = regexp->children();
	for ( RegExpListIt it(list); *it; ++it ) {
        TQString startPar = TQString::fromLocal8Bit("");
        TQString endPar = TQString::fromLocal8Bit("");
        if ( (*it)->precedence() < regexp->precedence() ) {
            startPar = TQString::fromLatin1( "\\(" );
            endPar = TQString::fromLatin1( "\\)" );
        }

		res += startPar + toStr( *it, markSelection ) + endPar;
	}

	return res;

}

TQString EmacsRegExpConverter::toString( LookAheadRegExp* /*regexp*/, bool /*markSelection*/ )
{
    static bool haveWarned = false;
    if ( ! haveWarned ) {
        KMessageBox::sorry( 0, i18n("Look ahead regular expressions not supported in Emacs style") );
        haveWarned = true;
    }

    return TQString::null;
}

TQString EmacsRegExpConverter::toString( TextRangeRegExp* regexp, bool /*markSelection*/ )
{
	TQString txt;

	bool foundCarrot = false;
	bool foundDash = false;
	bool foundParenthesis = false;

	// print the single characters, but keep "^" as the very
	// last element of the characters.
    TQStringList chars = regexp->chars();
	for (unsigned int i = 0; i< chars.count(); i++) {
		if ( *chars.at(i) == TQChar(']') ) {
			foundParenthesis = true;
		}
		else if ( *chars.at(i) == TQChar('-') ) {
			foundDash = true;
		}
		else if ( *chars.at(i) == TQChar('^') ) {
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

	// Insert equivalents to \s,\S,\d,\D,\w, and \W
    // non-digit, non-space, and non-word is not supported in Emacs style
    if ( regexp->digit() )
        res += TQString::fromLocal8Bit("0-9");
    if ( regexp->space() )
        res += TQString::fromLocal8Bit(" ") + TQString( TQChar( (char) 9 ) ); // Tab char
    if ( regexp->wordChar() )
        res += TQString::fromLocal8Bit("a-zA-Z");

	if ( foundCarrot ) {
		res.append( TQChar( '^' ) );
	}

	res.append(TQString::fromLatin1("]"));

	return res;
}

TQString EmacsRegExpConverter::toString( CompoundRegExp* regexp, bool markSelection )
{
    return  toStr( regexp->child(), markSelection );
}

TQString EmacsRegExpConverter::toString( DotRegExp* /*regexp*/, bool /*markSelection*/ )
{
    return TQString::fromLatin1( "." );
}

TQString EmacsRegExpConverter::toString( PositionRegExp* regexp, bool /*markSelection*/ )
{
    static bool haveWarned = false;
    switch ( regexp->position()) {
	case PositionRegExp::BEGLINE:
		return TQString::fromLatin1("^");
	case PositionRegExp::ENDLINE:
		return TQString::fromLatin1("$");
	case PositionRegExp::WORDBOUNDARY:
    case PositionRegExp::NONWORDBOUNDARY:
        if ( ! haveWarned ) {
            KMessageBox::sorry( 0, i18n( "Word boundary and non word boundary is not supported in Emacs syntax" ) );
            haveWarned = true;
            return TQString::fromLatin1("");
        }
    }
    return TQString::fromLatin1("");
}

TQString EmacsRegExpConverter::toString( RepeatRegExp* regexp, bool markSelection )
{
    RegExp* child = regexp->child();
    TQString cText = toStr( child, markSelection );
    TQString startPar;
    TQString endPar;

    if ( child->precedence() < regexp->precedence() ) {
        startPar = TQString::fromLatin1( "\\(" );
        endPar = TQString::fromLatin1( "\\)" );
    }

    if (regexp->min() == 0 && regexp->max() == -1) {
        return startPar + cText +endPar + TQString::fromLocal8Bit("*");
    }
    else if ( regexp->min() == 0 && regexp->max() == 1 ) {
        return startPar + cText + endPar + TQString::fromLocal8Bit("?");
    }
    else if ( regexp->min() == 1 && regexp->max() == -1 ) {
        return startPar + cText + endPar + TQString::fromLocal8Bit("+");
    }
    else {
        TQString res = TQString::fromLatin1("");
        for ( int i = 0; i < regexp->min(); ++i ) {
            res += TQString::fromLatin1( "\\(" ) + cText + TQString::fromLatin1( "\\)" );
        }
        if ( regexp->max() != -1 ) {
            for ( int i = regexp->min(); i < regexp->max(); ++i ) {
                res += TQString::fromLatin1("\\(") + cText + TQString::fromLatin1("\\)?");
            }
        }
        else
            res += TQString::fromLatin1("+");

        return res;
    }
}

TQString EmacsRegExpConverter::toString( TextRegExp* regexp, bool /*markSelection*/ )
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
         << TQChar('\\');

	TQString res = escape( regexp->text(), list, TQChar('\\') );
	return res;
}

TQString EmacsRegExpConverter::name()
{
    return TQString::fromLatin1( "Emacs" );
}

int EmacsRegExpConverter::features()
{
    return WordStart | WordEnd;
}
