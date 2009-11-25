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

#ifndef BROWSEDIALOG_H
#define BROWSEDIALOG_H

#include "browsedialogbase.h"

#include "hostconfig.h"
#include "identifier.h"
#include "value.h"
#include "snmp.h"
#include "walker.h"

#include <qvaluestack.h>

namespace KSim
{

namespace Snmp
{

class BrowseDialog : public BrowseDialogBase
{
    Q_OBJECT
public:
    BrowseDialog( const HostConfig &hostConfig, const QString &currentOid, QWidget *parent, const char *name = 0 );

    QString selectedObjectIdentifier() const;

protected:
    virtual void stopAllWalks();
    virtual void applyFilter();
    virtual void objectSelected( QListViewItem *item );

private slots:
    void insertBrowseItem( const Walker::Result &browseResult );
    void nextWalk();

private:
    void applyFilter( QListViewItem *item );
    void startWalk( const Identifier &startOid );
    void stopWalker();

    HostConfig m_host;
    Walker *m_walker;
    QValueStack<QString> baseOids;
};

}
}

#endif // BROWSEDIALOG_H
/* vim: et sw=4 ts=4
 */
