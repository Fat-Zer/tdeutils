/*
 *  Copyright (c) 2002-2004 Jesper K. Pedersen <blackie@kde.org>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef COMPAT_H
#define COMPAT_H
#include <tqobject.h>
#include <tqmessagebox.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqinputdialog.h>
#include <tqframe.h>

TQString i18n( const TQString& a);
TQString i18n( const TQString& a, const TQString& b);
#define isatty(x) 0

#define KTextBrowser QTextBrowser
#define KListBox QListBox
#define KFileDialog QFileDialog
#define KPushButton QPushButton

class KDialogBase :public QDialog
{
    Q_OBJECT

public:
    enum ButtonCode {Ok = 1, Cancel, Help};
    enum DialogType { Plain };

    KDialogBase ( int dialogFace, const TQString &caption, int buttonMask,
                  ButtonCode defaultButton,
                  TQWidget *parent=0, const char *name=0, bool modal=true );

    KDialogBase( TQWidget* parent, const char* name = 0, bool modal = true,
                 const TQString& caption = TQString::null,
                 int buttonMask = 0 );

    void init( int buttonMask, ButtonCode /*defaultButton*/, const TQString& caption );
    void setMainWidget( TQWidget* top );
    TQFrame* plainPage();
    void setHelp( const TQString&, const TQString& ) {}

public slots:
    void slotOk();
    void slotCancel();

signals:
    void okClicked();
    void cancelClicked();
    void finished();
    void helpClicked();

private:
    TQVBoxLayout* _layout;
};

class KMessageBox :public QMessageBox
{
    Q_OBJECT
public:
    enum ButtonCode { Ok = 1, Cancel = 2, Yes = 3, No = 4, Continue = 5 };
    static int  warningYesNo (TQWidget *parent, const TQString &text,
                              const TQString &caption = TQString::null );
    static int information( TQWidget* parent, const TQString& text, const TQString& caption = TQString::null,
                            const TQString& /*dontShowAgainName*/ = TQString::null );
    static int sorry( TQWidget* parent, const TQString& text, const TQString& caption = TQString::null );
};

#endif /* COMPAT_H */

