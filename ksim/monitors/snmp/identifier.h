/* This file is part of the KDE project
   Copyright (C) 2003 Simon Hausmann <hausmann@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef IDENTIFIER_H
#define IDENTIFIER_H

#include <qstring.h>
#include <qvaluelist.h>
#include <qmap.h>

#include "value.h"

namespace KSim
{

namespace Snmp
{

class Identifier
{
public:
    friend class PDU;
    struct Data;

    enum PrintFlags { PrintAscii, PrintNumeric };

    Identifier();
    Identifier( Data *data );
    Identifier( const Identifier &rhs );
    Identifier &operator=( const Identifier &rhs );
    ~Identifier();

    static Identifier fromString( const QString &name, bool *ok = 0 );

    QString toString( PrintFlags flags = PrintAscii ) const;

    bool isNull() const;

    bool operator==( const Identifier &rhs ) const;
    bool operator!=( const Identifier &rhs ) const
    { return !operator==( rhs ); }
    bool operator<( const Identifier &rhs ) const;

private:
    Data *d;
};

typedef QValueList<Identifier> IdentifierList;
typedef QMap<Identifier, Value> ValueMap;

}
}

#endif // IDENTIFIER_H
/* vim: et sw=4 ts=4
 */
