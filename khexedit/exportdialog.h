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


class QButtonGroup;
class QComboBox;
class QCheckBox;
class QFrame;
class QLabel;
class QLineEdit;
class QSpinBox;
class QWidgetStack;
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

    CExportDialog( QWidget *parent = 0, char *name = 0, bool modal = false );
    ~CExportDialog( void );
    void writeConfiguration( void );

  protected:
    void showEvent( QShowEvent *e );

  protected slots:
    virtual void slotOk( void );
  void  destinationChanged(const QString &);
  private:
    struct SDestinationWidgets
    {
      QComboBox    *formatCombo;
      QLabel       *fileExtraLabel;
      QLineEdit    *fileInput;
      QLabel       *fromLabel;
      QLabel       *toLabel;
      QButtonGroup *rangeBox;
      QLineEdit    *fromInput;
      QLineEdit    *toInput;
    };

    struct SHtmlWidgets 
    {
      QSpinBox  *lineSpin;
      QLineEdit *prefixInput;
      QComboBox *topCombo;
      QComboBox *bottomCombo;
      QCheckBox *navigatorCheck;
      QCheckBox *symlinkCheck;
      QCheckBox *bwCheck;
    };

    struct SArrayWidgets 
    {
      QLineEdit *nameInput;
      QComboBox *typeCombo;
      QSpinBox  *lineSizeSpin;
      QCheckBox *hexadecimalCheck;
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
    bool verifyPackage( const QString &path );
    
  signals:
    void exportText( const SExportText &e );
    void exportHtml( const SExportHtml &e );
    void exportCArray( const SExportCArray &e );

  private:
    QFrame *mFrame[ page_max ];
    QWidgetStack *mOptionStack;
    SDestinationWidgets  mDestination;
    SHtmlWidgets  mHtml;
    SArrayWidgets mArray;
    KSimpleConfig *mConfig;

    QString mWorkDir;
};





#endif
