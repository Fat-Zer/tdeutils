/*

  ark -- archiver for the KDE project

  Copyright (C)

  2004: Henrique Pinto <henrique.pinto@kdemail.net>
  2003: Helio Chissini de Castro <helio@conectiva.com>
  2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)

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
#include <tqtextcodec.h>

#include <tdeglobal.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <kurl.h>
#include <tdemessagebox.h>
#include <kpassdlg.h>
#include <kprocess.h>
#include <kstandarddirs.h>

#include "sevenzip.h"
#include "arkwidget.h"
#include "settings.h"
#include "arkutils.h"
#include "filelistview.h"

SevenZipArch::SevenZipArch( ArkWidget *gui, const TQString &filename )
  : Arch( gui, filename ), m_nameColumnPos( -1 )
{
  // Check if 7z is available
  bool have_7z = !TDEGlobal::dirs()->findExe( "7z" ).isNull();
  // Check if 7za is available
  bool have_7za = !TDEGlobal::dirs()->findExe( "7za" ).isNull();

  if ( have_7z )
    m_archiver_program = m_unarchiver_program = "7z";  // Use 7z
  else if ( have_7za )
    m_archiver_program = m_unarchiver_program = "7za"; // Try 7za
  else
    m_archiver_program = m_unarchiver_program = "7zr";

  verifyCompressUtilityIsAvailable( m_archiver_program );
  verifyUncompressUtilityIsAvailable( m_unarchiver_program );

  m_headerString = "------------------";

  m_repairYear = 5; m_fixMonth = 6; m_fixDay = 7; m_fixTime = 8;
  m_dateCol = 3;
  m_numCols = 5;

  m_archCols.append( new ArchColumns( 5, TQRegExp( "[0-2][0-9][0-9][0-9]" ), 4 ) ); // Year
  m_archCols.append( new ArchColumns( 6, TQRegExp( "[01][0-9]" ), 2 ) ); // Month
  m_archCols.append( new ArchColumns( 7, TQRegExp( "[0-3][0-9]" ), 2 ) ); // Day
  m_archCols.append( new ArchColumns( 8, TQRegExp( "[0-9:]+" ), 8 ) ); // Time
  m_archCols.append( new ArchColumns( 4, TQRegExp( "[^\\s]+" ) ) ); // Attributes
  m_archCols.append( new ArchColumns( 1, TQRegExp( "[0-9]+" ) ) ); // Size
  m_archCols.append( new ArchColumns( 2, TQRegExp( "[0-9]+" ), 64, true ) ); // Compressed Size
}

SevenZipArch::~SevenZipArch()
{
}

void SevenZipArch::setHeaders()
{
  ColumnList columns;
  columns.append( FILENAME_COLUMN );
  columns.append( SIZE_COLUMN );
  columns.append( PACKED_COLUMN );
  columns.append( TIMESTAMP_COLUMN );
  columns.append( PERMISSION_COLUMN );

  emit headers( columns );
}

void SevenZipArch::open()
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

void SevenZipArch::create()
{
  emit sigCreate( this, true, m_filename,
                  Arch::Extract | Arch::Delete | Arch::Add | Arch::View );
}

void SevenZipArch::createPassword()
{
  if( m_password.isEmpty() && ArkSettings::askCreatePassword() )
    KPasswordDialog::getNewPassword( m_password, i18n("Warning!\nUsing KGpg for encryption is more secure.\nCancel this dialog or enter password for %1 archiver:").arg(m_archiver_program) );
}

void SevenZipArch::addFile( const TQStringList & urls )
{
  TDEProcess *kp = m_currentProcess = new TDEProcess;

  kp->clearArguments();
  *kp << m_archiver_program << "a" ;

  if ( !m_password.isEmpty() )
    *kp << "-p" + m_password;

  KURL url( urls.first() );
  TQDir::setCurrent( url.directory() );

  *kp << m_filename;

  TQStringList::ConstIterator iter;
  for ( iter = urls.begin(); iter != urls.end(); ++iter )
  {
    KURL url( *iter );
    *kp << url.fileName();
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

void SevenZipArch::addDir( const TQString & dirName )
{
  if ( !dirName.isEmpty() )
  {
    TQStringList list;
    list.append( dirName );
    addFile( list );
  }
}

bool SevenZipArch::passwordRequired()
{
    return m_lastShellOutput.find("Enter password") >= 0;
}

void SevenZipArch::remove( TQStringList *list )
{
  if ( !list )
    return;

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  *kp << m_archiver_program << "d" << m_filename;

  TQStringList::Iterator it;
  for ( it = list->begin(); it != list->end(); ++it )
  {
    *kp << *it;
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

void SevenZipArch::unarchFileInternal( )
{
  if ( m_destDir.isEmpty() || m_destDir.isNull() )
  {
    kdError( 1601 ) << "There was no extract directory given." << endl;
    return;
  }

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  // extract (and maybe overwrite)
  *kp << m_unarchiver_program << "x";

  if ( ArkSettings::extractOverwrite() )
  {
    //*kp << "-ao";
  }

  // FIXME overwrite existing files created with wrong password
  *kp << "-y";

  if ( !m_password.isEmpty() )
    *kp << "-p" + m_password;

  *kp << m_filename;

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

  *kp << "-o" + m_destDir ;

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

bool SevenZipArch::processLine( const TQCString& _line )
{
  TQString line;
  TQString columns[ 11 ];
  unsigned int pos = 0;
  int strpos, len;

  TQTextCodec *codec = TQTextCodec::codecForLocale();
  line = codec->toUnicode( _line );

  columns[ 0 ] = line.right( line.length() - m_nameColumnPos);
  line.truncate( m_nameColumnPos );

  // Go through our columns, try to pick out data, return silently on failure
  for ( TQPtrListIterator <ArchColumns>col( m_archCols ); col.current(); ++col )
  {
    ArchColumns *curCol = *col;

    strpos = curCol->pattern.search( line, pos );
    len = curCol->pattern.matchedLength();

    if ( ( strpos == -1 ) || ( len > curCol->maxLength ) )
    {
      if ( curCol->optional )
        continue; // More?
      else
      {
        kdDebug(1601) << "processLine failed to match critical column" << endl;
        return false;
      }
    }

    pos = strpos + len;

    columns[ curCol->colRef ] = line.mid( strpos, len );
  }

  // Separated directories pass
  if(columns[4].length() && columns[4][0] == 'D') return true;

  if ( m_dateCol >= 0 )
  {
    TQString year = ( m_repairYear >= 0 ) ?
                   ArkUtils::fixYear( columns[ m_repairYear ].ascii())
                   : columns[ m_fixYear ];
    TQString month = ( m_repairMonth >= 0 ) ?
                   TQString( "%1" )
                   .arg( ArkUtils::getMonth( columns[ m_repairMonth ].ascii() ) )
                   : columns[ m_fixMonth ];
    TQString timestamp = TQString::fromLatin1( "%1-%2-%3 %4" )
                        .arg( year )
                        .arg( month )
                        .arg( columns[ m_fixDay ] )
                        .arg( columns[ m_fixTime ] );

    columns[ m_dateCol ] = timestamp;
  }

  TQStringList list;

  for ( int i = 0; i < m_numCols; ++i )
  {
    list.append( columns[ i ] );
  }

  m_gui->fileList()->addItem( list ); // send the entry to the GUI

  return true;
}

void SevenZipArch::slotReceivedTOC( TDEProcess*, char* data, int length )
{
  char endchar = data[ length ];
  data[ length ] = '\0';

  appendShellOutputData( data );

  int startChar = 0;

  while ( !m_finished )
  {
    int lfChar;
    for ( lfChar = startChar; data[ lfChar ] != '\n' && lfChar < length;
          lfChar++ );

    if ( data[ lfChar ] != '\n')
      break; // We are done all the complete lines

    data[ lfChar ] = '\0';
    m_buffer.append( data + startChar );
    data[ lfChar ] = '\n';
    startChar = lfChar + 1;

    // Check if the header was found
    if ( m_buffer.find( m_headerString.data() ) != -1 )
    {
      if ( !m_header_removed )
      {
        m_nameColumnPos = m_buffer.findRev( ' ' ) + 1;
        m_header_removed = true;
      }
      else
      {
        m_finished = true;
      }
    }
    else
    {
      // If the header was not found, process the line
      if ( m_header_removed && !m_finished )
      {
        if ( !processLine( m_buffer ) )
        {
          m_header_removed = false;
          m_error = true;
        }
      }
    }

    m_buffer.resize( 0 );
  }

  if ( !m_finished )
    m_buffer.append( data + startChar);	// Append what's left of the buffer

  data[ length ] = endchar;
}

void SevenZipArch::test()
{
  clearShellOutput();

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  *kp << m_unarchiver_program << "t";

  if ( !m_password.isEmpty() )
    *kp << "-p" + m_password;

  *kp << m_filename;

  connect( kp, SIGNAL( receivedStdout(TDEProcess*, char*, int) ),
           SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( receivedStderr(TDEProcess*, char*, int) ),
           SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( processExited(TDEProcess*) ),
           SLOT( slotTestExited(TDEProcess*) ) );

  if ( !kp->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigTest( false );
  }
}

#include "sevenzip.moc"
