/*****************************************************************

Copyright (c) 2001 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <math.h>

#include <tqlayout.h>
#include <tqpainter.h>
#include <tqclipboard.h>
#include <tqvbox.h>
#include <tqspinbox.h>
#include <tqlabel.h>

#include <klocale.h>
#include <kglobal.h>
#include <kconfig.h>
#include <klineedit.h>
#include <kaboutapplication.h>

#include "charselectapplet.h"
#include "charselectapplet.moc"

extern "C"
{
    KDE_EXPORT KPanelApplet* init(TQWidget *parent, const TQString& configFile)
    {
        KGlobal::locale()->insertCatalogue("kcharselectapplet");
        return new CharSelectApplet(configFile, KPanelApplet::Normal,
                                    KPanelApplet::About | KPanelApplet::Preferences,
                                    parent, "kcharselectapplet");
    }
}

static int cell_width = 16;
static int cell_height = 16;
static int char_count = 0;

CharSelectApplet::CharSelectApplet(const TQString& configFile, Type type, int actions,
                               TQWidget *parent, const char *name)
    : KPanelApplet(configFile, type, actions, parent, name),
      _aboutData(0), _configDialog(0)
{
    // read configuration
    KConfig *c = config();
    c->setGroup("General");
    cell_width = c->readNumEntry("CellWidth", cell_width);
    cell_height = c->readNumEntry("CellHeight", cell_height);
    TQString characters = c->readEntry("Characters", "������ߩ��");

    // setup tqlayout
    TQHBoxLayout *_layout = new TQHBoxLayout(this);
    _layout->setAutoAdd(true);

    // setup table view
    _table = new CharTable(this);

    // insert chars
    _table->setCharacters(characters);
}

int CharSelectApplet::widthForHeight(int height) const
{
    // number of rows depends on panel size
    int rows = (height - (lineWidth() * 2))/ cell_height;
    if(rows <= 0) rows = 1;

    // calculate number of columns
    float c = (float) char_count / rows;
    int columns = (int) ceil(c);
    if(columns <= 0) columns = 1;

    _table->setRowsAndColumns(rows, columns);

    // tell kicker how much space we need
    return columns * cell_width + lineWidth() * 2;
}

int CharSelectApplet::heightForWidth(int width) const
{
    // number of columns depends on panel size
    int columns = (width - (lineWidth() * 2))/ cell_width;
    if(columns <= 0) columns = 1;

    // calculate number of rows we need
    float r = (float) char_count / columns;
    int rows = (int) ceil(r);
    if(rows <= 0) rows = 1;

    _table->setRowsAndColumns(rows, columns);

    return rows * cell_height + lineWidth() *2;
}

void CharSelectApplet::preferences()
{
    if(!_configDialog)
        _configDialog = new ConfigDialog(this);

    _configDialog->setCharacters(_table->characters());
    _configDialog->setCellWidth(cell_width);
    _configDialog->setCellHeight(cell_height);
    _configDialog->setInitialSize(TQSize(300, 100));
    _configDialog->exec();

    cell_width = _configDialog->cellWidth();
    cell_height = _configDialog->cellHeight();
    _table->setCharacters(_configDialog->characters());

    emit updateLayout();

    // write configuration
    KConfig *c = config();
    c->setGroup("General");
    c->writeEntry("CellWidth", cell_width);
    c->writeEntry("CellHeight", cell_height);
    c->writeEntry("Characters", _configDialog->characters());
    c->sync();
}

void CharSelectApplet::about()
{
    if(!_aboutData) {
	_aboutData = new KAboutData("kcharselectapplet", I18N_NOOP("KCharSelectApplet"), "1.0",
                                    I18N_NOOP("A character picker applet.\n"
                                        "Used to copy single characters to the X11 clipboard.\n"
                                        "You can paste them to an application with the middle mouse button."),
                                    KAboutData::License_BSD, "(c) 2001, Matthias Elter");
	_aboutData->addAuthor("Matthias Elter", 0, "elter@kde.org");
    }

    KAboutApplication dialog(_aboutData);
    dialog.exec();
}

CharTable::CharTable(TQWidget* parent, const char* name)
    : TQFrame(parent, name), _rows(2), _cols(2),
      _activeRow(-1), _activeCol(-1),
      _cWidth(cell_width), _cHeight(cell_height)
{
    setFrameStyle(TQFrame::StyledPanel | TQFrame::Sunken);
    setFocusPolicy(TQ_NoFocus);
    setBackgroundMode(TQWidget::NoBackground);
}

void CharTable::setRowsAndColumns(int r, int c)
{
    _rows = r;
    _cols = c;
}

void CharTable::insertChar(TQChar c)
{
    _map.insert(char_count++, c);
}

void CharTable::insertString(TQString s)
{
    for (unsigned int i = 0; i < s.length(); i++)
        insertChar(s[i]);
}

void CharTable::setCharacters(const TQString& characters)
{
    _map.clear();
    char_count = 0;
    insertString(characters);
}

TQString CharTable::characters()
{
    TQString characters;
    for (int r = 0; r <_rows; r++)
        for (int c = 0; c <_cols; c++)
            characters += _map[c + r * _cols];

    return characters;
}

int CharTable::findRow(int y)
{
    return y / _cHeight;
}

int CharTable::findCol(int x)
{
    return x / _cWidth;
}

void CharTable::resizeEvent(TQResizeEvent*)
{
    _cWidth = contentsRect().width() / _cols;
    _cHeight = contentsRect().height() / _rows;
}

void CharTable::paintEvent(TQPaintEvent* e)
{
    TQPainter p(this);

    int xoffset = contentsRect().x();
    int yoffset = contentsRect().y();

    for (int r = 0; r <_rows; r++) {
        for (int c = 0; c <_cols; c++) {
            p.setViewport(xoffset + c * _cWidth, yoffset + r * _cHeight, _cWidth, _cHeight);
            p.setWindow(0, 0, _cWidth, _cHeight);
            paintCell(&p, r, c);
        }
    }
    TQFrame::paintEvent(e);
}

void CharTable::repaintCell(int r, int c)
{
    TQPainter p(this);

    int xoffset = contentsRect().x();
    int yoffset = contentsRect().y();

    p.setViewport(xoffset + c * _cWidth, yoffset + r * _cHeight, _cWidth, _cHeight);
    p.setWindow(0, 0, _cWidth, _cHeight);
    paintCell(&p, r, c);
}

void CharTable::paintCell(TQPainter* p, int row, int col)
{
    int w = _cWidth;
    int h = _cHeight;
    int x2 = w - 1;
    int y2 = h - 1;

    bool active = (row == _activeRow) && (col == _activeCol);

    // draw background
    if (active) {
	p->setBrush(TQBrush(tqcolorGroup().highlight()));
	p->setPen(NoPen);
	p->drawRect(0, 0, w, h);
	p->setPen(tqcolorGroup().highlightedText());
    }
    else {
	p->setBrush(TQBrush(tqcolorGroup().base()));
	p->setPen(NoPen);
	p->drawRect(0, 0, w, h);
	p->setPen(tqcolorGroup().text());
    }

    // set font
    TQFont f = font();
    f.setPixelSize(10);
    p->setFont(f);

    // draw char
    p->drawText(0, 0, x2, y2, AlignHCenter | AlignVCenter, TQString(_map[col + row * _cols]));
}

void CharTable::mousePressEvent(TQMouseEvent *e)
{
    int row = findRow(e->y());
    if (row == -1) return;

    int col = findCol(e->x());
    if (col == -1) return;

    selectCell(row, col);
}

void CharTable::mouseMoveEvent(TQMouseEvent *e)
{
    if(!(e->state() & (Qt::LeftButton | Qt::RightButton | Qt::MidButton))) return;

    int row = findRow(e->y());
    if (row == -1) return;

    int col = findCol(e->x());
    if (col == -1) return;

    selectCell(row, col);
}

void CharTable::selectCell(int row, int col)
{
    if (row >= _rows || row < 0) return;
    if (col >= _cols || col < 0) return;

    int oldRow = _activeRow;
    int oldCol = _activeCol;

    _activeRow = row;
    _activeCol = col;

    repaintCell(oldRow, oldCol);
    repaintCell(_activeRow, _activeCol);

    TQClipboard *cb = TQApplication::tqclipboard();
    TQObject::disconnect( cb, TQT_SIGNAL(dataChanged()), this, TQT_SLOT(clearCell()) );
    TQString text = TQString(_map[col + row * _cols]);
    bool oldMode = cb->selectionModeEnabled();
    cb->setSelectionMode( true );
    cb->setText( text );
    cb->setSelectionMode( false );
    cb->setText( text );
    cb->setSelectionMode(  oldMode );
    TQObject::connect( cb, TQT_SIGNAL(dataChanged()), this, TQT_SLOT(clearCell()) );
}

void CharTable::clearCell()
{
    int oldRow = _activeRow;
    int oldCol = _activeCol;

    _activeRow = -1;
    _activeCol = -1;

    repaintCell(oldRow, oldCol);
    
    TQObject::disconnect( TQApplication::tqclipboard(), TQT_SIGNAL(dataChanged()), this, TQT_SLOT(clearCell()) );
}


ConfigDialog::ConfigDialog(TQWidget* parent, const char* name)
    : KDialogBase(parent, name, true, i18n("Configuration"),
                  Ok | Cancel, Ok, true)
{
    TQVBox *page = makeVBoxMainWidget();

    TQHBox *whbox = new TQHBox(page);
    TQHBox *hhbox = new TQHBox(page);
    TQHBox *chbox = new TQHBox(page);

    TQLabel *wlabel = new TQLabel(i18n("Cell width:"), whbox);
    TQLabel *hlabel = new TQLabel(i18n("Cell height:"), hhbox);
    (void) new TQLabel(i18n("Characters:"), chbox);

    _widthSpinBox = new TQSpinBox(whbox);
    _widthSpinBox->setMinValue(1); 
    _heightSpinBox = new TQSpinBox(hhbox);
    _heightSpinBox->setMinValue(1);
    _characterInput = new KLineEdit(chbox);

    whbox->setSpacing(KDialog::spacingHint());
    hhbox->setSpacing(KDialog::spacingHint());
    chbox->setSpacing(KDialog::spacingHint());

    whbox->setStretchFactor(wlabel, 2);
    hhbox->setStretchFactor(hlabel, 2);
    chbox->setStretchFactor(_characterInput, 2);
}
