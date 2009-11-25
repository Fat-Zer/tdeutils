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

#include "KJotsMain.h"
#include "kjotsbookmarks.h"

KJotsBookmarks::KJotsBookmarks(KJotsMain* parent, const char* name)
    : QObject(parent, name),
      m_kjots(parent)
{
}

KJotsBookmarks::~KJotsBookmarks()
{
}

void KJotsBookmarks::openBookmarkURL(const QString& page)
{
    emit openPage(page);
}

QString KJotsBookmarks::currentURL() const
{
    return m_kjots->currentBookmarkURL();
}

QString KJotsBookmarks::currentTitle() const
{
    return m_kjots->currentBookmarkTitle();
}

#include "kjotsbookmarks.moc"
