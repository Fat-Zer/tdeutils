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

#ifndef __KJOTSENTRY_H
#define __KJOTSENTRY_H

#include <klistview.h>
#include <kprinter.h>

//
// class KJotsEntryBase
//

class KURL;
class KJotsPage;
class KProgressDialog;
class KJotsEdit;
class TQTextCodec;
class TQDomDocument;
class TQDomElement;
class TQFile;

class KJotsBook;

namespace TDEIO
{
    class Job;
}

class KJotsEntryBase : public TQObject, public KListViewItem
{
    Q_OBJECT
  
    public:
        KJotsEntryBase(KListView*, TQListViewItem*);
        KJotsEntryBase(KListViewItem*, TQListViewItem*);

    public:
        virtual void setSubject(const TQString&);
        TQString subject() { return text(0); }
        void setText(int,const TQString&);
        virtual void saveToFile(KURL, bool plainText, const TQString& encoding) = 0;
        virtual void print(TQFont& defaultFont) = 0;
        virtual int  print(KPrinter&, TQPainter&, int) = 0;
        virtual TQString defaultSubject() = 0; //!< "subject" is the caption name
        virtual void rename() = 0;
        virtual void generateXml( TQDomDocument&, TQDomElement& );
        virtual void parseXml( TQDomElement& );
        virtual TQString generateHtml( KJotsEntryBase*, bool ) = 0;
        virtual TQString generateText( void ) = 0;

        void setDirty(bool d) { m_dirty = d; } //!< Toggles the dirty flag.
        virtual bool isDirty() { return m_dirty; }; //!< Accessor for dirty flag.

        TQ_UINT64 id() { return m_id; }
        void setId(TQ_UINT64);

        KJotsBook *parentBook() { return m_parent; }
        void resetParent();

        bool isBook() const { return m_isBook; }
        bool isPage() const { return !m_isBook; }

    protected:
        int printTitleBox(TQString, KPrinter&, TQPainter&, int);

        bool m_saveInProgress; //!< Toggled during a manual disk save.
        bool m_isBook; //!< used for speed and code clarity.

    private:
        TQ_UINT64 m_id; //!< unique ID for this entry
        bool m_dirty; //!< Set when this entry has been changed.
        KJotsBook *m_parent; //!< used during drag-n-drop moving
};

//
// class KjotsBook
//

class KJotsBook : public KJotsEntryBase
{
    Q_OBJECT
  
    public:
        KJotsBook(KListView*, TQListViewItem* after = 0);
        KJotsBook(KListViewItem*, TQListViewItem* after = 0);
        ~KJotsBook();

        static bool isBookFile(const TQString& book);
        bool openBook(const TQString&);
        void saveBook();
        void deleteBook();
        void rename();
        void saveToFile(KURL, bool plainText, const TQString& encoding);
        KJotsPage* addPage(); //!< Add a new page to this book.
        void print(TQFont& defaultFont);
        int  print(KPrinter&, TQPainter&, int);
        bool isDirty();
        TQString defaultSubject();
        void generateXml( TQDomDocument&, TQDomElement& );
        void parseXml( TQDomElement& );
        TQString generateHtml( KJotsEntryBase*, bool );
        TQString generateText( void );

    protected slots:
        void saveDataReq(TDEIO::Job* job, TQByteArray& data);
        void slotSaveResult(TDEIO::Job*);

    private:
        TQString getToc();
        bool loadOldBook(TQFile &);
        void init();

        bool m_open;
        TQString m_fileName;
        bool m_saveToPlainText;
        TQTextCodec* m_saveEncoding;
        KProgressDialog* m_saveProgressDialog;
};

//
// class KJotsPage
//
class KJotsPage : public KJotsEntryBase
{
    Q_OBJECT
  
    public:
        KJotsPage(KJotsBook* parent, TQListViewItem* after = 0);
        ~KJotsPage();

    public:
        TQString body();
        void setBody(const TQString&);
        void saveToFile(KURL, bool plainText, const TQString& encoding);
        void print(TQFont& defaultFont);
        int  print(KPrinter&, TQPainter&, int);
        TQString defaultSubject();
        void rename();

        void setEditor(KJotsEdit*);
        bool isDirty();
        void generateXml( TQDomDocument&, TQDomElement& );
        void parseXml( TQDomElement& );
        void initNewPage(void);
        TQString generateHtml( KJotsEntryBase*, bool );
        TQString generateText( void );

    protected slots:
        void saveDataReq(TDEIO::Job* job, TQByteArray& data);
        void slotSaveResult(TDEIO::Job*);

    private:
        TQString m_text;
        TQTextCodec* m_saveEncoding;
        bool    m_saveToPlainText;
        KJotsEdit *m_editor; //!< ptr to editor if this is the active subject
        int m_paraPos; //< used to remrmber the cursor position
        int m_indexPos; //< used to remrmber the cursor position
};

#endif // __KJOTSENTRY_H
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
