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
  #include <tdelocale.h>
  #include <tdemessagebox.h>
#endif

#include "textrangeregexp.h"
#include "regexpconverter.h"

TextRangeRegExp::TextRangeRegExp( bool selected ) : RegExp( selected ),
    _negate(false), _digit(false), _nonDigit(false), _space(false), _nonSpace(false), _wordChar(false), _nonWordChar(false)
{
}

TextRangeRegExp::~TextRangeRegExp()
{
}


void TextRangeRegExp::addCharacter( TQString str )
{
	_chars.append( str );
}

void TextRangeRegExp::addRange(TQString from, TQString to)
{
	_ranges.append( new StringPair( from, to ) );
}

bool TextRangeRegExp::check( ErrorMap&, bool, bool )
{
    return false;
}

TQDomNode TextRangeRegExp::toXml( TQDomDocument* doc ) const
{
    TQDomElement top = doc->createElement( TQString::fromLocal8Bit( "TextRange" ) );

    if ( _negate )
        top.setAttribute( TQString::fromLocal8Bit("negate"), true );
    if ( _digit )
        top.setAttribute( TQString::fromLocal8Bit("digit"), true );
    if ( _nonDigit )
        top.setAttribute( TQString::fromLocal8Bit("nonDigit"), true );
    if ( _space )
        top.setAttribute( TQString::fromLocal8Bit("space"), true );
    if ( _nonSpace )
        top.setAttribute( TQString::fromLocal8Bit("nonSpace"), true );
    if ( _wordChar )
        top.setAttribute( TQString::fromLocal8Bit("wordChar"), true );
    if ( _nonWordChar )
        top.setAttribute( TQString::fromLocal8Bit("nonWordChar"), true );

    for ( TQStringList::ConstIterator it = _chars.begin(); it != _chars.end(); ++it ) {
        TQDomElement elm = doc->createElement( TQString::fromLocal8Bit( "Character" ) );
        elm.setAttribute( TQString::fromLocal8Bit( "char" ), *it );
        top.appendChild( elm );
    }

    for ( TQPtrListIterator<StringPair> it2(_ranges); *it2; ++it2 ) {
        TQDomElement elm = doc->createElement( TQString::fromLocal8Bit( "Range" ) );
        elm.setAttribute( TQString::fromLocal8Bit( "from" ), (*it2)->first() );
        elm.setAttribute( TQString::fromLocal8Bit( "to" ), (*it2)->second() );
        top.appendChild( elm );
    }
    return top;
}

bool TextRangeRegExp::load( TQDomElement top, const TQString& /*version*/ )
{
    Q_ASSERT( top.tagName() == TQString::fromLocal8Bit( "TextRange" ) );
    TQString str;
    TQString one = TQString::fromLocal8Bit("1");
    TQString zero = TQString::fromLocal8Bit("0");

    str = top.attribute( TQString::fromLocal8Bit("negate"), zero );
    _negate = ( str == one );

    str = top.attribute( TQString::fromLocal8Bit("digit"), zero );
    _digit = ( str == one );

    str = top.attribute( TQString::fromLocal8Bit("nonDigit"), zero );
    _nonDigit = ( str == one );

    str = top.attribute( TQString::fromLocal8Bit("space"), zero );
    _space = ( str == one );

    str = top.attribute( TQString::fromLocal8Bit("nonSpace"), zero );
    _nonSpace = ( str == one );

    str = top.attribute( TQString::fromLocal8Bit("wordChar"), zero );
    _wordChar = ( str == one );

    str = top.attribute( TQString::fromLocal8Bit("nonWordChar"), zero );
    _nonWordChar = ( str == one );

    for ( TQDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( !node.isElement() )
            continue; // Skip comments.
        TQDomElement child = node.toElement();

        if ( child.tagName() == TQString::fromLocal8Bit( "Character" ) ) {
            TQString ch = child.attribute( TQString::fromLocal8Bit( "char" ) );
            addCharacter( ch );
        }
        else if ( child.tagName() == TQString::fromLocal8Bit( "Range" ) ) {
            TQString from = child.attribute( TQString::fromLocal8Bit( "from" ) );
            TQString to = child.attribute( TQString::fromLocal8Bit( "to" ) );
            addRange( from, to );
        }
        else {
            KMessageBox::sorry( 0, i18n("<p>Invalid sub element to element <b>TextRange</b>. Tag was <b>%1</b></p>").arg(child.tagName()),
                                i18n("Error While Loading From XML File") ) ;
            return false;
        }
    }
    return true;
}

bool TextRangeRegExp::operator==( const RegExp& other ) const
{
    return ( RegExpConverter::current()->toStr( const_cast<TextRangeRegExp*>( this ), false ) ==
             RegExpConverter::current()->toStr( const_cast<RegExp*>( &other ), false ) );
}

