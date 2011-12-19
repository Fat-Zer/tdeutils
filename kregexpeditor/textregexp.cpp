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
  #include <kmessagebox.h>
  #include <klocale.h>
#endif

#include "textregexp.h"

TextRegExp::TextRegExp( bool selected, TQString text) :RegExp( selected )
{
	_text = text;
}

bool TextRegExp::check( ErrorMap&, bool, bool )
{
    return false;
}


void TextRegExp::append( TQString str )
{
    _text.append( str );
}

TQDomNode TextRegExp::toXml( TQDomDocument* doc ) const
{
    TQDomElement top = doc->createElement(TQString::fromLocal8Bit("Text"));
    TQDomText text = doc->createTextNode( _text );
    top.appendChild( text );
    return top;
}

bool TextRegExp::load( TQDomElement top, const TQString& /*version*/)
{
    Q_ASSERT( top.tagName() == TQString::fromLocal8Bit( "Text" ) );
    if ( top.hasChildNodes() ) {
        TQDomNode child = top.firstChild();
        if ( ! child.isText() ) {
            KMessageBox::sorry( 0, i18n("<p>Element <b>Text</b> did not contain any textual data.</p>"),
                                i18n("Error While Loading From XML File") ) ;
            return false;
        }
        TQDomText txtNode = child.toText();
        _text = txtNode.data();
    }
    else {
        _text = TQString::fromLatin1( "" );
    }

    return true;
}

bool TextRegExp::operator==( const RegExp& other ) const {
    if ( other.type() != type() )
        return false;

    const TextRegExp& theOther = dynamic_cast<const TextRegExp&>( other );
    if ( text() == theOther.text() )
        return true;

    return false;
}

