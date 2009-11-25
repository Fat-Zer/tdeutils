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

#ifndef HOSTDIALOG_H
#define HOSTDIALOG_H

#include "hostdialogbase.h"
#include "hostconfig.h"
#include "probedialog.h"

namespace KSim
{

namespace Snmp
{

class HostDialog : public HostDialogBase
{
    Q_OBJECT
public:
    HostDialog( QWidget *parent, const char *name = 0 );
    HostDialog( const HostConfig &src, QWidget *parent, const char *name = 0 );

    HostConfig settings() const;

private slots:
    void showSnmpAuthenticationDetailsForVersion( const QString &versionStr  );
    void enableDisabledAuthenticationAndPrivacyElementsForSecurityLevel( const QString &levelStr );
    void checkValidity();
    void testHost();

private:
    void init( const HostConfig &src = HostConfig() );
    void loadSettingsFromHostConfig( const HostConfig &src );
};

}

}

#endif // HOSTDIALOG_H
/* vim: et sw=4 ts=4
 */
