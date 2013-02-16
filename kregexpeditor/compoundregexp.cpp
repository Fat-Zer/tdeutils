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
#endif

#include "compoundregexp.h"
#include "widgetfactory.h"

CompoundRegExp::CompoundRegExp( bool selected, const TQString& title, const TQString& description, bool hidden,
                                bool allowReplace, RegExp* child)
    : RegExp( selected ), _title( title ), _description( description ), _hidden( hidden ), _allowReplace( allowReplace ), _child( child )
{
    if ( child )
        addChild( child );
}

bool CompoundRegExp::check( ErrorMap& map, bool first, bool last )
{
    return _child->check( map, first, last );
}

TQDomNode CompoundRegExp::toXml( TQDomDocument* doc ) const
{
    TQDomElement top = doc->createElement( TQString::fromLocal8Bit( "Compound" ) );
    if (_hidden)
        top.setAttribute( TQString::fromLocal8Bit("hidden"), true );
    if ( _allowReplace )
        top.setAttribute( TQString::fromLocal8Bit("allowReplace"), true );

    TQDomElement title = doc->createElement( TQString::fromLocal8Bit( "Title" ) );
    TQDomText titleTxt = doc->createTextNode( _title );
    title.appendChild( titleTxt );
    top.appendChild( title );

    TQDomElement description = doc->createElement( TQString::fromLocal8Bit( "Description" ) );
    TQDomText descriptionTxt = doc->createTextNode( _description );
    description.appendChild( descriptionTxt );
    top.appendChild( description );

    top.appendChild( _child->toXml( doc ) );

    return top;
}


bool CompoundRegExp::load( TQDomElement top, const TQString& version )
{
    Q_ASSERT( top.tagName() == TQString::fromLocal8Bit("Compound") );
    TQString str = top.attribute( TQString::fromLocal8Bit( "hidden" ), TQString::fromLocal8Bit("0") );
    _hidden = true; // alway hidden. (str == TQString::fromLocal8Bit("1") );

    str = top.attribute( TQString::fromLocal8Bit( "allowReplace" ), TQString::fromLocal8Bit("0") );
    _allowReplace = (str == TQString::fromLocal8Bit("1") );

    for ( TQDomNode node = top.firstChild(); !node.isNull(); node = node.nextSibling() ) {
        if ( !node.isElement() )
            continue; // skip past comments.
        TQString txt;
        TQDomElement child = node.toElement();
        TQDomNode txtNode = child.firstChild();
        if ( txtNode.isText() )
            txt = txtNode.toText().data();
        if ( child.tagName() == TQString::fromLocal8Bit( "Title" ) ) {
            if ( txt.isEmpty() )
                _title = txt;
            else
                _title = i18n(txt.utf8());
        }
        else if ( child.tagName() == TQString::fromLocal8Bit( "Description" ) ) {
            if ( txt.isEmpty() )
                _description = txt;
            else
                _description = i18n(txt.utf8());
        }
        else {
            _child = WidgetFactory::createRegExp( child, version );
            if ( _child ) {
                addChild( _child );
                return true;
            }
            else {
                return false;
            }
        }
    }
    return false;
}

bool CompoundRegExp::operator==( const RegExp& other ) const
{
    // Note the order is important in the comparison below.
    // Using other as the first argument, means that
    // the following will be evaluated: other.operator== rather than (*child).operator==
    // This means that if other is a CompoundRegExp too, then this level will be striped.
    return ( other == *_child );
}
