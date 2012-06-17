/*

 ark -- archiver for the KDE project

 Copyright (C)

 2004-2005: Henrique Pinto <henrique.pinto@kdemail.net>
 2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>
 2002-2003: Helio Chissini de Castro <helio@conectiva.com.br>
 2001-2002: Roberto Teixeira <maragato@kde.org>
 2001: Corel Corporation (author: Michael Jarrett, michaelj@corel.com)
 1999-2000: Corel Corporation (author: Emily Ezust, emilye@corel.com)
 1999: Francois-Xavier Duranceau duranceau@kde.org
 1997-1999: Rob Palmbos palm9744@kettering.edu

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

#include <sys/types.h>
#include <sys/stat.h>

// TQt includes
#include <tqlayout.h>
#include <tqstringlist.h>
#include <tqlabel.h>
#include <tqcheckbox.h>
#include <tqdir.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <kopenwith.h>
#include <ktempfile.h>
#include <kmimemagic.h>
#include <kmimetype.h>
#include <kstandarddirs.h>
#include <ktempdir.h>
#include <kprocess.h>
#include <kfiledialog.h>
#include <kdirselectdialog.h>
#include <kurldrag.h>
#include <klistviewsearchline.h>
#include <ktoolbar.h>
#include <kconfigdialog.h>
#include <ktrader.h>
#include <kurl.h>

// settings
#include "settings.h"
#include "general.h"
#include "addition.h"
#include "extraction.h"
#include <kpopupmenu.h>
#include <kdialog.h>

// ark includes
#include "arkapp.h"
#include "archiveformatdlg.h"
#include "extractiondialog.h"
#include "arkwidget.h"
#include "filelistview.h"
#include "arkutils.h"
#include "archiveformatinfo.h"
#include "compressedfile.h"
#include "searchbar.h"
#include "arkviewer.h"

static void viewInExternalViewer( ArkWidget* parent, const KURL& filename )
{
    TQString mimetype = KMimeType::findByURL( filename )->name();
    bool view = true;

    if ( KRun::isExecutable( mimetype ) )
    {
        TQString text = i18n( "The file you're trying to view may be an executable. Running untrusted executables may compromise your system's security.\nAre you sure you want to run that file?" );
        view = ( KMessageBox::warningContinueCancel( parent, text, TQString(), i18n("Run Nevertheless") ) == KMessageBox::Continue );
    }

    if ( view )
        KRun::runURL( filename, mimetype );

}

//----------------------------------------------------------------------
//
//  Class ArkWidget starts here
//
//----------------------------------------------------------------------

ArkWidget::ArkWidget( TQWidget *parent, const char *name )
   : TQVBox(parent, name), m_bBusy( false ), m_bBusyHold( false ),
     m_extractOnly( false ), m_extractRemote(false),
     m_openAsMimeType(TQString()), m_pTempAddList(NULL),
     m_bArchivePopupEnabled( false ),
     m_convert_tmpDir( NULL ), m_convertSuccess( false ),
     m_createRealArchTmpDir( NULL ),  m_extractRemoteTmpDir( NULL ),
     m_modified( false ), m_searchToolBar( 0 ), m_searchBar( 0 ),
     arch( 0 ), m_archType( UNKNOWN_FORMAT ), m_fileListView( 0 ),
     m_nSizeOfFiles( 0 ), m_nSizeOfSelectedFiles( 0 ), m_nNumFiles( 0 ),
     m_nNumSelectedFiles( 0 ), m_bIsArchiveOpen( false ),
     m_bIsSimpleCompressedFile( false ),
     m_bDropSourceIsSelf( false ), m_extractList( 0 )
{
    m_settingsAltered = false;
    m_tmpDir = new KTempDir( locateLocal( "tmp", "ark" ) );

    if ( m_tmpDir->status() != 0 )
    {
       kdWarning( 1601 ) << "Could not create a temporary directory. status() returned "
                         << m_tmpDir->status() << "." << endl;
       m_tmpDir = NULL;
    }

    m_searchToolBar = new KToolBar( this, "searchBar" );
    m_searchToolBar->boxLayout()->setSpacing( KDialog::spacingHint() );

    TQLabel * l1 = new TQLabel( i18n( "&Search:" ), m_searchToolBar, "kde toolbar widget" );
    m_searchBar = new SearchBar( m_searchToolBar, 0 );
    l1->setBuddy( m_searchBar );

    m_searchToolBar->setStretchableWidget( m_searchBar );

    if ( !ArkSettings::showSearchBar() )
        m_searchToolBar->hide();

    createFileListView();

    m_searchBar->setListView( m_fileListView );

    // enable DnD
    setAcceptDrops(true);
    setFocusProxy(m_searchBar);
}

ArkWidget::~ArkWidget()
{
    cleanArkTmpDir();
    ready();
    delete m_pTempAddList;
    delete m_fileListView;
    m_fileListView = 0;
    delete arch;
    if (m_settingsAltered) {
        ArkSettings::writeConfig();
    }
}

void ArkWidget::cleanArkTmpDir()
{
   removeDownloadedFiles();
   if ( m_tmpDir )
   {
      m_tmpDir->unlink();
      delete m_tmpDir;
      m_tmpDir = NULL;
   }
}

void ArkWidget::closeArch()
{
   if ( isArchiveOpen() )
   {
      delete arch;
      arch = 0;
      m_bIsArchiveOpen = false;
   }

   if ( m_fileListView )
   {
      m_fileListView->clear();
      m_fileListView->clearHeaders();
   }
}

////////////////////////////////////////////////////////////////////
///////////////////////// updateStatusTotals ///////////////////////
////////////////////////////////////////////////////////////////////

void
ArkWidget::updateStatusTotals()
{
    m_nNumFiles    = m_fileListView->totalFiles();
    m_nSizeOfFiles = m_fileListView->totalSize();

    TQString strInfo = i18n( "%n file  %1", "%n files  %1", m_nNumFiles )
                          .arg( KIO::convertSize( m_nSizeOfFiles ) );
    emit setStatusBarText(strInfo);
}

void
ArkWidget::busy( const TQString & text )
{
    emit setBusy( text );

    if ( m_bBusy )
        return;

    m_fileListView->setEnabled( false );

    TQApplication::setOverrideCursor( waitCursor );
    m_bBusy = true;
}

void
ArkWidget::holdBusy()
{
    if ( !m_bBusy || m_bBusyHold )
        return;

    m_bBusyHold = true;
    TQApplication::restoreOverrideCursor();
}

void
ArkWidget::resumeBusy()
{
    if ( !m_bBusyHold )
        return;

    m_bBusyHold = false;
    TQApplication::setOverrideCursor( waitCursor );
}

void
ArkWidget::ready()
{
    if ( !m_bBusy )
        return;

    m_fileListView->setEnabled( true );

    TQApplication::restoreOverrideCursor();
    emit setReady();
    m_bBusyHold = false;
    m_bBusy = false;
}

//////////////////////////////////////////////////////////////////////
////////////////////// file_save_as //////////////////////////////////
//////////////////////////////////////////////////////////////////////

KURL
ArkWidget::getSaveAsFileName()
{
    TQString defaultMimeType;
    if ( m_openAsMimeType.isNull() )
        defaultMimeType = KMimeType::findByPath( m_strArchName )->name();
    else
        defaultMimeType = m_openAsMimeType;

    KURL u;
    TQString suggestedName;
    if ( m_realURL.isLocalFile() )
        suggestedName = m_realURL.url();
    else
        suggestedName = m_realURL.fileName( false );

    do
    {
        u = getCreateFilename( i18n( "Save Archive As" ), defaultMimeType, true, suggestedName );
        if (  u.isEmpty() )
            return u;
        if( allowedArchiveName( u ) || ( ArchiveFormatInfo::self()->archTypeByExtension( u.path() ) != UNKNOWN_FORMAT ) )
            break;
        KMessageBox::error( this, i18n( "Please save your archive in the same format as the original.\nHint: Use one of the suggested extensions." ) );
    }
    while ( true );
    return u;
}

bool
ArkWidget::file_save_as( const KURL & u )
{
    bool success = KIO::NetAccess::upload( m_strArchName, u, this );
    if ( m_modified && success )
        m_modified = false;
    return success;
}

void
ArkWidget::convertTo( const KURL & u )
{
    busy( i18n( "Saving..." ) );
    m_convert_tmpDir =  new KTempDir( tmpDir() + "convtmp" );
    m_convert_tmpDir->setAutoDelete( true );
    connect( arch, TQT_SIGNAL( sigExtract( bool ) ), this, TQT_SLOT( convertSlotExtractDone( bool ) ) );
    m_convert_saveAsURL = u;
    arch->unarchFile( 0, m_convert_tmpDir->name() );
}

void
ArkWidget::convertSlotExtractDone( bool )
{
    kdDebug( 1601 ) << k_funcinfo << endl;
    disconnect( arch, TQT_SIGNAL( sigExtract( bool ) ), this, TQT_SLOT( convertSlotExtractDone( bool ) ) );
    TQTimer::singleShot( 0, this, TQT_SLOT( convertSlotCreate() ) );
}

void
ArkWidget::convertSlotCreate()
{
    file_close();
    connect( this, TQT_SIGNAL( createDone( bool ) ), this, TQT_SLOT( convertSlotCreateDone( bool ) ) );
    TQString archToCreate;
    if ( m_convert_saveAsURL.isLocalFile() )
        archToCreate = m_convert_saveAsURL.path();
    else
        archToCreate = tmpDir() + m_convert_saveAsURL.fileName();

    createArchive( archToCreate );
}


void
ArkWidget::convertSlotCreateDone( bool success )
{
    disconnect( this, TQT_SIGNAL( createDone( bool ) ), this, TQT_SLOT( convertSlotCreateDone( bool ) ) );
    kdDebug( 1601 ) << k_funcinfo << endl;
    if ( !success )
    {
        kdWarning( 1601 ) << "Error while converting. (convertSlotCreateDone)" << endl;
        return;
    }
    TQDir dir( m_convert_tmpDir->name() );
    TQStringList entries = dir.entryList();
    entries.remove( ".." );
    entries.remove( "." );
    TQStringList::Iterator it = entries.begin();
    for ( ; it != entries.end(); ++it )
    {
        ///////////////////////////////////////////////////////
        // BIG TODO: get rid of 'the assume                  //
        // 'file:/', do some  black magic                    //
        // to find the basedir, chdir there,                 //
        // and break the rest of the world'                  //
        // hack. See also action_edit ...                    //
        // addFile should be:                                //
        // addFile( const TQString & baseDir,                 //
        //          const TQStringList & filesToAdd )         //
        //////////////////////////////////////////////////////
        *it = TQString::fromLatin1( "file:" )+ m_convert_tmpDir->name() + *it;
    }
    bool bOldRecVal = ArkSettings::rarRecurseSubdirs();
    connect( arch, TQT_SIGNAL( sigAdd( bool ) ), this, TQT_SLOT( convertSlotAddDone( bool ) ) );
    arch->addFile( entries );
    ArkSettings::setRarRecurseSubdirs( bOldRecVal );
}

void
ArkWidget::convertSlotAddDone( bool success )
{
    disconnect( arch, TQT_SIGNAL( sigAdd( bool ) ), this, TQT_SLOT( convertSlotAddDone( bool ) ) );
    kdDebug( 1601 ) << k_funcinfo << endl;
    m_convertSuccess = success;
    // needed ? (TarArch, lzo)
    TQTimer::singleShot( 0, this, TQT_SLOT( convertFinish() ) );
}

void
ArkWidget::convertFinish()
{
    kdDebug( 1601 ) << k_funcinfo << endl;
    delete m_convert_tmpDir;
    m_convert_tmpDir = NULL;

    ready();
    if ( m_convertSuccess )
    {
        if ( m_convert_saveAsURL.isLocalFile() )
        {
            emit openURLRequest( m_convert_saveAsURL );
        }
        else
        {
            KIO::NetAccess::upload( tmpDir()
                       + m_convert_saveAsURL.fileName(), m_convert_saveAsURL, this );
            // TODO: save bandwidth - we already have a local tmp file ...
            emit openURLRequest( m_convert_saveAsURL );
        }
    }
    else
    {
        kdWarning( 1601 ) << "Error while converting (convertSlotAddDone)" << endl;
    }
}

bool
ArkWidget::allowedArchiveName( const KURL & u )
{
    if (u.isEmpty())
        return false;

    TQString archMimeType = KMimeType::findByURL( m_url )->name();
    if ( !m_openAsMimeType.isNull() )
        archMimeType = m_openAsMimeType;
    TQString newArchMimeType = KMimeType::findByPath( u.path() )->name();
    if ( archMimeType == newArchMimeType )
        return true;

    return false;
}

void
ArkWidget::extractTo( const KURL & targetDirectory, const KURL & archive, bool bGuessName )
{
    m_extractTo_targetDirectory = targetDirectory;

    if ( bGuessName ) // suggest an extract directory based on archive name
    {
        const TQString fileName = guessName( archive );
        m_extractTo_targetDirectory.setPath( targetDirectory.path( 1 ) + fileName + '/' );
    }

    if ( !KIO::NetAccess::exists( m_extractTo_targetDirectory, false, this ) )
    {
        if ( !KIO::NetAccess::mkdir( m_extractTo_targetDirectory, this ) )
        {
            KMessageBox::error( 0, i18n( "Could not create the folder %1" ).arg(
                                                            targetDirectory.prettyURL() ) );
            emit request_file_quit();
            return;
        }
    }

    connect( this, TQT_SIGNAL( openDone( bool ) ), this, TQT_SLOT( extractToSlotOpenDone( bool ) ) );
}

const TQString
ArkWidget::guessName( const KURL &archive )
{
  TQString fileName = archive.fileName();
  TQStringList list = KMimeType::findByPath( fileName )->patterns();
  TQStringList::Iterator it = list.begin();
  TQString ext;
  for ( ; it != list.end(); ++it )
  {
    ext = (*it).remove( '*' );
    if ( fileName.endsWith( ext ) )
    {
      fileName = fileName.left( fileName.findRev( ext ) );
      break;
    }
  }

  return fileName;
}

void
ArkWidget::extractToSlotOpenDone( bool success )
{
    disconnect( this, TQT_SIGNAL( openDone( bool ) ), this, TQT_SLOT( extractToSlotOpenDone( bool ) ) );
    if ( !success )
    {
        KMessageBox::error( this, i18n( "An error occurred while opening the archive %1." ).arg( m_url.prettyURL() ) );
        emit request_file_quit();
        return;
    }

    TQString extractDir = m_extractTo_targetDirectory.path();
    // little code duplication from action_extract():
    if ( !m_extractTo_targetDirectory.isLocalFile() )
    {
        m_extractRemoteTmpDir = new KTempDir( tmpDir() + "extremote" );
        m_extractRemoteTmpDir->setAutoDelete( true );

        extractDir = m_extractRemoteTmpDir->name();
        m_extractRemote = true;

        if ( m_extractRemoteTmpDir->status() != 0 )
        {
            kdWarning(1601) << "Unable to create " << extractDir << endl;
            m_extractRemote = false;
            emit request_file_quit();
            return;
        }
    }

    TQStringList empty;
    TQStringList alreadyExisting = existingFiles( extractDir, empty );
    kdDebug( 1601 ) << "Already existing files count: " << existingFiles( extractDir, empty ).count() << endl;
    bool keepGoing = true;
    if ( !ArkSettings::extractOverwrite() && !alreadyExisting.isEmpty() )
    {
        keepGoing = ( KMessageBox::Continue == KMessageBox::warningContinueCancelList( this,
                    i18n( "The following files will not be extracted\nbecause they "
                          "already exist:" ), alreadyExisting ) );
    }

    if ( keepGoing ) // if the user's OK with those failures, go ahead
    {
        // unless we have no space!
        if ( ArkUtils::diskHasSpace( extractDir, m_nSizeOfFiles ) )
        {
            disableAll();
            connect( arch, TQT_SIGNAL( sigExtract( bool ) ), this, TQT_SLOT( extractToSlotExtractDone( bool ) ) );
            arch->unarchFile( 0, extractDir );
        }
        else
        {
            KMessageBox::error( this, i18n( "Not enough free disc space to extract the archive." ) );
            emit request_file_quit();
            return;
        }
    }
    else
        emit request_file_quit();
}

void
ArkWidget::extractToSlotExtractDone( bool success )
{
    disconnect( arch, TQT_SIGNAL( sigExtract( bool ) ), this, TQT_SLOT( extractToSlotExtractDone( bool ) ) );
    if ( !success )
    {
        kdDebug( 1601 ) << "Last Shell Output" << arch->getLastShellOutput() << endl;
        KMessageBox::error( this, i18n( "An error occurred while extracting the archive." ) );
        emit request_file_quit();
        return;
    }

    if (  m_extractRemote )
    {
        connect( this, TQT_SIGNAL( extractRemoteMovingDone() ), this, TQT_SIGNAL( request_file_quit() ) );
        extractRemoteInitiateMoving( m_extractTo_targetDirectory );
    }
    else
        emit request_file_quit();
}

bool
ArkWidget::addToArchive( const KURL::List & filesToAdd, const KURL & archive)
{
    m_addToArchive_filesToAdd = filesToAdd;
    m_addToArchive_archive = archive;
    if ( !KIO::NetAccess::exists( archive, false, this ) )
    {
        if ( !m_openAsMimeType.isEmpty() )
        {
            TQStringList extensions = KMimeType::mimeType( m_openAsMimeType )->patterns();
            TQStringList::Iterator it = extensions.begin();
            TQString file = archive.path();
            for ( ; it != extensions.end() && !file.endsWith( ( *it ).remove( '*' ) ); ++it )
                ;

            if ( it == extensions.end() )
            {
                file += ArchiveFormatInfo::self()->defaultExtension( m_openAsMimeType );
                const_cast< KURL & >( archive ).setPath( file );
            }
        }

        connect( this, TQT_SIGNAL( createDone( bool ) ), this, TQT_SLOT( addToArchiveSlotCreateDone( bool ) ) );

        // TODO: remote Archives should be handled by createArchive
        if ( archive.isLocalFile() )
        {
            if ( !createArchive( archive.path() ) )
                 return false;
        }
        else
        {
            if ( !createArchive( tmpDir() + archive.fileName() ) )
                 return false;
        }
    return true;

    }
    connect( this, TQT_SIGNAL( openDone( bool ) ), this, TQT_SLOT( addToArchiveSlotOpenDone( bool ) ) );
    return true;
}

void
ArkWidget::addToArchiveSlotCreateDone( bool success )
{
    disconnect( this, TQT_SIGNAL( createDone( bool ) ), this, TQT_SLOT( addToArchiveSlotCreateDone( bool ) ) );
    if ( !success )
    {
        kdDebug( 1601 ) << "Could not create the archive" << endl;
        emit request_file_quit();
        return;
    }
    addToArchiveSlotOpenDone( true );
}

void
ArkWidget::addToArchiveSlotOpenDone( bool success )
{
    kdDebug( 1601 ) << k_funcinfo << endl;
    disconnect( this, TQT_SIGNAL( openDone( bool ) ), this, TQT_SLOT( addToArchiveSlotOpenDone( bool ) ) );
    // TODO: handle dirs with addDir ( or better+easier: get rid of the need to do that entirely )
    if ( !success )
    {
        emit request_file_quit();
        return;
    }

    if ( m_bIsSimpleCompressedFile && (m_nNumFiles == 1))
    {
        TQString strFilename;
        KURL url = askToCreateRealArchive();
        strFilename = url.path();
        if (!strFilename.isEmpty())
        {
            connect( this, TQT_SIGNAL( createRealArchiveDone( bool ) ), this, TQT_SLOT( addToArchiveSlotAddDone( bool ) ) );
            createRealArchive( strFilename, m_addToArchive_filesToAdd.toStringList() );
            return;
        }
        else
        {
            emit request_file_quit();
            return;
        }
    }

/*    TQStringList list = m_addToArchive_filesToAdd.toStringList();
    if ( !ArkUtils::diskHasSpace( tmpDir(), ArkUtils::getSizes( &list ) ) )
    {
        KMessageBox::error( this, i18n( "Not enough free disc space to extract the archive." ) );
        emit request_file_quit();
        return;
    }*/

    disableAll();
    // if they are URLs, we have to download them, replace the URLs
    // with filenames, and remember to delete the temporaries later.
/*    for ( TQStringList::Iterator it = list.begin();
         it != list.end(); ++it)
    {
        TQString str = *it;
        KURL url( toLocalFile( str ) );
        *it = url.prettyURL();
    }
*/
    KURL::List list = m_addToArchive_filesToAdd;


    // Remote URLs need to be downloaded.
    KURL::List::Iterator end( list.end() );
    for ( KURL::List::Iterator it = list.begin(); it != end; ++it )
    {
        if (!(*it).isLocalFile())
        {
            *it = toLocalFile( *it );
        }
    }

    kdDebug( 1601 ) << "Adding: " << list << endl;

    connect( arch, TQT_SIGNAL( sigAdd( bool ) ), this, TQT_SLOT( addToArchiveSlotAddDone( bool ) ) );
    arch->addFile( list.toStringList() );
}

void
ArkWidget::addToArchiveSlotAddDone( bool success )
{
    kdDebug( 1601 ) << k_funcinfo << endl;
    disconnect( this, TQT_SLOT( addToArchiveSlotAddDone( bool ) ) );
    if ( !success )
    {
        KMessageBox::error( this, i18n( "An error occurred while adding the files to the archive." ) );
    }
    if ( !m_addToArchive_archive.isLocalFile() )
        KIO::NetAccess::upload( m_strArchName, m_addToArchive_archive, this );
    emit request_file_quit();
    return;
}

void ArkWidget::setOpenAsMimeType( const TQString & mimeType )
{
    m_openAsMimeType = mimeType;
}

void
ArkWidget::file_open(const KURL& url)
{
    if ( url.isEmpty() )
    {
        kdDebug( 1601 ) << "file_open: url empty" << endl;
        return;
    }

    if ( isArchiveOpen() )
        file_close();  // close old arch. If we don't, our temp file is wrong!

    if ( !url.isLocalFile() )
    {
        kdWarning ( 1601 ) << url.prettyURL() << " is not a local URL in ArkWidget::file_open( KURL). Aborting. " << endl;
        return;
    }


    TQString strFile = url.path();

    kdDebug( 1601 ) << "File to open: " << strFile << endl;

    TQFileInfo fileInfo( strFile );
    if ( !fileInfo.exists() )
    {
        KMessageBox::error(this, i18n("The archive %1 does not exist.").arg(strFile));
        emit removeRecentURL( m_realURL );
        return;
    }
    else if ( !fileInfo.isReadable() )
    {
        KMessageBox::error(this, i18n("You do not have permission to access that archive.") );
        emit removeRecentURL( m_realURL );
        return;
    }

    // see if the user is just opening the same file that's already
    // open (erm...)

    if (strFile == m_strArchName && m_bIsArchiveOpen)
    {
        kdDebug( 1601 ) << "file_open: strFile == m_strArchName" << endl;
        return;
    }

    // no errors if we made it this far.

    // Set the current archive filename to the filename
    m_strArchName = strFile;
    m_url = url;
    //arch->clearShellOutput();

    if( url.hasPass() )
	openArchive( strFile, url.pass() );
    else
	openArchive( strFile );
}


// File menu /////////////////////////////////////////////////////////

KURL
ArkWidget::getCreateFilename(const TQString & _caption,
                                  const TQString & _defaultMimeType,
                                  bool allowCompressed,
                                  const TQString & _suggestedName )
{
    int choice=0;
    bool fileExists = true;
    TQString strFile;
    KURL url;

    KFileDialog dlg( ":ArkSaveAsDialog", TQString(), this, "SaveAsDialog", true );
    dlg.setCaption( _caption );
    dlg.setOperationMode( KFileDialog::Saving );
    dlg.setMimeFilter( ArchiveFormatInfo::self()->supportedMimeTypes( allowCompressed ),
                       _defaultMimeType.isNull() ?  "application/x-tgz" : _defaultMimeType );
    if ( !_suggestedName.isEmpty() )
        dlg.setSelection( _suggestedName );

    while ( fileExists )
        // keep asking for filenames as long as the user doesn't want to
        // overwrite existing ones; break if they agree to overwrite
        // or if the file doesn't already exist. Return if they cancel.
        // Also check for proper extensions.
    {
        dlg.exec();
        url = dlg.selectedURL();
        strFile = url.path();

        if (strFile.isEmpty())
            return TQString();

        //the user chose to save as the current archive
        //or wanted to create a new one with the same name
        //no need to do anything
        if (strFile == m_strArchName && m_bIsArchiveOpen)
            return TQString();

        TQStringList extensions = dlg.currentFilterMimeType()->patterns();
        TQStringList::Iterator it = extensions.begin();
        for ( ; it != extensions.end() && !strFile.endsWith( ( *it ).remove( '*' ) ); ++it )
            ;

        if ( it == extensions.end() )
        {
            strFile += ArchiveFormatInfo::self()->defaultExtension( dlg.currentFilterMimeType()->name() );
            url.setPath( strFile );
        }

        kdDebug(1601) << "Trying to create an archive named " << strFile << endl;
        fileExists = TQFile::exists( strFile );
        if( fileExists )
        {
            choice = KMessageBox::warningYesNoCancel(0,
               i18n("Archive already exists. Do you wish to overwrite it?"),
               i18n("Archive Already Exists"), i18n("Overwrite"), i18n("Do Not Overwrite"));

            if ( choice == KMessageBox::Yes )
            {
                TQFile::remove( strFile );
                break;
            }
            else if ( choice == KMessageBox::Cancel )
            {
                return TQString();
            }
            else
            {
                continue;
            }
        }
        // if we got here, the file does not already exist.
        if ( !ArkUtils::haveDirPermissions( url.directory() ) )
        {
            KMessageBox::error( this,
                i18n( "You do not have permission"
                      " to write to the directory %1" ).arg(url.directory() ) );
            return TQString();
        }
    } // end of while loop

    return url;
}

void
ArkWidget::file_new()
{
    TQString strFile;
    KURL url = getCreateFilename(i18n("Create New Archive") );
    strFile = url.path();
    if (!strFile.isEmpty())
    {
        file_close();
        createArchive( strFile );
    }
}

void
ArkWidget::extractOnlyOpenDone()
{
    bool done = action_extract();

    // last extract dir is still set, but this is not a problem
    if( !done )
    {
        emit request_file_quit();
    }

}

void
ArkWidget::slotExtractDone(bool success)
{
    disconnect( arch, TQT_SIGNAL( sigExtract( bool ) ),
                this, TQT_SLOT( slotExtractDone(bool) ) );
    ready();

    if(m_extractList != 0)
        delete m_extractList;
    m_extractList = 0;

    if( m_fileListView ) // avoid race condition, don't do updates if application is exiting
    {
        m_fileListView->setUpdatesEnabled(true);
        fixEnables();
    }

    if ( m_extractRemote )
    {
        extractRemoteInitiateMoving( m_extractURL );
    }
    else if( m_extractOnly )
    {
        emit request_file_quit();
    }

    if ( success && ArkSettings::openDestinationFolder() )
    {
            KRun::runURL( m_extractURL, "inode/directory" );
    }

    kdDebug(1601) << "-ArkWidget::slotExtractDone" << endl;
}

void
ArkWidget::extractRemoteInitiateMoving( const KURL & target )
{
    KURL srcDirURL;
    KURL src;
    TQString srcDir;

    srcDir = m_extractRemoteTmpDir->name();
    srcDirURL.setPath( srcDir );

    TQDir dir( srcDir );
    dir.setFilter( TQDir::All | TQDir::Hidden );
    TQStringList lst( dir.entryList() );
    lst.remove( "." );
    lst.remove( ".." );

    KURL::List srcList;
    for( TQStringList::ConstIterator it = lst.begin(); it != lst.end() ; ++it)
    {
        src = srcDirURL;
        src.addPath( *it );
        srcList.append( src );
    }

    m_extractURL.adjustPath( 1 );

    KIO::CopyJob *job = KIO::copy( srcList, target, this );
    connect( job, TQT_SIGNAL(result(KIO::Job*)),
            this, TQT_SLOT(slotExtractRemoteDone(KIO::Job*)) );

    m_extractRemote = false;
}

void
ArkWidget::slotExtractRemoteDone(KIO::Job *job)
{
    delete m_extractRemoteTmpDir;
    m_extractRemoteTmpDir = NULL;

    if ( job->error() )
        job->showErrorDialog();

    emit extractRemoteMovingDone();

    if ( m_extractOnly )
        emit request_file_quit();
}


void
ArkWidget::disableAll() // private
{
    emit disableAllActions();
    m_fileListView->setUpdatesEnabled(true);
}

void
ArkWidget::fixEnables() // private
{
    emit fixActions(); //connected to the part
}

void
ArkWidget::file_close()
{
    if ( isArchiveOpen() )
    {
        closeArch();
        emit setWindowCaption( TQString() );
        emit removeOpenArk( m_strArchName );
        updateStatusTotals();
        updateStatusSelection();
        fixEnables();
    }
    else
    {
        closeArch();
    }

    m_strArchName = TQString();
    m_url = KURL();
}


KURL
ArkWidget::askToCreateRealArchive()
{
    // ask user whether to create a real archive from a compressed file
    // returns filename if so
    KURL url;
    int choice =
        KMessageBox::warningYesNo(0, i18n("You are currently working with a simple compressed file.\nWould you like to make it into an archive so that it can contain multiple files?\nIf so, you must choose a name for your new archive."), i18n("Warning"),i18n("Make Into Archive"),i18n("Do Not Make"));
    if (choice == KMessageBox::Yes)
    {
        url = getCreateFilename( i18n("Create New Archive"),
                                 TQString(), false );
    }
    else
        url.setPath( TQString() );
    return url;
}

void
ArkWidget::createRealArchive( const TQString & strFilename, const TQStringList & filesToAdd )
{
    Arch * newArch = getNewArchive( strFilename );
    busy( i18n( "Creating archive..." ) );
    if ( !newArch )
        return;
    if ( !filesToAdd.isEmpty() )
        m_pTempAddList = new TQStringList( filesToAdd );
    m_compressedFile = static_cast< CompressedFile * >( arch )->tempFileName();
    KURL u1, u2;
    u1.setPath( m_compressedFile );
    m_createRealArchTmpDir = new KTempDir( tmpDir() + "create_real_arch" );
    u2.setPath( m_createRealArchTmpDir->name() + u1.fileName() );
    KIO::NetAccess::copy( u1, u2, this );
    m_compressedFile = "file:" + u2.path(); // AGAIN THE 5 SPACES Hack :-(
    connect( newArch, TQT_SIGNAL( sigCreate( Arch *, bool, const TQString &, int ) ),
             this, TQT_SLOT( createRealArchiveSlotCreate( Arch *, bool,
             const TQString &, int ) ) );
    file_close();
    newArch->create();
}

void
ArkWidget::createRealArchiveSlotCreate( Arch * newArch, bool success,
                                             const TQString & fileName, int nbr )
{
    slotCreate( newArch, success, fileName, nbr );

    if ( !success )
        return;

    TQStringList listForCompressedFile;
    listForCompressedFile.append(m_compressedFile);
    disableAll();

    connect( newArch, TQT_SIGNAL( sigAdd( bool ) ), this,
                      TQT_SLOT( createRealArchiveSlotAddDone( bool ) ) );

    newArch->addFile(listForCompressedFile);
}

void
ArkWidget::createRealArchiveSlotAddDone( bool success )
{
    kdDebug( 1601 ) << "createRealArchiveSlotAddDone+, success:" << success << endl;
    disconnect( arch, TQT_SIGNAL( sigAdd( bool ) ), this,
                      TQT_SLOT( createRealArchiveSlotAddDone( bool ) ) );

    m_createRealArchTmpDir->unlink();
    delete m_createRealArchTmpDir;
    m_createRealArchTmpDir = NULL;


    if ( !success )
        return;

    ready();

    if ( m_pTempAddList == NULL )
    {
        // now get the files to be added
        // we don't know which files to add yet
        action_add();
    }
    else
    {
        connect( arch, TQT_SIGNAL( sigAdd( bool ) ), this,
                 TQT_SLOT( createRealArchiveSlotAddFilesDone( bool ) ) );
        // files were dropped in
        addFile( m_pTempAddList );
    }
}

void
ArkWidget::createRealArchiveSlotAddFilesDone( bool success )
{
    //kdDebug( 1601 ) << "createRealArchiveSlotAddFilesDone+, success:" << success << endl;
    disconnect( arch, TQT_SIGNAL( sigAdd( bool ) ), this,
                      TQT_SLOT( createRealArchiveSlotAddFilesDone( bool ) ) );
    delete m_pTempAddList;
    m_pTempAddList = NULL;
    emit createRealArchiveDone( success );
}




// Action menu /////////////////////////////////////////////////////////

void
ArkWidget::action_add()
{
    if (m_bIsSimpleCompressedFile && (m_nNumFiles == 1))
    {
        TQString strFilename;
        KURL url = askToCreateRealArchive();
        strFilename = url.path();
        if (!strFilename.isEmpty())
        {
            createRealArchive(strFilename);
        }
        return;
    }

    KFileDialog fileDlg( ":ArkAddDir", TQString(), this, "adddlg", true );
    fileDlg.setMode( KFile::Mode( KFile::Files | KFile::ExistingOnly ) );
    fileDlg.setCaption(i18n("Select Files to Add"));

    if(fileDlg.exec())
    {
        KURL::List addList;
        addList = fileDlg.selectedURLs();
        TQStringList * list = new TQStringList();
        //Here we pre-calculate the end of the list
   KURL::List::ConstIterator endList = addList.end();
        for (KURL::List::ConstIterator it = addList.begin(); it != endList; ++it)
            list->append( KURL::decode_string( (*it).url() ) );

        if ( list->count() > 0 )
        {
            if ( m_bIsSimpleCompressedFile && list->count() > 1 )
            {
                TQString strFilename;
                KURL url = askToCreateRealArchive();
                strFilename = url.path();
                if (!strFilename.isEmpty())
                {
                    createRealArchive(strFilename);
                }
                delete list;
                return;
            }
            addFile( list );
        }
        delete list;
    }
}

void
ArkWidget::addFile(TQStringList *list)
{
    if ( !ArkUtils::diskHasSpace( tmpDir(), ArkUtils::getSizes( list ) ) )
        return;

    disableAll();
    busy( i18n( "Adding files..." ) );
    // if they are URLs, we have to download them, replace the URLs
    // with filenames, and remember to delete the temporaries later.
    for (TQStringList::Iterator it = list->begin(); it != list->end(); ++it)
    {
        TQString str = *it;
        *it = toLocalFile(KURL(str)).prettyURL();

    }

    connect( arch, TQT_SIGNAL( sigAdd( bool ) ), this, TQT_SLOT( slotAddDone( bool ) ) );
    arch->addFile( ( *list ) );
}

void
ArkWidget::action_add_dir()
{
    KURL u = KDirSelectDialog::selectDirectory( ":ArkAddDir",
                                                false, this,
                                                i18n("Select Folder to Add"));

    TQString dir = KURL::decode_string( u.url(-1) );
    if ( !dir.isEmpty() )
    {
        busy( i18n( "Adding folder..." ) );
        disableAll();
        u = toLocalFile(u);
        connect( arch, TQT_SIGNAL( sigAdd( bool ) ), this, TQT_SLOT( slotAddDone( bool ) ) );
        arch->addDir( u.prettyURL() );
    }

}

void
ArkWidget::slotAddDone(bool _bSuccess)
{
    disconnect( arch, TQT_SIGNAL( sigAdd( bool ) ), this, TQT_SLOT( slotAddDone( bool ) ) );
    m_fileListView->setUpdatesEnabled(true);
    m_fileListView->triggerUpdate();
    ready();

    if (_bSuccess)
    {
        m_modified = true;
        //simulate reload
        KURL u;
        u.setPath( arch->fileName() );
	if( !arch->password().isEmpty() )
    	    u.setPass( arch->password() );
        file_close();
        file_open( u );
        emit setWindowCaption( u.path() );
    }
    removeDownloadedFiles();
    fixEnables();
}



KURL
ArkWidget::toLocalFile( const KURL& url )
{
    KURL localURL = url;

    if(!url.isLocalFile())
    {
   TQString strURL = url.prettyURL();

        TQString tempfile = tmpDir();
        tempfile += strURL.right(strURL.length() - strURL.findRev("/") - 1);
        deleteAfterUse(tempfile);  // remember for deletion
        KURL tempurl; tempurl.setPath( tempfile );
        if( !KIO::NetAccess::dircopy(url, tempurl, this) )
            return KURL();
        localURL = tempfile;
    }
    return localURL;
}

void
ArkWidget::deleteAfterUse( const TQString& path )
{
    mpDownloadedList.append( path );
}

void
ArkWidget::removeDownloadedFiles()
{
    if (!mpDownloadedList.isEmpty())
    {
        // It is necessary to remove those files even if tmpDir() is getting deleted:
        // not all files in mpDownloadedList are from tmpDir() - e.g. when using --tempfile
        // But of course we could decide to not add files from tmpDir() into mpDownloadedList.
        TQStringList::ConstIterator it = mpDownloadedList.begin();
        TQStringList::ConstIterator end = mpDownloadedList.end();
        for ( ; it != end ; ++it )
            TQFile::remove( *it );
        mpDownloadedList.clear();
    }
}

void
ArkWidget::action_delete()
{
    // remove selected files and create a list to send to the archive
    // Warn the user if he/she/it tries to delete a directory entry in
    // a tar file - it actually deletes the contents of the directory
    // as well.

    if (m_fileListView->isSelectionEmpty())
    {
        return; // shouldn't happen - delete should have been disabled!
    }

    TQStringList list = m_fileListView->selectedFilenames();

    // ask for confirmation
    if ( KMessageBox::warningContinueCancelList( this,
                                              i18n( "Do you really want to delete the selected items?" ),
                                              list,
                                              TQString(),
                                              KStdGuiItem::del(),
                                              "confirmDelete" )
         != KMessageBox::Continue)
    {
        return;
    }

    // Remove the entries from the list view
    TQListViewItemIterator it( m_fileListView );
    while ( it.current() )
    {
        if ( it.current()->isSelected() )
            delete *it;
        else
            ++it;
    }

    disableAll();
    busy( i18n( "Removing..." ) );
    connect( arch, TQT_SIGNAL( sigDelete( bool ) ), this, TQT_SLOT( slotDeleteDone( bool ) ) );
    arch->remove(&list);
    kdDebug(1601) << "-ArkWidget::action_delete" << endl;
}

void
ArkWidget::slotDeleteDone(bool _bSuccess)
{
    disconnect( arch, TQT_SIGNAL( sigDelete( bool ) ), this, TQT_SLOT( slotDeleteDone( bool ) ) );
    kdDebug(1601) << "+ArkWidget::slotDeleteDone" << endl;
    m_fileListView->setUpdatesEnabled(true);
    m_fileListView->triggerUpdate();
    if (_bSuccess)
    {
        m_modified = true;
        updateStatusTotals();
        updateStatusSelection();
    }
    // disable the select all and extract options if there are no files left
    fixEnables();
    ready();
    kdDebug(1601) << "-ArkWidget::slotDeleteDone" << endl;

}



void
ArkWidget::slotOpenWith()
{
    connect( arch, TQT_SIGNAL( sigExtract( bool ) ), this,
            TQT_SLOT( openWithSlotExtractDone( bool ) ) );

    showCurrentFile();
}

void
ArkWidget::openWithSlotExtractDone( bool success )
{
    disconnect( arch, TQT_SIGNAL( sigExtract( bool ) ), this,
            TQT_SLOT( openWithSlotExtractDone( bool ) ) );

    if ( success )
    {
      KURL::List list;
      list.append(m_viewURL);
      KOpenWithDlg l( list, i18n("Open with:"), TQString(), (TQWidget*)0L);
      if ( l.exec() )
      {
          KService::Ptr service = l.service();
          if ( !!service )
          {
              KRun::run( *service, list );
          }
          else
          {
              TQString exec = l.text();
              exec += " %f";
              KRun::run( exec, list );
          }
      }
    }

    if( m_fileListView )
    {
        m_fileListView->setUpdatesEnabled(true);
        fixEnables();
    }
}


void
ArkWidget::prepareViewFiles( const TQStringList & fileList )
{
    TQString destTmpDirectory;
    destTmpDirectory = tmpDir();

    // Make sure to delete previous file already there...
    for(TQStringList::ConstIterator it = fileList.begin();
        it != fileList.end(); ++it)
        TQFile::remove(destTmpDirectory + *it);

    m_viewList = new TQStringList( fileList );
    arch->unarchFile( m_viewList, destTmpDirectory, true);
}

bool
ArkWidget::reportExtractFailures( const TQString & _dest, TQStringList *_list )
{
    // reports extract failures when Overwrite = False and the file
    // exists already in the destination directory.
    // If list is null, it means we are extracting all files.
    // Otherwise the list contains the files we are to extract.

    bool redoExtraction = false;
    TQString strFilename;

    TQStringList list = *_list;
    TQStringList filesExisting = existingFiles( _dest, list );

    int numFilesToReport = filesExisting.count();

    // now report on the contents
    holdBusy();
    if (numFilesToReport != 0)
    {
        redoExtraction =  ( KMessageBox::Cancel == KMessageBox::warningContinueCancelList( this,
                    i18n( "The following files will not be extracted\nbecause they "
                          "already exist:" ), filesExisting ) );
    }
    resumeBusy();
    return redoExtraction;
}

TQStringList
ArkWidget::existingFiles( const TQString & _dest, TQStringList & _list )
{
    TQString strFilename, tmp;

    TQString strDestDir = _dest;

    // make sure the destination directory has a / at the end.
    if ( !strDestDir.endsWith( "/" ) )
    {
        strDestDir += '/';
    }

    if (_list.isEmpty())
    {
        _list = m_fileListView->fileNames();
    }

    TQStringList existingFiles;
    // now the list contains all the names we must verify.
    for (TQStringList::Iterator it = _list.begin(); it != _list.end(); ++it)
    {
        strFilename = *it;
        TQString strFullName = strDestDir + strFilename;

        // if the filename ends with an "/", it means it is a directory
        if ( TQFile::exists( strFullName ) && !strFilename.endsWith("/") )
        {
            existingFiles.append( strFilename );
        }
    }
    return existingFiles;
}





bool
ArkWidget::action_extract()
{
    KURL fileToExtract;
    fileToExtract.setPath( arch->fileName() );

     //before we start, make sure the archive is still there
    if (!KIO::NetAccess::exists( fileToExtract.prettyURL(), true, this ) )
    {
        KMessageBox::error(0, i18n("The archive to extract from no longer exists."));
        return false;
    }

    //if more than one entry in the archive is root level, suggest a path prefix
    TQString prefix = m_fileListView->childCount() > 1 ?
                           TQChar( '/' ) + guessName( realURL() )
                         : TQString();

    // Should the extraction dialog show an option for extracting only selected files?
   bool enableSelected = ( m_nNumSelectedFiles > 0 ) &&
        ( m_fileListView->totalFiles() > 1);

    TQString base = ArkSettings::extractionHistory().isEmpty()?
                       TQString() : ArkSettings::extractionHistory().first();
    if ( base.isEmpty() )
    {
        // Perhaps the KDE Documents folder is a better choice?
        base = TQDir::homeDirPath();
    }

    // Default URL shown in the extraction dialog;
    KURL defaultDir( base );

    if ( m_extractOnly )
    {
        defaultDir = KURL::fromPathOrURL( TQDir::currentDirPath() );
    }

    ExtractionDialog *dlg = new ExtractionDialog( this, 0, enableSelected, defaultDir, prefix,  m_url.fileName() );

    bool bRedoExtract = false;

    // list of files to be extracted
    m_extractList = new TQStringList;
    if ( dlg->exec() )
    {
        //m_extractURL will always be the location the user chose to
        //m_extract to, whether local or remote
        m_extractURL = dlg->extractionDirectory();

        //extractDir will either be the real, local extract dir,
        //or in case of a extract to remote location, a local tmp dir
        TQString extractDir;

        if ( !m_extractURL.isLocalFile() )
        {
            m_extractRemoteTmpDir = new KTempDir( tmpDir() + "extremote" );
            m_extractRemoteTmpDir->setAutoDelete( true );

            extractDir = m_extractRemoteTmpDir->name();
            m_extractRemote = true;
            if ( m_extractRemoteTmpDir->status() != 0 )
            {
                kdWarning( 1601 ) << "Unable to create temporary directory" << extractDir << endl;
                m_extractRemote = false;
                delete dlg;
                return false;
            }
        }
        else
        {
            extractDir = m_extractURL.path();
        }

        // if overwrite is false, then we need to check for failure of
        // extractions.
        bool bOvwrt = ArkSettings::extractOverwrite();

        if ( dlg->selectedOnly() == false )
        {
            if (!bOvwrt)  // send empty list to indicate we're extracting all
            {
                bRedoExtract = reportExtractFailures(extractDir, m_extractList);
            }

            if (!bRedoExtract) // if the user's OK with those failures, go ahead
            {
                // unless we have no space!
                if ( ArkUtils::diskHasSpace( extractDir, m_nSizeOfFiles ) )
                {
                    disableAll();
                    busy( i18n( "Extracting..." ) );
                    connect( arch, TQT_SIGNAL( sigExtract( bool ) ), this, TQT_SLOT( slotExtractDone(bool) ) );
                    arch->unarchFile(0, extractDir);
                }
            }
        }
        else
        {
                KIO::filesize_t nTotalSize = 0;
                // make a list to send to unarchFile
                TQStringList selectedFiles = m_fileListView->selectedFilenames();
                for ( TQStringList::const_iterator it = selectedFiles.constBegin();
                      it != selectedFiles.constEnd();
                      ++it )
                {
                    m_extractList->append( TQFile::encodeName( *it ) );
                }

                if (!bOvwrt)
                {
                    bRedoExtract = reportExtractFailures(extractDir, m_extractList);
                }
                if (!bRedoExtract)
                {
                    if (ArkUtils::diskHasSpace(extractDir, nTotalSize))
                    {
                        disableAll();
                        busy( i18n( "Extracting..." ) );
                        connect( arch, TQT_SIGNAL( sigExtract( bool ) ),
                                 this, TQT_SLOT( slotExtractDone(bool) ) );
                        arch->unarchFile(m_extractList, extractDir); // extract selected files
                    }
                }
        }

        delete dlg;
    }
    else
    {
        delete dlg;
        return false;
    }

    // user might want to change some options or the selection...
    if (bRedoExtract)
    {
        return action_extract();
    }

    return true;
}

void
ArkWidget::action_edit()
{
    // begin an edit. This is like a view, but once the process exits,
    // the file is put back into the archive. If the user tries to quit or
    // close the archive, there will be a warning that any changes to the
    // files open under "Edit" will be lost unless the archive remains open.
    // [hmm, does that really make sense? I'll leave it for now.]

    busy( i18n( "Extracting..." ) );
    connect( arch, TQT_SIGNAL( sigExtract( bool ) ), this,
                        TQT_SLOT( editSlotExtractDone() ) );
    showCurrentFile();
}

void
ArkWidget::editSlotExtractDone()
{
    disconnect( arch, TQT_SIGNAL( sigExtract( bool ) ),
                this, TQT_SLOT( editSlotExtractDone() ) );
    ready();
    editStart();

    // avoid race condition, don't do updates if application is exiting
    if( m_fileListView )
    {
        m_fileListView->setUpdatesEnabled(true);
        fixEnables();
    }
}

void
ArkWidget::editStart()
{
    kdDebug(1601) << "Edit in progress..." << endl;
    KURL::List list;
    // edit will be in progress until the KProcess terminates.
    KOpenWithDlg l( list, i18n("Edit with:"),
            TQString(), (TQWidget*)0L );
    if ( l.exec() )
    {
        KProcess *kp = new KProcess;

        *kp << l.text() << m_strFileToView;
        connect( kp, TQT_SIGNAL(processExited(KProcess *)),
                this, TQT_SLOT(slotEditFinished(KProcess *)) );
        if ( kp->start(KProcess::NotifyOnExit, KProcess::AllOutput) == false )
        {
            KMessageBox::error(0, i18n("Trouble editing the file..."));
        }
    }
}

void
ArkWidget::slotEditFinished(KProcess *kp)
{
    kdDebug(1601) << "+ArkWidget::slotEditFinished" << endl;
    connect( arch, TQT_SIGNAL( sigAdd( bool ) ), this, TQT_SLOT( editSlotAddDone( bool ) ) );
    delete kp;
    TQStringList list;
    // now put the file back into the archive.
    list.append(m_strFileToView);
    disableAll();


    // BUG: this puts any edited file back at the archive toplevel...
    // there's only one file, and it's in the temp directory.
    // If the filename has more than three /'s then we should
    // change to the first level directory so that the paths
    // come out right.
    TQStringList::Iterator it = list.begin();
    TQString filename = *it;
    TQString path;
    if (filename.contains('/') > 3)
    {
        kdDebug(1601) << "Filename is originally: " << filename << endl;
        int i = filename.find('/', 5);
        path = filename.left(1+i);
        kdDebug(1601) << "Changing to dir: " << path << endl;
        TQDir::setCurrent(path);
        filename = filename.right(filename.length()-i-1);
        // HACK!! We need a relative path. If I have "file:", it
        // will look like an absolute path. So five spaces here to get
        // chopped off later....
        filename = "     " + filename;
        *it = filename;
    }

    busy( i18n( "Readding edited file..." ) );
    arch->addFile( list );

    kdDebug(1601) << "-ArkWidget::slotEditFinished" << endl;
}

void
ArkWidget::editSlotAddDone( bool success )
{
    ready();
    disconnect( arch, TQT_SIGNAL( sigAdd( bool ) ), this, TQT_SLOT( editSlotAddDone( bool ) ) );
    slotAddDone( success );
}

void
ArkWidget::action_view()
{
    connect( arch, TQT_SIGNAL( sigExtract( bool ) ), this,
             TQT_SLOT( viewSlotExtractDone( bool ) ) );
    busy( i18n( "Extracting file to view" ) );
    showCurrentFile();
}

void
ArkWidget::action_test()
{
    connect( arch, TQT_SIGNAL( sigTest( bool ) ), this,
             TQT_SLOT( slotTestDone( bool ) ) );
    busy( i18n( "Testing..." ) );
    arch->test();
}

void
ArkWidget::slotTestDone(bool ok)
{
    disconnect( arch, TQT_SIGNAL( sigTest( bool ) ), this,
             TQT_SLOT( slotTestDone( bool ) ) );
    ready();
    if( ok )
	KMessageBox::information(0, i18n("Test successful."));
}

void
ArkWidget::viewSlotExtractDone( bool success )
{
    if ( success )
    {
        chmod( TQFile::encodeName( m_strFileToView ), 0400 );
        bool view = true;

        if ( ArkSettings::useIntegratedViewer() )
        {
            ArkViewer * viewer = new ArkViewer( this, "viewer" );

            if ( !viewer->view( m_viewURL ) )
            {
                TQString text = i18n( "The internal viewer is not able to display this file. Would you like to view it using an external program?" );
                view = ( KMessageBox::warningYesNo( this, text, TQString(), i18n("View Externally"), i18n("Do Not View") ) == KMessageBox::Yes );

                if ( view )
                    viewInExternalViewer( this, m_viewURL );
            }
        }
        else
        {
            viewInExternalViewer( this, m_viewURL );
        }
    }

    disconnect( arch, TQT_SIGNAL( sigExtract( bool ) ), this,
                TQT_SLOT( viewSlotExtractDone( bool ) ) );

    delete m_viewList;

    // avoid race condition, don't do updates if application is exiting
    if( m_fileListView )
    {
        m_fileListView->setUpdatesEnabled(true);
        fixEnables();
    }
    ready();
}


void
ArkWidget::showCurrentFile()
{
    if ( !m_fileListView->currentItem() )
        return;

    TQString name = m_fileListView->currentItem()->fileName();

    TQString fullname = tmpDir();
    fullname += name;

    if(fullname.contains("../"))
        fullname.remove("../");

    //Convert the TQString filename to KURL to escape the bad characters
    m_viewURL.setPath(fullname);

    m_strFileToView = fullname;
    kdDebug(1601) << "File to be extracted: " << m_viewURL << endl;

    TQStringList extractList;
    extractList.append(name);

    if (ArkUtils::diskHasSpace( tmpDir(), m_fileListView->currentItem()->fileSize() ) )
    {
        disableAll();
        prepareViewFiles( extractList );
    }
}

// Popup /////////////////////////////////////////////////////////////

void
ArkWidget::setArchivePopupEnabled( bool b )
{
    m_bArchivePopupEnabled = b;
}

void
ArkWidget::doPopup( TQListViewItem *pItem, const TQPoint &pPoint, int nCol ) // slot
// do the right-click popup menus
{
    if ( nCol == 0 || !m_bArchivePopupEnabled )
    {
        m_fileListView->setCurrentItem(pItem);
        m_fileListView->setSelected(pItem, true);
        emit signalFilePopup( pPoint );
    }
    else // clicked anywhere else but the name column
    {
        emit signalArchivePopup( pPoint );
    }
}


void
ArkWidget::viewFile( TQListViewItem* item ) // slot
// show contents when double click
{
    // Preview, if it is a file
    if ( item->childCount() == 0)
        emit action_view();
    else  // Change opened state if it is a dir
        item->setOpen( !item->isOpen() );
}


// Service functions /////////////////////////////////////////////////

void
ArkWidget::slotSelectionChanged()
{
    updateStatusSelection();
}


////////////////////////////////////////////////////////////////////
//////////////////// updateStatusSelection /////////////////////////
////////////////////////////////////////////////////////////////////

void
ArkWidget::updateStatusSelection()
{
    m_nNumSelectedFiles    = m_fileListView->selectedFilesCount();
    m_nSizeOfSelectedFiles = m_fileListView->selectedSize();

    TQString strInfo;
    if (m_nNumSelectedFiles == 0)
    {
        strInfo = i18n("0 files selected");
    }
    else if (m_nNumSelectedFiles != 1)
    {
        strInfo = i18n("%1 files selected  %2")
                  .arg(KGlobal::locale()->formatNumber(m_nNumSelectedFiles, 0))
                  .arg(KIO::convertSize(m_nSizeOfSelectedFiles));
    }
    else
    {
        strInfo = i18n("1 file selected  %2")
                  .arg(KIO::convertSize(m_nSizeOfSelectedFiles));
    }

    emit setStatusBarSelectedFiles(strInfo);
    fixEnables();
}


// Drag & Drop ////////////////////////////////////////////////////////

void
ArkWidget::dragMoveEvent(TQDragMoveEvent *e)
{
    if (KURLDrag::canDecode(e) && !m_bDropSourceIsSelf)
    {
        e->accept();
    }
}


void
ArkWidget::dropEvent(TQDropEvent* e)
{
    kdDebug( 1601 ) << "+ArkWidget::dropEvent" << endl;

    KURL::List list;

    if ( KURLDrag::decode( e, list ) )
    {
        TQStringList urlList = list.toStringList();
        dropAction( urlList );
    }

    kdDebug(1601) << "-dropEvent" << endl;
}

//////////////////////////////////////////////////////////////////////
///////////////////////// dropAction /////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
ArkWidget::dropAction( TQStringList  & list )
{
    // Called by dropEvent

    // The possibilities treated are as follows:
    // drop a regular file into a window with
    //   * an open archive - add it.
    //   * no open archive - ask user to open an archive for adding file or cancel
    // drop an archive into a window with
    //   * an open archive - ask user to add to open archive or to open it freshly
    //   * no open archive - open it
    // drop many files (can be a mix of archives and regular) into a window with
    //   * an open archive - add them.
    //   * no open archive - ask user to open an archive for adding files or cancel

    // and don't forget about gzip files.

    TQString str;
    TQStringList urls; // to be sent to addFile

    str = list.first();

    if ( 1 == list.count() &&
         ( UNKNOWN_FORMAT != ArchiveFormatInfo::self()->archTypeByExtension( str ) ) )
    {
        // if there's one thing being dropped and it's an archive
        if (isArchiveOpen())
        {
            // ask them if they want to add the dragged archive to the current
            // one or open it as the new current archive
            int nRet = KMessageBox::warningYesNoCancel(this,
                       i18n("Do you wish to add this to the current archive or open it as a new archive?"),
                       TQString(),
                       i18n("&Add"), i18n("&Open"));
            if (KMessageBox::Yes == nRet) // add it
            {
                if (m_bIsSimpleCompressedFile && (m_nNumFiles == 1))
                {
                    TQString strFilename;
                    KURL url = askToCreateRealArchive();
                    strFilename = url.path();
                    if (!strFilename.isEmpty())
                    {
                        createRealArchive( strFilename, list );
                    }
                    return;
                }

                addFile( &list );
                return;
            }
            else if (KMessageBox::Cancel == nRet) // cancel
            {
                return;
            }
        }

        // if I made it here, there's either no archive currently open
        // or they selected "Open".
        KURL url = str;

        emit openURLRequest( url );
    }
    else
    {
        if (isArchiveOpen())
        {
            if (m_bIsSimpleCompressedFile && (m_nNumFiles == 1))
            {
                TQString strFilename;
                KURL url = askToCreateRealArchive();
                strFilename = url.path();
                if (!strFilename.isEmpty())
                {
                    createRealArchive( strFilename, list );
                }
                return;
            }
            // add the files to the open archive
            addFile( &list );
        }
        else
        {
            // no archive is open, so we ask if the user wants to open one
            // for this/these file/files.

            TQString str;
            str = (list.count() > 1)
                  ? i18n("There is no archive currently open. Do you wish to create one now for these files?")
                  : i18n("There is no archive currently open. Do you wish to create one now for this file?");
            int nRet = KMessageBox::warningYesNo(this, str, TQString(), i18n("Create Archive"), i18n("Do Not Create"));
            if (nRet == KMessageBox::Yes) // yes
            {
                file_new();
                if (isArchiveOpen()) // they still could have canceled!
                {
                    addFile( &list );
                }
            }
            // else // basically a cancel on the drop.
        }
    }
}

void
ArkWidget::startDrag( const TQStringList & fileList )
{
    mDragFiles = fileList;
    connect( arch, TQT_SIGNAL( sigExtract( bool ) ), this, TQT_SLOT( startDragSlotExtractDone( bool ) ) );
    prepareViewFiles( fileList );
}

void
ArkWidget::startDragSlotExtractDone( bool )
{
    disconnect( arch, TQT_SIGNAL( sigExtract( bool ) ),
                this, TQT_SLOT( startDragSlotExtractDone( bool ) ) );

    KURL::List list;

    for (TQStringList::Iterator it = mDragFiles.begin(); it != mDragFiles.end(); ++it)
    {
        KURL url;
        url.setPath( tmpDir() + *it );
        list.append( url );
    }

    KURLDrag *drg = new KURLDrag(list, m_fileListView->viewport(), "Ark Archive Drag" );
    m_bDropSourceIsSelf = true;
    drg->dragCopy();
    m_bDropSourceIsSelf = false;
}


void
ArkWidget::arkWarning(const TQString& msg)
{
    KMessageBox::information(this, msg);
}

void
ArkWidget::createFileListView()
{
   kdDebug(1601) << "ArkWidget::createFileListView" << endl;
   if ( !m_fileListView )
   {
      m_fileListView = new FileListView(this);

      connect( m_fileListView, TQT_SIGNAL( selectionChanged() ),
               this, TQT_SLOT( slotSelectionChanged() ) );
      connect( m_fileListView, TQT_SIGNAL( rightButtonPressed(TQListViewItem *, const TQPoint &, int) ),
            this, TQT_SLOT(doPopup(TQListViewItem *, const TQPoint &, int)));
      connect( m_fileListView, TQT_SIGNAL( startDragRequest( const TQStringList & ) ),
            this, TQT_SLOT( startDrag( const TQStringList & ) ) );
      connect( m_fileListView, TQT_SIGNAL( executed(TQListViewItem *, const TQPoint &, int ) ),
            this, TQT_SLOT( viewFile(TQListViewItem*) ) );
      connect( m_fileListView, TQT_SIGNAL( returnPressed(TQListViewItem * ) ),
            this, TQT_SLOT( viewFile(TQListViewItem*) ) );
    }
    m_fileListView->clear();
}


Arch * ArkWidget::getNewArchive( const TQString & _fileName, const TQString& _mimetype )
{
    Arch * newArch = 0;

    TQString type = _mimetype.isNull()? KMimeType::findByURL( KURL::fromPathOrURL(_fileName) )->name() : _mimetype;
    ArchType archtype = ArchiveFormatInfo::self()->archTypeForMimeType(type);
    kdDebug( 1601 ) << "archtype is recognised as: " << archtype << endl;
    if(0 == (newArch = Arch::archFactory(archtype, this,
                                         _fileName, _mimetype)))
    {
        KMessageBox::error(this, i18n("Unknown archive format or corrupted archive") );
        emit request_file_quit();
        return NULL;
    }

    if (!newArch->archUtilityIsAvailable())
    {
        KMessageBox::error(this, i18n("The utility %1 is not in your PATH.\nPlease install it or contact your system administrator.").arg(newArch->getArchUtility()));
        return NULL;
    }

    connect( newArch, TQT_SIGNAL(headers(const ColumnList&)),
             m_fileListView, TQT_SLOT(setHeaders(const ColumnList&)));

    m_archType = archtype;
    m_fileListView->setUpdatesEnabled(true);
    return newArch;
}

//////////////////////////////////////////////////////////////////////
////////////////////// createArchive /////////////////////////////////
//////////////////////////////////////////////////////////////////////


bool
ArkWidget::createArchive( const TQString & _filename )
{
    Arch * newArch = getNewArchive( _filename );
    if ( !newArch )
        return false;

    busy( i18n( "Creating archive..." ) );
    connect( newArch, TQT_SIGNAL(sigCreate(Arch *, bool, const TQString &, int) ),
             this, TQT_SLOT(slotCreate(Arch *, bool, const TQString &, int) ) );

    newArch->create();
    return true;
}

void
ArkWidget::slotCreate(Arch * _newarch, bool _success, const TQString & _filename, int)
{
    kdDebug( 1601 ) << k_funcinfo << endl;
    disconnect( _newarch, TQT_SIGNAL( sigCreate( Arch *, bool, const TQString &, int ) ),
                this, TQT_SLOT(slotCreate(Arch *, bool, const TQString &, int) ) );
    ready();
    if ( _success )
    {
        //file_close(); already called in ArkWidget::file_new()
        m_strArchName = _filename;
        // for the hack in compressedfile; needed when creating a compressedfile
        // then directly adding a file
        KURL u;
        u.setPath( _filename );
        setRealURL( u );

        emit setWindowCaption( _filename );
        emit addRecentURL( u );
        createFileListView();
   m_fileListView->show();
        m_bIsArchiveOpen = true;
        arch = _newarch;
        m_bIsSimpleCompressedFile =
            (m_archType == COMPRESSED_FORMAT);
        fixEnables();
	arch->createPassword();
    }
    else
    {
        KMessageBox::error(this, i18n("An error occurred while trying to create the archive.") );
    }
    emit createDone( _success );
}

//////////////////////////////////////////////////////////////////////
//////////////////////// openArchive /////////////////////////////////
//////////////////////////////////////////////////////////////////////

void
ArkWidget::openArchive( const TQString & _filename, const TQString & _password )
{
    Arch *newArch = 0;
    ArchType archtype;
    ArchiveFormatInfo * info = ArchiveFormatInfo::self();
    if ( m_openAsMimeType.isNull() )
    {
        archtype = info->archTypeForURL( m_url );
        if ( info->wasUnknownExtension() )
        {
            ArchiveFormatDlg * dlg = new ArchiveFormatDlg( this, info->findMimeType( m_url ) );
            if ( !dlg->exec() == TQDialog::Accepted )
            {
                emit setWindowCaption( TQString() );
                emit removeRecentURL( m_realURL );
                delete dlg;
                file_close();
                return;
            }
            m_openAsMimeType = dlg->mimeType();
            archtype = info->archTypeForMimeType( m_openAsMimeType );
            delete dlg;
        }
    }
    else
    {
       archtype = info->archTypeForMimeType( m_openAsMimeType );
    }

    kdDebug( 1601 ) << "m_openAsMimeType is: " << m_openAsMimeType << endl;
    if( 0 == ( newArch = Arch::archFactory( archtype, this,
                                            _filename, m_openAsMimeType) ) )
    {
        emit setWindowCaption( TQString() );
        emit removeRecentURL( m_realURL );
        KMessageBox::error( this, i18n("Unknown archive format or corrupted archive") );
        return;
    }

    if (!newArch->unarchUtilityIsAvailable())
    {
        KMessageBox::error(this, i18n("The utility %1 is not in your PATH.\nPlease install it or contact your system administrator.").arg(newArch->getUnarchUtility()));
        return;
    }

    m_archType = archtype;

    connect( newArch, TQT_SIGNAL(sigOpen(Arch *, bool, const TQString &, int)),
             this, TQT_SLOT(slotOpen(Arch *, bool, const TQString &,int)) );
    connect( newArch, TQT_SIGNAL(headers(const ColumnList&)),
             m_fileListView, TQT_SLOT(setHeaders(const ColumnList&)));

    disableAll();

    busy( i18n( "Opening the archive..." ) );
    m_fileListView->setUpdatesEnabled( false );
    arch = newArch;
    newArch->setPassword(_password);
    newArch->open();
    emit addRecentURL( m_url );
}

void
ArkWidget::slotOpen( Arch * /* _newarch */, bool _success, const TQString & _filename, int )
{
    ready();
    m_fileListView->setUpdatesEnabled(true);
    m_fileListView->triggerUpdate();

    m_fileListView->show();

    if ( _success )
    {
        TQFileInfo fi( _filename );
        TQString path = fi.dirPath( true );

        if ( !fi.isWritable() )
        {
            arch->setReadOnly(true);
            KMessageBox::information(this, i18n("This archive is read-only. If you want to save it under a new name, go to the File menu and select Save As."), i18n("Information"), "ReadOnlyArchive");
        }
        updateStatusTotals();
        m_bIsArchiveOpen = true;
        m_bIsSimpleCompressedFile = ( m_archType == COMPRESSED_FORMAT );

        if ( m_extractOnly )
        {
            extractOnlyOpenDone();
            return;
        }
        m_fileListView->adjustColumns();
        emit addOpenArk( _filename );
    }
    else
    {
        emit removeRecentURL( m_realURL );
        emit setWindowCaption( TQString() );
        KMessageBox::error( this, i18n( "An error occurred while trying to open the archive %1" ).arg( _filename ) );

        if ( m_extractOnly )
            emit request_file_quit();
    }

    fixEnables();
    emit openDone( _success );
}


void ArkWidget::slotShowSearchBarToggled( bool b )
{
   if ( b )
   {
      m_searchToolBar->show();
      ArkSettings::setShowSearchBar( true );
   }
   else
   {
      m_searchToolBar->hide();
      ArkSettings::setShowSearchBar( false );
   }
}

/**
 * Show Settings dialog.
 */
void ArkWidget::showSettings(){
  if(KConfigDialog::showDialog("settings"))
    return;

  KConfigDialog *dialog = new KConfigDialog(this, "settings", ArkSettings::self());

  General* genPage = new General(0, "General");
  dialog->addPage(genPage, i18n("General"), "ark", i18n("General Settings"));
  dialog->addPage(new Addition(0, "Addition"), i18n("Addition"), "ark_addfile", i18n("File Addition Settings"));
  dialog->addPage(new Extraction(0, "Extraction"), i18n("Extraction"), "ark_extract", i18n("Extraction Settings"));

  KTrader::OfferList offers;

  offers = KTrader::self()->query( "KonqPopupMenu/Plugin", "Library == 'libarkplugin'" );

  if ( offers.isEmpty() )
     genPage->kcfg_KonquerorIntegration->setEnabled( false );
  else
     genPage->konqIntegrationLabel->setText( TQString() );

  dialog->show();

  m_settingsAltered = true;
}

#include "arkwidget.moc"
