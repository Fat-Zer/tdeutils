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
#include <tqtextcodec.h>

// KDE includes
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kpassdlg.h>
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
  bool have_rar = !TDEGlobal::dirs()->findExe( "rar" ).isNull();
  bool have_unrar = !TDEGlobal::dirs()->findExe( "unrar" ).isNull();
  bool have_unrar_free = !TDEGlobal::dirs()->findExe( "unrar-free" ).isNull();

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

  if( l2[5].startsWith("d") )
  {
    m_isFirstLine = true;
    return true;
  }

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

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  *kp << m_unarchiver_program << "v" << "-c-";

  if ( !m_password.isEmpty() )
    *kp << "-p" + m_password;
  else
    *kp << "-p-";

  *kp << m_filename;

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

void RarArch::createPassword()
{
  if( m_password.isEmpty() && ArkSettings::askCreatePassword() )
    KPasswordDialog::getNewPassword( m_password, i18n("Warning!\nUsing KGpg for encryption is more secure.\nCancel this dialog or enter password for %1 archiver:").arg(m_archiver_program) );
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
  TDEProcess *kp = m_currentProcess = new TDEProcess;

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

  if ( !m_password.isEmpty() )
    *kp << "-p"+m_password;

  *kp << m_filename;

  KURL dir( urls.first() );
  TQDir::setCurrent( dir.directory() );

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

void RarArch::unarchFileInternal()
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

bool RarArch::passwordRequired()
{
    return m_lastShellOutput.find("Enter password") >= 0;
}

void RarArch::remove( TQStringList *list )
{
  if ( !list )
    return;

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  *kp << m_archiver_program << "d" << m_filename;

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

void RarArch::test()
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

#include "rar.moc"
