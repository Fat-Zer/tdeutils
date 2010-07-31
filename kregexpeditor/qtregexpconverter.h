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

#ifndef QTREGEXPCONVERTER_H
#define QTREGEXPCONVERTER_H
#include "regexpconverter.h"

class QtRegExpConverter :public RegExpConverter
{
public:
    virtual bool canParse();
    virtual RegExp* parse( const TQString&, bool* ok );
    virtual TQString name();
    virtual int features();
    virtual TQString toString( AltnRegExp*, bool markSelection );
    virtual TQString toString( ConcRegExp*, bool markSelection );
    virtual TQString toString( LookAheadRegExp*, bool markSelection );
    virtual TQString toString( TextRangeRegExp*, bool markSelection );
    virtual TQString toString( CompoundRegExp*, bool markSelection );
    virtual TQString toString( DotRegExp*, bool markSelection );
    virtual TQString toString( PositionRegExp*, bool markSelection );
    virtual TQString toString( RepeatRegExp*, bool markSelection );
    virtual TQString toString( TextRegExp*, bool markSelection );
    RegexpHighlighter* highlighter( TQTextEdit* edit );
};

#endif /* QTREGEXPCONVERTER_H */

