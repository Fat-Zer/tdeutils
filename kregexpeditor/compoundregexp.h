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
#ifndef COMPOUNDREGEXP_H
#define COMPOUNDREGEXP_H
#include "regexp.h"

/**
   Abstract syntax node for `compound content' regular expression
   @internal
*/
class CompoundRegExp :public RegExp
{
public:
	CompoundRegExp( bool selected, const TQString& title = TQString(),
                    const TQString& description = TQString(),
                    bool hidden = false, bool allowReplace = false, RegExp* child = 0);

    virtual bool check( ErrorMap&, bool first, bool last );
    virtual int precedence() const { return _child->precedence();}
    virtual TQDomNode toXml( TQDomDocument* doc ) const;
    virtual bool load( TQDomElement, const TQString& version );
    TQString title() const { return _title; }
    TQString description() const { return _description; }
    RegExp* child() const { return _child; }
    bool hidden() const { return _hidden; }
    bool allowReplace() const { return _allowReplace; }
    virtual RegExpType type() const { return COMPOUND;}
    virtual bool operator==( const RegExp& other ) const;

private:
    TQString _title;
    TQString _description;
    bool _hidden;
    bool _allowReplace;
	RegExp* _child;
};


#endif // COMPOUNDREGEXP_H
