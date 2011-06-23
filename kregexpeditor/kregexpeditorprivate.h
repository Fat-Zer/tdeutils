/*
 *  Copyright (c) 2002-2003 Jesper K. Pedersen <blackie@kde.org>
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
#ifndef kregexpeditorprivate_h
#define kregexpeditorprivate_h

#include <tqlabel.h>
#include <tqptrstack.h>
#include "regexp.h"
#include "errormap.h"
class TQTimer;

class RegExpScrolledEditorWindow;
class TQLineEdit;
class InfoPage;
class UserDefinedRegExps;
class TQSplitter;
class Verifier;
class VerifyButtons;
class AuxButtons;
class RegExpLineEdit;
class RegExpConverter;
class RegExpButtons;
class TQToolButton;

/**
   Widget used to build a regular expression

   @author Jesper K. Pedersen <blackie@kde.org>
   @version 0.1
**/
class KRegExpEditorPrivate  :public TQWidget
{
    Q_OBJECT
  TQ_OBJECT

public:
    KRegExpEditorPrivate( TQWidget *tqparent, const char *name = 0 );
    TQString regexp();
    void setMinimal( bool );
    void setCaseSensitive( bool );
    void setAllowNonTQtSyntax( bool );

protected slots:
    void slotUpdateEditor( const TQString & );
    void slotUpdateLineEdit();
    void slotShowEditor();
    void slotTriggerUpdate();
    void slotTimeout();
    void maybeVerify();
    void doVerify();
    void setAutoVerify( bool );
    void setVerifyText( const TQString& fileName );

public slots:
    void slotUndo();
    void slotRedo();
    void slotSetRegexp( TQString regexp );
    void setMatchText( const TQString& text );
    void setSyntax( const TQString& );
    void showHelp();

signals:
    void canUndo( bool );
    void canRedo( bool );
    void changes( bool );

protected:
    void recordUndoInfo();
    void emitUndoRedoSignals();

private:
	RegExpScrolledEditorWindow* _scrolledEditorWindow;
    RegExpButtons* _regExpButtons;
    VerifyButtons* _verifyButtons;
    AuxButtons *_auxButtons;
    InfoPage* _info;
    TQLineEdit* _regexpEdit;
    TQSplitter* _editor;
    bool _updating;
    TQLabel* _error;
    TQPtrStack<RegExp> _undoStack;
    TQPtrStack<RegExp> _redoStack;
    UserDefinedRegExps*  _userRegExps;
    TQTimer* _timer;
    Verifier* _verifier;
    bool _autoVerify;
    ErrorMap _errorMap;
    TQToolButton *clearButton;
};

#endif

