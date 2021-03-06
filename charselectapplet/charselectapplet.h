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

#ifndef __charselectapplet_h__
#define __charselectapplet_h__

#include <tqmap.h>

#include <kpanelapplet.h>
#include <kdialogbase.h>

class TQSpinBox;
class KLineEdit;
class TDEAboutData;

class ConfigDialog : public KDialogBase
{
    Q_OBJECT
  

public:
    ConfigDialog(TQWidget* parent = 0, const char* name = 0);

    void setCharacters(const TQString& s) { _characterInput->setText(s); }
    void setCellWidth(int w) { _widthSpinBox->setValue(w); }
    void setCellHeight(int h) { _heightSpinBox->setValue(h); }

    TQString characters() { return _characterInput->text(); }
    int cellWidth() { return _widthSpinBox->value(); }
    int cellHeight() { return _heightSpinBox->value(); }

private:
    TQSpinBox  *_widthSpinBox;
    TQSpinBox  *_heightSpinBox;
    KLineEdit *_characterInput;
};

class CharTable : public TQFrame
{
    Q_OBJECT
  

public:
    CharTable(TQWidget* parent = 0, const char* name = 0);

    void setRowsAndColumns(int, int);

    void setCharacters(const TQString&);
    TQString characters();

protected:
    void paintEvent(TQPaintEvent*);
    void resizeEvent(TQResizeEvent*);
    void mousePressEvent(TQMouseEvent*);
    void mouseMoveEvent(TQMouseEvent*);

    void paintCell(TQPainter*, int, int);
    void repaintCell(int, int);
    void selectCell(int row, int col);

    void insertString(TQString s);
    void insertChar(TQChar c);

    int findRow(int y);
    int findCol(int x);

protected slots:
    void clearCell();
private:
    int _rows, _cols;
    int _activeRow, _activeCol;
    int _cWidth, _cHeight;
    int _charcount;
    TQMap<int, TQChar> _map;
};

class CharSelectApplet : public KPanelApplet
{
    Q_OBJECT
  

public:
    CharSelectApplet(const TQString& configFile, Type t = Stretch, int actions = 0,
                     TQWidget *parent = 0, const char *name = 0);

    int widthForHeight(int height) const;
    int heightForWidth(int width) const;

    void preferences();
    void about();

private:
    CharTable    *_table;
    TDEAboutData   *_aboutData;
    ConfigDialog *_configDialog;
};

#endif
