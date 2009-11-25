//
//  kjotsbookmarks
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

#ifndef KJOTSBOOKMARKS
#define KJOTSBOOKMARKS

#include <kbookmarkmanager.h>

class KJotsMain;

class KJotsBookmarks : public QObject, public KBookmarkOwner
{
    Q_OBJECT

    public:
        KJotsBookmarks(KJotsMain* parent = 0, const char* name = 0);
        ~KJotsBookmarks();

        virtual void openBookmarkURL(const QString& url);
        virtual QString currentURL() const;
        QString currentTitle() const;

    signals:
        void openPage(const QString& page);

    private:
        KJotsMain* m_kjots;
};


#endif
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
