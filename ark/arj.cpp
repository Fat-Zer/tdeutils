/*

 ark -- archiver for the KDE project

 Copyright (C)

 1997-1999: Rob Palmbos palm9744@kettering.edu
 1999: Francois-Xavier Duranceau duranceau@kde.org
 1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
 2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)
 2007: ALT Linux (author: Sergey V Turchin, zerg@altlinux.org)

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


// Qt includes
#include <tqdir.h>
#include <tqtextcodec.h>

// KDE includes
#include <kdebug.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kprocess.h>
#include <kpassdlg.h>

// ark includes
#include "arj.h"
#include "arkwidget.h"
#include "settings.h"
#include "artdeutils.h"
#include "filelistview.h"

ArjArch::ArjArch( ArkWidget *_gui, const TQString & _fileName )
  : Arch(  _gui, _fileName )
{
  m_archiver_program = "arj";
  m_unarchiver_program = "arj";
  verifyCompressUtilityIsAvailable( m_archiver_program );
  verifyUncompressUtilityIsAvailable( m_unarchiver_program );

  m_headerString = "-----------";
  m_numCols = 6;
}

void ArjArch::setHeaders()
{
  ColumnList list;
  list.append( FILENAME_COLUMN );
  list.append( SIZE_COLUMN );
  list.append( PACKED_COLUMN );
  list.append( RATIO_COLUMN );
  list.append( TIMESTAMP_COLUMN );
  list.append( PERMISSION_COLUMN );

  emit headers( list );
}

void ArjArch::create()
{
  emit sigCreate( this, true, m_filename,
                  Arch::Extract | Arch::Delete | Arch::Add | Arch::View );
}

void ArjArch::createPassword()
{
  if( m_password.isEmpty() && ArkSettings::askCreatePassword() )
    KPasswordDialog::getNewPassword( m_password, i18n("Warning!\nUsing KGpg for encryption is more secure.\nCancel this dialog or enter password for %1 archiver:").arg(m_archiver_program) );
}


void ArjArch::addDir( const TQString & _dirName )
{
  if ( !_dirName.isEmpty() )
  {
    TQStringList list;
    list.append( _dirName );
    addFile( list );
  }
}

void ArjArch::addFile( const TQStringList & urls )
{
  TDEProcess *kp = m_currentProcess = new TDEProcess;

  kp->clearArguments();
  *kp << m_archiver_program;
  *kp << "a";

  if ( ArkSettings::replaceOnlyWithNewer() )
    *kp << "-u";

  if ( ArkSettings::rarRecurseSubdirs() )
    *kp << "-r";

  if ( !m_password.isEmpty() )
    *kp << "-g"+m_password;

  *kp << m_filename;

  KURL dir( urls.first() );
  TQDir::setCurrent( dir.directory() );

  TQStringList::ConstIterator iter;
  for ( iter = urls.begin(); iter != urls.end(); ++iter )
  {
    KURL url( *iter );
    *kp << url.fileName();
  }

  connect( kp, SIGNAL( receivedStdout(TDEProcess*, char*, int) ),
           SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( receivedStderr(TDEProcess*, char*, int) ),
           SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( processExited(TDEProcess*) ),
           SLOT( slotAddExited(TDEProcess*) ) );

  if ( !kp->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigAdd( false );
  }
}

bool ArjArch::processLine( const TQCString &line )
{
  TQString unicode_line;

  TQTextCodec *codec = TQTextCodec::codecForLocale();
  TQTextCodec *codec_alt = TQTextCodec::codecForName("CP1251");
  unicode_line = codec->toUnicode( line );

  TQStringList list;

  TQStringList l2 = TQStringList::split( ' ', line );
  if( l2.size() >= 2 && l2[0].endsWith(")") && l2[0].length() == 4 )
  {
    file_entry = line.mid(4);
  }
  else if( l2.size() > 3 )
  {
    if( l2[1] == "UNIX" )
	list << codec->toUnicode(file_entry).stripWhiteSpace(); // filename
    else
	list << codec_alt->toUnicode(file_entry).stripWhiteSpace(); // filename

    list << l2[ 2 ]; // size
    list << l2[ 3 ]; // packed
    double ratio = l2[4].toDouble();
    if( ratio == 0 )
	ratio = 1;
    list << TQString("%1").arg(100-100/ratio); // ratio

    TQStringList date =  TQStringList::split( '-', l2[ 5 ] );
    list << ArkUtils::fixYear( date[ 0 ].latin1() ) + '-' + date[ 1 ] + '-' + date [ 2 ] + ' ' + l2[6]; // date
    list << l2[ 7 ]; // attributes

    m_gui->fileList()->addItem( list ); // send to GUI
    
    file_entry = "";
  }

  return true;
}


void ArjArch::open()
{
  setHeaders();

  m_buffer = "";
  m_header_removed = false;
  m_finished = false;

  TDEProcess *kp = m_currentProcess = new TDEProcess;

  *kp << m_unarchiver_program << "v" << m_filename;

  connect( kp, SIGNAL( receivedStdout(TDEProcess*, char*, int) ),
           SLOT( slotReceivedTOC(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( receivedStderr(TDEProcess*, char*, int) ),
           SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( processExited(TDEProcess*) ),
           SLOT( slotOpenExited(TDEProcess*) ) );

  if ( !kp->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigOpen( this, false, TQString::null, 0 );
  }
}

void ArjArch::unarchFileInternal()
{
  // if fileList is empty, all files are extracted.
  // if destDir is empty, abort with error.
  if ( m_destDir.isEmpty() || m_destDir.isNull() )
  {
    kdError( 1601 ) << "There was no extract directory given." << endl;
    return;
  }

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  *kp << m_unarchiver_program;
  *kp << "x";

  if ( !m_password.isEmpty() )
    *kp << "-g" + m_password;

  if ( ArkSettings::extractOverwrite() )
    *kp << "-jyo";

  *kp << "-jycv";

  *kp << "-w" + m_destDir;
  *kp << "-ht" + m_destDir;

  TQTextCodec *codec = TQTextCodec::codecForLocale();
  *kp << codec->fromUnicode(m_filename);

  // if the list is empty, no filenames go on the command line,
  // and we then extract everything in the archive.
  if ( m_fileList )
  {
    TQStringList::Iterator it;

    for ( it = m_fileList->begin(); it != m_fileList->end(); ++it )
    {
      *kp << codec->fromUnicode(*it);
    }
  }

  connect( kp, SIGNAL( receivedStdout(TDEProcess*, char*, int) ),
           SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( receivedStderr(TDEProcess*, char*, int) ),
           SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( processExited(TDEProcess*) ),
           SLOT( slotExtractExited(TDEProcess*) ) );

  if ( !kp->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigExtract( false );
  }
}

bool ArjArch::passwordRequired()
{
    return m_lastShellOutput.findRev("File is password encrypted") != -1;
}

void ArjArch::remove( TQStringList *list )
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

  connect( kp, SIGNAL( receivedStdout(TDEProcess*, char*, int) ),
           SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( receivedStderr(TDEProcess*, char*, int) ),
           SLOT( slotReceivedOutput(TDEProcess*, char*, int) ) );
  connect( kp, SIGNAL( processExited(TDEProcess*) ),
           SLOT( slotDeleteExited(TDEProcess*) ) );

  if ( !kp->start( TDEProcess::NotifyOnExit, TDEProcess::AllOutput ) )
  {
    KMessageBox::error( 0, i18n( "Could not start a subprocess." ) );
    emit sigDelete( false );
  }
}

void ArjArch::test()
{
  clearShellOutput();

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  *kp << m_unarchiver_program << "t";

  if ( !m_password.isEmpty() )
    *kp << "-g" + m_password;

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

#include "arj.moc"
