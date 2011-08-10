/*
   Copyright (C) 2003,2004 George Staikos <staikos@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "kwmapeditor.h"

#include <kaction.h>
#include <kdebug.h>
#include <klocale.h>
#include <kpopupmenu.h>
#include <kstdaction.h>
#include <kwin.h>

#include <tqapplication.h>
#include <tqclipboard.h>
#include <tqpushbutton.h>
#include <tqtextedit.h>

KWMapEditor::KWMapEditor(TQMap<TQString,TQString>& map, TQWidget *parent, const char *name)
: TQTable(0, 3, parent, name), _map(map) {
	_ac = new KActionCollection(this);
	_copyAct = KStdAction::copy(TQT_TQOBJECT(this), TQT_SLOT(copy()), _ac);
	connect(this, TQT_SIGNAL(valueChanged(int,int)), this, TQT_SIGNAL(dirty()));
	connect(this, TQT_SIGNAL(contextMenuRequested(int,int,const TQPoint&)),
		this, TQT_SLOT(contextMenu(int,int,const TQPoint&)));
	setSelectionMode(TQTable::NoSelection);
	horizontalHeader()->setLabel(0, TQString());
	horizontalHeader()->setLabel(1, i18n("Key"));
	horizontalHeader()->setLabel(2, i18n("Value"));
	setColumnWidth(0, 20); // FIXME: this is arbitrary
	reload();
}

void KWMapEditor::reload() {
	unsigned row = 0;

	while ((row = numRows()) > _map.count()) {
		removeRow(row - 1);
	}

	if ((row = numRows()) < _map.count()) {
		insertRows(row, _map.count() - row);
		for (int x = row; x < numRows(); ++x) {
			TQPushButton *b = new TQPushButton("X", this);
			connect(b, TQT_SIGNAL(clicked()), this, TQT_SLOT(erase()));
			setCellWidget(x, 0, b);
		}
	}

	row = 0;
	for (TQMap<TQString,TQString>::Iterator it = _map.begin(); it != _map.end(); ++it) {
		setText(row, 1, it.key());
		setText(row, 2, it.data());
		row++;
	}
}


KWMapEditor::~KWMapEditor() {
}


void KWMapEditor::erase() {
	const TQObject *o = TQT_TQOBJECT(const_cast<TQT_BASE_OBJECT_NAME*>(sender()));
	for (int i = 0; i < numRows(); i++) {
		if (TQT_BASE_OBJECT_CONST(cellWidget(i, 0)) == TQT_BASE_OBJECT_CONST(o)) {
			removeRow(i);
			break;
		}
	}

	emit dirty();
}


void KWMapEditor::saveMap() {
	_map.clear();

	for (int i = 0; i < numRows(); i++) {
		_map[text(i, 1)] = text(i, 2);
	}
}


void KWMapEditor::addEntry() {
	int x = numRows();
	insertRows(x, 1);
	TQPushButton *b = new TQPushButton("X", this);
	connect(b, TQT_SIGNAL(clicked()), this, TQT_SLOT(erase()));
	setCellWidget(x, 0, b);
	ensureCellVisible(x, 1);
	setCurrentCell(x, 1);
	emit dirty();
}


void KWMapEditor::emitDirty() {
	emit dirty();
}


void KWMapEditor::contextMenu(int row, int col, const TQPoint& pos) {
	_contextRow = row;
	_contextCol = col;
	KPopupMenu *m = new KPopupMenu(this);
	m->insertItem(i18n("&New Entry"), this, TQT_SLOT(addEntry()));
	_copyAct->plug(m);
	m->popup(pos);
}


void KWMapEditor::copy() {
	TQApplication::tqclipboard()->setText(text(_contextRow, 2));
}


class InlineEditor : public TQTextEdit {
	public:
		InlineEditor(KWMapEditor *p, int row, int col) 
		  : TQTextEdit(), _p(p), row(row), col(col) { 
			setWFlags(WStyle_NoBorder | WDestructiveClose); 
			KWin::setType(winId(), NET::Override);
			connect(p, TQT_SIGNAL(destroyed()), TQT_SLOT(close()));
 		}
		virtual ~InlineEditor() { if (!_p) return; _p->setText(row, col, text()); _p->emitDirty(); }

	protected:
		virtual void focusOutEvent(TQFocusEvent* fe) { 
			if (fe->reason() == TQFocusEvent::Popup) {
				TQWidget *focusW = tqApp->tqfocusWidget();
				if (focusW && focusW == popup) {
					return;
				}
			}
			close(); 
		}
		virtual void keyPressEvent(TQKeyEvent *e) {
			if (e->key() == TQt::Key_Escape) {
				e->accept();
				close();
			} else {
				e->ignore();
				TQTextEdit::keyPressEvent(e);
			}
		}
		virtual TQPopupMenu *createPopupMenu(const TQPoint &p) {
			popup = TQTextEdit::createPopupMenu(p);
			return popup;
		}
		TQGuardedPtr<KWMapEditor> _p;
		int row, col;
		TQGuardedPtr<TQPopupMenu> popup;
};


TQWidget *KWMapEditor::beginEdit(int row, int col, bool replace) {
	//kdDebug(2300) << "EDIT COLUMN " << col << endl;
	if (col != 2) {
		return TQTable::beginEdit(row, col, replace);
	}

	TQRect geo = cellGeometry(row, col);
	TQTextEdit *e = new InlineEditor(this, row, col);
	e->setText(text(row, col));
	e->move(mapToGlobal(geo.topLeft()));
	e->resize(geo.width() * 2, geo.height() * 3);
	e->show();
	return e;
}


#include "kwmapeditor.moc"
