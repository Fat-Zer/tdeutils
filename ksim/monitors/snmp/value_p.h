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

#ifndef VALUE_P_H
#define VALUE_P_H

#define HAVE_STRLCPY 1
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <qvariant.h>

#include "identifier.h"

namespace KSim
{

namespace Snmp
{

struct ValueImpl
{
    ValueImpl()
        : type( Value::Invalid )
    {}

    ValueImpl( const ValueImpl &rhs )
    {
        ( *this ) = rhs;
    }

    ValueImpl( variable_list *var );

    Value::Type type;
    QVariant data;
    Identifier oid;
    QHostAddress addr;
    Q_UINT64 ctr64;
};

}

}

#endif // VALUE_P_H
/* vim: et sw=4 ts=4
 */
