/*
    ark: A program for modifying archives via a GUI.

    Copyright (C)

    2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
    2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)
    2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

*/

// C includes
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

// TQt includes
#include <tqdir.h>

// KDE includes
#include <kdebug.h>
#include <klargefile.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kstandarddirs.h>
#include <ktempdir.h>
#include <kprocess.h>
#include <kmimetype.h>
#include <tdeio/netaccess.h>
#include <tdeio/global.h>
#include <tdefileitem.h>
#include <tdeapplication.h>
#include <tdeglobal.h>

// ark includes
#include "arkwidget.h"
#include "filelistview.h"
#include "compressedfile.h"

// encapsulates the idea of a compressed file

CompressedFile::CompressedFile( ArkWidget *_gui, const TQString & _fileName, const TQString & _openAsMimeType )
  : Arch( _gui, _fileName )
{
  m_tempDirectory = NULL;
  m_openAsMimeType = _openAsMimeType;
  kdDebug(1601) << "CompressedFile constructor" << endl;
  m_tempDirectory = new KTempDir( _gui->tmpDir()
                          + TQString::fromLatin1( "compressed_file_temp" ) );
  m_tempDirectory->setAutoDelete( true );
  m_tmpdir = m_tempDirectory->name();
  initData();
  verifyCompressUtilityIsAvailable( m_archiver_program );
  verifyUncompressUtilityIsAvailable( m_unarchiver_program );

  if (!TQFile::exists(_fileName))
  {
    KMessageBox::information(0,
              i18n("You are creating a simple compressed archive which contains only one input file.\n"
                  "When uncompressed, the file name will be based on the name of the archive file.\n"
                  "If you add more files you will be prompted to convert it to a real archive."),
              i18n("Simple Compressed Archive"), "CreatingCompressedArchive");
  }
}

CompressedFile::~CompressedFile()
{
    if ( m_tempDirectory )
        delete m_tempDirectory;
}

void CompressedFile::setHeaders()
{
  ColumnList list;
  list.append(FILENAME_COLUMN);
  list.append(PERMISSION_COLUMN);
  list.append(OWNER_COLUMN);
  list.append(GROUP_COLUMN);
  list.append(SIZE_COLUMN);

  emit headers(list);
}

void CompressedFile::initData()
{
    m_unarchiver_program = TQString();
    m_archiver_program = TQString();

    TQString mimeType;
    if ( !m_openAsMimeType.isNull() )
        mimeType = m_openAsMimeType;
    else
        mimeType = KMimeType::findByPath( m_filename )->name();

    if ( mimeType == "application/x-gzip" )
    {
        m_unarchiver_program = "gunzip";
        m_archiver_program = "gzip";
        m_defaultExtensions << ".gz" << "-gz" << ".z" << "-z" << "_z" << ".Z";
    }
    if ( mimeType == "application/x-bzip" )
    {
        m_unarchiver_program = "bunzip";
        m_archiver_program = "bzip";
        m_defaultExtensions << ".bz";
    }
    if ( mimeType == "application/x-bzip2" )
    {
        m_unarchiver_program = "bunzip2";
        m_archiver_program = "bzip2";
        m_defaultExtensions << ".bz2" << ".bz";
    }
    if ( mimeType == "application/x-lzma" )
    {
        m_unarchiver_program = "unlzma";
        m_archiver_program = "lzma";
        m_defaultExtensions << ".lzma";
    }
    if ( mimeType == "application/x-xz" )
    {
        m_unarchiver_program = "unxz";
        m_archiver_program = "xz";
        m_defaultExtensions << ".xz";
    }
    if ( mimeType == "application/x-lzop" )
    { m_unarchiver_program = "lzop";
        m_archiver_program = "lzop";
        m_defaultExtensions << ".lzo";
    }
    if ( mimeType == "application/x-compress" )
    {
        m_unarchiver_program = TDEGlobal::dirs()->findExe( "uncompress" ).isNull()? "gunzip" : "uncompress";
        m_archiver_program   = "compress";
        m_defaultExtensions  = ".Z";
    }

}

TQString CompressedFile::extension()
{
  TQStringList::Iterator it = m_defaultExtensions.begin();
  for( ; it != m_defaultExtensions.end(); ++it )
    if( m_filename.endsWith( *it ) )
        return TQString();
  return m_defaultExtensions.first();
}

void CompressedFile::open()
{
  kdDebug(1601) << "+CompressedFile::open" << endl;
  setHeaders();

  // We copy the file into the temporary directory, uncompress it,
  // and when the uncompression is done, list it
  // (that code is in the slot slotOpenDone)

  m_tmpfile = m_gui->realURL().fileName();
  if ( m_tmpfile.isEmpty() )
    m_tmpfile = m_filename;
  m_tmpfile += extension();
  m_tmpfile = m_tmpdir + m_tmpfile;

  KURL src, target;
  src.setPath( m_filename );
  target.setPath( m_tmpfile );

  TDEIO::NetAccess::copy( src, target, m_gui );
  kdDebug(1601) << "Temp file name is " << target << endl;

  if ( !TDEIO::NetAccess::exists( target, true, NULL ) )
    return;

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();
  *kp << m_unarchiver_program << "-f" ;
  if ( m_unarchiver_program == "lzop")
  {
    *kp << "-d";
    // lzop hack, see comment in tar.cpp createTmp()
    kp->setUsePty( TDEProcess::Stdin, false );
  }
  // gunzip 1.3 seems not to like original names with directories in them
  // testcase: https://listman.redhat.com/pipermail/valhalla-list/2006-October.txt.gz
  /*if ( m_unarchiver_program == "gunzip" )
    *kp << "-N";
  */
  *kp << m_tmpfile;

  kdDebug(1601) << "Command is " << m_unarchiver_program << " " << m_tmpfile<< endl;

  connect( kp, TQT_SIGNAL(receivedStdout(TDEProcess*, char*, int)),
	   this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));
  connect( kp, TQT_SIGNAL(receivedStderr(TDEProcess*, char*, int)),
	   this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));
  connect( kp, TQT_SIGNAL(processExited(TDEProcess*)), this,
	   TQT_SLOT(slotUncompressDone(TDEProcess*)));

  if (kp->start(TDEProcess::NotifyOnExit, TDEProcess::AllOutput) == false)
    {
      KMessageBox::error( 0, i18n("Could not start a subprocess.") );
      emit sigOpen(this, false, TQString(), 0 );
    }

  kdDebug(1601) << "-CompressedFile::open" << endl;
}

void CompressedFile::slotUncompressDone(TDEProcess *_kp)
{
  bool bSuccess = false;
  kdDebug(1601) << "normalExit = " << _kp->normalExit() << endl;
  if( _kp->normalExit() )
    kdDebug(1601) << "exitStatus = " << _kp->exitStatus() << endl;

  if( _kp->normalExit() && (_kp->exitStatus()==0) )
  {
    bSuccess = true;
  }

  delete _kp;
  _kp = m_currentProcess = 0;

  if ( !bSuccess )
  {
      emit sigOpen( this, false, TQString(), 0 );
      return;
  }

  TQDir dir( m_tmpdir );
  TQStringList lst( dir.entryList() );
  lst.remove( ".." );
  lst.remove( "." );
  KURL url;
  url.setPath( m_tmpdir + lst.first() );
  m_tmpfile = url.path();
  TDEIO::UDSEntry udsInfo;
  TDEIO::NetAccess::stat( url, udsInfo, m_gui );
  KFileItem fileItem( udsInfo, url );
  TQStringList list;
  list << fileItem.name();
  list << fileItem.permissionsString();
  list << fileItem.user();
  list << fileItem.group();
  list << TDEIO::number( fileItem.size() );
  m_gui->fileList()->addItem(list); // send to GUI

  emit sigOpen( this, bSuccess, m_filename,
                Arch::Extract | Arch::Delete | Arch::Add | Arch::View );
}

void CompressedFile::create()
{
  emit sigCreate(this, true, m_filename,
		 Arch::Extract | Arch::Delete | Arch::Add
		  | Arch::View);
}

void CompressedFile::addFile( const TQStringList &urls )
{
  // only used for adding ONE file to an EMPTY gzip file, i.e., one that
  // has just been created

  kdDebug(1601) << "+CompressedFile::addFile" << endl;

  Q_ASSERT(m_gui->getNumFilesInArchive() == 0);
  Q_ASSERT(urls.count() == 1);

  KURL url = KURL::fromPathOrURL(urls.first());
  Q_ASSERT(url.isLocalFile());

  TQString file;
  file = url.path();

  TDEProcess proc;
  proc << "cp" << file << m_tmpdir;
  proc.start(TDEProcess::Block);

  m_tmpfile = file.right(file.length()
			 - file.findRev("/")-1);
  m_tmpfile = m_tmpdir + '/' + m_tmpfile;

  kdDebug(1601) << "Temp file name is " << m_tmpfile << endl;

  kdDebug(1601) << "File is " << file << endl;

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  // lzop hack, see comment in tar.cpp createTmp()
  if ( m_archiver_program == "lzop")
    kp->setUsePty( TDEProcess::Stdin, false );

  TQString compressor = m_archiver_program;

  *kp << compressor << "-c" << file;

  connect( kp, TQT_SIGNAL(receivedStdout(TDEProcess*, char*, int)),
	   this, TQT_SLOT(slotAddInProgress(TDEProcess*, char*, int)));
  connect( kp, TQT_SIGNAL(receivedStderr(TDEProcess*, char*, int)),
	   this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));
  connect( kp, TQT_SIGNAL(processExited(TDEProcess*)), this,
	   TQT_SLOT(slotAddDone(TDEProcess*)));

  int f_desc = KDE_open(TQFile::encodeName(m_filename), O_CREAT | O_TRUNC | O_WRONLY, 0666);
  if (f_desc != -1)
      fd = fdopen( f_desc, "w" );
  else
      fd = NULL;

  if (kp->start(TDEProcess::NotifyOnExit, TDEProcess::AllOutput) == false)
    {
      KMessageBox::error( 0, i18n("Could not start a subprocess.") );
    }

  kdDebug(1601) << "-CompressedFile::addFile" << endl;
}

void CompressedFile::slotAddInProgress(TDEProcess*, char* _buffer, int _bufflen)
{
  // we're trying to capture the output of a command like this
  //    gzip -c myfile
  // and feed the output to the compressed file
  int size;
  size = fwrite(_buffer, 1, _bufflen, fd);
  if (size != _bufflen)
    {
      KMessageBox::error(0, i18n("Trouble writing to the archive..."));
      exit(99);
    }
}

void CompressedFile::slotAddDone(TDEProcess *_kp)
{
  fclose(fd);
  slotAddExited(_kp);
}

void CompressedFile::unarchFileInternal()
{
  if (m_destDir != m_tmpdir)
    {
      TQString dest;
      if (m_destDir.isEmpty() || m_destDir.isNull())
      {
          kdError(1601) << "There was no extract directory given." << endl;
          return;
      }
      else
          dest=m_destDir;

      TDEProcess proc;
      proc << "cp" << m_tmpfile << dest;
      proc.start(TDEProcess::Block);
    }
  emit sigExtract(true);
}

void CompressedFile::remove(TQStringList *)
{
  kdDebug(1601) << "+CompressedFile::remove" << endl;
  TQFile::remove(m_tmpfile);

  // do not delete but truncate the compressed file in case someone
  // does a reload and finds it no longer exists!
  truncate(TQFile::encodeName(m_filename), 0);

  m_tmpfile = "";
  emit sigDelete(true);
  kdDebug(1601) << "-CompressedFile::remove" << endl;
}



#include "compressedfile.moc"

