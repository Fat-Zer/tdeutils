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

#include "monitordialog.h"
#include "identifier.h"
#include "browsedialog.h"

#include <qcombobox.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qlabel.h>

#include <klocale.h>
#include <kpushbutton.h>

#include <assert.h>

using namespace KSim::Snmp;

MonitorDialog::MonitorDialog( const HostConfigMap &hosts, QWidget *parent, const char *name )
    : MonitorDialogBase( parent, name ), m_hosts( hosts )
{
    init();
}

MonitorDialog::MonitorDialog( const MonitorConfig &monitor, const HostConfigMap &hosts, QWidget *parent, const char *name )
    : MonitorDialogBase( parent, name ), m_hosts( hosts )
{
    init( monitor );
}

MonitorConfig MonitorDialog::monitorConfig() const
{
    MonitorConfig result;

    result.host = currentHost();
    if ( result.host.isNull() )
        return MonitorConfig();

    result.name = monitorName->text();
    if ( result.name.isEmpty() )
        return MonitorConfig();

    result.oid = oid->text();
    if ( Identifier::fromString( result.oid ).isNull() )
        return MonitorConfig();

    result.refreshInterval.minutes = updateIntervalMinutes->value();
    result.refreshInterval.seconds = updateIntervalSeconds->value();

    result.display = stringToMonitorDisplayType( displayType->currentText() );

    if ( result.display == MonitorConfig::Label ) {
        result.useCustomFormatString = customFormatStringCheckBox->isChecked();
        if ( result.useCustomFormatString )
            result.customFormatString = customFormatString->text();
    } else
        result.displayCurrentValueInline = displayCurrentValueInline->isChecked();

    return result;
}

void MonitorDialog::checkValues()
{
    bool currentDisplayTypeIsLabel = stringToMonitorDisplayType( displayType->currentText() ) == MonitorConfig::Label;
    labelOptions->setEnabled( currentDisplayTypeIsLabel );
    chartOptions->setEnabled( !currentDisplayTypeIsLabel );

    QString statusText;

    if ( monitorName->text().isEmpty() )
        statusText = i18n( "Please enter a name for this monitor" );

    if ( statusText.isEmpty() && Identifier::fromString( oid->text() ).isNull() )
        statusText = i18n( "Please enter a valid name for the object identifier" );

    if ( statusText.isEmpty() )
        statusText = " "; // a space to keep the height

    status->setText( statusText );

    buttonOk->setEnabled( !monitorConfig().isNull() );
}

void MonitorDialog::testObject()
{
    // ### FIXME
}

void MonitorDialog::browse()
{
    HostConfig host = currentHost();
    if ( host.isNull() )
        return;

    BrowseDialog dlg( host, oid->text(), this );
    if ( dlg.exec() )
        oid->setText( dlg.selectedObjectIdentifier() );
}

void MonitorDialog::init( const MonitorConfig &monitor )
{
    customFormatString->setText( "%s" );

    status->setText( " " );

    host->clear();

    QStringList allHosts;
    for ( HostConfigMap::ConstIterator it = m_hosts.begin(); it != m_hosts.end(); ++it )
        allHosts << it.key();

    host->insertStringList( allHosts );

    displayType->clear();
    displayType->insertStringList( allDisplayTypes() );

    if ( monitor.isNull() ) {
        checkValues();
        return;
    }

    monitorName->setText( monitor.name );

    assert( allHosts.findIndex( monitor.host.name ) != -1 );

    host->setCurrentItem( allHosts.findIndex( monitor.host.name ) );

    oid->setText( monitor.oid );

    updateIntervalMinutes->setValue( monitor.refreshInterval.minutes );
    updateIntervalSeconds->setValue( monitor.refreshInterval.seconds );

    displayType->setCurrentItem( allDisplayTypes().findIndex( monitorDisplayTypeToString( monitor.display ) ) );

    customFormatStringCheckBox->setChecked( monitor.useCustomFormatString );
    customFormatString->setEnabled( monitor.useCustomFormatString );

    if ( monitor.useCustomFormatString )
        customFormatString->setText( monitor.customFormatString );

    displayCurrentValueInline->setChecked( monitor.displayCurrentValueInline );

    checkValues();
}

HostConfig MonitorDialog::currentHost() const
{
    HostConfigMap::ConstIterator hostIt = m_hosts.find( host->currentText() );
    if ( hostIt == m_hosts.end() )
        return HostConfig();

    return *hostIt;
}

#include "monitordialog.moc"
/* vim: et sw=4 ts=4
 */
