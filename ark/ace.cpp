/*

  ark -- archiver for the KDE project

  Copyright (C)

  2005: Henrique Pinto <henrique.pinto@kdemail.net>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

#include <tqdir.h>

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>

#include "ace.h"
#include "arkwidget.h"
#include "settings.h"

AceArch::AceArch( ArkWidget *gui, const TQString &filename )
	: Arch( gui, filename )
{
	//m_archiver_program = m_unarchiver_program = "/usr/local/bin/unace";
	m_archiver_program = m_unarchiver_program = "/home/henrique/ArkTest/teste.sh";
	verifyCompressUtilityIsAvailable( m_archiver_program );

	m_headerString = "Date    ³Time ³Packed     ³Size     ³Ratio³File";

	m_repairYear = 5; m_fixMonth = 6; m_fixDay = 7; m_fixTime = 8;
	m_dateCol = 3;
	m_numCols = 5;

	m_archCols.append( new ArchColumns( 7, TQRegExp( "[0-3][0-9]" ), 2 ) ); // Day
	m_archCols.append( new ArchColumns( 6, TQRegExp( "[01][0-9]" ), 2 ) ); // Month
	m_archCols.append( new ArchColumns( 5, TQRegExp( "[0-9][0-9]" ), 4 ) ); // Year
	m_archCols.append( new ArchColumns( 8, TQRegExp( "[0-9:]+" ), 8 ) ); // Time
	m_archCols.append( new ArchColumns( 2, TQRegExp( "[0-9]+" ) ) ); // Compressed Size
	m_archCols.append( new ArchColumns( 1, TQRegExp( "[0-9]+" ) ) ); // Size
	m_archCols.append( new ArchColumns( 9, TQRegExp( "[0-9][0-9]%" ) ) ); // Ratio
	m_archCols.append( new ArchColumns( 0, TQRegExp( "[^\\n]+" ), 4096 ) ); // Name
}

AceArch::~AceArch()
{
}

void AceArch::setHeaders()
{
	ColumnList list;
	list.append( FILENAME_COLUMN  );
	list.append( SIZE_COLUMN      );
	list.append( PACKED_COLUMN    );
	list.append( TIMESTAMP_COLUMN );

	emit headers( list );
}

void AceArch::open()
{
	kdDebug(1601) << "+AceArch::open()" << endl;
	setHeaders();

	m_buffer = "";
	m_header_removed = false;
	m_finished = false;

	KProcess *kp = m_currentProcess = new KProcess;
	*kp << m_archiver_program << "v" << m_filename;
	//kp->setUseShell( true );

	kdDebug() << "AceArch::open(): kp->args(): " << kp->args() << endl;

	connect( kp, TQT_SIGNAL( receivedStdout(KProcess*, char*, int) ),
			 TQT_SLOT( slotReceivedTOC(KProcess*, char*, int) ) );
	connect( kp, TQT_SIGNAL( receivedStderr(KProcess*, char*, int) ),
			 TQT_SLOT( slotReceivedOutput(KProcess*, char*, int) ) );
	connect( kp, TQT_SIGNAL( processExited(KProcess*) ),
			 TQT_SLOT( slotOpenExited(KProcess*) ) );

	connect( kp, TQT_SIGNAL( receivedStdout(KProcess*, char*, int) ),
			 this, TQT_SLOT( catchMeIfYouCan(KProcess*, char*, int) ) );

	if ( !kp->start( KProcess::NotifyOnExit, KProcess::AllOutput ) )
	{
		KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
		emit sigOpen( this, false, TQString(), 0 );
	}
}

void AceArch::create()
{
	emit sigCreate( this, true, m_filename,
	                Arch::Extract | Arch::View );
}

void AceArch::addFile( const TQStringList & urls )
{
}

void AceArch::addDir( const TQString & dirName )
{
}

void AceArch::remove( TQStringList *list )
{
}

void AceArch::unarchFileInternal( )
{
	if ( m_destDir.isEmpty() || m_destDir.isNull() )
	{
		kdError( 1601 ) << "There was no extract directory given." << endl;
		return;
	}

	KProcess *kp = m_currentProcess = new KProcess;
	kp->clearArguments();

	// extract (and maybe overwrite)
	*kp << m_unarchiver_program << "x" << "-y";

	if ( ArkSettings::extractOverwrite() )
	{
		*kp << "-o";
	}

	*kp << m_filename;

	*kp << m_destDir ;

	// if the file list is empty, no filenames go on the command line,
	// and we then extract everything in the archive.
	if ( m_fileList )
	{
		TQStringList::Iterator it;
		for ( it = m_fileList->begin(); it != m_fileList->end(); ++it )
		{
			*kp << (*it);
		}
	}

	connect( kp, TQT_SIGNAL( receivedStdout(KProcess*, char*, int) ),
			 TQT_SLOT( slotReceivedOutput(KProcess*, char*, int) ) );
	connect( kp, TQT_SIGNAL( receivedStderr(KProcess*, char*, int) ),
			 TQT_SLOT( slotReceivedOutput(KProcess*, char*, int) ) );
	connect( kp, TQT_SIGNAL( processExited(KProcess*) ),
			 TQT_SLOT( slotExtractExited(KProcess*) ) );

	if ( !kp->start( KProcess::NotifyOnExit, KProcess::AllOutput ) )
	{
		KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
		emit sigExtract( false );
	}
}

void AceArch::catchMeIfYouCan( KProcess*, char *buffer, int buflen )
{
	TQString myBuf = TQString::tqfromLatin1( buffer, buflen );
	kdDebug(1601) << "	Wololo!:	" << myBuf << endl;
}

#include "ace.moc"
