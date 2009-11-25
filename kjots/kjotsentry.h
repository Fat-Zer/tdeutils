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
class QTextCodec;
class QDomDocument;
class QDomElement;
class QFile;

class KJotsBook;

namespace KIO
{
    class Job;
}

class KJotsEntryBase : public QObject, public KListViewItem
{
    Q_OBJECT
    public:
        KJotsEntryBase(KListView*, QListViewItem*);
        KJotsEntryBase(KListViewItem*, QListViewItem*);

    public:
        virtual void setSubject(const QString&);
        QString subject() { return text(0); }
        void setText(int,const QString&);
        virtual void saveToFile(KURL, bool plainText, const QString& encoding) = 0;
        virtual void print(QFont& defaultFont) = 0;
        virtual int  print(KPrinter&, QPainter&, int) = 0;
        virtual QString defaultSubject() = 0; //!< "subject" is the caption name
        virtual void rename() = 0;
        virtual void generateXml( QDomDocument&, QDomElement& );
        virtual void parseXml( QDomElement& );
        virtual QString generateHtml( KJotsEntryBase*, bool ) = 0;
        virtual QString generateText( void ) = 0;

        void setDirty(bool d) { m_dirty = d; } //!< Toggles the dirty flag.
        virtual bool isDirty() { return m_dirty; }; //!< Accessor for dirty flag.

        Q_UINT64 id() { return m_id; }
        void setId(Q_UINT64);

        KJotsBook *parentBook() { return m_parent; }
        void resetParent();

        bool isBook() const { return m_isBook; }
        bool isPage() const { return !m_isBook; }

    protected:
        int printTitleBox(QString, KPrinter&, QPainter&, int);

        bool m_saveInProgress; //!< Toggled during a manual disk save.
        bool m_isBook; //!< used for speed and code clarity.

    private:
        Q_UINT64 m_id; //!< unique ID for this entry
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
        KJotsBook(KListView*, QListViewItem* after = 0);
        KJotsBook(KListViewItem*, QListViewItem* after = 0);
        ~KJotsBook();

        static bool isBookFile(const QString& book);
        bool openBook(const QString&);
        void saveBook();
        void deleteBook();
        void rename();
        void saveToFile(KURL, bool plainText, const QString& encoding);
        KJotsPage* addPage(); //!< Add a new page to this book.
        void print(QFont& defaultFont);
        int  print(KPrinter&, QPainter&, int);
        bool isDirty();
        QString defaultSubject();
        void generateXml( QDomDocument&, QDomElement& );
        void parseXml( QDomElement& );
        QString generateHtml( KJotsEntryBase*, bool );
        QString generateText( void );

    protected slots:
        void saveDataReq(KIO::Job* job, QByteArray& data);
        void slotSaveResult(KIO::Job*);

    private:
        QString getToc();
        bool loadOldBook(QFile &);
        void init();

        bool m_open;
        QString m_fileName;
        bool m_saveToPlainText;
        QTextCodec* m_saveEncoding;
        KProgressDialog* m_saveProgressDialog;
};

//
// class KJotsPage
//
class KJotsPage : public KJotsEntryBase
{
    Q_OBJECT
    public:
        KJotsPage(KJotsBook* parent, QListViewItem* after = 0);
        ~KJotsPage();

    public:
        QString body();
        void setBody(const QString&);
        void saveToFile(KURL, bool plainText, const QString& encoding);
        void print(QFont& defaultFont);
        int  print(KPrinter&, QPainter&, int);
        QString defaultSubject();
        void rename();

        void setEditor(KJotsEdit*);
        bool isDirty();
        void generateXml( QDomDocument&, QDomElement& );
        void parseXml( QDomElement& );
        void initNewPage(void);
        QString generateHtml( KJotsEntryBase*, bool );
        QString generateText( void );

    protected slots:
        void saveDataReq(KIO::Job* job, QByteArray& data);
        void slotSaveResult(KIO::Job*);

    private:
        QString m_text;
        QTextCodec* m_saveEncoding;
        bool    m_saveToPlainText;
        KJotsEdit *m_editor; //!< ptr to editor if this is the active subject
        int m_paraPos; //< used to remrmber the cursor position
        int m_indexPos; //< used to remrmber the cursor position
};

#endif // __KJOTSENTRY_H
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
