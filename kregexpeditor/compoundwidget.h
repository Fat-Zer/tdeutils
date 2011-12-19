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
#ifndef compoundwidget
#define compoundwidget
class TQLineEdit;
class TQMultiLineEdit;
class KDialogBase;
class TQCheckBox;

#include "singlecontainerwidget.h"
#include "compoundregexp.h"

/**
   Widget containing configuration details for @ref CompoundWidget
   @internal
*/
class CompoundDetailWindow :public TQWidget
{
public:
  CompoundDetailWindow(TQWidget* parent, const char* name = 0);
  TQString title() const;
  TQString description() const;
  bool allowReplace() const;
  void setTitle( TQString );
  void setDescription( TQString );
  void setAllowReplace( bool );

private:
  TQLineEdit* _title;
  TQMultiLineEdit* _description;
  TQCheckBox* _allowReplace;
};


/**
   Comopund RegExp widget.

   This widget has two purposes:
   @li To make it possible for a user to collapse a huge regular expression
   to take up less screen space.
   @li To act as back references for later use.

   @internal
*/
class CompoundWidget :public SingleContainerWidget
{
Q_OBJECT
  TQ_OBJECT

public:
  CompoundWidget( RegExpEditorWindow* editorWindow, TQWidget* parent,
                  const char* name = 0);
  CompoundWidget( CompoundRegExp* regexp, RegExpEditorWindow* editorWindow,
                  TQWidget* parent, const char* name = 0);

  virtual bool updateSelection( bool parentSelected );
  virtual TQSize sizeHint() const;
  virtual RegExp* regExp() const;
  virtual RegExpType type() const { return COMPOUND; }
  virtual int edit();

protected:
  virtual void paintEvent( TQPaintEvent *e );
  virtual void mousePressEvent( TQMouseEvent* e );
  virtual void mouseReleaseEvent( TQMouseEvent* e);
  void init();
  TQPixmap getIcon( const TQString& name );

protected slots:
  void slotConfigCanceled();
  void slotConfigWindowClosed();

private:
  bool _hidden;
  TQPixmap _up, _down;
  mutable TQSize _pixmapSize;
  mutable TQPoint _pixmapPos;

  KDialogBase* _configWindow;
  CompoundDetailWindow* _content;

  mutable TQSize _textSize;
  mutable TQSize _childSize;
  TQByteArray _backup;

  int _backRefId;
};


#endif // compoundwidget
