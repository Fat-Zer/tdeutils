//
//  kjots
//
//  Copyright (C) 1997 Christoph Neerfeld
//  Copyright (C) 2002, 2003 Aaron J. Seigo
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

#include <tqdir.h>
#include <tqpainter.h>
#include <tqpaintdevicemetrics.h>
#include <tqtextcodec.h>
#include <tqdom.h>

#include <tdeapplication.h>
#include <kiconloader.h>
#include <kinputdialog.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kprogress.h>
#include <kstandarddirs.h>
#include <tdetempfile.h>

#include <tdeio/job.h>
#include <assert.h>

#include "kjotsedit.h"
#include "KJotsMain.h"
#include "KJotsSettings.h"
#include "kjotsentry.h"

// helper functions for HTML output
TQString prepForHTML(TQString text)
{
    text.replace("<", "&lt;");
    text.replace(">", "&gt;");
    text.replace("\n", "<br>");
    return text;
}

TQString htmlHeader(const TQString& title, TQTextCodec* codec)
{
    TQString head = "<html><head><meta http-equiv=\"content-type\" content=\"text/html; charset=";
    head+=codec->mimeName();
    head+="\" ><title>" + prepForHTML(title) + "</title></head><body>";
    return head;
}

TQString htmlFooter()
{
    return "</body></html>";
}

//
// KJotsEntryBase
//

KJotsEntryBase::KJotsEntryBase(TDEListView* parent, TQListViewItem* after=0)
    :TDEListViewItem(parent,after)
{
    m_id = 0;
    m_saveInProgress = m_dirty = false;
    m_parent = 0;
}

KJotsEntryBase::KJotsEntryBase(TDEListViewItem* parent, TQListViewItem* after=0)
    :TDEListViewItem(parent,after)
{
    m_id = 0;
    m_saveInProgress = m_dirty = false;
    m_parent = dynamic_cast<KJotsBook*>(parent);
}

void KJotsEntryBase::setSubject(const TQString& subj)
{
    setText(0, subj);
}

void KJotsEntryBase::setText(int column, const TQString& text)
{
    if (column == 0 && text.isEmpty())
        TDEListViewItem::setText(0, defaultSubject());
    else
        TDEListViewItem::setText(column, text);
}

/*!
    \brief Sets a new parent.
    This is mostly just used during drag-and-drop reordering in the list view.
    We need to keep track of the previous parent in case the move is invalid.
*/
void KJotsEntryBase::resetParent() 
{ 
    m_parent = dynamic_cast<KJotsBook*>(TQListViewItem::parent()); 
}

/*! 
*    \brief Set the ID number.
*    \param id The ID to set, or 0 to pick a fresh one.
*
*    Set ourselves to a particular ID number, or pick a new one. The lastId used is
*    stored in the settings. Now in theory it will eventually wrap, and may collide
*    with preexisting IDs. Seeing as how we are using an unsigned long long, I am
*    comfortable with the amount of time we have to worry about this. 
*/
void KJotsEntryBase::setId(TQ_UINT64 id)
{
    //Find an unused id...
    if ( id == 0 )
    {
        TQ_UINT64 lastId = KJotsSettings::lastId();

        //TODO: In theory, we could wrap. The universe may end beforehand tho.
        id = ++lastId; 
        if ( id == 0 ) id = ++lastId; //*IF* by some chance we do wrap, this will prevent bad things.
        KJotsSettings::setLastId(id);
    }

    m_id = id;
    return;
}

/*!
    \brief Performs functions necessary to save the entry to a KJots file.
    This function should ONLY be called when saving the file, as it performs
    other functions than generating XML.
*/
void KJotsEntryBase::generateXml( TQDomDocument &doc, TQDomElement &parent )
{
    TQDomElement title = doc.createElement( "Title" );
    title.appendChild( doc.createTextNode( subject() ));
    parent.appendChild( title );

    TQDomElement id = doc.createElement( "ID" );
    TQString id_string;
    id_string.setNum(m_id);
    id.appendChild( doc.createTextNode(id_string) );
    parent.appendChild( id );

    setDirty( false );
    return;
}

/*!
    Parses through XML data for settings inherent to the base class. 
*/
void KJotsEntryBase::parseXml( TQDomElement &e )
{
    if ( !e.isNull() ) 
    {
        if ( e.tagName() == "Title" ) 
        {
            setSubject(e.text());
        } 
        else
        if ( e.tagName() == "ID" ) 
        {
            setId(e.text().toULongLong());
        } 
    }

    return;
}

/*!
    \brief Draws the Title box when printing.
    This is a helper function for the derived classes. Code consolidation==the good.
*/
int KJotsEntryBase::printTitleBox(TQString title, KPrinter& printer, TQPainter& painter, int y)
{
    TQPaintDeviceMetrics metrics( &printer );
    int maxWidth = metrics.width();
    int maxHeight = metrics.height();

    TQRect r = painter.boundingRect(0, y,
              maxWidth, maxHeight,
              TQPainter::ExpandTabs | TQPainter::WordBreak | TQPainter::AlignHCenter,
              title);
    y += 10;

    if ((y + r.height()) > maxHeight)
    {
        printer.newPage();
        y = 0;
    }
    else
    {
        r.moveBy(0, 10);
    }

    r.setLeft(0);
    r.setRight(maxWidth);
    painter.drawRect(r);
    painter.drawText(0, y, maxWidth, maxHeight - y,
            TQPainter::ExpandTabs | TQPainter::WordBreak | TQPainter::AlignHCenter,
            title);
    y += r.height() + 15;

    return y;
}

//
// KJotsBook
//

KJotsBook::KJotsBook(TDEListView* parent, TQListViewItem* after)
    : KJotsEntryBase(parent, after)
{
    init();
}

KJotsBook::KJotsBook(TDEListViewItem* parent, TQListViewItem* after)
    : KJotsEntryBase(parent, after)
{
    init();
}

KJotsBook::~KJotsBook()
{
}

/*!
    \brief Initialize a KJotsBook object.

    This function is used for code consolidation. Otherwise, you'de have to change these 
    things in both constructors.
*/
void KJotsBook::init()
{
    m_isBook = true;
    m_open = false;
    m_saveProgressDialog = 0;
    setExpandable(true);
    setPixmap(0, kapp->iconLoader()->loadIcon(TQString("contents"),TDEIcon::Small));
}

/*!
    \brief Is this a KJots file?
    \param filename Filename to check.

    This function does a cursory check to see if this looks like it is a KJots file.
    It's a little too quick and dirty for my taste, but seems to work OK.
*/
bool KJotsBook::isBookFile(const TQString& filename)
{
    TQFile folder(filename);
    bool test = false;

    if ( folder.exists() )
    {
        if ( folder.open(IO_ReadWrite) )
        {
            TQTextStream st(TQT_TQIODEVICE(&folder));
            st.setEncoding( KJotsSettings::unicode() ? TQTextStream::UnicodeUTF8 : TQTextStream::Locale );
            TQString buf = st.readLine().stripWhiteSpace();

            //Check for new-style book. Not very pretty, but it is faster than loading the DOM.
            if (buf == "<!DOCTYPE KJots>")
            {
                test=true;
            }
            else
            //Check for old-style book. See KJotsBook::loadOldBook()
            if (buf.left(9) == "\\NewEntry")
            {
                test=true;
            }

        }
    }

    return test;
}

/*!
    \brief Reads a book in from a disk file.
    This function is only called for root-level books.
*/
bool KJotsBook::openBook(const TQString& filename)
{
    if ( !m_open ) //sanity check
    {
        listView()->setUpdatesEnabled(false);
        m_fileName = filename;

        if ( m_fileName.isEmpty() ) //new books names are empty.
        {
            addPage();
            m_open = true;
        } else {
            TQFile file(m_fileName);
    
            if ( file.exists() && file.open(IO_ReadWrite) ) //TODO: Implement read-only mode?
            {
                TQTextStream st(TQT_TQIODEVICE(&file));
                st.setEncoding(  KJotsSettings::unicode() ? TQTextStream::UnicodeUTF8 : TQTextStream::Locale );
                TQString data = st.read();

                TQDomDocument doc( "KJots" );
                if ( doc.setContent( data ) ) 
                {
                    TQDomElement docElem = doc.documentElement();
            
                    if ( docElem.tagName() == "KJots" )
                    {
                        TQDomNode n = docElem.firstChild();
                        while( !n.isNull() ) 
                        {
                            TQDomElement e = n.toElement(); // try to convert the node to an element.
                            if( !e.isNull() && e.tagName() == "KJotsBook" ) 
                            {
                                    parseXml(e);
                            }
                            n = n.nextSibling();
                        }
                    }

                } else {
                    //Woah. Something didn't work right. Maybe this is an old file?
                    file.reset();
                    if ( loadOldBook(file) ) 
                    {
                        TQFileInfo fi( file );
                        setSubject(fi.fileName());
                    }
                }
            }
    
            if ( !childCount() ) //sanity check
            {
                //We should NOT be here. That must not have been a book file, or it is damaged. 
                //To make things happy-happy, we just create a blank page and continue. 
                //TODO: Add an information message? Make a translater work for something so unlikely?
                addPage();
            }
    
            m_open = true;

            // Support legacy files. Can be removed at some point. CREATION DATE: 2005/06/04
            if ( !m_fileName.isEmpty() && !m_fileName.endsWith(".book") )
            {
                //some old books have incorrect names. So we save a new copy, then kill the old file
                m_fileName = TQString(); //trick ourselves into thinking we're a new book
                saveBook();
                file.remove();
            }
        }
    
        listView()->setUpdatesEnabled(true);

        // Support legacy files. Can be removed at some point. CREATION DATE: 2005/06/07
        if ( m_open && !id() )
        {
            //This book (and its pages) need new IDs!
            setId(0);

            KJotsPage *page = dynamic_cast<KJotsPage*>(firstChild());
            while ( page )
            {
                page->setId(0); //TODO: Make setId() a protected function if this legacy code is removed.
                page = dynamic_cast<KJotsPage*>(page->nextSibling());
            }
        }
    }

    return m_open;
}

/*!
    \brief Load an old-style book file.

    This is a function for files made from older KJots versions. It could probably be removed
    at some point in the future. CREATION DATE: 2005/05/30 
*/
bool KJotsBook::loadOldBook(TQFile &file)
{
    TQTextStream st(TQT_TQIODEVICE(&file));
    st.setEncoding(  KJotsSettings::unicode() ? TQTextStream::UnicodeUTF8 : TQTextStream::Locale );
    TQString buf = st.readLine();

    if (buf.left(9) != "\\NewEntry")
    {
        addPage();
        return false;
    }

    KJotsPage *entry = 0;
    TQString body;

    bool quit=false;

    while (1)
    {
        if (buf.left(9) == "\\NewEntry")
        {
            if (entry)
            {
                entry->setBody(body);
            }

            body = TQString();
            TQString title = buf.mid(10, buf.length());

            // now catch the page id
            buf = st.readLine();
            if (buf.left(3) == "\\ID")
            {
                buf = st.readLine();
            }

            entry = new KJotsPage(this, entry);
            entry->setSubject(title);
        }

        int pos = 0;

        while ((pos = buf.find('\\',pos)) != -1)
            if (buf[++pos] == '\\')
                buf.remove(pos, 1 );

        body.append( buf + "\n");

        if (quit)
            break;

        buf = st.readLine();

        quit = st.eof();
    }

    entry->setBody(body);
    return true;
}

/*!
    \brief Saves this book and everything in it to the data directory.
*/
void KJotsBook::saveBook(void)
{
    if (!m_open) //sanity check
        return;

    // Are we a new book?
    if ( m_fileName.isEmpty() )
    {
        KTempFile temp(locateLocal("appdata",""), ".book", 0644);
        m_fileName = temp.name();
    }

    TQFile file(m_fileName);
    if (file.open(IO_WriteOnly | IO_Truncate)) 
    {
        TQDomDocument doc("KJots");
        TQDomElement root = doc.createElement( "KJots" );
        doc.appendChild( root );

        this->generateXml( doc, root ); //recursive

        TQTextStream st((TQIODevice *) &file);
        st.setEncoding(  KJotsSettings::unicode() ? TQTextStream::UnicodeUTF8 : TQTextStream::Locale );
        st << doc.toString();

        file.close();
    }

}

/*!
    \brief Deletes a book by removing the data file. 
    This does not affect the list display, and can be called for reasons other than 
    choosing to delete an entry from the list. If you want the object to dissappear
    from the list, then you have to delete it.
*/
void KJotsBook::deleteBook()
{
    TQFile::remove(m_fileName);
    m_fileName = TQString();
}

void KJotsBook::rename()
{
    bool ok;
    TQString name = KInputDialog::getText(i18n( "Rename Book" ),
                                         i18n( "Book name:" ),
                                         subject(), &ok, listView());
    if (ok)
    {
        setSubject(name);

        KJotsMain* m = dynamic_cast<KJotsMain*>(kapp->mainWidget());

        if (m)
            m->updateCaption();

        setDirty(true);
    }
}

/*!
    \brief Initiated when the user chooses to save this to a file.
    This function gets everything ready to go to save to the disk.
*/
void KJotsBook::saveToFile(KURL url, bool plainText, const TQString& encoding)
{
    if (url.isEmpty() || m_saveInProgress)
        return;

    m_saveToPlainText = plainText;
    m_saveEncoding = TQTextCodec::codecForName(encoding.ascii());
    TDEIO::TransferJob* job = TDEIO::put(url, -1, true, false, false);

    if (!job)
    {
        return;
    }

    //NOTE: The current implementation does not need the progress dialog, but further
    //revisions will likely use it. I don't want to make the translators add, remove,
    //and re-add translations, so I'm just keeping this here for now. 
    m_saveProgressDialog = new KProgressDialog(listView(), "bookSaveInProgress",
                                               i18n("Saving %1").arg(subject()),
                                               i18n("Saving the contents of %1 to %2")
                                                    .arg(subject(), url.prettyURL()),
                                               true);

    m_saveProgressDialog->progressBar()->setTotalSteps(1);
    m_saveProgressDialog->showCancelButton(false);
    m_saveProgressDialog->setAutoClose(true);

    connect(job, TQT_SIGNAL(dataReq(TDEIO::Job*, TQByteArray&)), TQT_SLOT(saveDataReq(TDEIO::Job*, TQByteArray&)));
    connect(job, TQT_SIGNAL(result( TDEIO::Job *)), TQT_SLOT(slotSaveResult( TDEIO::Job *)));

    m_saveInProgress = true;
}

/*!
    \brief Pumps out data when saving to a file.
    This function pumps out page data during a disk save. 
*/
void KJotsBook::saveDataReq(TDEIO::Job* /* job */, TQByteArray& data)
{
    if (!m_saveInProgress) return; //sanity check

    TQTextStream stream(data, IO_WriteOnly);
    stream.setCodec(m_saveEncoding);

    if (!m_saveToPlainText)
    {
        stream << htmlHeader(subject(), m_saveEncoding);
        stream << generateHtml(this, true);
        stream << htmlFooter();
    } else {
        stream << generateText();
    }

    m_saveInProgress = false;
}

/*!
    \brief Completes a disk save.
    This function is called after a disk save. The disk save may or may 
    not have completed normally.
*/
void KJotsBook::slotSaveResult(TDEIO::Job *)
{
    m_saveInProgress = false;
    delete m_saveProgressDialog;
    m_saveProgressDialog = 0;
    /* if (job->error() != 0) {} */
}

/*!
    \brief Add a new page to the end of this book.
*/
KJotsPage* KJotsBook::addPage()
{
    TQListViewItem *after = 0;

    // append new pages
    TQListViewItem* tmp = firstChild();
    while (tmp)
    {
        after = tmp;
        tmp = tmp->nextSibling();
    }

    KJotsPage *page = new KJotsPage(this, after);
    page->initNewPage();
    return page;
}

/*!
    \brief Called when the user wants to print the book.
*/
void KJotsBook::print(TQFont& defaultFont)
{
    KPrinter printer;
    printer.setDocName(subject());
    printer.setFullPage(false);
    printer.setCreator("KJots");

    if (!printer.setup(listView(), i18n("Print: %1").arg(subject())))
    {
        return;
    }

    TQPainter painter(&printer);
    painter.setFont(defaultFont);
    print(printer, painter, 0);

    painter.end();
}

/*!
    \brief Prints this book.
*/
int KJotsBook::print(KPrinter& printer, TQPainter& painter, int y)
{
    y = printTitleBox(subject(), printer, painter, y);

    KJotsEntryBase* entry = dynamic_cast<KJotsEntryBase*>(firstChild());
    while (entry)
    {
        y = entry->print(printer, painter, y);
        entry = dynamic_cast<KJotsEntryBase*>(entry->nextSibling());
    }

    return y;
}

TQString KJotsBook::defaultSubject()
{
    return i18n("Untitled Book");
}

/*!
    \brief Does this book need to be saved?
    This function recursively checks itself, and all books and pages it owns to
    see if any changes have been made.
*/
bool KJotsBook::isDirty()
{
    //Am I dirty?
    if ( KJotsEntryBase::isDirty() ) return true;

    //Check all children to see if any of them are dirty
    KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(firstChild());
    while ( entry )
    {
        if ( entry->isDirty() ) return true;
        entry = dynamic_cast<KJotsEntryBase*>(entry->nextSibling());
    }

    return false;
}

/*!
    \brief Creates XML code and performs necessary tasks to save file.
    This function should ONLY be called when saving the file.
*/
void KJotsBook::generateXml( TQDomDocument &doc, TQDomElement &parent )
{
    TQDomElement book = doc.createElement( "KJotsBook" );
    parent.appendChild( book );

    KJotsEntryBase::generateXml(doc, book); //let the base class save important stuff

    TQDomElement open = doc.createElement( "Open" );
    open.appendChild( this->isOpen() ? doc.createTextNode("1") : doc.createTextNode("0") );
    book.appendChild( open );

    KJotsEntryBase* entry = dynamic_cast<KJotsEntryBase*>(firstChild());
    while ( entry )
    {
        entry->generateXml( doc, book );
        entry = dynamic_cast<KJotsEntryBase*>(entry->nextSibling());
    }

    if ( !m_fileName.isEmpty() && TQListViewItem::parent() )
    {
        //Hmmmm... We were originally loaded from a file, but now we have a parent, so
        //we must have been moved into another tree. Remove the old file now that we
        //have saved ourselves into the new tree.
        deleteBook();
    }

    return;
}

/*!
    Parses through XML code from a file.
*/
void KJotsBook::parseXml( TQDomElement &me )
{
    KJotsEntryBase *lastEntry = 0; //keep track of the last entry inserted for speed

    if ( me.tagName() == "KJotsBook" )
    {
        TQDomNode n = me.firstChild();
        while( !n.isNull() ) 
        {
            TQDomElement e = n.toElement(); // try to convert the node to an element.
            if ( !e.isNull() ) 
            {
                if ( e.tagName() == "KJotsPage" ) 
                {
                    KJotsPage *page = new KJotsPage(this, lastEntry);
                    page->parseXml(e);
                    lastEntry=page;
                } 
                else
                if ( e.tagName() == "KJotsBook" ) 
                {
                    KJotsBook *book = new KJotsBook(this, lastEntry);
                    book->parseXml(e);
                    lastEntry=book;
                } 
                else
                if ( e.tagName() == "Open" ) 
                {
                    if ( e.text() == "1" )
                    {
                        this->setOpen(true);
                    }
                } 
                else
                {
                    //What was this? Send it to the base class and see if it can figure it out.
                    KJotsEntryBase::parseXml(e);
                }
            }
            n = n.nextSibling();
        }
    }

    return;
}

/*!
    \brief Returns an HTML Table of contents for this book.
    This is a helper function for generateHtml().
*/
TQString KJotsBook::getToc()
{
    TQString toc;

    toc += "<ul>";

    KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(firstChild());
    while ( entry ) {
        TQString htmlSubject = prepForHTML(entry->subject());
        toc += TQString("<li><a href=\"#%1\">").arg(entry->id()) + htmlSubject + "</a></li>";

        KJotsBook *book = dynamic_cast<KJotsBook*>(entry);
        if ( book ) toc += book->getToc();

        entry = dynamic_cast<KJotsEntryBase*>(entry->nextSibling());
    }

    toc += "</ul>";
    return toc;
}

/*!
    \brief Returns HTML for the read-only "book" view.
    \param top Pointer to the "starting point" of this tree.
    \param diskMode Files saved to disk have a slightly different format.
*/
TQString KJotsBook::generateHtml( KJotsEntryBase* top, bool diskMode )
{
    TQString toc, body;
    TQString htmlTitle = prepForHTML(subject());

    if ( top == this )
    {
        toc = TQString("<h1><a name=\"%1\">%2</a></h1>").arg(id()).arg(htmlTitle);
    } else {
        if ( diskMode )
        {
            toc = TQString("<h2><a name=\"%1\">%2</a></h2>").arg(id()).arg(htmlTitle);
        } else {
            toc = TQString("<h2><a name=\"%1\" href=\"%2\">%3</a></h2>").arg(id()).arg(id()).arg(htmlTitle);
        }
    }

    toc += "<h3>" + i18n("Table of Contents") + "</h3>";
    toc += getToc();
    toc += "<hr>";

    // Do the body text of the entries.
    KJotsEntryBase* entry = dynamic_cast<KJotsEntryBase*>(firstChild());
    while ( entry )
    {
        body += entry->generateHtml(top, diskMode);
        entry = dynamic_cast<KJotsEntryBase*>(entry->nextSibling());
    }

    return toc + body;
}

/*!
    \brief Returns Text when saving to a file.

    This functions output moderately formatted text when the user chooses to save as
    a text file. 
*/
TQString KJotsBook::generateText( void )
{
    TQString out;

    //Print Fancy Text header
    TQString line, buf;
    line.fill('#', subject().length() + 2);
    line += "\n";
    out = line + TQString("# ") + subject() + TQString("\n") + line;

    KJotsEntryBase* entry = dynamic_cast<KJotsEntryBase*>(firstChild());
    while ( entry )
    {
        out += entry->generateText();
        entry = dynamic_cast<KJotsEntryBase*>(entry->nextSibling());
    }

    out += "\n";
    return out;
}

//
// KJotsPage
//

KJotsPage::KJotsPage(KJotsBook* parent, TQListViewItem *after)
    : KJotsEntryBase(parent,after),
    m_editor(0)
{
    m_isBook = false;
    m_paraPos = m_indexPos = 0;
    setPixmap(0, kapp->iconLoader()->loadIcon(TQString("edit"), TDEIcon::Small));
}

KJotsPage::~KJotsPage()
{
    if ( m_editor )
    {
        //Well now, this isn't good. We're going bye-bye but the editor
        //still thinks we're here. Set ourselves to 0 (IMPORTANT!!) and
        //then clear the editor.
        KJotsEdit *temp = m_editor;
        m_editor=0;
        temp->setEntry(0);
    }
}

/*!
    \brief Sets up info for a new page.

    This should be called on brand new pages.
*/
void KJotsPage::initNewPage(void)
{
    setId(0);
    setSubject(defaultSubject());
}

/*!
    \brief Return the text of this page.
*/
TQString KJotsPage::body()
{
    //if we're being edited we want the current text, not whatever we saved before.
    if ( m_editor && m_editor->edited() )
    {
        m_text = m_editor->text();
        m_editor->setEdited(false);
        setDirty(true);
    }

    return m_text;
}

/*!
    \brief Sets the text of this page.
*/
void KJotsPage::setBody(const TQString& text)
{
    assert ( !m_editor ); //m_editor should *never* be set.
    m_text = text;
}

void KJotsPage::rename()
{
    bool ok;

    TQString name = KInputDialog::getText(i18n( "Rename Page" ),
                                         i18n( "Page title:" ),
                                         subject(), &ok, listView() );

    if (ok)
    {
        setSubject(name);

        KJotsMain* m = dynamic_cast<KJotsMain*>(kapp->mainWidget());

        if (m)
            m->updateCaption();

        setDirty(true);
    }
}

/*!
    \brief Initiated when the user chooses to save this to a file.
    This function gets everything ready to go to save to the disk.
*/
void KJotsPage::saveToFile(KURL url, bool plainText, const TQString& encoding)
{
    if (url.isEmpty() || m_saveInProgress)
        return;

    m_saveToPlainText = plainText;
    m_saveEncoding = TQTextCodec::codecForName(encoding.ascii());
    TDEIO::TransferJob* job = TDEIO::put(url, -1, true, false, false);
    if (!job)
    {
        return;
    }

    connect(job, TQT_SIGNAL(dataReq(TDEIO::Job*, TQByteArray&)), TQT_SLOT(saveDataReq(TDEIO::Job*, TQByteArray&)));
    connect(job, TQT_SIGNAL(result( TDEIO::Job *)), TQT_SLOT(slotSaveResult( TDEIO::Job *)));
    m_saveInProgress = true;
}

/*!
    \brief Pumps out data when saving to a file.
    This function pumps out page data during a disk save. 

    \todo This should be augmented to cycle in case of REALLY long pages.
*/
void KJotsPage::saveDataReq(TDEIO::Job* /* job */, TQByteArray& data)
{
    if (!m_saveInProgress) return; //sanity check

    TQTextStream stream(data, IO_WriteOnly);
    stream.setCodec(m_saveEncoding);

    if ( !m_saveToPlainText ) 
    {
        stream << htmlHeader(subject(), m_saveEncoding);
        stream << generateHtml(this, true);
        stream << htmlFooter();
    } else {
        stream << generateText();
    }

    m_saveInProgress = false;
}

/*!
    \brief Completes a disk save.
    This function is called after a disk save. The disk save may or may 
    not have completed normally.
*/
void KJotsPage::slotSaveResult(TDEIO::Job *)
{
    m_saveInProgress = false;
    /* if (job->error() != 0) {} */
}

void KJotsPage::print(TQFont& defaultFont)
{
    KJotsEntryBase* book = dynamic_cast<KJotsEntryBase*>(TDEListViewItem::parent());

    TQString docName = book->subject();
    if (!subject().isNull())
    {
        docName += ": " + subject();
    }

    KPrinter printer;
    printer.setDocName(docName);
    printer.setFullPage(false);
    printer.setCreator("KJots");

    if (printer.setup(listView(), i18n("Print: %1").arg(docName)))
    {
        TQPainter painter( &printer );
        painter.setFont(defaultFont);
        print(printer, painter, 0);
        painter.end();
    }
}

/*!
    \brief Prints this page.
*/
int KJotsPage::print(KPrinter& printer, TQPainter& painter, int y)
{
    TQPaintDeviceMetrics metrics( &printer );
    int maxWidth = metrics.width();
    int maxHeight = metrics.height();
    //TODO: Is it really necessary to copy the entire body when printing?
    TQStringList paragraphs = TQStringList::split(TQChar('\n'), body(), true);

    y = printTitleBox(subject(), printer, painter, y);

    for (TQStringList::iterator para = paragraphs.begin();
         para != paragraphs.end();
         ++para)
    {
        //Watch for blank lines inserted as spacers.
        if ( (*para).isNull() ) *para = " ";

        TQRect r = painter.boundingRect(0, y,
                                       maxWidth, maxHeight,
                                       TQPainter::ExpandTabs | TQPainter::WordBreak,
                                       *para);

        if ((y + r.height()) > maxHeight)
        {
            printer.newPage();
            y = 0;
        }

        painter.drawText(0, y, maxWidth, maxHeight - y,
                TQPainter::ExpandTabs | TQPainter::WordBreak,
                *para);
        y += r.height();
    }

    return y;
}

/*!
    \brief Get the default title of this page.
    \note This is only used by initNewPage(); it could probably be removed.
*/
TQString KJotsPage::defaultSubject()
{
    int page = 0;

    if ( parentBook() )
    {
        //We can't use childCount() here because it will count books, too.
        KJotsEntryBase *entry = dynamic_cast<KJotsEntryBase*>(parentBook()->firstChild());
        while ( entry )
        {
            if ( entry->isPage() ) ++page;
            entry = dynamic_cast<KJotsEntryBase*>(entry->nextSibling());
        }
    } else {
        //We should never be here.
        page = 1;
    }

    return i18n("Page %1").arg(page);
}

/*!
    \brief Tells this page that it has been loaded into the edit window.
*/
void KJotsPage::setEditor( KJotsEdit *editor )
{
    //out with the old...
    if ( m_editor )
    {
        m_editor->getCursorPosition(&m_paraPos, &m_indexPos);

        if ( m_editor->edited() )
        {
            m_text = m_editor->text();
            setDirty(true);
        }
    }

    m_editor = editor;

    //and in with the new
    if ( m_editor )
    {
        m_editor->setEdited(false);
        m_editor->setCursorPosition(m_paraPos, m_indexPos);
    }

    return;
}

/*!
    \brief Has the data in this page been changed?
    We first check the editor status, if we are being edited. Then we let 
    the base class handle things for us.
*/
bool KJotsPage::isDirty()
{
    if ( m_editor && m_editor->edited() )
    {
        setDirty(true);
    }

    return KJotsEntryBase::isDirty();
}

/*!
    \brief Creates XML code and performs necessary tasks to save file.
    This function should ONLY be called when saving the file.
*/void KJotsPage::generateXml( TQDomDocument &doc, TQDomElement &parent )
{
    TQDomElement page = doc.createElement( "KJotsPage" );
    parent.appendChild( page );

    KJotsEntryBase::generateXml(doc, page); //let the base class save important stuff

    TQDomElement text = doc.createElement( "Text" );
    TQString saveText = body();
    if ( saveText.contains("]]>") ) {
        saveText.replace("]]>","]]&gt;");
        text.setAttribute("fixed", "1");
    }
    text.appendChild( doc.createCDATASection( saveText ));
    page.appendChild( text );

    return;
}

/*!
    Parses through XML code from a file.
*/
void KJotsPage::parseXml( TQDomElement &me )
{
    if ( me.tagName() == "KJotsPage" )
    {
        TQDomNode n = me.firstChild();
        while( !n.isNull() ) 
        {
            TQDomElement e = n.toElement(); // try to convert the node to an element.
            if ( !e.isNull() ) 
            {
                if ( e.tagName() == "Text" ) 
                {
                    TQString bodyText = e.text();
                    if ( e.hasAttribute("fixed") ) 
                    {
                        bodyText.replace("]]&gt;", "]]>");
                    }
                    setBody(bodyText);
                } 
                else
                {
                    //What was this? Send it to the base class and see if it can figure it out.
                    KJotsEntryBase::parseXml(e);
                }
            }
            n = n.nextSibling();
        }
    }

    return;
}

/*!
    \brief Returns HTML for the read-only "book" overview.
    \param top This tells us that we are the toplevel.
    \param diskMode Files saved to disk have a slightly different format.
*/
TQString KJotsPage::generateHtml( KJotsEntryBase *top, bool diskMode )
{
    TQString html;
    TQString htmlSubject = prepForHTML(subject());

    if ( top == this || diskMode ) {
        html += TQString("<h3><a name=\"%1\">%2</a></h3>").arg(id()).arg(htmlSubject);
    } else {
        html += TQString("<h3><a name=\"%1\" href=\"%2\">%3</a></h3>").arg(id()).arg(id()).arg(htmlSubject);
    }
    html += prepForHTML(body());

    html += "<br><table border=1><tr>";
    html += TQString("<td><a href=\"#%1\">%2</a></td>").arg(id()).arg(subject());

    if ( top != this ) 
    {
        KJotsBook *parent = parentBook();
        while ( parent )
        {
            html += TQString("<td><a href=\"#%1\">%2</a></td>").arg(parent->id()).arg(parent->subject());
            if ( parent == top ) break;
            parent = parent->parentBook();
        }
    }
    html += TQString("</tr></table>");

    if ( top != this ) html += "<hr>";
    return html;
}

/*!
    \brief Returns Text when saving to a file.
    \param top This is a hint to specify if this book is the "starting point" or not.

    This functions output moderately formatted text when the user chooses to save as
    a text file. 
*/
TQString KJotsPage::generateText( void )
{
    TQString out;

    //Print Fancy Text header
    TQString line, buf;
    line.fill('#', subject().length() + 2);
    line += "\n";
    out = line + TQString("# ") + subject() + TQString("\n") + line;

    out += body();

    out += "\n";
    return out;
}

/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */

#include "kjotsentry.moc"
