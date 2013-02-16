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

#include <tqpainter.h>
#include <tqpaintdevicemetrics.h>
#include <tqcursor.h>

#include <tdepopupmenu.h>
#include <keditcl.h>
#include <tdelocale.h>
#include <kopenwith.h>
#include <kprinter.h>

#include "kjotsedit.h"
#include "kjotsentry.h"

//----------------------------------------------------------------------
// MYMULTIEDIT
//----------------------------------------------------------------------
KJotsEdit::KJotsEdit (TQWidget* parent, const char* name)
    : KEdit(parent, name),
    m_entry(0)
{
    // no rich text until printing and other such issues are worked out
    setTextFormat(TQt::PlainText);
    setWordWrap(TQTextEdit::WidgetWidth);
    setLinkUnderline(true);
    web_menu = new TDEPopupMenu(this);;
    web_menu->insertItem(i18n("Open URL"), this, TQT_SLOT(openUrl()) );
}

KJotsEdit::~KJotsEdit()
{

}

void KJotsEdit::mousePressEvent( TQMouseEvent *e )
{
    if(e->button() == Qt::RightButton &&
            hasSelectedText())
    {
        KURL url(selectedText());

        if(url.isValid())
        {
            web_menu->popup(TQCursor::pos());
            return;
        }
    }

    KEdit::mousePressEvent(e);
}

void KJotsEdit::openUrl()
{
    if (hasSelectedText())
    {
        KURL url(selectedText());
        if(url.isValid())
        {
            new KRun(url);
        }
    }
}

void KJotsEdit::print(TQString title)
{
    KPrinter printer;
    printer.setDocName(title);
    printer.setFullPage(false);
    printer.setCreator("KJots");

    if (printer.setup(this))
    {
        TQFont printFont = font();
        TQPainter painter( &printer );
        TQPaintDeviceMetrics metrics( &printer );
        int y = 0;
        int maxWidth = metrics.width();
        int maxHeight = metrics.height();
        TQString currentParagraph;

        for (int paragraphCount = 0; paragraphCount < paragraphs(); ++paragraphCount )
        {
            currentParagraph = text(paragraphCount);
            TQRect r = painter.boundingRect(0, y, maxWidth, maxHeight,
                    TQPainter::ExpandTabs | TQPainter::WordBreak,
                    currentParagraph);

            if ((y + r.height()) > maxHeight)
            {
                printer.newPage();
                y = 0;
            }

            painter.drawText(0, y, maxWidth, maxHeight - y,
                    TQPainter::ExpandTabs | TQPainter::WordBreak,
                    currentParagraph);
            y += r.height();
        }
        painter.end();
    }
}

void KJotsEdit::setEntry (KJotsPage *entry)
{
    //tell the old entry to take a hike
    if ( m_entry )
    {
        m_entry->setEditor(0);
    }

    //load up the new entry (assuming there is one)
    if ( entry )
    {
        m_entry = entry;
        setText(entry->body());
        removeSelection();
        repaint();
        setEnabled(true);
        setFocus();
        entry->setEditor(this);
    } else {
        clear();
    }

    m_entry = entry;
}

#include "kjotsedit.moc"
/* ex: set tabstop=4 softtabstop=4 shiftwidth=4 expandtab: */
