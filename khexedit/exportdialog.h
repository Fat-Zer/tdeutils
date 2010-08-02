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

#ifndef _EXPORT_DIALOG_H_
#define _EXPORT_DIALOG_H_


class TQButtonGroup;
class TQComboBox;
class TQCheckBox;
class TQFrame;
class TQLabel;
class TQLineEdit;
class TQSpinBox;
class TQWidgetStack;
class KSimpleConfig;

#include <kdialogbase.h>
#include "hexbuffer.h"


class CExportDialog : public KDialogBase
{
  Q_OBJECT

  public:
    enum EPage
    {
      page_destination = 0,
      page_option,
      page_max
    };

    enum OptionPage
    {
      option_text = 0,
      option_html,
      option_rtf,
      option_carray,
      option_max
    };

    CExportDialog( TQWidget *parent = 0, char *name = 0, bool modal = false );
    ~CExportDialog( void );
    void writeConfiguration( void );

  protected:
    void showEvent( TQShowEvent *e );

  protected slots:
    virtual void slotOk( void );
  void  destinationChanged(const TQString &);
  private:
    struct SDestinationWidgets
    {
      TQComboBox    *formatCombo;
      TQLabel       *fileExtraLabel;
      TQLineEdit    *fileInput;
      TQLabel       *fromLabel;
      TQLabel       *toLabel;
      TQButtonGroup *rangeBox;
      TQLineEdit    *fromInput;
      TQLineEdit    *toInput;
    };

    struct SHtmlWidgets 
    {
      TQSpinBox  *lineSpin;
      TQLineEdit *prefixInput;
      TQComboBox *topCombo;
      TQComboBox *bottomCombo;
      TQCheckBox *navigatorCheck;
      TQCheckBox *symlinkCheck;
      TQCheckBox *bwCheck;
    };

    struct SArrayWidgets 
    {
      TQLineEdit *nameInput;
      TQComboBox *typeCombo;
      TQSpinBox  *lineSizeSpin;
      TQCheckBox *hexadecimalCheck;
    };

  private slots:
    void rangeChanged( int id );
    void formatChanged( int index );
    void browserClicked( void );

  private:
    void setupDestinationPage( void );
    void setupOptionPage( void );
    void makeTextOption( void );
    void makeHtmlOption( void );
    void makeRtfOption( void );
    void makeCArrayOption( void );
    void readConfiguration( void );
    bool collectRange( uint &mode, uint &start, uint &stop );
    bool verifyPackage( const TQString &path );
    
  signals:
    void exportText( const SExportText &e );
    void exportHtml( const SExportHtml &e );
    void exportCArray( const SExportCArray &e );

  private:
    TQFrame *mFrame[ page_max ];
    TQWidgetStack *mOptionStack;
    SDestinationWidgets  mDestination;
    SHtmlWidgets  mHtml;
    SArrayWidgets mArray;
    KSimpleConfig *mConfig;

    TQString mWorkDir;
};





#endif
