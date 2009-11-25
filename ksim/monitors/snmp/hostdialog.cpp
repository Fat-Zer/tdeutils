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

#include "hostdialog.h"
#include "probedialog.h"
#include "proberesultdialog.h"

#include <qcombobox.h>
#include <qwidgetstack.h>
#include <klineedit.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qspinbox.h>

#include <klocale.h>
#include <kmessagebox.h>

#include <assert.h>

using namespace KSim::Snmp;

HostDialog::HostDialog( QWidget *parent, const char *name )
    : HostDialogBase( parent, name )
{
    init();
    port->setValue( 161 );
}

HostDialog::HostDialog( const HostConfig &src, QWidget *parent, const char *name )
    : HostDialogBase( parent, name )
{
    init( src );
}

HostConfig HostDialog::settings() const
{
    HostConfig result;

    result.name = hostName->text();
    result.port = port->value();

    result.version = stringToSnmpVersion( snmpVersion->currentText() );
    if ( result.version != SnmpVersion3 ) {
        result.community = communityString->text();
    }

    result.securityName = securityName->text();

    result.securityLevel = stringToSecurityLevel( securityLevel->currentText() );

    if ( result.securityLevel == NoAuthPriv )
        return result;

    result.authentication.protocol = stringToAuthenticationProtocol( authenticationType->currentText() );
    result.authentication.key = authenticationPassphrase->text();

    if ( result.securityLevel == AuthNoPriv )
        return result;

    result.privacy.protocol = stringToPrivacyProtocol( privacyType->currentText() );
    result.privacy.key = privacyPassphrase->text();

    return result;
}

void HostDialog::showSnmpAuthenticationDetailsForVersion( const QString &versionStr )
{
    bool ok = false;
    SnmpVersion version = stringToSnmpVersion( versionStr, &ok );
    assert( ok );

    authenticationDetails->raiseWidget( version == SnmpVersion3 ? snmpV3Page : snmpV1Page );
}

void HostDialog::enableDisabledAuthenticationAndPrivacyElementsForSecurityLevel( const QString &levelStr )
{
    bool ok = false;
    SecurityLevel level = stringToSecurityLevel( levelStr, &ok );
    assert( ok );

    bool enableAuthentication = level != NoAuthPriv;
    bool enablePrivacy = level == AuthPriv;

    authenticationTypeLabel->setEnabled( enableAuthentication );
    authenticationType->setEnabled( enableAuthentication );
    authenticationPassphraseLabel->setEnabled( enableAuthentication );
    authenticationPassphrase->setEnabled( enableAuthentication );

    privacyTypeLabel->setEnabled( enablePrivacy );
    privacyType->setEnabled( enablePrivacy );
    privacyPassphraseLabel->setEnabled( enablePrivacy );
    privacyPassphrase->setEnabled( enablePrivacy );
}

void HostDialog::checkValidity()
{
    bool enableOk = true;
    bool ok = false;
    SnmpVersion version = stringToSnmpVersion( snmpVersion->currentText(), &ok );
    assert( ok );

    enableOk &= !hostName->text().isEmpty();

    if ( version == SnmpVersion3 )
        enableOk &= !securityName->text().isEmpty();
    else
        enableOk &= !communityString->text().isEmpty();

    buttonOk->setEnabled( enableOk );
    testHostButton->setEnabled( enableOk );
}

void HostDialog::testHost()
{
    ProbeDialog dlg( settings(), this );
    if ( dlg.exec() ) {
        ProbeDialog::ProbeResultList results = dlg.probeResults();

        ProbeResultDialog resultDlg( settings(), results, this );
        resultDlg.exec();
    }
}

void HostDialog::init( const HostConfig &src )
{
    // hide these, there's nothing to choose right now. might be that
    // net-snmp will support different privacy types in the future, but
    // apparently not now.
    privacyTypeLabel->hide();
    privacyType->hide();

    snmpVersion->insertStringList( allSnmpVersions() );
    securityLevel->insertStringList( allSecurityLevels() );
    authenticationType->insertStringList( allAuthenticationProtocols() );
    privacyType->insertStringList( allPrivacyProtocols() );

    if ( !src.isNull() )
        loadSettingsFromHostConfig( src );

    showSnmpAuthenticationDetailsForVersion( snmpVersion->currentText() );
    enableDisabledAuthenticationAndPrivacyElementsForSecurityLevel( securityLevel->currentText() );
    checkValidity();
}

void HostDialog::loadSettingsFromHostConfig( const HostConfig &src )
{
    hostName->setText( src.name );
    port->setValue( src.port );

    snmpVersion->setCurrentItem( allSnmpVersions().findIndex( snmpVersionToString( src.version ) ) );

    if ( src.version != SnmpVersion3 ) {
        communityString->setText( src.community );
        return;
    }

    securityName->setText( src.securityName );

    securityLevel->setCurrentItem( allSecurityLevels().findIndex( securityLevelToString( src.securityLevel ) ) );

    if ( src.securityLevel == NoAuthPriv )
        return;

    authenticationType->setCurrentItem( allAuthenticationProtocols().findIndex( authenticationProtocolToString( src.authentication.protocol ) ) );
    authenticationPassphrase->setText( src.authentication.key );

    if ( src.securityLevel == AuthNoPriv )
        return;

    privacyType->setCurrentItem( allPrivacyProtocols().findIndex( privacyProtocolToString( src.privacy.protocol ) ) );
    privacyPassphrase->setText( src.privacy.key );
}

#include "hostdialog.moc"

/* vim: et sw=4 ts=4
 */
