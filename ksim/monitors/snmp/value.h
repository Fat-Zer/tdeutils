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

#ifndef VALUE_H
#define VALUE_H

#include <qhostaddress.h>

namespace KSim
{

namespace Snmp
{

class ValueImpl;

class Identifier;

struct ValueImpl;

class Value
{
public:
    enum Type {
        Invalid,
        Int,
        UInt,
        Double,
        ByteArray,
        Null,
        Oid,
        IpAddress,
        Counter,
        Gauge,
        TimeTicks,
        Counter64,
        NoSuchObject,
        NoSuchInstance,
        EndOfMIBView
    };

    enum StringConversionFlags { TimeTicksWithSeconds = 0x1 };

    Value();

    Value( ValueImpl *imp );

    Value( int val, Type type );
    Value( uint val, Type type );
    Value( float val );
    Value( double val );
    Value( const QByteArray &data );
    Value( const Identifier &oid );
    Value( const QHostAddress &address );
    Value( Q_UINT64 counter );

    Value( const Value &rhs );
    Value &operator=( const Value &rhs );
    ~Value();

    Type type() const;

    bool isCounterType() const { return type() == Value::Counter || type() == Value::Counter64; }

    int toInt() const;
    uint toUInt() const;
    double toDouble() const;
    const QByteArray toByteArray() const;
    const Identifier toOID() const;
    const QHostAddress toIpAddress() const;
    uint toCounter() const;
    uint toGauge() const;
    int toTimeTicks() const;
    Q_UINT64 toCounter64() const;

    QString toString( int conversionFlags = 0 ) const;

private:
    static QString formatTimeTicks( int ticks, int conversionFlags );

    ValueImpl *d;
};

}

}

#endif // VALUE_H
/* vim: et sw=4 ts=4
 */
