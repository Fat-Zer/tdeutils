/*
    This file is part of KDE.

    Copyright (c) 2006 Henrique Pinto <henrique.pinto@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,
    USA.
*/


#include <tqstring.h>
#include <tqapplication.h>

#include <karchive.h>
#include <ktar.h>
#include <kdebug.h>

#include "tarlistingthread.h"

// Helper function
// copied from KonqTreeViewItem::makeAccessString()
static char *makeAccessString(mode_t mode)
{
	static char buffer[10];

	char uxbit,gxbit,oxbit;

	if ( (mode & (S_IXUSR|S_ISUID)) == (S_IXUSR|S_ISUID) )
		uxbit = 's';
	else if ( (mode & (S_IXUSR|S_ISUID)) == S_ISUID )
		uxbit = 'S';
	else if ( (mode & (S_IXUSR|S_ISUID)) == S_IXUSR )
		uxbit = 'x';
	else
		uxbit = '-';

	if ( (mode & (S_IXGRP|S_ISGID)) == (S_IXGRP|S_ISGID) )
		gxbit = 's';
	else if ( (mode & (S_IXGRP|S_ISGID)) == S_ISGID )
		gxbit = 'S';
	else if ( (mode & (S_IXGRP|S_ISGID)) == S_IXGRP )
		gxbit = 'x';
	else
		gxbit = '-';

	if ( (mode & (S_IXOTH|S_ISVTX)) == (S_IXOTH|S_ISVTX) )
		oxbit = 't';
	else if ( (mode & (S_IXOTH|S_ISVTX)) == S_ISVTX )
		oxbit = 'T';
	else if ( (mode & (S_IXOTH|S_ISVTX)) == S_IXOTH )
		oxbit = 'x';
	else
		oxbit = '-';

	buffer[0] = ((( mode & S_IRUSR ) == S_IRUSR ) ? 'r' : '-' );
	buffer[1] = ((( mode & S_IWUSR ) == S_IWUSR ) ? 'w' : '-' );
	buffer[2] = uxbit;
	buffer[3] = ((( mode & S_IRGRP ) == S_IRGRP ) ? 'r' : '-' );
	buffer[4] = ((( mode & S_IWGRP ) == S_IWGRP ) ? 'w' : '-' );
	buffer[5] = gxbit;
	buffer[6] = ((( mode & S_IROTH ) == S_IROTH ) ? 'r' : '-' );
	buffer[7] = ((( mode & S_IWOTH ) == S_IWOTH ) ? 'w' : '-' );
	buffer[8] = oxbit;
	buffer[9] = 0;

	return buffer;
}

TarListingThread::TarListingThread( TQObject *parent, const TQString& filename )
	: TQThread(), m_parent( parent ), m_archive(NULL)
{
	Q_ASSERT( m_parent );
	m_archiveFileName = filename;
}

TarListingThread::~TarListingThread()
{
	if (m_archive) {
		delete m_archive;
		m_archive = 0;
	}
}

void TarListingThread::run()
{
	m_archive = new KTar( m_archiveFileName );

	if (!m_archive->open( IO_ReadOnly ))
	{
		ListingEvent *ev = new ListingEvent( TQStringList(), ListingEvent::Error );
		tqApp->postEvent( m_parent, ev );
		return;
	}

	processDir( m_archive->directory(), TQString() );

	// Send an empty TQStringList in an Event to signal the listing end.
	ListingEvent *ev = new ListingEvent( TQStringList(), ListingEvent::ListingFinished );
	tqApp->postEvent( m_parent, ev );
}

void TarListingThread::processDir( const KTarDirectory *tardir, const TQString & root )
{
	TQStringList list = tardir->entries();
	
	TQStringList::const_iterator itEnd = list.constEnd();

	for ( TQStringList::const_iterator it = list.constBegin(); it != itEnd; ++it )
	{
		const KTarEntry* tarEntry = tardir->entry((*it));
		if (!tarEntry)
			continue;

		TQStringList col_list;
		TQString name;
		if (root.isEmpty() || root.isNull())
			name = tarEntry->name();
		else
			name = root + tarEntry->name();
		if ( !tarEntry->isFile() )
			name += '/';
		col_list.append( name );
		TQString perms = makeAccessString(tarEntry->permissions());
		if (!tarEntry->isFile())
			perms = "d" + perms;
		else if (!tarEntry->symlink().isEmpty())
			perms = "l" + perms;
		else
			perms = "-" + perms;
		col_list.append(perms);
		col_list.append( tarEntry->user() );
		col_list.append( tarEntry->group() );
		TQString strSize = "0";
		if (tarEntry->isFile())
		{
			strSize.sprintf("%d", ((KTarFile *)tarEntry)->size());
		}
		col_list.append(strSize);
		TQString timestamp = tarEntry->datetime().toString(Qt::ISODate);
		col_list.append(timestamp);
		col_list.append(tarEntry->symlink());
		
		ListingEvent *ev = new ListingEvent( col_list );
		tqApp->postEvent( m_parent, ev );

		// if it's a directory, process it.
		// remember that name is root + / + the name of the directory
		if ( tarEntry->isDirectory() )
		{
			processDir( static_cast<const KTarDirectory *>( tarEntry ), name );
		}
	}
}
