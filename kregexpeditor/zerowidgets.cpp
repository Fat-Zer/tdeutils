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

#ifdef TQT_ONLY
  #include "compat.h"
#else
  #include <klocale.h>
#endif

#include "zerowidgets.h"
#include "dotregexp.h"
#include "positionregexp.h"
#include <tqpainter.h>
#include "myfontmetrics.h"
//--------------------------------------------------------------------------------
//                                ZeroWidget
//--------------------------------------------------------------------------------
ZeroWidget::ZeroWidget(TQString txt, RegExpEditorWindow* editorWindow,
                       TQWidget *tqparent, const char *name)
  : RegExpWidget(editorWindow, tqparent, name ? name : "ZeroWidget" )
{
  _text = txt;
}

void ZeroWidget::addNewChild(DragAccepter *, RegExpWidget *)
{
  qFatal("No tqchildren should be added to this widget!");
}

TQSize ZeroWidget::tqsizeHint() const
{
  TQFontMetrics metrics = fontMetrics();
  _textSize = HackCalculateFontSize( metrics, _text );
  //  _textSize = metrics.size(0,_text);
  _boxSize = TQSize(_textSize.width() + 2*space, _textSize.height() + 2 *space);
  return _boxSize;
}

void ZeroWidget::paintEvent( TQPaintEvent *e)
{
  // So what is my Size?
  TQSize mySize = tqsizeHint();

  TQPainter painter(this);
  drawPossibleSelection( painter, mySize);

  // Write the text and the rectangle
  painter.drawText(space, space, _textSize.width(), _textSize.height(), 0, _text);
  painter.drawRoundRect(0, 0, _boxSize.width(), _boxSize.height());

  RegExpWidget::paintEvent(e);
}


//--------------------------------------------------------------------------------
//                                AnyCharWidget
//--------------------------------------------------------------------------------
AnyCharWidget::AnyCharWidget(RegExpEditorWindow* editorWindow, TQWidget *tqparent,
                             const char *name)
  : ZeroWidget(i18n("Any\nCharacter"), editorWindow, tqparent,
               name ? name : "AnyCharWidget")
{
}

RegExp* AnyCharWidget::regExp() const
{
	return new DotRegExp( isSelected() );
}


//--------------------------------------------------------------------------------
//                                BegLineWidget
//--------------------------------------------------------------------------------
BegLineWidget::BegLineWidget(RegExpEditorWindow* editorWindow, TQWidget *tqparent,
                             const char *name)
  : ZeroWidget(i18n("Line\nStart"), editorWindow, tqparent,
               name ? name : "BegLineWidget")
{
}

RegExp* BegLineWidget::regExp() const
{
	return new PositionRegExp( isSelected(), PositionRegExp::BEGLINE );

}

//--------------------------------------------------------------------------------
//                                EndLineWidget
//--------------------------------------------------------------------------------
EndLineWidget::EndLineWidget(RegExpEditorWindow* editorWindow, TQWidget *tqparent,
                             const char *name)
  : ZeroWidget(i18n("Line\nEnd"), editorWindow, tqparent, name)
{
}

RegExp* EndLineWidget::regExp() const
{
	return new PositionRegExp( isSelected(), PositionRegExp::ENDLINE );
}

//--------------------------------------------------------------------------------
//                                WordBoundaryWidget
//--------------------------------------------------------------------------------
WordBoundaryWidget::WordBoundaryWidget(RegExpEditorWindow* editorWindow, TQWidget *tqparent,
                                       const char *name)
  : ZeroWidget(i18n("Word\nBoundary"), editorWindow, tqparent,
               name ? name : "WordBoundaryWidget" )
{
}

RegExp* WordBoundaryWidget::regExp() const
{
	return new PositionRegExp( isSelected(), PositionRegExp::WORDBOUNDARY );
}

//--------------------------------------------------------------------------------
//                                NonWordBoundaryWidget
//--------------------------------------------------------------------------------
NonWordBoundaryWidget::NonWordBoundaryWidget(RegExpEditorWindow* editorWindow, TQWidget *tqparent,
                                             const char *name)
  : ZeroWidget(i18n("Non-word\nBoundary"), editorWindow, tqparent,
               name ? name : "NonWordBoundaryWidget" )
{
}

RegExp* NonWordBoundaryWidget::regExp() const
{
	return new PositionRegExp( isSelected(), PositionRegExp::NONWORDBOUNDARY );
}

