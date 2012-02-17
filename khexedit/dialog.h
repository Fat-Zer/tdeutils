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


#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqdialog.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqradiobutton.h>
#include <tqspinbox.h>
#include <tqstring.h>
#include <tqwidgetstack.h>

#include <kdialogbase.h>

#include "bitswapwidget.h"
#include "hexbuffer.h"
#include "hexvalidator.h"

const int repYes      = TQDialog::Accepted; 
const int repNo       = 11;
const int repAll      = 12;
const int repClose    = TQDialog::Rejected; 
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
    CGotoDialog( TQWidget *parent, const char *name = 0, bool modal = false );
    ~CGotoDialog( void );

  protected:
    virtual void showEvent( TQShowEvent *e );

  private slots:
    void slotOk( void );

  signals:
    void gotoOffset( uint offset, uint bit, bool fromCursor, bool forward );

  private:
    TQComboBox *mComboBox;
    TQCheckBox *mCheckBackward;
    TQCheckBox *mCheckFromCursor;
    TQCheckBox *mCheckVisible;
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
    CFindDialog( TQWidget *parent=0, const char *name=0, bool modal=false );
    ~CFindDialog( void );
    bool isEmpty( void );

  public slots:
    void findAgain( EOperation operation );

  protected:
    virtual void showEvent( TQShowEvent *e );

  private slots:
    void slotOk( void );
    void selectorChanged( int index );
    void inputChanged( const TQString &text );

  signals: 
    void findData( SSearchControl &sc, uint mode, bool navigator );

  private:
    TQComboBox *mSelector;
    TQLineEdit *mInput;
    TQCheckBox *mCheckBackward;
    TQCheckBox *mCheckFromCursor;
    TQCheckBox *mCheckInSelection;
    TQCheckBox *mCheckUseNavigator;
    TQCheckBox *mCheckIgnoreCase;

    TQString    mFindString[5];
    TQByteArray mFindData;
    CHexValidator *mFindValidator;
};



class CFindNavigatorDialog : public KDialogBase
{
  Q_OBJECT
  
  
  public:
    CFindNavigatorDialog( TQWidget *parent=0, const char *name=0, 
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
    TQLineEdit *mKey;
    SSearchControl mSearchControl;
};


class CReplaceDialog : public KDialogBase
{
  Q_OBJECT
  
  
  public:
    CReplaceDialog( TQWidget *parent=0, const char *name=0, bool modal=false );
    ~CReplaceDialog( void );

  protected:
    virtual void showEvent( TQShowEvent *e );

  private slots:
    void slotOk( void );
    void findSelectorChanged( int index );
    void findInputChanged( const TQString &text );
    void replaceSelectorChanged( int index );
    void replaceInputChanged( const TQString &text );

  signals:
    void replaceData( SSearchControl &sc, uint mode );

  private:
    TQComboBox *mFindSelector;
    TQComboBox *mReplaceSelector;
    TQLineEdit *mFindInput;
    TQLineEdit *mReplaceInput;

    TQCheckBox *mCheckBackward;
    TQCheckBox *mCheckFromCursor;
    TQCheckBox *mCheckInSelection;
    TQCheckBox *mCheckPrompt;
    TQCheckBox *mCheckIgnoreCase;

    TQString    mFindString[5];
    TQString    mReplaceString[5];
    TQByteArray mFindData;
    TQByteArray mReplaceData;
    CHexValidator *mFindValidator;
    CHexValidator *mReplaceValidator;
};



class CReplacePromptDialog : public KDialogBase
{
  Q_OBJECT
  
  
  public:
    CReplacePromptDialog( TQWidget *parent=0, const char *name=0, 
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
    CFilterDialog( TQWidget *parent=0, const char *name=0, bool modal=false );
    ~CFilterDialog( void );

  protected:
    virtual void showEvent( TQShowEvent *e );

  private:
    void makeEmptyLayout( void );
    void makeOperandLayout( void );
    void makeBitSwapLayout( void );
    void makeRotateLayout( void );

  private slots:
    void slotOk( void );
    void operandSelectorChanged( int index );
    void operandInputChanged( const TQString &text );
    void operationSelectorChanged( int index );

  signals:
    void filterData( SFilterControl &fc );

  private:
    TQWidgetStack *mWidgetStack;
    TQLabel    *mOperandFormatLabel;
    TQLabel    *mOperandInputLabel;
    TQComboBox *mOperandSelector;
    TQLineEdit *mOperandInput;
    TQComboBox *mOperationSelector;
    TQSpinBox  *mGroupSpin;
    TQSpinBox  *mBitSpin;
    CByteWidget *mByteWidget;

    TQCheckBox *mCheckBackward;
    TQCheckBox *mCheckFromCursor;
    TQCheckBox *mCheckInSelection;
    TQCheckBox *mCheckVisible;

    TQString    mOperandString[5];
    TQByteArray mOperandData;
    CHexValidator *mOperandValidator;
};




class CInsertDialog : public KDialogBase
{
  Q_OBJECT
  
  
  public:
    CInsertDialog( TQWidget *parent=0, const char *name=0, bool modal=false );
    ~CInsertDialog( void );

  protected:
    virtual void showEvent( TQShowEvent *e );

  private slots:
    void slotOk( void );
    void cursorCheck( void );
    void patternSelectorChanged( int index );
    void patternInputChanged( const TQString &text );

  signals:
    void execute( SInsertData &id );

  private:
    TQSpinBox *mSizeBox;
    TQLabel *mOffsetLabel;
    TQComboBox *mPatternSelector;
    TQLineEdit *mPatternInput;
    TQLineEdit *mOffsetInput;
    TQCheckBox *mCheckPattern;
    TQCheckBox *mCheckOnCursor;

    TQString    mPatternString[5];
    TQByteArray mPatternData;
    CHexValidator *mPatternValidator;
};




void centerDialog( TQWidget *widget, TQWidget *centerParent );
void centerDialogBottom( TQWidget *widget, TQWidget *centerParent );
void comboMatchText( TQComboBox *combo, const TQString &text );
bool stringToOffset( const TQString & text, uint &offset );

void showEntryFailure( TQWidget *parent, const TQString &msg );
bool verifyFileDestnation( TQWidget *parent, const TQString &title, 
			   const TQString &path );



#endif





