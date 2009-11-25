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

#include "probedialog.h"
#include "monitor.h"

#include <qtimer.h>

#include <klocale.h>
#include <kdebug.h>

using namespace KSim::Snmp;

static const char * const probeIdentifiers[] =
{
    "system.sysDescr.0",
    "system.sysName.0",
    "system.sysUpTime.0",
    "system.sysContact.0",
    "system.sysObjectID.0",
    ".1.3.6.1.2.1.25.1.1.0",
    0
};

ProbeDialog::ProbeDialog( const HostConfig &hostConfig, QWidget *parent, const char *name )
    : KProgressDialog( parent, name, i18n( "SNMP Host Probe" ), QString::null, true /* modal */ ),
      m_host( hostConfig ), m_currentMonitor( 0 ), m_canceled( false )
{
    setLabel( i18n( "Probing for common object identifiers..." ) );

    for ( uint i = 0; probeIdentifiers[ i ]; ++i ) {
        Identifier id = Identifier::fromString( probeIdentifiers[ i ] );
        if ( !id.isNull() )
            m_probeOIDs << id;
    }

    progressBar()->setTotalSteps( m_probeOIDs.count() );

    setAutoClose( false );

    nextProbe();
}

void ProbeDialog::done( int code )
{
    if ( code == QDialog::Rejected && m_currentMonitor ) {
        setLabel( "Probe aborted. Waiting for job to finish..." );
        m_canceled = true;
        return;
    }
    KProgressDialog::done( code );
}

void ProbeDialog::probeOne()
{
    if ( m_probeOIDs.isEmpty() ) {
        accept();
        return;
    }

    Identifier oid = m_probeOIDs.pop();

    delete m_currentMonitor;
    m_currentMonitor = new Monitor( m_host, oid, 0 /* no refresh */, this );

    connect( m_currentMonitor, SIGNAL( newData( const Identifier &, const Value & ) ),
             this, SLOT( probeResult( const Identifier &, const Value & ) ) );
    connect( m_currentMonitor, SIGNAL( error( const Identifier &, const ErrorInfo & ) ),
             this, SLOT( probeError( const Identifier &, const ErrorInfo & ) ) );
}

void ProbeDialog::probeResult( const Identifier &oid, const Value &value )
{
    if ( !m_canceled )
        m_results << ProbeResult( oid, value );

    nextProbe();
}

void ProbeDialog::probeError( const Identifier &oid, const ErrorInfo &errorInfo )
{
    if ( !m_canceled )
        m_results << ProbeResult( oid, errorInfo );

    nextProbe();
}

void ProbeDialog::nextProbe()
{
    progressBar()->setProgress( progressBar()->totalSteps() - m_probeOIDs.count() );

    if ( m_canceled )
        KProgressDialog::done( QDialog::Rejected );
    else
        QTimer::singleShot( 0, this, SLOT( probeOne() ) );
}

#include "probedialog.moc"

/* vim: et sw=4 ts=4
 */
