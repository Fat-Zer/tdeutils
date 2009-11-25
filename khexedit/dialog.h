/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

#ifndef _DIALOG_H_
#define _DIALOG_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif 


#include <qcheckbox.h>
#include <qcombobox.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qstring.h>
#include <qwidgetstack.h>

#include <kdialogbase.h>

#include "bitswapwidget.h"
#include "hexbuffer.h"
#include "hexvalidator.h"

const int repYes      = QDialog::Accepted; 
const int repNo       = 11;
const int repAll      = 12;
const int repClose    = QDialog::Rejected; 
const int repNext     = 13;
const int repPrevious = 14;
const int repNewKey   = 15;

const uint Replace_All     = 1;
const uint Replace_AllInit = 2;
const uint Replace_First   = 3;
const uint Replace_Next    = 4;
const uint Replace_Ignore  = 5;
const uint Find_First      = 6;
const uint Find_Next       = 7;




class CGotoDialog : public KDialogBase 
{
  Q_OBJECT
  
  public:
    CGotoDialog( QWidget *parent, const char *name = 0, bool modal = false );
    ~CGotoDialog( void );

  protected:
    virtual void showEvent( QShowEvent *e );

  private slots:
    void slotOk( void );

  signals:
    void gotoOffset( uint offset, uint bit, bool fromCursor, bool forward );

  private:
    QComboBox *mComboBox;
    QCheckBox *mCheckBackward;
    QCheckBox *mCheckFromCursor;
    QCheckBox *mCheckVisible;
};



class CFindDialog : public KDialogBase
{
  Q_OBJECT

  public:
    enum EOperation
    {
      find_Again = 0,
      find_Next,
      find_Previous
    };

  public:
    CFindDialog( QWidget *parent=0, const char *name=0, bool modal=false );
    ~CFindDialog( void );
    bool isEmpty( void );

  public slots:
    void findAgain( EOperation operation );

  protected:
    virtual void showEvent( QShowEvent *e );

  private slots:
    void slotOk( void );
    void selectorChanged( int index );
    void inputChanged( const QString &text );

  signals: 
    void findData( SSearchControl &sc, uint mode, bool navigator );

  private:
    QComboBox *mSelector;
    QLineEdit *mInput;
    QCheckBox *mCheckBackward;
    QCheckBox *mCheckFromCursor;
    QCheckBox *mCheckInSelection;
    QCheckBox *mCheckUseNavigator;
    QCheckBox *mCheckIgnoreCase;

    QString    mFindString[5];
    QByteArray mFindData;
    CHexValidator *mFindValidator;
};



class CFindNavigatorDialog : public KDialogBase
{
  Q_OBJECT
  
  public:
    CFindNavigatorDialog( QWidget *parent=0, const char *name=0, 
			  bool modal=false );
    ~CFindNavigatorDialog( void );
    void defineData( SSearchControl &sc );

  private slots:
    void slotUser1( void );
    void slotUser2( void );
    void slotUser3( void );
    void slotClose( void );

  private:
    void done( int returnCode );

  signals:
    void findData( SSearchControl &sc, uint mode, bool navigator );
    void makeKey( void );

  private:
    QLineEdit *mKey;
    SSearchControl mSearchControl;
};


class CReplaceDialog : public KDialogBase
{
  Q_OBJECT
  
  public:
    CReplaceDialog( QWidget *parent=0, const char *name=0, bool modal=false );
    ~CReplaceDialog( void );

  protected:
    virtual void showEvent( QShowEvent *e );

  private slots:
    void slotOk( void );
    void findSelectorChanged( int index );
    void findInputChanged( const QString &text );
    void replaceSelectorChanged( int index );
    void replaceInputChanged( const QString &text );

  signals:
    void replaceData( SSearchControl &sc, uint mode );

  private:
    QComboBox *mFindSelector;
    QComboBox *mReplaceSelector;
    QLineEdit *mFindInput;
    QLineEdit *mReplaceInput;

    QCheckBox *mCheckBackward;
    QCheckBox *mCheckFromCursor;
    QCheckBox *mCheckInSelection;
    QCheckBox *mCheckPrompt;
    QCheckBox *mCheckIgnoreCase;

    QString    mFindString[5];
    QString    mReplaceString[5];
    QByteArray mFindData;
    QByteArray mReplaceData;
    CHexValidator *mFindValidator;
    CHexValidator *mReplaceValidator;
};



class CReplacePromptDialog : public KDialogBase
{
  Q_OBJECT
  
  public:
    CReplacePromptDialog( QWidget *parent=0, const char *name=0, 
			  bool modal=false );
    ~CReplacePromptDialog( void );
    void defineData( SSearchControl &sc );

  private slots:
    void slotUser1( void );
    void slotUser2( void );
    void slotUser3( void );
    void slotClose( void );

  private:
    void done( int returnCode );

  signals:
    void replaceData( SSearchControl &sc, uint mode );

  private:
    SSearchControl mSearchControl;
};




class CFilterDialog : public KDialogBase
{
  Q_OBJECT

  public:
    enum EStackMode
    {
      EmptyPage = 0,
      OperandPage,
      BitSwapPage,
      RotatePage
    };

  public:
    CFilterDialog( QWidget *parent=0, const char *name=0, bool modal=false );
    ~CFilterDialog( void );

  protected:
    virtual void showEvent( QShowEvent *e );

  private:
    void makeEmptyLayout( void );
    void makeOperandLayout( void );
    void makeBitSwapLayout( void );
    void makeRotateLayout( void );

  private slots:
    void slotOk( void );
    void operandSelectorChanged( int index );
    void operandInputChanged( const QString &text );
    void operationSelectorChanged( int index );

  signals:
    void filterData( SFilterControl &fc );

  private:
    QWidgetStack *mWidgetStack;
    QLabel    *mOperandFormatLabel;
    QLabel    *mOperandInputLabel;
    QComboBox *mOperandSelector;
    QLineEdit *mOperandInput;
    QComboBox *mOperationSelector;
    QSpinBox  *mGroupSpin;
    QSpinBox  *mBitSpin;
    CByteWidget *mByteWidget;

    QCheckBox *mCheckBackward;
    QCheckBox *mCheckFromCursor;
    QCheckBox *mCheckInSelection;
    QCheckBox *mCheckVisible;

    QString    mOperandString[5];
    QByteArray mOperandData;
    CHexValidator *mOperandValidator;
};




class CInsertDialog : public KDialogBase
{
  Q_OBJECT
  
  public:
    CInsertDialog( QWidget *parent=0, const char *name=0, bool modal=false );
    ~CInsertDialog( void );

  protected:
    virtual void showEvent( QShowEvent *e );

  private slots:
    void slotOk( void );
    void cursorCheck( void );
    void patternSelectorChanged( int index );
    void patternInputChanged( const QString &text );

  signals:
    void execute( SInsertData &id );

  private:
    QSpinBox *mSizeBox;
    QLabel *mOffsetLabel;
    QComboBox *mPatternSelector;
    QLineEdit *mPatternInput;
    QLineEdit *mOffsetInput;
    QCheckBox *mCheckPattern;
    QCheckBox *mCheckOnCursor;

    QString    mPatternString[5];
    QByteArray mPatternData;
    CHexValidator *mPatternValidator;
};




void centerDialog( QWidget *widget, QWidget *centerParent );
void centerDialogBottom( QWidget *widget, QWidget *centerParent );
void comboMatchText( QComboBox *combo, const QString &text );
bool stringToOffset( const QString & text, uint &offset );

void showEntryFailure( QWidget *parent, const QString &msg );
bool verifyFileDestnation( QWidget *parent, const QString &title, 
			   const QString &path );



#endif





