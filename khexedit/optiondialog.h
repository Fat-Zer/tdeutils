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

#ifndef _OPTION_DIALOG_H_
#define _OPTION_DIALOG_H_


class QComboBox;
class QFrame;
class QCheckBox;
class QLabel;
class QLineEdit;
class QPushbutton;
class QRadioButton;
class QSpinBox;
class QComboBox;
class KFontChooser;


#include <kdialogbase.h>
#include <klistbox.h> 
#include "hexeditstate.h"



class CColorListBox : public KListBox
{
  Q_OBJECT

  public:
    CColorListBox( QWidget *parent=0, const char * name=0, WFlags f=0 );
    void setColor( uint index, const QColor &color );
    const QColor color( uint index );

  public slots:
    virtual void setEnabled( bool state );

  protected:
    void dragEnterEvent( QDragEnterEvent *e );
    void dragLeaveEvent( QDragLeaveEvent *e );
    void dragMoveEvent( QDragMoveEvent *e );
    void dropEvent( QDropEvent *e );

  private slots:
    void newColor( int index );

  private:
    int mCurrentOnDragEnter;

  signals:
    void dataChanged();
};


class CColorListItem : public QListBoxItem
{
  public:
    CColorListItem( const QString &text, const QColor &color=Qt::black );
    const QColor &color( void );
    void  setColor( const QColor &color );
  
  protected:
    virtual void paint( QPainter * );
    virtual int height( const QListBox * ) const;
    virtual int width( const QListBox * ) const;

  private:
    QColor mColor;
    int mBoxWidth;
};




class COptionDialog : public KDialogBase
{
  Q_OBJECT

  public:
    enum EPage
    {
      page_layout = 0,
      page_cursor,
      page_font,
      page_color,
      page_file,
      page_misc,
      page_max
    };

    COptionDialog( QWidget *parent = 0, char *name = 0, bool modal = false );
    ~COptionDialog( void );

    void setLayout( SDisplayLayout &layout, SDisplayLine &line );
    void setCursor( SDisplayCursor &cursor );
    void setColor( SDisplayColor &color );
    void setFont( SDisplayFont &font );
    void setFile( SDisplayMisc &misc );
    void setMisc( SDisplayMisc &misc );
    void setState( SDisplayState &state );

  protected:  
    virtual void showEvent( QShowEvent *e );

  protected slots:
    void slotModeSelectorChanged( int index );
    void slotLineSizeChanged( int value );
    void slotColumnSizeChanged( int value );
    void slotColumnSepCheck( bool state );
    void slotBlinkIntervalCheck( bool state );
    void slotBlockCursorCheck( bool state );
    void slotFontSystem( bool state );
    void slotColorSystem( bool state );
    void slotThresholdConfirm( void );

    virtual void slotDefault( void );
    virtual void slotOk( void );
    virtual void slotApply( void );
    void slotChanged();

  private:
    struct SLayoutWidgets
    {
      QComboBox *formatCombo;
      QSpinBox  *lineSizeSpin;
      QSpinBox  *columnSizeSpin;
      QCheckBox *lockLineCheck;
      QCheckBox *lockColumnCheck;
      QCheckBox *columnCheck;
      QComboBox *gridCombo;
      QLabel    *leftSepLabel;
      QLabel    *rightSepLabel;
      QSpinBox  *leftSepWidthSpin;
      QSpinBox  *rightSepWidthSpin;
      QSpinBox  *separatorSpin;
      QSpinBox  *edgeSpin;
      QLabel    *columnSepLabel;
      QSpinBox  *columnSepSpin;

    };
    struct SCursorWidgets 
    {
      QCheckBox *blinkCheck;
      QSpinBox  *blinkSpin;
      QLabel    *blinkLabel;  
      QCheckBox *blockCheck;
      QCheckBox *thickCheck;
      QRadioButton *stopRadio;
      QRadioButton *hideRadio;
      QRadioButton *nothingRadio;
    };
    struct SColorWidgets
    {
      enum colorType
      {
	FirstTextBg = 0,
	SecondTextBg,
	OffsetBg,
	InactiveBg,
	EvenColumnFg,
	OddColumnFg,
	NonPrintFg,
	OffsetFg,
	SecondaryFg,
	MarkedBg,
	MarkedFg,
	CursorBg,
	CursorFg,
	BookmarkBg,
	BookmarkFg,
	SeparatorFg,
	GridFg,
	MAX_COLOR
      };

      QCheckBox     *checkSystem;
      CColorListBox *colorList; 
    };
    struct SFontWidgets
    {
      QCheckBox    *checkSystem;
      KFontChooser *chooser;
      QLineEdit    *nonPrintInput;
    };
    struct SFileWidgets
    {
      QComboBox *openCombo;
      QCheckBox *gotoOffsetCheck;
      QCheckBox *reloadOffsetCheck;
      QCheckBox *writeProtectCheck;
      QCheckBox *backupCheck;
      QCheckBox *discardRecentCheck;
    };
    struct SMiscWidgets
    {
      QSpinBox  *undoSpin;
      QCheckBox *inputCheck;
      QCheckBox *fatalCheck;
      QCheckBox *autoCheck;
      QCheckBox *insertCheck;
      QCheckBox *confirmWrapCheck;
      QCheckBox *cursorJumpCheck;
      QCheckBox *thresholdCheck;
      QCheckBox *bookmarkColumnCheck;
      QCheckBox *bookmarkEditorCheck;
      QLabel    *thresholdLabel;
      QSpinBox  *thresholdSpin;
    };

  private:
    void setupLayoutPage( void );
    void setupCursorPage( void );
    void setupColorPage( void );
    void setupFontPage( void );
    void setupFilePage( void );
    void setupMiscPage( void );
    SDisplayCursor::EFocusMode cursorFocusMode( void );


  signals:
    void lineSizeChoice( const SDisplayLine &lineSize );
    void layoutChoice( const SDisplayLayout &layout );
    void cursorChoice( const SDisplayCursor &cursor ); 
    void colorChoice( const SDisplayColor &color );
    void fontChoice( const SDisplayFont &font );
    void miscChoice( const SDisplayMisc &misc );
    void removeRecentFiles( void );

  private:
    SDisplayState  mDisplayState;
    SLayoutWidgets mLayout;
    SCursorWidgets mCursor;
    SColorWidgets  mColor;
    SFontWidgets   mFont;
    SFileWidgets   mFile;
    SMiscWidgets   mMisc;
    bool configChanged;

};





#endif







