/*

 ark -- archiver for the KDE project

 Copyright (C)

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

// Std includes
#include <sys/errno.h>
#include <unistd.h>
#include <iostream>
#include <string>

// QT includes
#include <tqfile.h>
#include <tqdir.h>
#include <textcodec.h>

// KDE includes
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>

// ark includes
#include <config.h>
#include "arkwidget.h"
#include "arch.h"
#include "settings.h"
#include "rar.h"
#include "arkutils.h"
#include "filelistview.h"

RarArch::RarArch( ArkWidget *_gui, const TQString & _fileName )
  : Arch( _gui, _fileName )
{
  // Check if rar is available
  bool have_rar = !KGlobal::dirs()->findExe( "rar" ).isNull();
  bool have_unrar = !KGlobal::dirs()->findExe( "unrar" ).isNull();
  bool have_unrar_free = !KGlobal::dirs()->findExe( "unrar-free" ).isNull();

  if ( have_rar )
  {
    // If it is, then use it as archiver and unarchiver
    m_archiver_program = m_unarchiver_program = "rar";
    verifyCompressUtilityIsAvailable( m_archiver_program );
    verifyUncompressUtilityIsAvailable( m_unarchiver_program );
  }
  else if (have_unrar)
  {
    // If rar is not available, try to use unrar to open the archive read-only
    m_unarchiver_program = "unrar";
    verifyUncompressUtilityIsAvailable( m_unarchiver_program );
    setReadOnly( true );
  }
  else
  {
    // If rar is not available, try to use unrar to open the archive read-only
    m_unarchiver_program = "unrar-free";
    verifyUncompressUtilityIsAvailable( m_unarchiver_program );
    setReadOnly( true );
  }



  m_headerString = "-------------------------------------------------------------------------------";

  m_isFirstLine = true;
}

bool RarArch::processLine( const TQCString &line )
{
  TQString unicode_line;

  TQTextCodec *codec = TQTextCodec::codecForLocale();
  unicode_line = codec->toUnicode( line );

  if ( m_isFirstLine )
  {
    m_entryFilename = TQString::fromLocal8Bit( line );
    m_entryFilename.remove( 0, 1 );
    m_isFirstLine = false;
    return true;
  }

  TQStringList list;

  TQStringList l2 = TQStringList::split( ' ', line );

  list << m_entryFilename; // filename
  list << l2[ 0 ]; // size
  list << l2[ 1 ]; // packed
  list << l2[ 2 ]; // ratio

  TQStringList date =  TQStringList::split( '-', l2[ 3 ] );
  list << ArkUtils::fixYear( date[ 2 ].latin1() ) + '-' + date[ 1 ] + '-' + date [ 0 ] + ' ' + l2[4]; // date
  list << l2[ 5 ]; // attributes
  list << l2[ 6 ]; // crc
  list << l2[ 7 ]; // method
  list << l2[ 8 ]; // Version

  m_gui->fileList()->addItem( list ); // send to GUI

  m_isFirstLine = true;
  return true;
}

void RarArch::open()
{
  setHeaders();

  m_buffer = "";
  m_header_removed = false;
  m_finished = false;

  KProcess *kp = m_currentProcess = new KProcess;
  *kp << m_unarchiver_program << "v" << "-c-";

  if ( !m_password.isEmpty() )
    *kp << "-p" + m_password;
  else
    *kp << "-p-";

  *kp << m_filename;

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

void RarArch::setHeaders()
{
  ColumnList list;
  list.append( FILENAME_COLUMN );
  list.append( SIZE_COLUMN );
  list.append( PACKED_COLUMN );
  list.append( RATIO_COLUMN );
  list.append( TIMESTAMP_COLUMN );
  list.append( PERMISSION_COLUMN );
  list.append( CRC_COLUMN );
  list.append( METHOD_COLUMN );
  list.append( VERSION_COLUMN );

  emit headers( list );
}

void RarArch::create()
{
  emit sigCreate( this, true, m_filename,
                  Arch::Extract | Arch::Delete | Arch::Add | Arch::View );
}

void RarArch::addDir( const TQString & _dirName )
{
  if ( !_dirName.isEmpty() )
  {
    TQStringList list;
    list.append( _dirName );
    addFile( list );
  }
}

void RarArch::addFile( const TQStringList & urls )
{
  KProcess *kp = m_currentProcess = new KProcess;

  kp->clearArguments();
  *kp << m_archiver_program;

  if ( ArkSettings::replaceOnlyWithNewer() )
    *kp << "u";
  else
    *kp << "a";

  if ( ArkSettings::rarStoreSymlinks() )
    *kp << "-ol";
  if ( ArkSettings::rarRecurseSubdirs() )
    *kp << "-r";

  *kp << m_filename;

  KURL dir( urls.first() );
  TQDir::setCurrent( dir.directory() );

  TQStringList::ConstIterator iter;
  for ( iter = urls.begin(); iter != urls.end(); ++iter )
  {
    KURL url( *iter );
    *kp << url.fileName();
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

void RarArch::unarchFileInternal()
{
  if ( m_destDir.isEmpty() || m_destDir.isNull() )
  {
    kdError( 1601 ) << "There was no extract directory given." << endl;
    return;
  }

  KProcess *kp = m_currentProcess = new KProcess;
  kp->clearArguments();

  // extract (and maybe overwrite)
  *kp << m_unarchiver_program << "x";

  if ( !m_password.isEmpty() )
    *kp << "-p" + m_password;
  else
    *kp << "-p-";

  if ( !ArkSettings::extractOverwrite() )
  {
    *kp << "-o+";
  }
  else
  {
    *kp << "-o-";
  }

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

  *kp << m_destDir ;

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

bool RarArch::passwordRequired()
{
    return m_lastShellOutput.findRev("password incorrect ?)")+1;
}

void RarArch::remove( TQStringList *list )
{
  if ( !list )
    return;

  KProcess *kp = m_currentProcess = new KProcess;
  kp->clearArguments();

  *kp << m_archiver_program << "d" << m_filename;

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

#include "rar.moc"
