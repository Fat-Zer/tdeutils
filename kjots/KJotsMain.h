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

#ifndef KJotsMain_included
#define KJotsMain_included

#include <qlayout.h>
#include <qpushbutton.h>

#include <kdialogbase.h>
#include <kmainwindow.h>

class QGroupBox;
class QWidgetStack;
class KTextBrowser;
class KAccel;
class KJotsMain;
class KToolBar;
class KListBox;
class KListView;
class KPopupMenu;
class ConfigureDialog;
class KEdFind;
class KEdReplace;
class QTimer;
class KAction;
class KActionMenu;

class KJotsPage;
class KJotsBook;
class KJotsEntryBase;
class KJotsEdit;

//
// KJotsMain
//
#define ACTION_NEXT_BOOK 0
#define ACTION_PREV_BOOK 1
#define ACTION_NEXT_PAGE 2
#define ACTION_PREV_PAGE 3
#define ACTION_NEW_PAGE 4
#define ACTION_NEW_BOOK 5
#define ACTION_PAGE2TEXT 6
#define ACTION_PAGE2HTML 7
#define ACTION_BOOK2TEXT 8
#define ACTION_BOOK2HTML 9
#define ACTION_DELETE_PAGE 10
#define ACTION_DELETE_BOOK 11
#define ACTION_MANUAL_SAVE 12
#define ACTION_PRINT 13
#define ACTION_QUIT 14
#define ACTION_CUT 15
#define ACTION_COPY 16
#define ACTION_PASTE2TITLE 17
#define ACTION_PASTE 18
#define ACTION_FIND 19
#define ACTION_FIND_NEXT 20
#define ACTION_REPLACE 21
#define ACTION_RENAME 22
#define ACTION_INSERT_DATE 23
#define ACTION_MAX 24

class KJotsMain : public KMainWindow
{
    Q_OBJECT

    public:
        KJotsMain( const char* name = 0 );

        QString currentBookmarkTitle();
        QString currentBookmarkURL();

    public:
        void updateCaption();

    protected slots:
        void slotSetupInterface();
        bool createNewBook();
        void deleteBook();
        void newEntry();
        void deleteEntry();
        void slotRenameEntry();
        void configure();
        void updateConfiguration();

        void copySelection();
        void insertDate();
        void slotPrint();
        void slotSearch();
        void slotRepeatSearch();
        void slotReplace();
        void slotQuit();
        void slotFindSuccessful();

        void jumpToEntry(QListViewItem* entry, bool=false);

        void slotExpandBook(QListViewItem* item);
        void slotCollapseBook(QListViewItem* item);
        void slotItemRenamed(QListViewItem*,const QString&,int);
        void slotItemMoved(QListViewItem*,QListViewItem*,QListViewItem*);
        void writeBook();
        void writeBookToHTML();
        void saveBookToFile(bool plainText);
        void writePage();
        void writePageToHTML();
        void savePageToFile(bool plainText);

        void nextBook();
        void prevBook();
        void nextPage();
        void prevPage();

        // handling page changes in the listview
        void showListviewContextMenu(KListView* l, QListViewItem* i, const QPoint& p);
        void linkClicked(const QString&);
        void autoSave(void);

        // bookmarks
        void jumpToBookmark(const QString& page);

    protected:
        void saveProperties();
        bool queryClose();
        void updateMenu();
        void loadBooks();
        KJotsBook* currentBook();
        KJotsPage* currentPage();

        QHBoxLayout   *bg_top_layout;
        QButtonGroup  *bg_top;
        KJotsEdit   *me_text;
        KTextBrowser *roTextView;
        KListView     *subjectList;
        KJotsEntryBase *currentEntry;
        QSplitter *splitter;
        QWidgetStack *textStack;
        QFont m_font;
        QTimer*  m_autosaveTimer;
        bool invalidMoveFlag; //!< Used to fix a race condition. See Bug #109299

        KAction* actions[ACTION_MAX];
        KActionMenu *exportPageMenu, *exportBookMenu, *bookmarkMenu;
};

#endif // KJotsMain_included
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
