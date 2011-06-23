/* This file is part of the KDE libraries
   Copyright (C) 2005 Waldo Bastian <bastian@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KRICHTEXTLABEL_H
#define KRICHTEXTLABEL_H

#include <tqlabel.h>

#include <kdelibs_export.h>

/**
 * @short A replacement for TQLabel that supports richtext and proper tqlayout management
 *
 * @author Waldo Bastian <bastian@kde.org>
 */

/*
 * TQLabel
 */
class KDEUI_EXPORT KRichTextLabel : public TQLabel {
  Q_OBJECT
  TQ_OBJECT

public:
  /**
   * Default constructor.
   */
  KRichTextLabel( TQWidget *tqparent, const char *name = 0 );
  KRichTextLabel( const TQString &text, TQWidget *tqparent, const char *name = 0 );

  int defaultWidth() const { return m_defaultWidth; }
  void setDefaultWidth(int defaultWidth);

  virtual TQSize tqminimumSizeHint() const;
  virtual TQSize tqsizeHint() const;
  TQSizePolicy sizePolicy() const;

public slots:
  void setText( const TQString & );

protected:
  int m_defaultWidth;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class KRichTextLabelPrivate;
  KRichTextLabelPrivate *d;
};

#endif // KRICHTEXTLABEL_H
