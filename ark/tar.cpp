/*

 ark -- archiver for the KDE project

 Copyright (C)

 1997-1999: Rob Palmbos palm9744@kettering.edu
 1999: Francois-Xavier Duranceau duranceau@kde.org
 1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
 2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)
 2001: Roberto Selbach Teixeira <maragato@conectiva.com>
 2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>
 2006: Henrique Pinto <henrique.pinto@kdemail.net>

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

// Note: When maintaining tar files with ark, the user should be
// aware that these options have been improved (IMHO). When you append a file
// to a tarchive, tar does not check if the file exists already, and just
// tacks the new one on the end. ark deletes the old one.
// When you update a file that exists in a tarchive, it does check if
// it exists, but once again, it creates a duplicate at the end (only if
// the file is newer though). ark deletes the old one in this case as well.
//
// Basically, tar files are great for creating and extracting, but
// not especially for maintaining. The original purpose of a tar was of
// course, for tape backups, so this is not so surprising!      -Emily
//

// C includes
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

// TQt includes
#include <tqdir.h>
#include <tqregexp.h>
#include <tqeventloop.h>

// KDE includes
#include <kapplication.h>
#include <kdebug.h>
#include <klargefile.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <ktempfile.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <ktempdir.h>
#include <kprocess.h>
#include <ktar.h>

// ark includes
#include "arkwidget.h"
#include "settings.h"
#include "tar.h"
#include "filelistview.h"
#include "tarlistingthread.h"

TarArch::TarArch( ArkWidget *_gui,
                  const TQString & _filename, const TQString & _openAsMimeType)
  : Arch( _gui, _filename), m_tmpDir( 0 ), createTmpInProgress(false),
    updateInProgress(false), deleteInProgress(false), fd(0),
    m_pTmpProc( 0 ), m_pTmpProc2( 0 ), failed( false ), 
    m_dotslash( false ), m_listingThread( 0 )
{
    m_filesToAdd = m_filesToRemove = TQStringList();
    m_archiver_program = m_unarchiver_program = ArkSettings::tarExe();
    verifyCompressUtilityIsAvailable( m_archiver_program );
    verifyUncompressUtilityIsAvailable( m_unarchiver_program );

    m_fileMimeType = _openAsMimeType;
    if ( m_fileMimeType.isNull() )
        m_fileMimeType = KMimeType::findByPath( _filename )->name();

    kdDebug(1601) << "TarArch::TarArch:  mimetype is " << m_fileMimeType << endl;

    if ( m_fileMimeType == "application/x-tbz2" )
    {
        // ark treats .tar.bz2 as x-tbz, instead of duplicating the mimetype
        // let's just alias it to the one we already handle.
        m_fileMimeType = "application/x-tbz";
    }

    if ( m_fileMimeType == "application/x-tar" )
    {
        compressed = false;
    }
    else
    {
        compressed = true;
        m_tmpDir = new KTempDir( _gui->tmpDir()
                                 + TQString::fromLatin1( "temp_tar" ) );
        m_tmpDir->setAutoDelete( true );
        m_tmpDir->qDir()->cd( m_tmpDir->name() );
        // build the temp file name
        KTempFile *pTempFile = new KTempFile( m_tmpDir->name(),
                TQString::fromLatin1(".tar") );

        tmpfile = pTempFile->name();
        delete pTempFile;

        kdDebug(1601) << "Tmpfile will be " << tmpfile << "\n" << endl;
    }
}

TarArch::~TarArch()
{
    delete m_tmpDir;
    m_tmpDir = 0;

    if ( m_listingThread && m_listingThread->finished() != true )
    {
      m_listingThread->wait();
      delete m_listingThread;
      m_listingThread = 0;
    }
}

int TarArch::getEditFlag()
{
    return Arch::Extract;
}

void TarArch::updateArch()
{
  if (compressed)
    {
      updateInProgress = true;
      int f_desc = KDE_open(TQFile::encodeName(m_filename), O_CREAT | O_TRUNC | O_WRONLY, 0666);
      if (f_desc != -1)
          fd = fdopen( f_desc, "w" );
      else
          fd = NULL;

      TDEProcess *kp = m_currentProcess = new TDEProcess;
      kp->clearArguments();
      TDEProcess::Communication flag = TDEProcess::AllOutput;
      if ( getCompressor() == "lzop" )
      {
        kp->setUsePty( TDEProcess::Stdin, false );
        flag = TDEProcess::Stdout;
      }
      if ( !getCompressor().isNull() )
          *kp << getCompressor() << "-c" << tmpfile;
      else
          *kp << "cat" << tmpfile;


      connect(kp, TQT_SIGNAL(receivedStdout(TDEProcess*, char*, int)),
              this, TQT_SLOT(updateProgress( TDEProcess *, char *, int )));
      connect( kp, TQT_SIGNAL(receivedStderr(TDEProcess*, char*, int)),
               (Arch *)this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));

      connect(kp, TQT_SIGNAL(processExited(TDEProcess *)),
               this, TQT_SLOT(updateFinished(TDEProcess *)) );

      if ( !fd || kp->start(TDEProcess::NotifyOnExit, flag) == false)
        {
          KMessageBox::error(0, i18n("Trouble writing to the archive..."));
          emit updateDone();
        }
    }
}

void TarArch::updateProgress( TDEProcess * _proc, char *_buffer, int _bufflen )
{
  // we're trying to capture the output of a command like this
  //    gzip -c myarch.tar
  // and feed the output to the archive
  int size;
  size = fwrite(_buffer, 1, _bufflen, fd);
  if (size != _bufflen)
    {
      _proc->kill();
      KMessageBox::error(0, i18n("Trouble writing to the archive..."));
      kdWarning( 1601 ) << "trouble updating tar archive" << endl;
      //kdFatal( 1601 ) << "trouble updating tar archive" << endl;
    }
}



TQString TarArch::getCompressor()
{
    if ( m_fileMimeType == "application/x-tarz" )
        return TQString( "compress" );

    if ( m_fileMimeType == "application/x-tgz" )
        return TQString( "gzip" );

    if (  m_fileMimeType == "application/x-tbz" )
        return TQString( "bzip2" );

   if (  m_fileMimeType == "application/x-tlz" )
       return TQString( "lzma" );

   if (  m_fileMimeType == "application/x-txz" )
       return TQString( "xz" );

    if( m_fileMimeType == "application/x-tzo" )
        return TQString( "lzop" );

    return TQString();
}


TQString TarArch::getUnCompressor()
{
    if ( m_fileMimeType == "application/x-tarz" )
        return TQString( "uncompress" );

    if ( m_fileMimeType == "application/x-tgz" )
        return TQString( "gunzip" );

    if (  m_fileMimeType == "application/x-tbz" )
        return TQString( "bunzip2" );

   if (  m_fileMimeType == "application/x-tlz" )
       return TQString( "unlzma" );

   if (  m_fileMimeType == "application/x-txz" )
       return TQString( "unxz" );

    if( m_fileMimeType == "application/x-tzo" )
        return TQString( "lzop" );

    return TQString();
}

void
TarArch::open()
{
    if ( compressed )
        TQFile::remove(tmpfile); // just to make sure
    setHeaders();

    clearShellOutput();

    // might as well plunk the output of tar -tvf in the shell output window...
    //
    // Now it's essential - used later to decide whether pathnames in the
    // tar archive are plain or start with "./"
    TDEProcess *kp = m_currentProcess = new TDEProcess;

    *kp << m_archiver_program;

    if ( compressed )
    {
        *kp << "--use-compress-program=" + getUnCompressor();
    }

    *kp << "-tvf" << m_filename;

    m_buffer = "";
    m_header_removed = false;
    m_finished = false;

    connect(kp, TQT_SIGNAL(processExited(TDEProcess *)),
            this, TQT_SLOT(slotListingDone(TDEProcess *)));
    connect(kp, TQT_SIGNAL(receivedStdout(TDEProcess*, char*, int)),
        this, TQT_SLOT(slotReceivedOutput( TDEProcess *, char *, int )));
    connect( kp, TQT_SIGNAL(receivedStderr(TDEProcess*, char*, int)),
        this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));

    if (kp->start(TDEProcess::NotifyOnExit, TDEProcess::AllOutput) == false)
    {
        KMessageBox::error( 0, i18n("Could not start a subprocess.") );
    }

    // We list afterwards because we want the signals at the end
    // This unconfuses Extract Here somewhat

    if ( m_fileMimeType == "application/x-tgz"
            || m_fileMimeType == "application/x-tbz" || !compressed )
    {
        openFirstCreateTempDone();
    }
    else
    {
        connect( this, TQT_SIGNAL( createTempDone() ), this, TQT_SLOT( openFirstCreateTempDone() ) );
        createTmp();
    }
}

void TarArch::openFirstCreateTempDone()
{
    if ( compressed && ( m_fileMimeType != "application/x-tgz" )
            && ( m_fileMimeType != "application/x-tbz" ) )
    {
        disconnect( this, TQT_SIGNAL( createTempDone() ), this, TQT_SLOT( openFirstCreateTempDone() ) );
        Q_ASSERT( !m_listingThread );
        m_listingThread = new TarListingThread( this, tmpfile );
    }
    else {
        Q_ASSERT( !m_listingThread );
        m_listingThread = new TarListingThread( this, m_filename );
    }
    m_listingThread->start();
}

void TarArch::slotListingDone(TDEProcess *_kp)
{
  const TQString list = getLastShellOutput();
  FileListView *flv = m_gui->fileList();
  if (flv!=NULL && flv->totalFiles()>0)
  {
    const TQString firstfile = ((FileLVI *) flv->firstChild())->fileName();
    if (list.find(TQRegExp(TQString("\\s\\./%1[/\\n]").arg(firstfile)))>=0)
    {
      m_dotslash = true;
      kdDebug(1601) << k_funcinfo << "archive has dot-slash" << endl;
    }
    else
    {
      if (list.find(TQRegExp(TQString("\\s%1[/\\n]").arg(firstfile)))>=0)
      {
        // archive doesn't have dot-slash
        m_dotslash = false;
      }
      else
      {
        kdDebug(1601) << k_funcinfo << "cannot match '" << firstfile << "' in listing!" << endl;
      }
    }
  }

  delete _kp;
  _kp = m_currentProcess = NULL;
}

void TarArch::create()
{
  emit sigCreate(this, true, m_filename,
                 Arch::Extract | Arch::Delete | Arch::Add
                  | Arch::View);
}

void TarArch::setHeaders()
{
  ColumnList list;

  list.append(FILENAME_COLUMN);
  list.append(PERMISSION_COLUMN);
  list.append(OWNER_COLUMN);
  list.append(GROUP_COLUMN);
  list.append(SIZE_COLUMN);
  list.append(TIMESTAMP_COLUMN);
  list.append(LINK_COLUMN);

  emit headers( list );
}

void TarArch::createTmp()
{
    if ( compressed )
    {
        if ( !TQFile::exists(tmpfile) )
        {
            TQString strUncompressor = getUnCompressor();
            // at least lzop doesn't want to pipe zerosize/nonexistent files
            TQFile originalFile( m_filename );
            if ( strUncompressor != "gunzip" && strUncompressor !="bunzip2" &&
                ( !originalFile.exists() || originalFile.size() == 0 ) )
            {
                TQFile temp( tmpfile );
                temp.open( IO_ReadWrite );
                temp.close();
                emit createTempDone();
                return;
            }
            // the tmpfile does not yet exist, so we create it.
            createTmpInProgress = true;
            int f_desc = KDE_open(TQFile::encodeName(tmpfile), O_CREAT | O_TRUNC | O_WRONLY, 0666);
            if (f_desc != -1)
                fd = fdopen( f_desc, "w" );
            else
                fd = NULL;

            TDEProcess *kp = m_currentProcess = new TDEProcess;
            kp->clearArguments();
            kdDebug(1601) << "Uncompressor is " << strUncompressor << endl;
            *kp << strUncompressor;
            TDEProcess::Communication flag = TDEProcess::AllOutput;
            if (strUncompressor == "lzop")
            {
                // setting up a pty for lzop, since it doesn't like stdin to
                // be /dev/null ( "no filename allowed when reading from stdin" )
                // - but it used to work without this ? ( Feb 13, 2003 )
                kp->setUsePty( TDEProcess::Stdin, false );
                flag = TDEProcess::Stdout;
                *kp << "-d";
            }
            *kp << "-c" << m_filename;

            connect(kp, TQT_SIGNAL(processExited(TDEProcess *)),
                    this, TQT_SLOT(createTmpFinished(TDEProcess *)));
            connect(kp, TQT_SIGNAL(receivedStdout(TDEProcess*, char*, int)),
                    this, TQT_SLOT(createTmpProgress( TDEProcess *, char *, int )));
            connect( kp, TQT_SIGNAL(receivedStderr(TDEProcess*, char*, int)),
                    this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));
            if (kp->start(TDEProcess::NotifyOnExit, flag ) == false)
            {
                KMessageBox::error(0, i18n("Unable to fork a decompressor"));
		emit sigOpen( this, false, TQString(), 0 );
            }
        }
        else
        {
            emit createTempDone();
            kdDebug(1601) << "Temp tar already there..." << endl;
        }
    }
    else
    {
        emit createTempDone();
    }
}

void TarArch::createTmpProgress( TDEProcess * _proc, char *_buffer, int _bufflen )
{
  // we're trying to capture the output of a command like this
  //    gunzip -c myarch.tar.gz
  // and put the output into tmpfile.

  int size;
  size = fwrite(_buffer, 1, _bufflen, fd);
  if (size != _bufflen)
    {
      _proc->kill();
      KMessageBox::error(0, i18n("Trouble writing to the tempfile..."));
      //kdFatal( 1601 ) << "Trouble writing to archive(createTmpProgress)" << endl;
      kdWarning( 1601 ) << "Trouble writing to archive(createTmpProgress)" << endl;
      //exit(99);
    }
}

void TarArch::deleteOldFiles(const TQStringList &urls, bool bAddOnlyNew)
  // because tar is broken. Used when appending: see addFile.
{
  TQStringList list;
  TQString str;

  TQStringList::ConstIterator iter;
  for (iter = urls.begin(); iter != urls.end(); ++iter )
  {
    KURL url( *iter );
    // find the file entry in the archive listing
    const FileLVI * lv = m_gui->fileList()->item( url.fileName() );
    if ( !lv ) // it isn't in there, so skip it.
      continue;

    if (bAddOnlyNew)
    {
      // compare timestamps. If the file to be added is newer, delete the
      // old. Otherwise we aren't adding it anyway, so we can go on to the next
      // file with a "continue".

      TQFileInfo fileInfo( url.path() );
      TQDateTime addFileMTime = fileInfo.lastModified();
      TQDateTime oldFileMTime = lv->timeStamp();

      kdDebug(1601) << "Old file: " << oldFileMTime.date().year() << '-' <<
        oldFileMTime.date().month() << '-' << oldFileMTime.date().day() <<
        ' ' << oldFileMTime.time().hour() << ':' <<
        oldFileMTime.time().minute() << ':' << oldFileMTime.time().second() <<
        endl;
      kdDebug(1601) << "New file: " << addFileMTime.date().year()  << '-' <<
        addFileMTime.date().month()  << '-' << addFileMTime.date().day() <<
        ' ' << addFileMTime.time().hour()  << ':' <<
        addFileMTime.time().minute() << ':' << addFileMTime.time().second() <<
        endl;

      if (oldFileMTime >= addFileMTime)
      {
        kdDebug(1601) << "Old time is newer or same" << endl;
        continue; // don't add this file to the list to be deleted.
      }
    }
    list.append(str);

    kdDebug(1601) << "To delete: " << str << endl;
  }
  if(!list.isEmpty())
    remove(&list);
  else
    emit removeDone();
}


void TarArch::addFile( const TQStringList&  urls )
{
  m_filesToAdd = urls;
  // tar is broken. If you add a file that's already there, it gives you
  // two entries for that name, whether you --append or --update. If you
  // extract by name, it will give you
  // the first one. If you extract all, the second one will overwrite the
  // first. So we'll first delete all the old files matching the names of
  // those in urls.
  m_bNotifyWhenDeleteFails = false;
  connect( this, TQT_SIGNAL( removeDone() ), this, TQT_SLOT( deleteOldFilesDone() ) );
  deleteOldFiles(urls, ArkSettings::replaceOnlyWithNewer());
}

void TarArch::deleteOldFilesDone()
{
  disconnect( this, TQT_SIGNAL( removeDone() ), this, TQT_SLOT( deleteOldFilesDone() ) );
  m_bNotifyWhenDeleteFails = true;

  connect( this, TQT_SIGNAL( createTempDone() ), this, TQT_SLOT( addFileCreateTempDone() ) );
  createTmp();
}

void TarArch::addFileCreateTempDone()
{
  disconnect( this, TQT_SIGNAL( createTempDone() ), this, TQT_SLOT( addFileCreateTempDone() ) );
  TQStringList * urls = &m_filesToAdd;

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  *kp << m_archiver_program;

  if( ArkSettings::replaceOnlyWithNewer())
    *kp << "uvf";
  else
    *kp << "rvf";

  if (compressed)
    *kp << tmpfile;
  else
    *kp << m_filename;

  TQStringList::ConstIterator iter;
  KURL url( urls->first() );
  TQDir::setCurrent( url.directory() );
  for (iter = urls->begin(); iter != urls->end(); ++iter )
  {
    KURL fileURL( *iter );
    *kp << fileURL.fileName();
  }

  // debugging info
  TQValueList<TQCString> list = kp->args();
  TQValueList<TQCString>::Iterator strTemp;
  for ( strTemp=list.begin(); strTemp != list.end(); ++strTemp )
    {
      kdDebug(1601) << *strTemp << " " << endl;
    }

  connect( kp, TQT_SIGNAL(receivedStdout(TDEProcess*, char*, int)),
           this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));
  connect( kp, TQT_SIGNAL(receivedStderr(TDEProcess*, char*, int)),
           this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));

  connect( kp, TQT_SIGNAL(processExited(TDEProcess*)), this,
           TQT_SLOT(slotAddFinished(TDEProcess*)));

  if (kp->start(TDEProcess::NotifyOnExit, TDEProcess::AllOutput) == false)
    {
      KMessageBox::error( 0, i18n("Could not start a subprocess.") );
      emit sigAdd(false);
    }
}

void TarArch::slotAddFinished(TDEProcess *_kp)
{
  disconnect( _kp, TQT_SIGNAL(processExited(TDEProcess*)), this,
              TQT_SLOT(slotAddFinished(TDEProcess*)));
  m_pTmpProc = _kp;
  m_filesToAdd = TQStringList();
  if ( compressed )
  {
    connect( this, TQT_SIGNAL( updateDone() ), this, TQT_SLOT( addFinishedUpdateDone() ) );
    updateArch();
  }
  else
    addFinishedUpdateDone();
}

void TarArch::addFinishedUpdateDone()
{
  if ( compressed )
    disconnect( this, TQT_SIGNAL( updateDone() ), this, TQT_SLOT( addFinishedUpdateDone() ) );
  Arch::slotAddExited( m_pTmpProc ); // this will delete _kp
  m_pTmpProc = NULL;
}

void TarArch::unarchFileInternal()
{
  TQString dest;

  if (m_destDir.isEmpty() || m_destDir.isNull())
    {
      kdError(1601) << "There was no extract directory given." << endl;
      return;
    }
  else dest = m_destDir;

  TQString tmp;

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  *kp << m_archiver_program;
  if (compressed)
    *kp << "--use-compress-program="+getUnCompressor();

  TQString options = "-x";
  if (!ArkSettings::extractOverwrite())
    options += "k";
  if (ArkSettings::preservePerms())
    options += "p";
  options += "f";

  kdDebug(1601) << "Options were: " << options << endl;
  *kp << options << m_filename << "-C" << dest;

  // if the list is empty, no filenames go on the command line,
  // and we then extract everything in the archive.
  if (m_fileList)
    {
      for ( TQStringList::Iterator it = m_fileList->begin();
            it != m_fileList->end(); ++it )
        {
            *kp << TQString(m_dotslash ? "./" : "")+(*it);
        }
    }

  connect( kp, TQT_SIGNAL(receivedStdout(TDEProcess*, char*, int)),
           this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));
  connect( kp, TQT_SIGNAL(receivedStderr(TDEProcess*, char*, int)),
           this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));

  connect( kp, TQT_SIGNAL(processExited(TDEProcess*)), this,
           TQT_SLOT(slotExtractExited(TDEProcess*)));

  if (kp->start(TDEProcess::NotifyOnExit, TDEProcess::AllOutput) == false)
    {
      KMessageBox::error( 0, i18n("Could not start a subprocess.") );
      emit sigExtract(false);
    }

}

void TarArch::remove(TQStringList *list)
{
  deleteInProgress = true;
  m_filesToRemove = *list;
  connect( this, TQT_SIGNAL( createTempDone() ), this, TQT_SLOT( removeCreateTempDone() ) );
  createTmp();
}

void TarArch::removeCreateTempDone()
{
  disconnect( this, TQT_SIGNAL( createTempDone() ), this, TQT_SLOT( removeCreateTempDone() ) );

  TQString name, tmp;
  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();
  *kp << m_archiver_program << "--delete" << "-f" ;
  if (compressed)
    *kp << tmpfile;
  else
    *kp << m_filename;

  TQStringList::Iterator it = m_filesToRemove.begin();
  for ( ; it != m_filesToRemove.end(); ++it )
    {
        *kp << TQString(m_dotslash ? "./" : "")+(*it);
    }
  m_filesToRemove = TQStringList();

  connect( kp, TQT_SIGNAL(receivedStdout(TDEProcess*, char*, int)),
           this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));
  connect( kp, TQT_SIGNAL(receivedStderr(TDEProcess*, char*, int)),
           this, TQT_SLOT(slotReceivedOutput(TDEProcess*, char*, int)));

  connect( kp, TQT_SIGNAL(processExited(TDEProcess*)), this,
           TQT_SLOT(slotDeleteExited(TDEProcess*)));

  if (kp->start(TDEProcess::NotifyOnExit, TDEProcess::AllOutput) == false)
    {
      KMessageBox::error( 0, i18n("Could not start a subprocess.") );
      emit sigDelete(false);
    }
}

void TarArch::slotDeleteExited(TDEProcess *_kp)
{
  m_pTmpProc2 = _kp;
  if ( compressed )
  {
    connect( this, TQT_SIGNAL( updateDone() ), this, TQT_SLOT( removeUpdateDone() ) );
    updateArch();
  }
  else
    removeUpdateDone();
}

void TarArch::removeUpdateDone()
{
  if ( compressed )
    disconnect( this, TQT_SIGNAL( updateDone() ), this, TQT_SLOT( removeUpdateDone() ) );

  deleteInProgress = false;
  emit removeDone();
  Arch::slotDeleteExited( m_pTmpProc2 );
  m_pTmpProc = NULL;
}

void TarArch::addDir(const TQString & _dirName)
{
  TQStringList list;
  list.append(_dirName);
  addFile(list);
}

void TarArch::openFinished( TDEProcess * )
{
  // do nothing
  // turn off busy light (when someone makes one)
  kdDebug(1601) << "Open finshed" << endl;
}

void TarArch::createTmpFinished( TDEProcess *_kp )
{
  createTmpInProgress = false;
  fclose(fd);
  delete _kp;
  _kp = m_currentProcess = NULL;

  emit createTempDone();
}

void TarArch::updateFinished( TDEProcess *_kp )
{
  fclose(fd);
  updateInProgress = false;
  delete _kp;
  _kp = m_currentProcess = NULL;

  emit updateDone();
}

void TarArch::customEvent( TQCustomEvent *ev )
{
  if ( ev->type() == 65442 )
  {
    ListingEvent *event = static_cast<ListingEvent*>( ev );
    switch ( event->status() )
    {
      case ListingEvent::Normal:
        m_gui->fileList()->addItem( event->columns() );
        break;

      case ListingEvent::Error:
        m_listingThread->wait();
        delete m_listingThread;
        m_listingThread = 0;
        emit sigOpen( this, false, TQString(), 0 );
        break;

      case ListingEvent::ListingFinished:
        m_listingThread->wait();
        delete m_listingThread;
        m_listingThread = 0;
        emit sigOpen( this, true, m_filename,
                      Arch::Extract | Arch::Delete | Arch::Add | Arch::View );
    }
  }
}

void TarArch::test()
{
  clearShellOutput();

  TDEProcess *kp = m_currentProcess = new TDEProcess;
  kp->clearArguments();

  TQString uncomp = getUnCompressor();

  *kp << uncomp;

  if( uncomp == "bunzip2" || uncomp == "gunzip" || uncomp == "lzop" )
  {
    *kp << "-t";
  }
  else
  {
    Arch::test();
    return;
  }

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

#include "tar.moc"
// kate: space-indent on;
