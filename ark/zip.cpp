/*

 ark -- archiver for the KDE project

 Copyright (C)

 1997-1999: Rob Palmbos palm9744@kettering.edu
 1999: Francois-Xavier Duranceau duranceau@kde.org
 1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
 2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)

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


// TQt includes
#include <tqdir.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>

// ark includes
#include "zip.h"
#include "arkwidget.h"
#include "settings.h"


ZipArch::ZipArch( ArkWidget *_gui, const TQString & _fileName )
  : Arch(  _gui, _fileName )
{
  m_archiver_program = "zip";
  m_unarchiver_program = "unzip";
  verifyCompressUtilityIsAvailable( m_archiver_program );
  verifyUncompressUtilityIsAvailable( m_unarchiver_program );

  m_headerString = "----";
  m_repairYear = 9; m_fixMonth = 7; m_fixDay = 8; m_fixTime = 10;
  m_dateCol = 5;
  m_numCols = 7;

  m_archCols.append( new ArchColumns( 1, TQRegExp( "[0-9]+" ) ) );
  m_archCols.append( new ArchColumns( 2, TQRegExp( "[^\\s]+" ) ) );
  m_archCols.append( new ArchColumns( 3, TQRegExp( "[0-9]+" ) ) );
  m_archCols.append( new ArchColumns( 4, TQRegExp( "[0-9.]+%" ) ) );
  m_archCols.append( new ArchColumns( 7, TQRegExp( "[01][0-9]" ), 2 ) );
  m_archCols.append( new ArchColumns( 8, TQRegExp( "[0-3][0-9]" ), 2 ) );
  m_archCols.append( new ArchColumns( 9, TQRegExp( "[0-9][0-9]" ), 2 ) );
  m_archCols.append( new ArchColumns( 10, TQRegExp( "[0-9:]+" ), 6 ) );
  m_archCols.append( new ArchColumns( 6, TQRegExp( "[a-fA-F0-9]+ {2}" ) ) );
  m_archCols.append( new ArchColumns( 0, TQRegExp( "[^\\n]+" ), 4096 ) );

}

void ZipArch::setHeaders()
{
  ColumnList list;
  list.append( FILENAME_COLUMN );
  list.append( SIZE_COLUMN );
  list.append( METHOD_COLUMN );
  list.append( PACKED_COLUMN );
  list.append( RATIO_COLUMN );
  list.append( TIMESTAMP_COLUMN );
  list.append( CRC_COLUMN );

  emit headers( list );
}

void ZipArch::open()
{
  setHeaders();

  m_buffer = "";
  m_header_removed = false;
  m_finished = false;

  KProcess *kp = m_currentProcess = new KProcess;

  *kp << m_unarchiver_program << "-v" << m_filename;

  connect( kp, TQT_SIGNAL( receivedStdout(KProcess*, char*, int) ),
           TQT_SLOT( slotReceivedTOC(KProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( receivedStderr(KProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(KProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( processExited(KProcess*) ),
           TQT_SLOT( slotOpenExited(KProcess*) ) );

  if ( !kp->start( KProcess::NotifyOnExit, KProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigOpen( this, false, TQString(), 0 );
  }
}


void ZipArch::create()
{
  emit sigCreate( this, true, m_filename,
                 Arch::Extract | Arch::Delete | Arch::Add | Arch::View );
}

void ZipArch::addDir( const TQString & _dirName )
{
  if ( !_dirName.isEmpty() )
  {
    bool bOldRecVal = ArkSettings::rarRecurseSubdirs();
    // must be true for add directory - otherwise why would user try?
    ArkSettings::setRarRecurseSubdirs( true );

    TQStringList list;
    list.append( _dirName );
    addFile( list );
    ArkSettings::setRarRecurseSubdirs( bOldRecVal ); // reset to old val
  }
}

void ZipArch::addFile( const TQStringList &urls )
{
  KProcess *kp = m_currentProcess = new KProcess;
  kp->clearArguments();

  *kp << m_archiver_program;

  if ( !m_password.isEmpty() )
    *kp << "-P" << m_password;

  if ( ArkSettings::rarRecurseSubdirs() )
    *kp << "-r";

  if ( ArkSettings::rarStoreSymlinks() )
    *kp << "-y";

  if ( ArkSettings::forceMSDOS() )
    *kp << "-k";
  if ( ArkSettings::convertLF2CRLF() )
    *kp << "-l";

  if ( ArkSettings::replaceOnlyWithNewer() )
    *kp << "-u";


  *kp << m_filename;

  TQStringList::ConstIterator iter;
  KURL url( urls.first() );
  TQDir::setCurrent( url.directory() );
  for ( iter = urls.begin(); iter != urls.end(); ++iter )
  {
    KURL fileURL( *iter );
    *kp << fileURL.fileName();
  }

  connect( kp, TQT_SIGNAL( receivedStdout(KProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(KProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( receivedStderr(KProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(KProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( processExited(KProcess*) ),
           TQT_SLOT( slotAddExited(KProcess*) ) );

  if ( !kp->start( KProcess::NotifyOnExit, KProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigAdd( false );
  }
}

void ZipArch::unarchFileInternal()
{
  // if fileList is empty, all files are extracted.
  // if destDir is empty, abort with error.
  if ( m_destDir.isEmpty() || m_destDir.isNull() )
  {
    kdError( 1601 ) << "There was no extract directory given." << endl;
    return;
  }

  KProcess *kp = m_currentProcess = new KProcess;
  kp->clearArguments();

  *kp << m_unarchiver_program;

  if ( !m_password.isEmpty() )
    *kp << "-P" << m_password;

  if ( ArkSettings::extractJunkPaths() && !m_viewFriendly )
    *kp << "-j" ;

  if ( ArkSettings::rarToLower() )
    *kp << "-L";

  if ( ArkSettings::extractOverwrite() )
    *kp << "-o";
  else
    *kp << "-n";

  *kp << m_filename;

  // if the list is empty, no filenames go on the command line,
  // and we then extract everything in the archive.
  if ( m_fileList )
  {
    TQStringList::Iterator it;

    for ( it = m_fileList->begin(); it != m_fileList->end(); ++it )
    {
      *kp << (*it);
    }
  }

  *kp << "-d" << m_destDir;

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

bool ZipArch::passwordRequired()
{
    return m_lastShellOutput.tqfindRev("unable to get password\n")!=-1 || m_lastShellOutput.endsWith("password inflating\n") || m_lastShellOutput.tqfindRev("password incorrect--reenter:")!=-1 || m_lastShellOutput.endsWith("incorrect password\n");
}

void ZipArch::remove( TQStringList *list )
{
  if ( !list )
    return;

  KProcess *kp = m_currentProcess = new KProcess;
  kp->clearArguments();

  *kp << m_archiver_program << "-d" << m_filename;

  TQStringList::Iterator it;
  for ( it = list->begin(); it != list->end(); ++it )
  {
    TQString str = *it;
    *kp << str;
  }

  connect( kp, TQT_SIGNAL( receivedStdout(KProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(KProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( receivedStderr(KProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(KProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( processExited(KProcess*) ),
           TQT_SLOT( slotDeleteExited(KProcess*) ) );

  if ( !kp->start( KProcess::NotifyOnExit, KProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigDelete( false );
  }
}

#include "zip.moc"
