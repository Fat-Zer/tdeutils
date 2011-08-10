/*

 ark -- archiver for the KDE project

 Copyright (C) 2002-2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>
 Copyright (C) 2003: Helio Chissini de Castro <helio@conectiva.com>

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

// QT includes
#include <tqlayout.h>

// KDE includes
#include <kdebug.h>
#include <klocale.h>
#include <kedittoolbar.h>
#include <kstatusbar.h>
#include <kfiledialog.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <kparts/componentfactory.h>
#include <kparts/browserextension.h>
#include <kkeydialog.h>
#include <kcombobox.h>
#include <kio/netaccess.h>
#include <kaccel.h>

// ark includes
#include "arkapp.h"
#include "settings.h"
#include "archiveformatinfo.h"
#include "arkwidget.h"

MainWindow::MainWindow( TQWidget * /*tqparent*/, const char *name )
	: KParts::MainWindow(), progressDialog( 0 )
{
    setXMLFile( "arkui.rc" );
    m_part = KParts::ComponentFactory::createPartInstanceFromLibrary<KParts::ReadWritePart>( "libarkpart", this, name, TQT_TQOBJECT(this), "ArkPart");
    if (m_part )
    {
        //Since most of the functionality is still in ArkWidget:
        m_widget = static_cast< ArkWidget* >( m_part->widget() );

        setStandardToolBarMenuEnabled( true );
        setupActions();

        connect( m_part->widget(), TQT_SIGNAL( request_file_quit() ), this, TQT_SLOT(  file_quit() ) );
        connect( KParts::BrowserExtension::childObject( m_part ), TQT_SIGNAL( openURLRequestDelayed
                                              ( const KURL &, const KParts::URLArgs & ) ),
                 m_part, TQT_SLOT( openURL( const KURL & ) ) );

        m_widget->setArchivePopupEnabled( true );
        connect( m_part->widget(), TQT_SIGNAL( signalArchivePopup( const TQPoint & ) ), this,
                 TQT_SLOT( slotArchivePopup( const TQPoint & ) ) );

        connect( m_part, TQT_SIGNAL( removeRecentURL( const KURL & ) ), this,
                 TQT_SLOT( slotRemoveRecentURL( const KURL & ) ) );
        connect( m_part, TQT_SIGNAL( addRecentURL( const KURL & ) ), this,
                 TQT_SLOT( slotAddRecentURL( const KURL & ) ) );
        connect( m_part, TQT_SIGNAL( fixActionState( const bool & ) ), this,
                 TQT_SLOT( slotFixActionState( const bool & ) ) );
        connect( m_widget, TQT_SIGNAL( disableAllActions() ), this,
                 TQT_SLOT( slotDisableActions() ) );

        ArkApplication::getInstance()->addWindow();
        connect( m_widget, TQT_SIGNAL( removeOpenArk( const  KURL &) ), this,
                 TQT_SLOT( slotRemoveOpenArk( const KURL & ) ) );
        connect( m_widget, TQT_SIGNAL( addOpenArk( const  KURL & ) ), this,
                 TQT_SLOT( slotAddOpenArk( const KURL & ) ) );

        setCentralWidget( m_part->widget() );
        createGUI( m_part );

        if ( !initialGeometrySet() )
        {
            resize( 640, 300 );
        }
        setAutoSaveSettings( "MainWindow" );
    }
    else
        kdFatal( 1601 ) << "libark could not found. Aborting. " << endl;

}



MainWindow::~MainWindow()
{
    ArkApplication::getInstance()->removeWindow();
    delete m_part;
    delete progressDialog;
    progressDialog = 0;
}

void
MainWindow::setupActions()
{
    newWindowAction = new KAction(i18n("New &Window"), "window_new", KShortcut(), TQT_TQOBJECT(this),
                                  TQT_SLOT(file_newWindow()), actionCollection(), "new_window");

    newArchAction = KStdAction::openNew(TQT_TQOBJECT(this), TQT_SLOT(file_new()), actionCollection());
    openAction = KStdAction::open(TQT_TQOBJECT(this), TQT_SLOT(file_open()), actionCollection());

    reloadAction = new KAction(i18n("Re&load"), "reload", KStdAccel::shortcut( KStdAccel::Reload ), TQT_TQOBJECT(this),
                               TQT_SLOT(file_reload()), actionCollection(), "reload_arch");
    closeAction = KStdAction::close(TQT_TQOBJECT(this), TQT_SLOT(file_close()), actionCollection(), "file_close");

    recent = KStdAction::openRecent(TQT_TQOBJECT(this), TQT_SLOT(openURL(const KURL&)), actionCollection());
    recent->loadEntries(kapp->config());

    createStandardStatusBarAction();

    KStdAction::quit(TQT_TQOBJECT(this), TQT_SLOT(window_close()), actionCollection());
    KStdAction::configureToolbars(TQT_TQOBJECT(this), TQT_SLOT(editToolbars()), actionCollection());
    KStdAction::keyBindings(TQT_TQOBJECT(this), TQT_SLOT( slotConfigureKeyBindings()), actionCollection());

    openAction->setEnabled( true );
    recent->setEnabled( true );
    closeAction->setEnabled( false );
    reloadAction->setEnabled( false );
}

void
MainWindow::slotDisableActions()
{
    openAction->setEnabled(false);
    newArchAction->setEnabled(false);
    closeAction->setEnabled(false);
    reloadAction->setEnabled(false);
}

void
MainWindow::slotFixActionState( const bool & bHaveFiles )
{
    openAction->setEnabled(true);
    newArchAction->setEnabled(true);
    closeAction->setEnabled(bHaveFiles);
    reloadAction->setEnabled(bHaveFiles);
}

void
MainWindow::file_newWindow()
{
    MainWindow *kw = new MainWindow;
    kw->resize( 640, 300 );
    kw->show();
}

void
MainWindow::file_new()
{
    m_widget->file_new();
}

void
MainWindow::file_reload()
{
    KURL url( m_part->url() );
    file_close();
    m_part->openURL( url );
}

void
MainWindow::editToolbars()
{
    saveMainWindowSettings( KGlobal::config(), TQString::tqfromLatin1("MainWindow") );
    KEditToolbar dlg( factory(), this );
    connect(&dlg, TQT_SIGNAL( newToolbarConfig() ), this, TQT_SLOT( slotNewToolbarConfig() ));
    dlg.exec();
}

void
MainWindow::slotNewToolbarConfig()
{
    createGUI( m_part );
    applyMainWindowSettings( KGlobal::config(), TQString::tqfromLatin1("MainWindow") );
}

void
MainWindow::slotConfigureKeyBindings()
{
    KKeyDialog dlg( true, this );

    dlg.insert( actionCollection() );
    dlg.insert( m_part->actionCollection() );

    dlg.configure();
}

void
MainWindow::slotArchivePopup( const TQPoint &pPoint)
{
    static_cast<KPopupMenu *>(factory()->container("archive_popup", this))->popup(pPoint);
}

// see if the ark is already open in another window
bool
MainWindow::arkAlreadyOpen( const KURL & url )
{
    if (ArkApplication::getInstance()->isArkOpenAlready(url))
    {
        if ( m_part->url() == url ) return true;
        // raise the window containing the already open archive
        ArkApplication::getInstance()->raiseArk(url);

        // close this window
        window_close();

        // notify the user what's going on
        KMessageBox::information(0, i18n("The archive %1 is already open and has been raised.\nNote: if the filename does not match, it only means that one of the two is a symbolic link.").tqarg(url.prettyURL()));
        return true;
    }
    return false;
}


void
MainWindow::openURL( const KURL & url, bool tempFile )
{
    if( !arkAlreadyOpen( url ) )
    {
        if ( tempFile && url.isLocalFile() )
            m_widget->deleteAfterUse( url.path() );
        m_part->openURL( url );
    }
}

KURL
MainWindow::getOpenURL( bool addOnly, const TQString & caption,
                               const TQString & startDir, const TQString & suggestedName )
{
    kdDebug( 1601 ) << "startDir is: " << startDir << endl;
    TQWidget * forceFormatWidget = new TQWidget( this );
    TQHBoxLayout * l = new TQHBoxLayout( forceFormatWidget );

    TQLabel * label = new TQLabel( forceFormatWidget );
    label->setText( i18n( "Open &as:" ) );
    label->adjustSize();

    KComboBox * combo = new KComboBox( forceFormatWidget );

    TQStringList list;
    list = ArchiveFormatInfo::self()->allDescriptions();
    list.sort();
    list.prepend( i18n( "Autodetect (default)" ) );
    combo->insertStringList( list );

    TQString filter = ArchiveFormatInfo::self()->filter();
    if ( !suggestedName.isEmpty() )
    {
        filter = TQString();
        combo->setCurrentItem( list.findIndex( ArchiveFormatInfo::self()->descriptionForMimeType(
                                 KMimeType::findByPath( suggestedName, 0, true )->name() ) ) );
    }

    label->setBuddy( combo );

    l->addWidget( label );
    l->addWidget( combo, 1 );

    TQString dir;
    if ( addOnly )
        dir = startDir;
    else
        dir = ":ArkOpenDir";

    KFileDialog dlg( dir, filter, this, "filedialog", true, forceFormatWidget );
    dlg.setOperationMode( addOnly ? KFileDialog::Saving
                                  : KFileDialog::Opening );

    dlg.setCaption( addOnly ? caption : i18n("Open") );
    dlg.setMode( addOnly ? ( KFile::File | KFile::ExistingOnly )
                                  :  KFile::File );
    dlg.setSelection( suggestedName );

    dlg.exec();

    KURL url;
    url = dlg.selectedURL();

    if ( combo->currentItem() !=0 ) // i.e. != "Autodetect"
        m_widget->setOpenAsMimeType(
            ArchiveFormatInfo::self()->mimeTypeForDescription( combo->currentText() ) );
    else
        m_widget->setOpenAsMimeType( TQString() );

    return url;
}

void
MainWindow::file_open()
{
    KURL url = getOpenURL();
    if( !arkAlreadyOpen( url ) )
        m_part->openURL( url );
}

void
MainWindow::file_close()
{
    m_part->closeURL();
}

void
MainWindow::window_close()
{
    file_close();
    slotSaveProperties();
    //kdDebug(1601) << "-ArkWidget::window_close" << endl;
    close();
}

bool
MainWindow::queryClose()
{
    window_close();
    return true;
}

void
MainWindow::file_quit()
{
    window_close();
}

void
MainWindow::slotSaveProperties()
{
    recent->saveEntries(kapp->config());
}

void
MainWindow::saveProperties( KConfig* config )
{
    //TODO: make it work for URLS
    config->writePathEntry( "SMOpenedFile",m_widget->getArchName() );
    config->sync();
}


void
MainWindow::readProperties( KConfig* config )
{
    TQString file = config->readPathEntry("SMOpenedFile");
    kdDebug(1601) << "ArkWidget::readProperties( KConfig* config ) file=" << file << endl;
    if ( !file.isEmpty() )
        openURL( KURL::fromPathOrURL( file ) );
}

void
MainWindow::slotAddRecentURL( const KURL & url )
{
    recent->addURL( url );
    recent->saveEntries(kapp->config());
}

void
MainWindow::slotRemoveRecentURL( const KURL & url )
{
    recent->removeURL( url );
    recent->saveEntries(kapp->config());
}

void
MainWindow::slotAddOpenArk( const KURL & _arkname )
{
    ArkApplication::getInstance()->addOpenArk( _arkname, this );
}

void
MainWindow::slotRemoveOpenArk( const KURL & _arkname )
{
    ArkApplication::getInstance()->removeOpenArk( _arkname );
}

void
MainWindow::setExtractOnly ( bool b )
{
    m_widget->setExtractOnly(  b );
}

void
MainWindow::extractTo( const KURL & targetDirectory, const KURL & archive, bool guessName )
{
    startProgressDialog( i18n( "Extracting..." ) );
    m_widget->extractTo( targetDirectory, archive, guessName );
    m_part->openURL( archive );
}

void
MainWindow::addToArchive( const KURL::List & filesToAdd, const TQString & /*cwd*/,
                                 const KURL & archive, bool askForName )
{
    KURL archiveFile;
    if ( askForName || archive.isEmpty() )
    {
        // user definded actions in servicemus are being started by konq
        // from konqis working directory, not from the one being shown when
        // the popupmenu was requested; work around that so the user
        // sees a list of the archives in the diretory he is looking at.
        // makes it show the 'wrong' dir when being called from the commandline
        // like: /dira> ark -add /dirb/file, but well...
        KURL cwdURL;
        cwdURL.setPath( filesToAdd.first().path() );
        TQString dir = cwdURL.directory( false );

        archiveFile = getOpenURL( true, i18n( "Select Archive to Add Files To" ),
                                  dir /*cwd*/, archive.fileName() );
    }
    else
        archiveFile = archive;

    if ( archiveFile.isEmpty() )
    {
        kdDebug( 1601 ) << "no archive selected." << endl;
        file_quit();
        return;
    }

    startProgressDialog( i18n( "Compressing..." ) );

    bool exists = KIO::NetAccess::exists( archiveFile, false, m_widget );
    kdDebug( 1601 ) << archiveFile << endl;

    if ( !m_widget->addToArchive( filesToAdd, archiveFile ) )
        file_quit();
    if ( exists )
        m_part->openURL( archiveFile );
}

void
MainWindow::startProgressDialog( const TQString & text )
{
    if ( !progressDialog )
        progressDialog = new KProgressDialog( this, "progress_dialog", TQString(), text, false );
    else
        progressDialog->setLabel( text );

//    progressDialog->setWFlags( TQt::WType_TopLevel );

    progressDialog->setAllowCancel( true );
    progressDialog->setPlainCaption( i18n( "Please Wait" ) );

    progressDialog->progressBar()->setTotalSteps( 0 );
    progressDialog->progressBar()->setPercentageVisible( false );

//    progressDialog->setInitialSize( TQSize(200,100), true );
    progressDialog->setMinimumDuration( 500 );
    progressDialog->show();
    KDialog::centerOnScreen( progressDialog );
    connect( progressDialog, TQT_SIGNAL( cancelClicked() ), this, TQT_SLOT( window_close() ) );

    timer = new TQTimer( this );
    connect( timer, TQT_SIGNAL( timeout() ), this, TQT_SLOT( slotProgress() ) );

    timer->start( 200, false );
}

void
MainWindow::slotProgress()
{
    progressDialog->progressBar()->setProgress( progressDialog->progressBar()->progress() + 4 );
}


#include "mainwindow.moc"

