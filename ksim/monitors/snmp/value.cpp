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

#include "config.h"
#include "value.h"
#include "value_p.h"
#include "snmp_p.h"

#include <qvariant.h>

#include <klocale.h>

#include <assert.h>

using namespace KSim::Snmp;

ValueImpl::ValueImpl( variable_list *var )
{
    switch ( var->type ) {
        case ASN_INTEGER: {
                type = Value::Int;
                data = QVariant( static_cast<int>( *var->val.integer ) );
                break;
            }
        case ASN_UINTEGER: {
                type = Value::UInt;
                data = QVariant( static_cast<uint>( *var->val.integer ) );
                break;
            }
        case ASN_OCTET_STR: {
                type = Value::ByteArray;
                QByteArray d;
                d.setRawData( reinterpret_cast<char *>( var->val.string ), var->val_len );
                QByteArray copy = d;
                copy.detach();
                d.resetRawData( reinterpret_cast<char *>( var->val.string ), var->val_len );
                data = QVariant( copy );
                break;
            }
        case ASN_NULL: {
                type = Value::Null;
                break;
            }
        case ASN_OBJECT_ID: {
                type = Value::Oid;
                oid = Identifier( new Identifier::Data( var->val.objid, var->val_len / sizeof( oid ) ) );
                break;
            }
        case ASN_IPADDRESS: {
                type = Value::IpAddress;
                addr = QHostAddress( static_cast<uint>( *var->val.integer ) );
                break;
           }
        case ASN_COUNTER: {
                type = Value::Counter;
                data = QVariant( static_cast<uint>( *var->val.integer ) );
                break;
            }
        case ASN_GAUGE: {
                type = Value::Gauge;
                data = QVariant( static_cast<uint>( *var->val.integer ) );
                break;
            }
       case ASN_COUNTER64: {
                type = Value::Counter64;
                ctr64 = ( ( ( Q_UINT64 )var->val.counter64->high ) << 32 ) | ( var->val.counter64->low );
                break;
            }
        case ASN_TIMETICKS: {
                type = Value::TimeTicks;
                data = QVariant( static_cast<int>( *var->val.integer ) );
                break;
            }
        case SNMP_NOSUCHOBJECT: {
                type = Value::NoSuchObject;
                break;
            }
        case SNMP_NOSUCHINSTANCE: {
                type = Value::NoSuchInstance;
                break;
            }
        case SNMP_ENDOFMIBVIEW: {
                type = Value::EndOfMIBView;
                break;
            }
#if defined( OPAQUE_SPECIAL_TYPES )
        case ASN_OPAQUE_FLOAT: {
                type = Value::Double;
                data = QVariant( static_cast<float>( *var->val.floatVal ) );
                break;
            }
        case ASN_OPAQUE_DOUBLE: {
                type = Value::Double;
                data = QVariant( static_cast<float>( *var->val.doubleVal ) );
                break;
            }
#endif
        default: {
            qDebug( "ValueImp: converting from %i to invalid", var->type );
            type = Value::Invalid; break;
        }
    }
}

Value::Value()
{
    d = new ValueImpl;
}

Value::Value( ValueImpl *impl )
    : d( impl )
{
}

Value::Value( int val, Type type )
{
    d = new ValueImpl;

    assert( type == Int || type == TimeTicks );

    d->type = type;
    d->data = QVariant( val );
}

Value::Value( uint val, Type type )
{
    d = new ValueImpl;

    assert( type == UInt || type == Counter || type == Gauge );

    d->type = type;
    d->data = QVariant( val );
}

Value::Value( double val )
{
    d = new ValueImpl;
    d->type = Double;
    d->data = QVariant( val );
}

Value::Value( const QByteArray &data )
{
    d = new ValueImpl;
    d->type = ByteArray;
    d->data = QVariant( data );
}

Value::Value( const KSim::Snmp::Identifier &oid )
{
    d = new ValueImpl;
    d->type = Value::Oid;
    d->oid = oid;
}

Value::Value( const QHostAddress &address )
{
    d = new ValueImpl;
    d->type = IpAddress;
    d->addr = address;
}

Value::Value( Q_UINT64 counter )
{
    d = new ValueImpl;
    d->type = Counter64;
    d->ctr64 = counter;
}

Value::Value( const Value &rhs )
{
    d = new ValueImpl( *rhs.d );
}

Value &Value::operator=( const Value &rhs )
{
    if ( this == &rhs )
        return *this;

    delete d;
    d = new ValueImpl( *rhs.d );

    return *this;
}

Value::~Value()
{
    delete d;
}

Value::Type Value::type() const
{
    return d->type;
}

int Value::toInt() const
{
    switch ( d->type ) {
        case Int:
        case TimeTicks: return d->data.toInt();
        case Invalid: qDebug( "cannot convert from invalid to int" );
        default: assert( false );
    }
    assert( false );
    return -1;
}

uint Value::toUInt() const
{
    switch ( d->type ) {
        case UInt:
        case Counter:
        case Gauge: return d->data.toUInt();
        case Invalid: qDebug( "cannot convert from invalid to uint" );
        default: assert( false );
    }
    assert( false );
    return 0;
}

double Value::toDouble() const
{
    assert( d->type == Double );
    return d->data.toDouble();
}

const QByteArray Value::toByteArray() const
{
    assert( d->type == ByteArray );
    return d->data.toByteArray();
}

const Identifier Value::toOID() const
{
    assert( d->type == Oid );
    return d->oid;
}

const QHostAddress Value::toIpAddress() const
{
    assert( d->type == IpAddress );
    return d->addr;
}

uint Value::toCounter() const
{
    return toUInt();
}

uint Value::toGauge() const
{
    return toUInt();
}

int Value::toTimeTicks() const
{
    return toInt();
}

Q_UINT64 Value::toCounter64() const
{
    assert( d->type == Counter64 );
    return d->ctr64;
}

QString Value::toString( int conversionFlags ) const
{
    switch ( type() ) {
        case Value::Int: return QString::number( toInt() );
        case Value::Gauge:
        case Value::Counter:
        case Value::UInt: return QString::number( toUInt() );
        case Value::Double: return QString::number( toDouble() );
        case Value::Counter64: return QString::number( toCounter64() );
        case Value::ByteArray: return QString::fromAscii( toByteArray().data(), toByteArray().size() );
        case Value::IpAddress: return toIpAddress().toString();
        case Value::Oid: return toOID().toString();
        case Value::TimeTicks: return formatTimeTicks( toTimeTicks(), conversionFlags );
        // not using i18n here, because it may be called from within a worker thread, and I'm
        // not sure it makes sense to translate it anyway
        case Value::NoSuchObject: return QString::fromLatin1( "No Such Object" );
        case Value::NoSuchInstance: return QString::fromLatin1( "No Such Instance" );
        case Value::EndOfMIBView: return QString::fromLatin1( "End Of MIB View" );
        case Value::Invalid:
        case Value::Null: return QString::null;
    }
    return QString::null;
}

QString Value::formatTimeTicks( int ticks, int conversionFlags )
{
    ticks /= 100;

    int days = ticks / ( 60 * 60 * 24 );
    ticks %= 60 * 60 * 24;

    int hours = ticks / ( 60 * 60 );
    ticks %= 60 * 60;

    int minutes = ticks / 60;
    int seconds = ticks % 60;

    QString result;

    if ( days > 0 )
        result += QString::number( days ) + "d:";

    result += QString::fromAscii( "%1h:%2m" ).arg( hours ).arg( minutes );

    if ( conversionFlags & TimeTicksWithSeconds )
        result += ":" + QString::number( seconds ) + "s";

    return result;
}

/* vim: et sw=4 ts=4
 */
