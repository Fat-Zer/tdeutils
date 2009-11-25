//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld
//  Copyright (C) 2002-2004 Aaron J. Seigo
//  Copyright (C) 2003 Stanislav Kljuhhin
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
#include <qheader.h>
#include <qwidgetstack.h>

#include <kio/netaccess.h>

#include <kaction.h>
#include <kapplication.h>
#include <kbookmarkmenu.h>
#include <kconfigdialog.h>
#include <kdebug.h>
#include <keditcl.h>
#include <kencodingfiledialog.h>
#include <kglobalsettings.h>
#include <kinputdialog.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <krun.h>
#include <kstandarddirs.h>
#include <kstdaccel.h>
#include <ktextbrowser.h>

#include "KJotsMain.h"
#include "KJotsSettings.h"
#include "kjotsbookmarks.h"
#include "kjotsedit.h"
#include "kjotsentry.h"
#include "confpagemisc.h"
#include "confpagefont.h"

#define KJOTSVERSION "0.7"

extern KIconLoader* global_pix_loader;

//----------------------------------------------------------------------
// KJOTSMAIN
//----------------------------------------------------------------------
KJotsMain::KJotsMain(const char* name)
    : KMainWindow(0,name),
      currentEntry(0),
      invalidMoveFlag(false)
{
    //
    // Main widget
    //
    QWidget* f_main = new QFrame( this, "Frame_0" );
    setCentralWidget(f_main);

    splitter = new QSplitter(f_main);
    splitter->setOpaqueResize( KGlobalSettings::opaqueResize() );

    // the subject list
    subjectList = new KListView(splitter, "subjectList");
    subjectList->setRootIsDecorated(true);
    subjectList->setSizePolicy(QSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding));
    subjectList->setMinimumWidth(subjectList->fontMetrics().maxWidth() * 10 + 5);
    subjectList->addColumn(i18n("Pages"));
    subjectList->setFullWidth(true);
    subjectList->setHScrollBarMode(QScrollView::AlwaysOff);
    subjectList->setDefaultRenameAction(QListView::Accept);
    subjectList->setSorting(-1);
    subjectList->setAlternateBackground(QColor());
    subjectList->header()->hide();
    subjectList->setDragEnabled(true);
    subjectList->setAcceptDrops(true);
    subjectList->setItemsMovable(true);
    subjectList->setItemsRenameable(true);
    subjectList->setRenameable(0,true);

    textStack = new QWidgetStack(splitter);

    me_text = new KJotsEdit(textStack, "me_text");
    me_text->setFocusPolicy(QWidget::StrongFocus);
    me_text->setEnabled(false);
    textStack->addWidget(me_text);
    connect(me_text, SIGNAL(findSuccessful()), this, SLOT(slotFindSuccessful()));

    roTextView = new KTextBrowser(textStack, "roTextView", true);
    textStack->addWidget(roTextView);

    QVBoxLayout *bookGrid = new QVBoxLayout(f_main, KDialog::marginHint(), KDialog::spacingHint());
    bookGrid->addWidget(splitter, 0, 0);
    bookGrid->setMargin(0);
    splitter->setSizePolicy(QSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding, 2, 1));

    // create actions
    actions[ACTION_NEXT_BOOK] = new KAction(i18n("Next Book"), QString::null, CTRL + Key_D, this,
            SLOT(nextBook()), actionCollection(), "go_next_book");
    actions[ACTION_PREV_BOOK] = new KAction(i18n("Previous Book"), QString::null, CTRL + SHIFT + Key_D, this,
            SLOT(prevBook()), actionCollection(), "go_prev_book");
    actions[ACTION_NEXT_PAGE] = new KAction(i18n("Next Page"), QString::null, CTRL + Key_PageDown, this,
            SLOT(nextPage()), actionCollection(), "go_next_page");
    actions[ACTION_PREV_PAGE] = new KAction(i18n("Previous Page"), QString::null, CTRL + Key_PageUp, this,
            SLOT(prevPage()), actionCollection(), "go_prev_page");

    actions[ACTION_NEW_PAGE] = KStdAction::openNew(this, SLOT(newEntry()), actionCollection());
    actions[ACTION_NEW_PAGE]->setText(i18n("&New Page"));
    actions[ACTION_NEW_BOOK] = new KAction(i18n("New &Book..."), "contents", CTRL + SHIFT + Key_N, this,
            SLOT(createNewBook()), actionCollection(), "new_book");

    exportPageMenu = new KActionMenu(i18n("Export Page"), actionCollection(), "save_page_to");
    actions[ACTION_PAGE2TEXT] = new KAction(i18n("To Text File..."), QString::null, 0, this,
            SLOT(writePage()), actionCollection(), "save_page_to_ascii");
    actions[ACTION_PAGE2TEXT]->plug(exportPageMenu->popupMenu());
    actions[ACTION_PAGE2HTML] = new KAction(i18n("To HTML File..."), QString::null, 0, this,
            SLOT(writePageToHTML()), actionCollection(), "save_page_to_html");
    actions[ACTION_PAGE2HTML]->plug(exportPageMenu->popupMenu());

    exportBookMenu = new KActionMenu(i18n("Export Book"), actionCollection(), "save_book_to");
    actions[ACTION_BOOK2TEXT] = new KAction(i18n("To Text File..."), QString::null, 0, this,
            SLOT(writeBook()), actionCollection(), "save_book_to_ascii");
    actions[ACTION_BOOK2TEXT]->plug(exportBookMenu->popupMenu());
    actions[ACTION_BOOK2HTML] = new KAction(i18n("To HTML File..."), QString::null, 0, this,
            SLOT(writeBookToHTML()), actionCollection(), "save_book_to_html");
    actions[ACTION_BOOK2HTML]->plug(exportBookMenu->popupMenu());

    actions[ACTION_DELETE_PAGE] = new KAction(i18n("&Delete Page"), "editdelete", CTRL + Key_Delete, this,
            SLOT(deleteEntry()), actionCollection(), "del_page");
    actions[ACTION_DELETE_BOOK] = new KAction(i18n("Delete Boo&k"), "editdelete", CTRL + SHIFT + Key_Delete, this,
            SLOT(deleteBook()), actionCollection(), "del_folder");

    actions[ACTION_MANUAL_SAVE] = new KAction(i18n("Manual Save"), 0, KStdAccel::save(),
                         this, SLOT(autoSave()), actionCollection(), "manual_save");
    actions[ACTION_PRINT] = KStdAction::print(this, SLOT(slotPrint()), actionCollection());
    actions[ACTION_QUIT] = KStdAction::quit(this, SLOT(slotQuit()), actionCollection());

    actions[ACTION_CUT] = KStdAction::cut(me_text, SLOT(cut()), actionCollection());
    connect(me_text, SIGNAL(copyAvailable(bool)), actions[ACTION_CUT], SLOT(setEnabled(bool)));
    actions[ACTION_CUT]->setEnabled(false);
    actions[ACTION_COPY]  = KStdAction::copy(me_text, SLOT(copy()), actionCollection());
    connect(me_text, SIGNAL(copyAvailable(bool)), actions[ACTION_COPY], SLOT(setEnabled(bool)));
    actions[ACTION_COPY]->setEnabled(false);
    actions[ACTION_PASTE2TITLE] = new KAction(i18n("Copy &into Page Title"), "editcopy", CTRL+Key_T, this,
            SLOT(copySelection()), actionCollection(), "copyIntoTitle");
    connect(me_text, SIGNAL(copyAvailable(bool)), actions[ACTION_PASTE2TITLE], SLOT(setEnabled(bool)));
    actions[ACTION_PASTE2TITLE]->setEnabled(false);
    actions[ACTION_PASTE]  = KStdAction::pasteText(me_text, SLOT(paste()), actionCollection());

    actions[ACTION_FIND] = KStdAction::find( this, SLOT( slotSearch() ), actionCollection() );
    actions[ACTION_FIND_NEXT] = KStdAction::findNext( this, SLOT( slotRepeatSearch() ), actionCollection() );
    actions[ACTION_FIND_NEXT]->setEnabled(false);
    actions[ACTION_REPLACE] = KStdAction::replace( this, SLOT( slotReplace() ), actionCollection() );

    actions[ACTION_RENAME] = new KAction(i18n("Rename..."), QString::null, CTRL + Key_M, this,
            SLOT(slotRenameEntry()), actionCollection(), "rename_entry");
    actions[ACTION_INSERT_DATE] = new KAction(i18n("Insert Date"), "date", CTRL + Key_I, this,
            SLOT(insertDate()), actionCollection(), "insert_date");

    KStdAction::preferences(this, SLOT(configure()), actionCollection());

    bookmarkMenu = new KActionMenu(i18n("&Bookmarks"), "bookmarks", actionCollection(), "bookmarks");
    KJotsBookmarks* bookmarks = new KJotsBookmarks(this);
    connect(bookmarks, SIGNAL(openPage(const QString&)), SLOT(jumpToBookmark(const QString&)));
    new KBookmarkMenu(KBookmarkManager::managerForFile(locateLocal("appdata","bookmarks.xml")),
                      bookmarks, bookmarkMenu->popupMenu(),
                      actionCollection(), true);
    setupGUI();
    /*
     * TODO: add a bookmark bar, something like this
    KToolBar* bar = dynamic_cast<KToolBar *>(child("bookmarkToolBar", "KToolBar"));
    new KBookmarkBar(KBookmarkManager::managerForFile(locateLocal("appdata","bookmarks.xml")),
                     bookmarks, bar, actionCollection(), this, "BookmarkBar");
    if (bar->count() == 0)
    {
        bar->hide();
    }
    */

    m_autosaveTimer = new QTimer(this, "autosave_timer");

    //
    // Set startup size.
    //
    if (!KJotsSettings::splitterSizes().isEmpty())
    {
        splitter->setSizes(KJotsSettings::splitterSizes());
    }

    //
    // Now load the books
    //
    loadBooks();

    updateConfiguration();
    updateMenu();

    QTimer::singleShot(0, this, SLOT(slotSetupInterface()));

    //connect new slots
    connect(subjectList, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(jumpToEntry(QListViewItem*)));
    connect(subjectList, SIGNAL(moved(QListViewItem*, QListViewItem*, QListViewItem*)),
            this, SLOT(slotItemMoved(QListViewItem*, QListViewItem*, QListViewItem*)));
    connect(subjectList, SIGNAL(expanded(QListViewItem*)),
            this, SLOT(slotExpandBook(QListViewItem*)));
    connect(subjectList, SIGNAL(collapsed(QListViewItem*)),
            this, SLOT(slotCollapseBook(QListViewItem*)));
    connect(subjectList, SIGNAL(contextMenu(KListView*,QListViewItem*,const QPoint&)),
            this, SLOT(showListviewContextMenu(KListView*,QListViewItem*,const QPoint&)));
    connect(subjectList, SIGNAL(itemRenamed(QListViewItem*,const QString&,int)),
            this, SLOT(slotItemRenamed(QListViewItem*,const QString&,int)));
    connect(roTextView, SIGNAL(urlClick(const QString&)),
            this, SLOT(linkClicked(const QString&)));
    connect(m_autosaveTimer, SIGNAL(timeout()),
            this, SLOT(autoSave()));
}

void KJotsMain::slotSetupInterface()
{
    if (subjectList->childCount() == 0) 
    {
        if ( !createNewBook() )
        {
            roTextView->clear();
            textStack->raiseWidget(roTextView);
        }
    }
}

/*!
    \brief Loads all the books it can find in the appdata directory.
    Needless to say: this should only ever be called once.
*/
void KJotsMain::loadBooks()
{
    QDir dir(locateLocal("appdata",""));
    QStringList files = dir.entryList(QDir::Files|QDir::Readable);
    Q_UINT64 currentSelection = KJotsSettings::currentSelection();

    //TODO: Eventually we can make use of the ".book" extension here, but for 
    //right now (2005/06/04) there's just too much legacy data out there.

    for (QStringList::Iterator it = files.begin(); it != files.end(); it++)
    {
        QString filepath = dir.absFilePath(*it);

        if (!KJotsBook::isBookFile(filepath) )
        {
            continue;
        }

        KJotsBook* book = new KJotsBook(subjectList, 0);
        book->openBook(filepath);
    }

    QListViewItemIterator it( subjectList );
    while ( it.current() ) {
        KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(it.current());
        if ( entry && entry->id() == currentSelection )
        {
            subjectList->ensureItemVisible(entry);
            subjectList->setSelected(entry, true);
            jumpToEntry(entry);
            break;
        }
        ++it;
    }

    return;
}

bool KJotsMain::createNewBook()
{
    bool ok;

    QString name = KInputDialog::getText( i18n( "New Book" ),
        i18n( "Book name:" ), QString::null, &ok, this );

    if (!ok)
        return false;

    KJotsBook* book = new KJotsBook(subjectList, 0);
    book->setSubject(name);
    book->openBook("");
    book->setOpen(true);
    subjectList->setSelected(book->firstChild(), true);
    return true;
}

void KJotsMain::slotRenameEntry()
{
    KJotsEntryBase* entry = dynamic_cast<KJotsEntryBase*>(subjectList->currentItem());

    if (entry)
    {
        entry->rename();
        jumpToEntry(entry, true); //reflect the changes on screen
    }
}

void KJotsMain::deleteBook()
{
    KJotsBook* b = currentBook();

    if (!b)
        return;

    QString msg = i18n("<qt>Are you sure you want to delete the <strong>%1</strong> book?</qt>");
    int result = KMessageBox::warningContinueCancel(topLevelWidget(), msg.arg(b->subject()), i18n("Delete Book"),KStdGuiItem::del());

    if (result!=KMessageBox::Continue)
        return;

    KJotsBook* nxt=dynamic_cast<KJotsBook*>(b->nextSibling());

    b->deleteBook();
    currentEntry = 0L;
    delete b;

    if (!nxt)
       nxt=dynamic_cast<KJotsBook*>(subjectList->firstChild());

    if (nxt)
    {
        subjectList->setSelected(nxt, true);
    }
    else
    {
        roTextView->clear();
        textStack->raiseWidget(roTextView);
        updateMenu();
    }
}

void KJotsMain::newEntry()
{
    KJotsBook* b = currentBook();

    if (b)
    {
        KJotsPage* s = b->addPage();
        subjectList->setSelected(s, true);
    }
}

void KJotsMain::deleteEntry()
{
    KJotsPage* cur = currentPage();

    if (!cur ||
        KMessageBox::warningContinueCancel(topLevelWidget(),
                                  i18n("<qt>Are you sure you want to delete the <strong>%1</strong> page?</qt>")
                                  .arg(cur->subject()),
                                  i18n("Delete Page"),KStdGuiItem::del()) != KMessageBox::Continue)
    {
        return;
    }

    KJotsBook* bk = cur->parentBook();
    KJotsPage* nxt = dynamic_cast<KJotsPage*>(cur->nextSibling());

    me_text->setEntry(0);
    delete cur;
    currentEntry = 0;

    if (!nxt)
        nxt = dynamic_cast<KJotsPage*>(bk->firstChild());

    if (nxt)
    {
        subjectList->setSelected(nxt, true);
    } else {
        subjectList->setSelected(bk, true);
    }

    updateMenu();
}

void KJotsMain::configure()
{
    if(KConfigDialog::showDialog("settings"))
        return;

    KConfigDialog* dialog = new KConfigDialog(this, "settings", KJotsSettings::self());

    confPageFont* cpFont = new confPageFont(0, "Font");
    dialog->addPage(cpFont, i18n("Font"), "fonts");

    confPageMisc* cpMisc = new confPageMisc(0, "Autosave");
    dialog->addPage(cpMisc, i18n("Autosave"), "misc");

    connect(dialog, SIGNAL(settingsChanged()), this, SLOT(updateConfiguration()));

    dialog->show();
}

void KJotsMain::slotSearch()
{
    me_text->search();
}

void KJotsMain::slotRepeatSearch()
{
    me_text->repeatSearch();
}

void KJotsMain::slotReplace()
{
    me_text->replace();
}

void KJotsMain::updateConfiguration()
{
    static int encoding = -1;

    //Keep track of the encoding setting. If the user changes it, we
    //need to mark all books dirty so they are saved correctly.
    if ( encoding == -1 ) {
        encoding = KJotsSettings::unicode();
    } else {
        if ( (int)encoding != KJotsSettings::unicode() ) {
            KJotsBook *book = dynamic_cast<KJotsBook*>(subjectList->firstChild());
            while (book) {
                book->setDirty(true);
                book = dynamic_cast<KJotsBook*>(book->nextSibling());
            }

            encoding = (int)KJotsSettings::unicode();
        }
    }

    me_text->setFont(KJotsSettings::font());
    if (KJotsSettings::autoSave())
        m_autosaveTimer->changeInterval(KJotsSettings::autoSaveInterval()*1000*60);
    else
        m_autosaveTimer->stop();
}

/*!
    \brief Saves any books that need saving.
*/
void KJotsMain::autoSave()
{
    KJotsBook *book = dynamic_cast<KJotsBook*>(subjectList->firstChild());
    while (book)
    {
        if ( book->isDirty() )
        {
            book->saveBook();
        }

        book = dynamic_cast<KJotsBook*>(book->nextSibling());
    }
}

void KJotsMain::writeBook()
{
    saveBookToFile(true);
}

void KJotsMain::writeBookToHTML()
{
    saveBookToFile(false);
}

void KJotsMain::saveBookToFile(bool plainText)
{
    KJotsBook* b = currentBook();

    if (!b)
        return;

    bool tryAgain = true;
    KEncodingFileDialog::Result res;
    while (tryAgain)
    {
        
        res = KEncodingFileDialog::getSaveURLAndEncoding();

        if (res.URLs[0].isEmpty())
        {
            tryAgain = false;
        }

        if (!KIO::NetAccess::exists(res.URLs[0], true, this) ||
            KMessageBox::warningYesNo(this, "<qt>" + i18n("The file <strong>%1</strong> already exists. Do you wish to overwrite it?").arg(res.URLs[0].prettyURL()) + "</qt>", i18n("File Exists"), i18n("Overwrite"), KStdGuiItem::cancel()) == KMessageBox::Yes)
        {
            tryAgain = false;
        }
    }

    if (!res.URLs[0].isEmpty())
    {
        b->saveToFile(res.URLs[0], plainText, res.encoding);
    }
}

void KJotsMain::writePage()
{
    savePageToFile(true);
}

void KJotsMain::writePageToHTML()
{
    savePageToFile(false);
}

void KJotsMain::savePageToFile(bool plainText)
{
    KJotsPage* e = currentPage();

    if (!e)
        return;

    bool tryAgain = true;
    KEncodingFileDialog::Result res;
    while (tryAgain)
    {
        res = KEncodingFileDialog::getSaveURLAndEncoding();

        if (res.URLs[0].isEmpty())
        {
            tryAgain = false;
        }

        if (!KIO::NetAccess::exists(res.URLs[0], true, this) ||
            KMessageBox::warningYesNo(this, "<qt>" + i18n("The file <strong>%1</strong> already exists. Do you wish to overwrite it?").arg(res.URLs[0].prettyURL()) + "</qt>", i18n("File Exists"), i18n("Overwrite"), KStdGuiItem::cancel()) == KMessageBox::Yes)
        {
            tryAgain = false;
        }
    }

    if (!res.URLs[0].isEmpty())
    {
        e->saveToFile(res.URLs[0], plainText, res.encoding);
    }
}

void KJotsMain::copySelection()
{
    //TODO: Strip HTML tags
    KJotsPage* cur = currentPage();

    if (cur && me_text->hasSelectedText())
        cur->setSubject(me_text->selectedText());
}

void KJotsMain::slotPrint()
{
    autoSave();
    KJotsEntryBase* entry = dynamic_cast<KJotsEntryBase*>(subjectList->currentItem());

    if (entry)
    {
        entry->print(m_font);
    }
}

void KJotsMain::slotQuit()
{
    close();
}

void KJotsMain::insertDate()
{
  me_text->insert(KGlobal::locale()->formatDateTime(QDateTime::currentDateTime(), true) + " ");
}

void KJotsMain::updateMenu()
{
    KJotsEntryBase* cur = dynamic_cast<KJotsEntryBase*>(subjectList->currentItem());

    if (!cur)
    {
        // no (meaningful?) selection
        actions[ACTION_DELETE_BOOK]->setEnabled(false);
        actions[ACTION_DELETE_PAGE]->setEnabled(false);
        actions[ACTION_NEW_PAGE]->setEnabled(false);
        actions[ACTION_PRINT]->setEnabled(false);
        actions[ACTION_CUT]->setEnabled(false);
        actions[ACTION_PASTE]->setEnabled(false);
        actions[ACTION_FIND]->setEnabled(false);
        actions[ACTION_REPLACE]->setEnabled(false);
        actions[ACTION_BOOK2TEXT]->setEnabled(false);
        actions[ACTION_BOOK2HTML]->setEnabled(false);
        actions[ACTION_PAGE2TEXT]->setEnabled(false);
        actions[ACTION_PAGE2HTML]->setEnabled(false);
        actions[ACTION_RENAME]->setEnabled(false);
        actions[ACTION_INSERT_DATE]->setEnabled(false);
        actions[ACTION_NEXT_BOOK]->setEnabled(false);
        actions[ACTION_PREV_BOOK]->setEnabled(false);
        actions[ACTION_NEXT_PAGE]->setEnabled(false);
        actions[ACTION_PREV_PAGE]->setEnabled(false);
        actions[ACTION_MANUAL_SAVE]->setEnabled(false);
        bookmarkMenu->setEnabled(false);
        exportPageMenu->setEnabled(false);
        exportBookMenu->setEnabled(false);
    }
    else
    {
        actions[ACTION_DELETE_BOOK]->setEnabled(true);
        actions[ACTION_NEW_PAGE]->setEnabled(true);
        actions[ACTION_PRINT]->setEnabled(true);
        actions[ACTION_FIND]->setEnabled(true);
        actions[ACTION_BOOK2TEXT]->setEnabled(true);
        actions[ACTION_BOOK2HTML]->setEnabled(true);
        actions[ACTION_RENAME]->setEnabled(true);
        actions[ACTION_MANUAL_SAVE]->setEnabled(true);
        bookmarkMenu->setEnabled(true);
        exportBookMenu->setEnabled(true);

        //TODO: Only activate if there is a next/prev book/page.
        actions[ACTION_NEXT_BOOK]->setEnabled(true);
        actions[ACTION_PREV_BOOK]->setEnabled(true);
        actions[ACTION_NEXT_PAGE]->setEnabled(true);
        actions[ACTION_PREV_PAGE]->setEnabled(true);

        if ( cur->isBook() ) 
        {
            actions[ACTION_CUT]->setEnabled(false);
            actions[ACTION_PASTE]->setEnabled(false);
            actions[ACTION_REPLACE]->setEnabled(false);
            actions[ACTION_DELETE_PAGE]->setEnabled(false);
            actions[ACTION_PAGE2TEXT]->setEnabled(false);
            actions[ACTION_PAGE2HTML]->setEnabled(false);
            actions[ACTION_INSERT_DATE]->setEnabled(false);
            exportPageMenu->setEnabled(false);
        }
        else
        {
            actions[ACTION_PASTE]->setEnabled(true);
            actions[ACTION_REPLACE]->setEnabled(true);
            actions[ACTION_DELETE_PAGE]->setEnabled(true);
            actions[ACTION_PAGE2TEXT]->setEnabled(true);
            actions[ACTION_PAGE2HTML]->setEnabled(true);
            actions[ACTION_INSERT_DATE]->setEnabled(true);
            exportPageMenu->setEnabled(true);
        }
    }
}

void KJotsMain::slotFindSuccessful()
{
    actions[ACTION_FIND_NEXT]->setEnabled(true);
}

void KJotsMain::showListviewContextMenu(KListView*, QListViewItem* i, const QPoint& p)
{
    if ( invalidMoveFlag ) return; //Prevent race condition
    KActionMenu* am = new KActionMenu(this);

    if (!i)
    {
        // clicked on empty space
        am->insert(actions[ACTION_NEW_BOOK]);
    }
    else if (dynamic_cast<KJotsBook*>(i))
    {
        // clicked on book
        am->insert(actions[ACTION_NEW_PAGE]);
        am->insert(actions[ACTION_RENAME]);
        am->insert(exportBookMenu);
        am->insert(new KActionSeparator(am));
        am->insert(actions[ACTION_DELETE_BOOK]);
    }
    else
    {
        //clicked on page
        am->insert(actions[ACTION_NEW_PAGE]);
        am->insert(actions[ACTION_RENAME]);
        am->insert(exportPageMenu);
        am->insert(new KActionSeparator(am));
        am->insert(actions[ACTION_DELETE_PAGE]);
    }

    am->popup(p);
}

/*!
    \brief Handle link clicks in read-only HTML view
    All URL clicks in the read-only window come here. For some reason
    KTextBrowser doesn't handle anchor links itself, so we have to take
    care of those, too.
*/
void KJotsMain::linkClicked(const QString& link)
{
    if (link[0] == '#')
    {
        // internal link from TOC
        roTextView->scrollToAnchor(link.mid(1));
        return;
    }

    Q_UINT64 target = link.toULongLong();
    QListViewItemIterator it( subjectList );
    while ( it.current() ) {
        KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(it.current());
        if ( entry && entry->id() == target )
        {
            subjectList->ensureItemVisible(entry);
            subjectList->setSelected(entry, true);
            jumpToEntry(entry);
            break;
        }
        ++it;
    }

    return;
}

bool KJotsMain::queryClose()
{
    autoSave();
    saveProperties();
    return true;
}

/*!
    \brief Saves settings to the ini-like config file.
    This function only needs to be called before program termination.
*/
void KJotsMain::saveProperties()
{
    KJotsSettings::setSplitterSizes(splitter->sizes());

    KJotsEntryBase* entry = dynamic_cast<KJotsEntryBase*>(subjectList->currentItem());
    KJotsSettings::setCurrentSelection( entry ? entry->id() : 0);

    KJotsSettings::writeConfig();
    return;
}

/*!
    \brief Called when a book is opened/expanded/whatever.
*/
void KJotsMain::slotExpandBook(QListViewItem *item)
{
    KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(item);
    if ( entry ) entry->setDirty(true);
}

/*!
    \brief Called when a book is closed/collapsed/whatever.
*/
void KJotsMain::slotCollapseBook(QListViewItem *item)
{
    KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(item);
    if ( entry ) entry->setDirty(true);
}

/*!
    Returns the name of the active book or page.
*/
QString KJotsMain::currentBookmarkTitle()
{
    QString title;
    QListViewItem *item = subjectList->currentItem();

    while ( item )
    {
        KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(item);
        item = item->parent();

        if ( item )
        {
            title = ": " + entry->subject() + title;
        } else {
            title = entry->subject() + title;
        }
    }

    return title;
}

/*!
    Returns a bookmark that identifies the active book or page.
*/
QString KJotsMain::currentBookmarkURL()
{
    QString url;
    KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(subjectList->currentItem());

    if ( entry )
    {
        url.setNum( entry->id() );
    }

    return url;
}

/*!
    Accepts a bookmark from the bookmark manager and jumps to the given entry.
    \param bookmark The bookmark.
*/
void KJotsMain::jumpToBookmark(const QString& bookmark)
{
    Q_UINT64 id = bookmark.toULongLong();

    if ( id )
    {
        QListViewItemIterator it( subjectList );
        while ( it.current() ) {
            KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(it.current());
            if ( entry && entry->id() == id )
            {
                subjectList->ensureItemVisible(entry);
                subjectList->setSelected(entry, true);
                jumpToEntry(entry);
                break;
            }
            ++it;
        }
    } else {
        //This is an old bookmark. Warn, and then continue. This can all be removed at some point
        //in the future. CREATION DATE: 2005/06/10

        KMessageBox::information(this, 
            i18n("This bookmark is from an older version of KJots and is not fully supported. It may or may not work. You should delete and recreate this bookmark."), QString::null, "oldBookmarkWarning"); 

        int slash = bookmark.findRev('/');

        if (slash != -1)
        {
            QString bookName = bookmark.left(slash);
            int pageNo = bookmark.mid(slash+1).toInt();
            int pageCount = 0;

            //First, find the book. It'll only be root-level.
            KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(subjectList->firstChild());

            while (entry)
            {
                if ( entry->subject() == bookName )
                {
                    break;
                }
                entry = dynamic_cast<KJotsEntryBase*>(entry->nextSibling());
            }

            if ( !entry ) return;

            if ( !pageNo )
            {
                subjectList->setSelected(entry, true);
                jumpToEntry(entry);
            } else {
                //Now, find the right page
                entry = dynamic_cast<KJotsEntryBase*>(entry->firstChild());

                while (entry)
                {
                    KJotsPage *page = dynamic_cast<KJotsPage*>(entry);
    
                    if ( page )
                    {
                        if ( ++pageCount == pageNo )
                        {
                            static_cast<QListViewItem*>(entry)->parent()->setOpen(true);
                            subjectList->ensureItemVisible(entry);
                            subjectList->setSelected(entry, true);
                            jumpToEntry(entry);
                            break;
                        }
                    }
    
                    entry = dynamic_cast<KJotsEntryBase*>(entry->nextSibling());
                }
            }
        }
    }
}

/*!
    \brief Moves to the next book after the current selection.
*/
void KJotsMain::nextBook()
{
    QListViewItem *item = subjectList->currentItem();
    if ( !item ) return; //sanity check

    QListViewItemIterator it ( item );
    QListViewItemIterator start ( it );

    do {
        it++;

        //if we iterate off the list, start over again.
        if ( !it.current() )
        {
            it = QListViewItemIterator ( subjectList );
        }

        if ( it.current() == start.current() ) return; //Bail out if we loop.
    } while ( !dynamic_cast<KJotsBook*>(it.current()));

    subjectList->ensureItemVisible(it.current());
    subjectList->setSelected(it.current(), true);

    return;
}

/*!
    \brief Moves to the previous book before the current selection.
*/
void KJotsMain::prevBook()
{
    //Don't get caught by parent book.
    QListViewItem *startingPoint = subjectList->currentItem();
    if ( !startingPoint ) return; //sanity check

    if ( dynamic_cast<KJotsPage*>(startingPoint) )
        startingPoint = startingPoint->parent();

    QListViewItemIterator it ( startingPoint );
    do {
        it--;

        //if we iterate off the list, start over again.
        if ( !it.current() )
        {
            it = QListViewItemIterator ( subjectList->lastItem() );
        }

        if ( it.current() == startingPoint ) return; //Bail out if we loop.
    } while ( !dynamic_cast<KJotsBook*>(it.current()));

    subjectList->ensureItemVisible(it.current());
    subjectList->setSelected(it.current(), true);

    return;
}

/*!
    \brief Moves to the next page after the current selection.
*/
void KJotsMain::nextPage()
{
    QListViewItem *item = subjectList->currentItem();
    if ( !item ) return; //sanity check

    QListViewItemIterator it ( item );
    QListViewItemIterator start ( it );

    do {
        it++;

        //if we iterate off the list, start over again.
        if ( !it.current() )
        {
            it = QListViewItemIterator ( subjectList );
        }

        if ( it.current() == start.current() ) return; //Bail out if we loop.
    } while ( !dynamic_cast<KJotsPage*>(it.current()));

    subjectList->ensureItemVisible(it.current());
    subjectList->setSelected(it.current(), true);

    return;
}

/*!
    \brief Moves to the previous page before the current selection.
*/
void KJotsMain::prevPage()
{
    QListViewItem *item = subjectList->currentItem();
    if ( !item ) return; //sanity check

    QListViewItemIterator it ( item );
    QListViewItemIterator start ( it );

    do {
        it--;

        //if we iterate off the list, start over again.
        if ( !it.current() )
        {
            it = QListViewItemIterator ( subjectList->lastItem() );
        }

        if ( it.current() == start.current() ) return; //Bail out if we loop.
    } while ( !dynamic_cast<KJotsPage*>(it.current()));

    subjectList->ensureItemVisible(it.current());
    subjectList->setSelected(it.current(), true);

    return;
}

void KJotsMain::jumpToEntry(QListViewItem* entry, bool force)
{
    //This won't lose data or anything; but it does reset the cursor postion and waste cycles.
    if (currentEntry == entry && !force)
        return;

    if (entry == 0)
    {
        me_text->setEntry(0);
        currentEntry = 0;
    }
    else if (dynamic_cast<KJotsBook*>(entry))
    {
        me_text->setEntry(0); //clear any old data

        KJotsBook* book = static_cast<KJotsBook*>(entry);
        QString html = book->generateHtml(book, false);

        roTextView->setText(html);
        roTextView->setCursorPosition(0, 0);
        textStack->raiseWidget(roTextView);
        currentEntry = 0;
    }
    else
    {
        textStack->raiseWidget(me_text);
        me_text->setEntry(dynamic_cast<KJotsPage*>(entry));
        currentEntry = static_cast<KJotsEntryBase*>(entry);
    }

    updateCaption();
    updateMenu();
}

/*!
    Sets the window caption. 
*/
void KJotsMain::updateCaption()
{
    QString caption;
    QListViewItem *item = subjectList->currentItem();

    while ( item )
    {
        KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(item);
        item = item->parent();

        if ( item )
        {
            caption = " / " + entry->subject() + caption;
        } else {
            caption = entry->subject() + caption;
        }
    }

    setCaption(kapp->makeStdCaption(caption));
    return;
}

void KJotsMain::slotItemRenamed(QListViewItem* item, const QString& str, int  /*col*/)
{
    KJotsEntryBase* entry = dynamic_cast<KJotsEntryBase*>(item);

    if (entry)
    {
        entry->setSubject(str);
        entry->setDirty(true);
        updateCaption();
        jumpToEntry(entry, true); //reflect the changes on screen
    }
}

/*!
    \brief Called when an item is moved via drag-and-drop.
    \warning The whole drag-n-drop reordering system is completely booched.
    Tread lightly and test well when modifying things here.
*/
void KJotsMain::slotItemMoved(QListViewItem *item, QListViewItem* afterFirst, QListViewItem */*afterNow*/ )
{
    KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(item);

    //afterFirst doesn't work like we need it to. We want it to point to the
    //previous sibling, not just the entry directly above it.
    if ( afterFirst && afterFirst->parent() != entry->parentBook() )
    {
        QListViewItemIterator it ( afterFirst );
        afterFirst = 0;

        //Rewind backwards up the tree...
        while ( it.current() )
        {
            //No other items were above this one.
            if ( it.current() == entry->parentBook() ) 
            {
                break;
            }

            //Found a previous sibling!
            if ( it.current()->parent() == entry->parentBook() )
            {
                afterFirst = it.current();
                break;
            }

            --it;
        }
    }

    KJotsEntryBase *entryAfter = dynamic_cast<KJotsEntryBase*>(afterFirst);

    if ( entry->isPage() && !item->parent() )
    {
        invalidMoveFlag=true; //Don't do anything while we're stuck here.
        subjectList->setItemsMovable(false);
        subjectList->setItemsRenameable(false);

        // We don't accept pages in the root. Create a new book?
        if (KMessageBox::questionYesNo(this,
                                       i18n("All pages must be inside a book. "
                                            "Would you like to create a new book to put the page in, "
                                            "or would you prefer to not move the page at all?"),
                                       QString::null,
                                       i18n("Create New Book"), i18n("Do Not Move Page")) ==
            KMessageBox::No ||
            !createNewBook())
        {
            //We have to move item back.
            subjectList->takeItem(item);
            entry->parentBook()->insertItem(item);
            entry->moveItem(afterFirst);
        } else {
            subjectList->takeItem(item);
            delete currentBook()->firstChild();
            currentBook()->insertItem(item);
            subjectList->ensureItemVisible(item);
            subjectList->setSelected(item, true);
        }

        invalidMoveFlag=false;
        subjectList->setItemsMovable(true);
        subjectList->setItemsRenameable(true);
    }

    if ( entry ) 
    {
        entry->resetParent();
        entry->setDirty(true);
    }

    if ( entryAfter )
    {
        //Set the old parent book to dirty, since it just lost a page.
        if ( entryAfter->isBook() )
            entryAfter->setDirty(true);
        else
            entryAfter->parentBook()->setDirty(true);
    }

    return;
}

/*!
    Returns a pointer to the currently selected book, or the book that owns
    the currently selected page.
*/
KJotsBook* KJotsMain::currentBook()
{
    KJotsEntryBase* e = dynamic_cast<KJotsEntryBase*>(subjectList->currentItem());

    while (e)
    {
        if (dynamic_cast<KJotsBook*>(e))
        {
            return dynamic_cast<KJotsBook*>(e);
        }
        e = dynamic_cast<KJotsEntryBase*>(e->KListViewItem::parent());
    }

    return 0;
}

/*!
    Returns the currently selected page or 0 if no page is selected. 
*/
KJotsPage* KJotsMain::currentPage()
{
    return dynamic_cast<KJotsPage*>(subjectList->currentItem());
}

void addBookmark()
{
}

void editBookmarks()
{
}

#include "KJotsMain.moc"
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
