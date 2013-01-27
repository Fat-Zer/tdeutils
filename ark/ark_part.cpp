/*
  Copyright (C)

  2001: Macadamian Technologies Inc (author: Jian Huang, jian@macadamian.com)
  2003: Georg Robbers <Georg.Robbers@urz.uni-hd.de>
  2005: Henrique Pinto <henrique.pinto@kdemail.net>

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

#include "ark_part.h"
#include "arkfactory.h"
#include "arkwidget.h"
#include "settings.h"
#include "filelistview.h"
#include "searchbar.h"

#include <kdebug.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kaboutdata.h>
#include <kxmlguifactory.h>
#include <kstatusbar.h>
#include <kiconloader.h>
#include <tdeio/netaccess.h>
#include <kpushbutton.h>
#include <ksqueezedtextlabel.h>

#include <tqfile.h>
#include <tqtimer.h>

TDEAboutData *ArkPart::createAboutData()
{
    TDEAboutData *about = new TDEAboutData("ark", I18N_NOOP("ark"),
                                       "1.0",
                                       I18N_NOOP("Ark KParts Component"),
                                       TDEAboutData::License_GPL,
                                       I18N_NOOP( "(c) 1997-2003, The Various Ark Developers" ));
    about->addAuthor("Robert Palmbos",0, "palm9744@kettering.edu");
    about->addAuthor("Francois-Xavier Duranceau",0, "duranceau@kde.org");
    about->addAuthor("Corel Corporation (author: Emily Ezust)",0,
                     "emilye@corel.com");
    about->addAuthor("Corel Corporation (author: Michael Jarrett)", 0,
                     "michaelj@corel.com");
    about->addAuthor("Jian Huang");
    about->addAuthor( "Roberto Teixeira", 0, "maragato@kde.org" );

    return about;
}



ArkPart::ArkPart( TQWidget *parentWidget, const char * /*widgetName*/, TQObject *parent,
                  const char *name, const TQStringList &, bool readWrite )
        : KParts::ReadWritePart(parent, name)
{
    kdDebug(1601)<<"ArkPart::ArkPart"<<endl;
    setInstance(ArkFactory::instance());
    awidget = new  ArkWidget( parentWidget, "ArkWidget" );

    setWidget(awidget);
    connect( awidget, TQT_SIGNAL( fixActions() ), this, TQT_SLOT( fixEnables() ) );
    connect( awidget, TQT_SIGNAL( disableAllActions() ), this, TQT_SLOT( disableActions() ) );
    connect( awidget, TQT_SIGNAL( signalFilePopup( const TQPoint& ) ), this, TQT_SLOT( slotFilePopup( const TQPoint& ) ) );
    connect( awidget, TQT_SIGNAL( setWindowCaption( const TQString & ) ), this, TQT_SIGNAL( setWindowCaption( const TQString & ) ) );
    connect( awidget, TQT_SIGNAL( removeRecentURL( const KURL & ) ), this, TQT_SIGNAL( removeRecentURL( const KURL & ) ) );
    connect( awidget, TQT_SIGNAL( addRecentURL( const KURL & ) ), this, TQT_SIGNAL( addRecentURL( const KURL & ) ) );

    if( readWrite )
        setXMLFile( "ark_part.rc" );
    else
    {
        setXMLFile( "ark_part_readonly.rc" );
    }
    setReadWrite( readWrite );

    setupActions();

    m_ext = new ArkBrowserExtension( this, "ArkBrowserExtension" );
    connect( awidget, TQT_SIGNAL( openURLRequest( const KURL & ) ),
             m_ext, TQT_SLOT( slotOpenURLRequested( const KURL & ) ) );

    m_bar = new ArkStatusBarExtension( this );
    connect( awidget, TQT_SIGNAL( setStatusBarText( const TQString & ) ), m_bar,
                 TQT_SLOT( slotSetStatusBarText( const TQString & ) ) );
    connect( awidget, TQT_SIGNAL( setStatusBarSelectedFiles( const TQString & ) ), m_bar,
                 TQT_SLOT( slotSetStatusBarSelectedFiles( const TQString & ) ) );
    connect( awidget, TQT_SIGNAL( setBusy( const TQString & ) ), m_bar,
                 TQT_SLOT( slotSetBusy( const TQString & ) ) );
    connect( awidget, TQT_SIGNAL( setReady() ), m_bar,
                 TQT_SLOT( slotSetReady() ) );
    connect( this, TQT_SIGNAL( started(TDEIO::Job*) ), TQT_SLOT( transferStarted(TDEIO::Job*) ) );
    connect( this, TQT_SIGNAL( completed() ), TQT_SLOT( transferCompleted() ) );
    connect( this, TQT_SIGNAL( canceled(const TQString&) ),
             TQT_SLOT( transferCanceled(const TQString&) ) );

    setProgressInfoEnabled( false );
}

ArkPart::~ArkPart()
{}

void
ArkPart::setupActions()
{
    addFileAction = new KAction(i18n("Add &File..."), "ark_addfile", 0, TQT_TQOBJECT(awidget),
                                TQT_SLOT(action_add()), actionCollection(), "addfile");

    addDirAction = new KAction(i18n("Add Folde&r..."), "ark_adddir", 0, TQT_TQOBJECT(awidget),
                               TQT_SLOT(action_add_dir()), actionCollection(), "adddir");

    extractAction = new KAction(i18n("E&xtract..."), "ark_extract", 0, TQT_TQOBJECT(awidget),
                                TQT_SLOT(action_extract()),	actionCollection(), "extract");

    deleteAction = new KAction(i18n("De&lete"), "ark_delete", KShortcut(TQt::Key_Delete), TQT_TQOBJECT(awidget),
                               TQT_SLOT(action_delete()), actionCollection(), "delete");

    viewAction = new KAction(i18n("to view something","&View"), "ark_view", 0, TQT_TQOBJECT(awidget),
                             TQT_SLOT(action_view()), actionCollection(), "view");


    openWithAction = new KAction(i18n("&Open With..."), 0, TQT_TQOBJECT(awidget),
                                 TQT_SLOT(slotOpenWith()), actionCollection(), "open_with");


    editAction = new KAction(i18n("Edit &With..."), 0, TQT_TQOBJECT(awidget),
                             TQT_SLOT(action_edit()), actionCollection(), "edit");

    testAction = new KAction(i18n("&Test integrity"), 0, awidget,
                                TQT_SLOT(action_test()),	actionCollection(), "test");

    selectAllAction = KStdAction::selectAll(TQT_TQOBJECT(awidget->fileList()), TQT_SLOT(selectAll()), actionCollection(), "select_all");

    deselectAllAction =  new KAction(i18n("&Unselect All"), 0, TQT_TQOBJECT(awidget->fileList()),TQT_SLOT(unselectAll()), actionCollection(), "deselect_all");

    invertSelectionAction = new KAction(i18n("&Invert Selection"), 0, TQT_TQOBJECT(awidget->fileList()),TQT_SLOT(invertSelection()), actionCollection(), "invert_selection");

    saveAsAction = KStdAction::saveAs(this, TQT_SLOT(file_save_as()), actionCollection());

    //KStdAction::preferences(awidget, TQT_SLOT(showSettings()), actionCollection());

    ( void ) new KAction( i18n( "Configure &Ark..." ), "configure" , 0, TQT_TQOBJECT(awidget),
                                       TQT_SLOT( showSettings() ), actionCollection(), "options_configure_ark" );


    showSearchBar = new KToggleAction( i18n( "Show Search Bar" ), KShortcut(), actionCollection(), "options_show_search_bar" );
    showSearchBar->setCheckedState(i18n("Hide Search Bar"));

    showSearchBar->setChecked( ArkSettings::showSearchBar() );

    connect( showSearchBar, TQT_SIGNAL( toggled( bool ) ), TQT_TQOBJECT(awidget), TQT_SLOT( slotShowSearchBarToggled( bool ) ) );

    initialEnables();
}


void ArkPart::fixEnables()
{
    bool bHaveFiles = ( awidget->getNumFilesInArchive() > 0 );
    bool bReadOnly = false;
    bool bAddDirSupported = true;
    TQString extension;
    if ( awidget->archiveType() == ZOO_FORMAT || awidget->archiveType() == AA_FORMAT
            || awidget->archiveType() == COMPRESSED_FORMAT)
        bAddDirSupported = false;

    if (awidget->archive())
        bReadOnly = awidget->archive()->isReadOnly();

    saveAsAction->setEnabled(bHaveFiles);
    selectAllAction->setEnabled(bHaveFiles);
    deselectAllAction->setEnabled(bHaveFiles);
    invertSelectionAction->setEnabled(bHaveFiles);

    deleteAction->setEnabled(bHaveFiles && awidget->numSelectedFiles() > 0
                             && awidget->archive() && !bReadOnly);
    addFileAction->setEnabled(awidget->isArchiveOpen() &&
                              !bReadOnly);
    addDirAction->setEnabled(awidget->isArchiveOpen() &&
                             !bReadOnly && bAddDirSupported);
    extractAction->setEnabled(bHaveFiles);
    testAction->setEnabled(true);
    awidget->searchBar()->setEnabled(bHaveFiles);

    bool b = ( bHaveFiles
               && (awidget->numSelectedFiles() == 1)
               && (awidget->fileList()->currentItem()->childCount() == 0)
             );
    viewAction->setEnabled( b );
    openWithAction->setEnabled( b );
    editAction->setEnabled( b && !bReadOnly ); // You can't edit files in read-only archives
    emit fixActionState( bHaveFiles );
}

void ArkPart::initialEnables()
{
    saveAsAction->setEnabled( false );
    selectAllAction->setEnabled(false);
    deselectAllAction->setEnabled(false);
    invertSelectionAction->setEnabled(false);

    viewAction->setEnabled(false);

    deleteAction->setEnabled(false);
    extractAction->setEnabled(false);
    addFileAction->setEnabled(false);
    addDirAction->setEnabled(false);
    openWithAction->setEnabled(false);
    editAction->setEnabled(false);
    testAction->setEnabled(false);

    awidget->searchBar()->setEnabled(false);
}

void ArkPart::disableActions()
{
    saveAsAction->setEnabled(false);
    selectAllAction->setEnabled(false);
    deselectAllAction->setEnabled(false);
    invertSelectionAction->setEnabled(false);

    viewAction->setEnabled(false);
    deleteAction->setEnabled(false);
    extractAction->setEnabled(false);
    addFileAction->setEnabled(false);
    addDirAction->setEnabled(false);
    openWithAction->setEnabled(false);
    editAction->setEnabled(false);
    testAction->setEnabled(false);
    awidget->searchBar()->setEnabled(false);
}

bool ArkPart::openURL( const KURL & url )
{
    awidget->setRealURL( url );
    return KParts::ReadWritePart::openURL( TDEIO::NetAccess::mostLocalURL( url, awidget ) );
}

bool ArkPart::openFile()
{
    KURL url;
    url.setPath( m_file );
    if( !TQFile::exists( m_file ) )
    {
        emit setWindowCaption(  TQString() );
        emit removeRecentURL( awidget->realURL() );
        return false;
    }
    emit addRecentURL( awidget->realURL() );
    awidget->setModified( false );
    awidget->file_open( url );
    return true;
}

void ArkPart::file_save_as()
{
    KURL u = awidget->getSaveAsFileName();
    if ( u.isEmpty() ) // user canceled
        return;

    if ( !awidget->allowedArchiveName( u ) )
        awidget->convertTo( u );
    else if ( awidget->file_save_as( u ) )
        m_ext->slotOpenURLRequested( u );
    else
        kdWarning( 1601 ) <<  "Save As failed." << endl;
}

bool ArkPart::saveFile()
{
    return true;
}

bool ArkPart::closeArchive()
{
    awidget->file_close();
    awidget->setModified( false );
    return ReadWritePart::closeURL();
}

bool ArkPart::closeURL()
{
  if ( !isReadWrite() || !awidget->isModified() || awidget->realURL().isLocalFile() )
    return closeArchive();

  TQString docName = awidget->realURL().prettyURL();

  int res = KMessageBox::warningYesNoCancel( widget(),
          i18n( "The archive \"%1\" has been modified.\n"
                "Do you want to save it?" ).arg( docName ),
          i18n( "Save Archive?" ), KStdGuiItem::save(), KStdGuiItem::discard() );

  switch ( res )
  {
    case KMessageBox::Yes :
        return awidget->file_save_as( awidget->realURL() ) && closeArchive();

    case KMessageBox::No :
        return closeArchive();

    default : // case KMessageBox::Cancel
        return false;
  }
}

void ArkPart::slotFilePopup( const TQPoint &pPoint )
{
    if ( factory() )
        static_cast<KPopupMenu *>(factory()->container("file_popup", this))->popup(pPoint);
}

void ArkPart::transferStarted( TDEIO::Job *job )
{
    m_job = job;

    m_bar->slotSetBusy( i18n( "Downloading %1..." ).arg( m_url.prettyURL() ),
                        (job != 0), (job != 0) );

    if ( job )
    {
        disableActions();
        connect( job, TQT_SIGNAL( percent(TDEIO::Job*, unsigned long) ),
                 TQT_SLOT( progressInformation(TDEIO::Job*, unsigned long) ) );
        connect( m_bar->cancelButton(), TQT_SIGNAL( clicked() ),
                 TQT_SLOT( cancelTransfer() ) );
    }
}

void ArkPart::transferCompleted()
{
    if ( m_job )
    {
        disconnect( m_job, TQT_SIGNAL( percent(TDEIO::Job*, unsigned long) ),
                    this, TQT_SLOT( progressInformation(TDEIO::Job*, unsigned long) ) );
        m_job = 0;
    }

    m_bar->slotSetReady();
}

void ArkPart::transferCanceled( const TQString& errMsg )
{
    m_job = 0;
    if ( !errMsg.isEmpty() )
    {
        KMessageBox::error( awidget, errMsg );
    }
    initialEnables();
    m_bar->slotSetReady();
}

void ArkPart::progressInformation( TDEIO::Job *, unsigned long progress )
{
    m_bar->setProgress( progress );
}

void ArkPart::cancelTransfer()
{
    disconnect( m_bar->cancelButton(), TQT_SIGNAL( clicked() ),
                this, TQT_SLOT( cancelTransfer() ) );
    if ( m_job )
    {
        m_job->kill( false );
        transferCanceled( TQString() );
    }
}

ArkBrowserExtension::ArkBrowserExtension( KParts::ReadOnlyPart * parent, const char * name )
                : KParts::BrowserExtension( parent, name )
{
}

void ArkBrowserExtension::slotOpenURLRequested( const KURL & url )
{
    emit openURLRequest( url, KParts::URLArgs() );
}

ArkStatusBarExtension::ArkStatusBarExtension( KParts::ReadWritePart * parent )
                : KParts::StatusBarExtension( parent ),
                  m_bBusy( false ),
                  m_pStatusLabelSelect( 0 ),
                  m_pStatusLabelTotal( 0 ),
                  m_pBusyText( 0 ),
                  m_cancelButton( 0 ),
                  m_pProgressBar( 0 ),
                  m_pTimer( 0 )
{
}

ArkStatusBarExtension::~ArkStatusBarExtension()
{
}

void ArkStatusBarExtension::setupStatusBar()
{
    if ( m_pTimer                      // setup already done
         || !statusBar() )
    {
        return;
    }

    m_pTimer = new TQTimer( this );
    connect( m_pTimer, TQT_SIGNAL( timeout() ), this, TQT_SLOT( slotProgress() ) );

    m_pStatusLabelTotal = new KSqueezedTextLabel( statusBar(), "StatusLabelTotal" );
    m_pStatusLabelTotal->setFrameStyle( TQFrame::NoFrame );
    m_pStatusLabelTotal->setAlignment( AlignRight );
    m_pStatusLabelTotal->setText( i18n( "Total: 0 files" ) );

    m_pStatusLabelSelect = new TQLabel( statusBar(), "StatusLabelSelect" );
    m_pStatusLabelSelect->setFrameStyle( TQFrame::NoFrame );
    m_pStatusLabelSelect->setAlignment( AlignLeft );
    m_pStatusLabelSelect->setText(i18n( "0 files selected" ) );

    m_cancelButton = new KPushButton( SmallIcon( "cancel" ), TQString(), statusBar(), "CancelButton" );

    addStatusBarItem( m_pStatusLabelSelect, 3000, false );
    addStatusBarItem( m_pStatusLabelTotal, 3000, false );
}

void ArkStatusBarExtension::slotSetStatusBarText( const TQString & text )
{
    if ( !statusBar() )
        return;

    setupStatusBar();
    m_pStatusLabelTotal->setText( text );
}

void ArkStatusBarExtension::slotSetStatusBarSelectedFiles( const TQString & text )
{

    if ( !statusBar() )
        return;

    setupStatusBar();
    m_pStatusLabelSelect->setText( text );
}

void ArkStatusBarExtension::slotSetBusy( const TQString & text, bool showCancelButton, bool detailedProgress )
{
    if ( m_bBusy || !statusBar() )
        return;

    setupStatusBar();
    if ( !m_pBusyText )
    {
        m_pBusyText = new TQLabel( statusBar() );

        m_pBusyText->setAlignment( AlignLeft );
        m_pBusyText->setFrameStyle( TQFrame::Panel | TQFrame::Raised );
    }

    if ( !m_pProgressBar )
    {
        m_pProgressBar = new KProgress( statusBar() );
        m_pProgressBar->setFixedHeight( m_pBusyText->fontMetrics().height() );
    }

    if ( !detailedProgress )
    {
        m_pProgressBar->setTotalSteps( 0 );
        m_pProgressBar->setPercentageVisible( false );
    }
    else
    {
        m_pProgressBar->setTotalSteps(100);
        m_pProgressBar->setPercentageVisible( true );
    }

    m_pBusyText->setText( text );

    removeStatusBarItem( m_pStatusLabelSelect );
    removeStatusBarItem( m_pStatusLabelTotal );

    addStatusBarItem( m_pBusyText, 5, true );
    addStatusBarItem( m_pProgressBar, 1, true );
    if ( showCancelButton )
    {
        addStatusBarItem( m_cancelButton, 0, true );
    }

    if ( !detailedProgress )
    {
        m_pTimer->start( 200, false );
    }
    m_bBusy = true;
}

void ArkStatusBarExtension::slotSetReady()
{
    if ( !m_bBusy || !statusBar() )
        return;

    setupStatusBar();
    m_pTimer->stop();
    m_pProgressBar->setProgress( 0 );

    removeStatusBarItem( m_pBusyText );
    removeStatusBarItem( m_pProgressBar );
    removeStatusBarItem( m_cancelButton );

    addStatusBarItem( m_pStatusLabelSelect, 3000, false );
    addStatusBarItem( m_pStatusLabelTotal, 3000, false );

    m_bBusy = false;
}

void ArkStatusBarExtension::slotProgress()
{
    if ( !statusBar() )
        return;

    setupStatusBar();
    m_pProgressBar->setProgress( m_pProgressBar->progress() + 4 );
}

void ArkStatusBarExtension::setProgress( unsigned long progress )
{
    if ( m_pProgressBar && ( m_pProgressBar->totalSteps() != 0 ) )
    {
        m_pProgressBar->setProgress( progress );
    }
}

#include "ark_part.moc"
