/*  ksim - a system monitor for kde
 *
 *  Copyright (C) 2001  Robbie Ward <linuxphreak@gmx.co.uk>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "filesystemstats.h"

#include <qglobal.h>
#include <qfile.h>
#include <qstringlist.h>

#include <kdebug.h>

#include <config.h>

#include <sys/types.h>
#include <sys/param.h>

#if defined(HAVE_SYS_STATVFS_H) && !defined(__DragonFly__)
#include <sys/statvfs.h>
#elif defined( HAVE_SYS_STATFS_H )
#include <sys/statfs.h>
#endif
#ifdef HAVE_SYS_VFS_H
#include <sys/vfs.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#ifdef _HPUX_SOURCE
#define _PATH_MOUNTED MNTTAB
#endif
#endif
#ifdef HAVE_SYS_UCRED_H
#include <sys/ucred.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#undef HAVE_MNTENT_H
#define _PATH_MOUNTED MNTTAB
#endif

#include <stdio.h>
#include <unistd.h>

#if defined(HAVE_STATVFS) && !defined(__DragonFly__)
typedef struct statvfs ksim_statfs;
#define ksim_getfsstat getvfsstat
#elif defined( HAVE_STATFS ) || defined( Q_OS_FREEBSD )
typedef struct statfs ksim_statfs;
#define ksim_getfsstat getfsstat
#else
typedef struct // fall back for (possibly) non-supported systems
{
  int f_blocks;
  int f_bfree;
} ksim_statfs;
#endif

int fsystemStats( const char * file, ksim_statfs & stats )
{
#if defined(HAVE_STATVFS) && !defined(__DragonFly__)
  return statvfs( file, &stats );
#elif defined( HAVE_STATFS ) || defined( Q_OS_FREEBSD )
  return statfs( file, &stats );
#else // fall back for (possibly) non-supported systems
  (void)file;

  stats.f_blocks = 0;
  stats.f_bfree = 0;
  return -1;
#endif
}

#ifdef HAVE_SYS_MNTTAB_H
#define USE_MNTENT

typedef struct
{
  const char * mnt_dir;
  const char * mnt_fsname;
  const char * mnt_type;
} ksim_mntent;

FILE * setmntent( const char * mtab, const char * mode )
{
  return fopen( mtab, mode );
}

int endmntent( FILE * file )
{
  return fclose( file );
}

ksim_mntent * ksim_getmntent( FILE * file )
{
  ksim_mntent * entry;
  struct mnttab tab;

  if ( getmntent( file, &tab ) != 0 )
    return NULL;

  entry = new ksim_mntent;
  entry->mnt_dir = tab.mnt_mountp;
  entry->mnt_fsname = tab.mnt_special;
  entry->mnt_type = tab.mnt_fstype;
  return entry;
}

#define delete_mntent( x ) delete x
#elif defined( HAVE_MNTENT_H )
#define USE_MNTENT

// Dummy setup
typedef struct mntent ksim_mntent;
ksim_mntent * ksim_getmntent( FILE * file )
{
  return getmntent( file );
}

#define delete_mntent( x )
#elif defined( HAVE_SYS_UCRED_H ) || defined( HAVE_SYS_MOUNT_H )
#define USE_FSSTAT
#else
#define USE_FAILSAFE
#endif

FilesystemStats::List FilesystemStats::readEntries()
{
  List list;

#ifdef USE_MNTENT
  FILE * fp = setmntent( _PATH_MOUNTED, "r" );
  ksim_mntent * point;

  while ( ( point = ksim_getmntent( fp ) ) != 0 )
  {
    Entry entry;
    entry.dir = point->mnt_dir;
    entry.fsname = point->mnt_fsname;
    entry.type = point->mnt_type;
    list.append( entry );

    delete_mntent( point );
  }

  endmntent( fp );
#endif

#ifdef USE_FSSTAT
  ksim_statfs sfs[32];
  int fs_count;
  if ( ( fs_count = ksim_getfsstat( sfs, sizeof( sfs ), 0 ) ) != -1 )
  {
    for ( int i = 0; i < fs_count; i++ )
    {
      Entry entry;
      entry.dir = sfs[i].f_mntonname;
      entry.fsname = sfs[i].f_mntfromname;

#ifndef __osf__
      entry.type = sfs[i].f_fstypename;
#endif

      list.append( entry );
    }
  }
#endif

#ifdef USE_FAILSAFE
  QFile file( QString::fromLatin1( _PATH_MOUNTED ) );

  if ( !file.open( IO_ReadOnly ) )
    return list;

  QTextStream stream( &file );

  while ( !stream.atEnd() )
  {
    QStringList line = QStringList::split( " ", stream.readLine() );

    Entry entry;
    entry.dir = line[1].stripWhiteSpace();
    entry.fsname = line[0].stripWhiteSpace();
    entry.type = line[2].stripWhiteSpace();
    list.append( entry );
  }
#endif

  return list;
}

bool FilesystemStats::readStats( const QString & mntPoint, int & totalBlocks, int & freeBlocks )
{
  ksim_statfs sysStats;
  if ( fsystemStats( QFile::encodeName( mntPoint ).data(), sysStats ) < 0 )
  {
    kdError() << "While reading filesystem information for " << mntPoint << endl;
    totalBlocks = 0;
    freeBlocks = 0;
  }

  totalBlocks = sysStats.f_blocks;
  freeBlocks = sysStats.f_bfree;

  // Return true if our filesystem is statable
  return totalBlocks > 0;
}
