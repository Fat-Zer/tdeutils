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

#ifndef PROBERESULTDIALOG_H
#define PROBERESULTDIALOG_H

#include "proberesultdialogbase.h"

#include "probedialog.h"

namespace KSim
{

namespace Snmp
{

class ProbeResultDialog : public ProbeResultDialogBase
{
    Q_OBJECT
public:
    ProbeResultDialog( const HostConfig &hostConfig, const ProbeDialog::ProbeResultList &probeResults,
                       QWidget *parent, const char *name = 0 );

private:
    void addResultItem( const ProbeDialog::ProbeResult &result );
};

}

}
#endif // PROBERESULTDIALOG_H
/* vim: et sw=4 ts=4
 */
