//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld
//  Copyright (C) 2002, 2003 Aaron J. Seigo
//  Copyright (C) 2003 Stanislav Kljuhhin
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//

#ifndef __KJOTSEDIT_H
#define __KJOTSEDIT_H

#include <keditcl.h>
class KPopupMenu;
class KJotsPage;

class KJotsEdit : public KEdit
{
    Q_OBJECT
    public:
        KJotsEdit (QWidget* parent=0, const char* name=0);
        ~KJotsEdit ();

        void print(QString title = QString::null);
        void setEntry(KJotsPage*);

    signals:
        void findSuccessful();

    protected slots:
          void openUrl();

    protected:
        virtual void mousePressEvent (QMouseEvent *e);

        KPopupMenu *web_menu;
        KJotsPage *m_entry; //!< The entry we are editing. It needs to be kept informed.
};

#endif // __KJOTSEDIT_H
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
