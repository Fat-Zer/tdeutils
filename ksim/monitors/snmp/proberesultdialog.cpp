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

#include "proberesultdialog.h"

#include <qlabel.h>

#include <klocale.h>
#include <klistview.h>

using namespace KSim::Snmp;

ProbeResultDialog::ProbeResultDialog( const HostConfig &hostConfig, const ProbeDialog::ProbeResultList &probeResults, 
                                      QWidget *parent, const char *name )
        : ProbeResultDialogBase( parent, name )
{
    info->setText( i18n( "Results of scanning host %1:" ).arg( hostConfig.name ) );

    for ( ProbeDialog::ProbeResultList::ConstIterator it = probeResults.begin();
          it != probeResults.end(); ++it )
        addResultItem( *it );
}

void ProbeResultDialog::addResultItem( const ProbeDialog::ProbeResult &result )
{
    QListViewItem *item = new QListViewItem( probeResultView );

    item->setText( 0, result.oid.toString() );

    if ( result.success )
        item->setText( 1, result.value.toString( Value::TimeTicksWithSeconds ) );
    else
        item->setText( 1, QString( "ERROR: " ) + result.errorInfo.errorMessage() );
}

#include "proberesultdialog.moc"

/* vim: et sw=4 ts=4
 */
