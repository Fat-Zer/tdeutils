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

#ifndef SNMPLIB_H
#define SNMPLIB_H

// ### REMOVEME: move snmplib somewhere else
#define HAVE_STRLCPY 1
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

#include <qmutex.h>

template <class> class KStaticDeleter;

namespace KSim
{

namespace Snmp
{

template <class T>
class ClassLocker
{
public:
    ClassLocker( QMutex *guard, T *obj )
        : m_guard( guard ), m_obj( obj )
    {}

    struct ClassLockerHelper
    {
        ClassLockerHelper( QMutex *guard, T *obj )
            : m_guard( guard ), m_obj( obj ), m_locked( false )
        {}

        ~ClassLockerHelper()
        {
            if ( m_locked )
                m_guard->unlock();
        }

        T *operator->()
        {
            m_guard->lock();
            m_locked = true;
            return m_obj;
        }

    private:
        QMutex *m_guard;
        T *m_obj;
        bool m_locked;
    };

    ClassLockerHelper operator->()
    {
        return ClassLockerHelper( m_guard, m_obj );
    }

private:
    ClassLocker( const ClassLocker & );
    ClassLocker &operator=( const ClassLocker & );

    QMutex *m_guard;
    T *m_obj;
};

class SnmpLib
{
    friend class KStaticDeleter<SnmpLib>;
public:

    static ClassLocker<SnmpLib> &self();

    oid *snmp_parse_oid( const char *name, oid *oid, size_t *length );

    int netsnmp_ds_set_int( int storeid, int which, int value );
    int netsnmp_ds_get_int( int storeid, int which );

    struct tree *netsnmp_sprint_realloc_objid_tree( u_char ** buf,
                                                    size_t * buf_len,
                                                    size_t * out_len,
                                                    int allow_realloc,
                                                    int *buf_overflow,
                                                    const oid * objid,
                                                    size_t objidlen );

    int netsnmp_oid_equals( const oid *firstOid, size_t firstOidLen, const oid *secondOid, size_t secondOidLen );
    int snmp_oid_compare( const oid *firstOid, size_t firstOidLen, const oid *secondOid, size_t secondOidLen );

    void snmp_error( netsnmp_session *session, int *p_errno, int *p_snmp_errno, char **p_str );
    const char *snmp_api_errstring( int code );
    const char *snmp_errstring( int code );

    netsnmp_pdu *snmp_pdu_create( int pduType );
    void snmp_free_pdu( netsnmp_pdu *pdu );
    netsnmp_variable_list *snmp_add_null_var( netsnmp_pdu *pdu, oid *var, size_t varlen );

    void snmp_sess_init( netsnmp_session *session );
    netsnmp_session *snmp_open( netsnmp_session *session );
    int snmp_close( netsnmp_session *session );

    int snmp_synch_response( netsnmp_session *session, netsnmp_pdu *pdu,
                             netsnmp_pdu **response );

    int generate_Ku( const oid * hashtype, u_int hashtype_len,
                     u_char * P, size_t pplen,
                     u_char * Ku, size_t * kulen );

private:
    SnmpLib();
    ~SnmpLib();

    QMutex m_guard;
    ClassLocker<SnmpLib> *m_lockHelper;

    static SnmpLib *s_self;
};

}
}


#endif // SNMPLIB_H
/* vim: et sw=4 ts=4
 */

