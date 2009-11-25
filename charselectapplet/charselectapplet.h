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

#include <qmap.h>

#include <kpanelapplet.h>
#include <kdialogbase.h>

class QSpinBox;
class KLineEdit;
class KAboutData;

class ConfigDialog : public KDialogBase
{
    Q_OBJECT

public:
    ConfigDialog(QWidget* parent = 0, const char* name = 0);

    void setCharacters(const QString& s) { _characterInput->setText(s); }
    void setCellWidth(int w) { _widthSpinBox->setValue(w); }
    void setCellHeight(int h) { _heightSpinBox->setValue(h); }

    QString characters() { return _characterInput->text(); }
    int cellWidth() { return _widthSpinBox->value(); }
    int cellHeight() { return _heightSpinBox->value(); }

private:
    QSpinBox  *_widthSpinBox;
    QSpinBox  *_heightSpinBox;
    KLineEdit *_characterInput;
};

class CharTable : public QFrame
{
    Q_OBJECT

public:
    CharTable(QWidget* parent = 0, const char* name = 0);

    void setRowsAndColumns(int, int);

    void setCharacters(const QString&);
    QString characters();

protected:
    void paintEvent(QPaintEvent*);
    void resizeEvent(QResizeEvent*);
    void mousePressEvent(QMouseEvent*);
    void mouseMoveEvent(QMouseEvent*);

    void paintCell(QPainter*, int, int);
    void repaintCell(int, int);
    void selectCell(int row, int col);

    void insertString(QString s);
    void insertChar(QChar c);

    int findRow(int y);
    int findCol(int x);

protected slots:
    void clearCell();
private:
    int _rows, _cols;
    int _activeRow, _activeCol;
    int _cWidth, _cHeight;
    int _charcount;
    QMap<int, QChar> _map;
};

class CharSelectApplet : public KPanelApplet
{
    Q_OBJECT

public:
    CharSelectApplet(const QString& configFile, Type t = Stretch, int actions = 0,
                     QWidget *parent = 0, const char *name = 0);

    int widthForHeight(int height) const;
    int heightForWidth(int width) const;

    void preferences();
    void about();

private:
    CharTable    *_table;
    KAboutData   *_aboutData;
    ConfigDialog *_configDialog;
};

#endif
