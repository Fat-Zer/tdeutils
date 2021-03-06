/*

 ark -- archiver for the KDE project

 Copyright (C)

 2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
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

#include <config.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/errno.h>
#include <string.h>

// QT includes
#include <tqfile.h>
#include <tqdir.h>

// KDE includes
#include <kdebug.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kprocess.h>

// ark includes
#include "settings.h"
#include "arkwidget.h"
#include "arch.h"
#include "zoo.h"
#include "arkutils.h"
#include "filelistview.h"

static TQString fixTime( const TQString &_strTime );

ZooArch::ZooArch( ArkWidget *gui, const TQString & fileName )
  : Arch( gui, fileName )
{
  m_archiver_program = m_unarchiver_program = "zoo";
  verifyCompressUtilityIsAvailable( m_archiver_program );
  verifyUncompressUtilityIsAvailable( m_unarchiver_program );

  m_headerString = "----";
}

bool ZooArch::processLine( const TQCString &line )
{
  const char *_line = ( const char * )line;
  char columns[11][80];
  char filename[4096];

  // Note: I'm reversing the ratio and the length for better display

  sscanf( _line,
          " %79[0-9] %79[0-9%] %79[0-9] %79[0-9] %79[a-zA-Z] %79[0-9]%79[ ]%11[ 0-9:+-]%2[C ]%4095[^\n]",
          columns[1], columns[0], columns[2], columns[3], columns[7],
          columns[8], columns[9], columns[4], columns[10], filename );

  TQString year = ArkUtils::fixYear( columns[8] );

  TQString strDate;
  strDate.sprintf( "%s-%.2d-%.2d", year.utf8().data(),
                   ArkUtils::getMonth( columns[7] ), atoi( columns[3] ) );

  strlcpy( columns[3], strDate.ascii(), sizeof( columns[3]) );
  kdDebug( 1601 ) << "New timestamp is " << columns[3] << endl;

  strlcat( columns[3], " ", sizeof( columns[3] ) );
  strlcat( columns[3], fixTime( columns[4] ).ascii(), sizeof( columns[3] ) );

  TQStringList list;
  list.append( TQFile::decodeName( filename ) );

  for ( int i=0; i<4; i++ )
  {
    list.append( TQString::fromLocal8Bit( columns[i] ) );
  }

  m_gui->fileList()->addItem( list ); // send to GUI

  return true;
}

void ZooArch::open()
{
  setHeaders();

  m_buffer = "";
  m_header_removed = false;
  m_finished = false;


  TDEProcess *kp = m_currentProcess = new TDEProcess;
  *kp << m_archiver_program << "l" << m_filename;
  connect( kp, TQT_SIGNAL( receivedStdout(TDEProcess*, char*, int) ),
           TQT_SLOT( slotReceivedTOC(TDEProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( receivedStderr(TDEProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( processExited(TDEProcess*) ),
           TQT_SLOT( slotOpenExited(TDEProcess*) ) );

  if ( !kp->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigOpen( this, false, TQString(), 0 );
  }
}

void ZooArch::setHeaders()
{
  ColumnList list;
  list.append( FILENAME_COLUMN );
  list.append( RATIO_COLUMN );
  list.append( SIZE_COLUMN );
  list.append( PACKED_COLUMN );
  list.append( TIMESTAMP_COLUMN );

  emit headers( list );
}


void ZooArch::create()
{
  emit sigCreate( this, true, m_filename,
                  Arch::Extract | Arch::Delete | Arch::Add | Arch::View);
}

void ZooArch::addDir( const TQString & _dirName )
{
  if ( ! _dirName.isEmpty() )
  {
    TQStringList list;
    list.append( _dirName );
    addFile( list );
  }
}

void ZooArch::addFile( const TQStringList &urls )
{
  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();
  *kp << m_archiver_program;

  if ( ArkSettings::replaceOnlyWithNewer() )
    *kp << "-update";
  else
    *kp << "-add";

  *kp << m_filename;

  KURL url( urls.first() );
  TQDir::setCurrent( url.directory() );

  TQStringList::ConstIterator iter;

  for ( iter = urls.begin(); iter != urls.end(); ++iter )
  {
    KURL fileURL( *iter );
    *kp << fileURL.fileName();
  }

  connect( kp, TQT_SIGNAL( receivedStdout(TDEProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( receivedStderr(TDEProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( processExited(TDEProcess*) ),
           TQT_SLOT( slotAddExited(TDEProcess*) ) );

  if ( !kp->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigAdd( false );
  }
}

void ZooArch::unarchFileInternal()
{
  // if _fileList is empty, we extract all.
  // if _destDir is empty, abort with error.

  if ( m_destDir.isEmpty() || m_destDir.isNull() )
  {
    kdError( 1601 ) << "There was no extract directory given." << endl;
    return;
  }

  // zoo has no option to specify the destination directory
  // so we have to change to it.

  bool ret = TQDir::setCurrent( m_destDir );
  // We already checked the validity of the dir before coming here
  Q_ASSERT(ret);

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  *kp << m_archiver_program;

  if ( ArkSettings::extractOverwrite() )
  {
    *kp << "xOOS";
  }
  else
  {
    *kp << "x";
  }

  *kp << m_filename;

  // if the list is empty, no filenames go on the command line,
  // and we then extract everything in the archive.
  if (m_fileList)
  {
    TQStringList::Iterator it;
    for ( it = m_fileList->begin(); it != m_fileList->end(); ++it )
    {
      *kp << (*it);
    }
  }

  connect( kp, TQT_SIGNAL( receivedStdout(TDEProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( receivedStderr(TDEProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( processExited(TDEProcess*) ),
           TQT_SLOT( slotExtractExited(TDEProcess*) ) );

  if ( !kp->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigExtract( false );
  }
}

void ZooArch::remove( TQStringList *list )
{
  if (!list)
    return;

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  *kp << m_archiver_program << "D" << m_filename;

  TQStringList::Iterator it;
  for ( it = list->begin(); it != list->end(); ++it )
  {
    TQString str = *it;
    *kp << str;
  }

  connect( kp, TQT_SIGNAL( receivedStdout(TDEProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( receivedStderr(TDEProcess*, char*, int) ),
           TQT_SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, TQT_SIGNAL( processExited(TDEProcess*) ),
           TQT_SLOT( slotDeleteExited(TDEProcess*) ) );

  if ( !kp->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigDelete( false );
  }
}

TQString fixTime( const TQString &_strTime )
{
  // it may have come from a different time zone... get rid of trailing
  // +3 or -3 etc.
  TQString strTime = _strTime;

  if ( strTime.contains("+") || strTime.contains("-") )
  {
    TQCharRef c = strTime.at( 8 );
    int offset = strTime.right( strTime.length() - 9 ).toInt();
    TQString strHour = strTime.left( 2 );
    int nHour = strHour.toInt();
    if ( c == '+' || c == '-' )
    {
      if ( c == '+' )
        nHour = ( nHour + offset ) % 24;
      else if ( c == '-' )
      {
        nHour -= offset;
        if ( nHour < 0 )
          nHour += 24;
      }
      strTime = strTime.left( 8 );
      strTime.sprintf( "%2.2d%s", nHour, strTime.right( 6 ).utf8().data() );
    }
  }
  else
  {
    strTime = strTime.left( 8 );
  }
  return strTime;
}

#include "zoo.moc"
