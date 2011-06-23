/*
 *   khexedit - Versatile hex editor
 *   Copyright (C) 1999  Espen Sand, espensa@online.no
 *   This file is based on the work by F. Zigterman, fzr@dds.nl
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

#ifndef _PRINT_DIALOG_PAGE_H_
#define _PRINT_DIALOG_PAGE_H_

class TQCheckBox;
class TQComboBox;
class TQLabel;
class TQSpinBox;

#include <kdeprint/kprintdialogpage.h>
#include <ksimpleconfig.h>

class LayoutDialogPage : public KPrintDialogPage
{
 Q_OBJECT
  TQ_OBJECT

 public:
    LayoutDialogPage( TQWidget *tqparent = 0, const char *name = 0 );
    ~LayoutDialogPage( void );

    void getOptions( TQMap<TQString,TQString>& opts, bool incldef = false );

 private slots:
   void slotDrawHeader( bool state );
   void slotDrawFooter( bool state );

 private:
   void setupLayoutPage( void );

   void readConfiguration( void );
   void writeConfiguration( void );

   TQString headerText( uint index );
   TQString headerLine( uint index );
   int headerTextIndex( const TQString & headerText );
   int headerLineIndex( const TQString & headerLine );

   struct SLayoutWidgets
   {
     TQSpinBox     *marginSpin[4];
     TQCheckBox    *headerCheck;
     TQCheckBox    *footerCheck;
     TQLabel       *headerLabel[4];
     TQComboBox    *headerCombo[4];
     TQLabel       *footerLabel[4];
     TQComboBox    *footerCombo[4];
   };

   KSimpleConfig *mConfig;
   SLayoutWidgets mLayout;
};

#endif
