// -*- c++ -*-
/* This file is part of the KDE libraries
    Copyright (C) 2001 Wolfram Diestel <wolfram@steloj.de>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#ifndef __KTEXTFILEDLG_H__
#define __KTEXTFILEDLG_H__

#include <tdefiledialog.h>

class TQComboBox;
class TQTextCodec;
class TQString;
class KURL;

/**
 * Provides filedialog with a tollbar button for
   a char encoding selection. The selected encoding
   is returned as parameter charset in the query part
   of the URL. This is helps to save the charset together with
   the filename in the recent files list.
 */
class KTextFileDialog : public KFileDialog
{
  Q_OBJECT
  

 public:
    KTextFileDialog(const TQString& startDir, const TQString& filter,
		    TQWidget *parent, const char *name,
		    bool modal);

    ~KTextFileDialog();

    static KURL getOpenURLwithEncoding(
         const TQString& startDir = TQString(),
	 const TQString& filter= TQString(),
	 TQWidget *parent= 0,
	 const TQString& caption = TQString(),
	 const TQString& encoding = TQString(),
	 const TQString& buttonText = TQString());

    static KURL getSaveURLwithEncoding(
	 const TQString& dir, const TQString& filter,
	 TQWidget *parent,
	 const TQString& caption,
	 const TQString& encoding = TQString());

    TQString &encoding() { return enc; }
    void setEncoding(const TQString& encoding);

  protected slots:
    void slotShowEncCombo();

  private:
   TQString enc;
};


#endif
