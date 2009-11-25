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

class QCheckBox;
class QComboBox;
class QLabel;
class QSpinBox;

#include <kdeprint/kprintdialogpage.h>
#include <ksimpleconfig.h>

class LayoutDialogPage : public KPrintDialogPage
{
 Q_OBJECT

 public:
    LayoutDialogPage( QWidget *parent = 0, const char *name = 0 );
    ~LayoutDialogPage( void );

    void getOptions( QMap<QString,QString>& opts, bool incldef = false );

 private slots:
   void slotDrawHeader( bool state );
   void slotDrawFooter( bool state );

 private:
   void setupLayoutPage( void );

   void readConfiguration( void );
   void writeConfiguration( void );

   QString headerText( uint index );
   QString headerLine( uint index );
   int headerTextIndex( const QString & headerText );
   int headerLineIndex( const QString & headerLine );

   struct SLayoutWidgets
   {
     QSpinBox     *marginSpin[4];
     QCheckBox    *headerCheck;
     QCheckBox    *footerCheck;
     QLabel       *headerLabel[4];
     QComboBox    *headerCombo[4];
     QLabel       *footerLabel[4];
     QComboBox    *footerCombo[4];
   };

   KSimpleConfig *mConfig;
   SLayoutWidgets mLayout;
};

#endif
