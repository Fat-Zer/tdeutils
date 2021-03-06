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
  #include "images.h"
#else
  #include <tdelocale.h>
  #include <kdialogbase.h>
  #include <kiconloader.h>
  #include "compoundwidget.moc"
#endif

#include "compoundwidget.h"
#include <tqlayout.h>
#include <tqcursor.h>
#include <tqlineedit.h>
#include <tqpainter.h>
#include <tqtooltip.h>
#include "concwidget.h"
#include "kwidgetstreamer.h"
#include "tqcheckbox.h"

//================================================================================

CompoundDetailWindow::CompoundDetailWindow( TQWidget* parent, const char* name )
  :TQWidget( parent, name )
{
  TQVBoxLayout* layout = new TQVBoxLayout( this );
  layout->setAutoAdd( true );

  TQLabel* label = new TQLabel( i18n("&Title:"), this);
  _title = new TQLineEdit( this );
  label->setBuddy( _title );

  label = new TQLabel( i18n("&Description:"), this );
  _description  = new TQMultiLineEdit( this );
  label->setBuddy( _description );

  _allowReplace = new TQCheckBox( i18n("&Automatically replace using this item"), this );
  TQToolTip::add( _allowReplace, i18n("When the content of this box is typed in to the ASCII line,<br>"
                                     "this box will automatically be added around it,<br>"
                                     "if this check box is selected.") );
  _allowReplace->setChecked( true );

  _title->setFocus();

}

TQString CompoundDetailWindow::title() const
{
  return _title->text();
}

TQString CompoundDetailWindow::description() const
{
  return _description->text();
}

bool CompoundDetailWindow::allowReplace() const
{
  return _allowReplace->isChecked();
}

void CompoundDetailWindow::setTitle( TQString txt )
{
  _title->setText( txt );
}

void CompoundDetailWindow::setDescription( TQString txt )
{
  _description->setText( txt );
}

void CompoundDetailWindow::setAllowReplace( bool b )
{
  _allowReplace->setChecked( b );
}

//================================================================================

CompoundWidget::CompoundWidget( RegExpEditorWindow* editorWindow, TQWidget* parent,
                                const char* name )
  :SingleContainerWidget( editorWindow, parent, name == 0 ? "CompoundWidget" : name )
{
  _child = new ConcWidget( editorWindow, this );
  init();
}

CompoundWidget::CompoundWidget( CompoundRegExp* regexp, RegExpEditorWindow* editorWindow,
                                TQWidget* parent, const char* name )
  : SingleContainerWidget( editorWindow, parent, name  == 0 ? "CompoundWidget" : name )
{
  init();
  _content->setTitle( regexp->title() );
  _content->setDescription( regexp->description() );
  _content->setAllowReplace( regexp->allowReplace() );
  RegExpWidget* child = WidgetFactory::createWidget( regexp->child(), _editorWindow, this );
  if ( !( _child = dynamic_cast<ConcWidget*>(child) ) ) {
    _child = new ConcWidget( _editorWindow, child, this );
  }

  _hidden = regexp->hidden();
}

void CompoundWidget::init( )
{
  _configWindow = new KDialogBase( this, "_configWindow", true,
                                   i18n("Configure Compound"),
                                   KDialogBase::Ok | KDialogBase::Cancel );
  _content = new CompoundDetailWindow( _configWindow );
  _configWindow->setMainWidget( _content );

  connect( _configWindow, TQT_SIGNAL(cancelClicked()), this, TQT_SLOT(slotConfigCanceled())) ;
  connect(_configWindow, TQT_SIGNAL(finished()), this, TQT_SLOT(slotConfigWindowClosed()));

  _down = getIcon( TQString::fromLocal8Bit( "1downarrow" ));
  _up = getIcon( TQString::fromLocal8Bit( "1uparrow" ) );

  _hidden = false;
  _backRefId = -1;
}

TQSize CompoundWidget::sizeHint() const
{
  TQFontMetrics metrics = fontMetrics();
  _childSize = _child->sizeHint();
  _textSize = metrics.size( 0, _content->title() );

  int width, height;

  if ( _hidden ) {
    _pixmapSize = _up.size();
    width = 2*pw + TQMAX( 2*bdSize+_textSize.width(), 2*bdSize+_pixmapSize.width());
    height = _pixmapSize.height() + 2*bdSize + _textSize.height()+pw;
  }
  else {
    _pixmapSize = _down.size();
    int headerLineWidth = 2*pw + 2*bdSize + _pixmapSize.width();
    if ( _textSize.width() != 0)
      headerLineWidth += 3*bdSize + _textSize.width();

    width = TQMAX( 2*pw + _childSize.width(), headerLineWidth );
    height = TQMAX( _textSize.height(), _pixmapSize.height() ) +
      2*bdSize + _childSize.height() + pw;
  }
  return TQSize( width, height );

}

void CompoundWidget::paintEvent( TQPaintEvent *e )
{
  TQSize mySize = sizeHint();

  TQPainter painter(this);
  drawPossibleSelection( painter, mySize);

  int horLineY, childY;

  // draw top line
  if ( _hidden ) {
    horLineY = _pixmapSize.height()/2;
    childY = _pixmapSize.height() + bdSize;
    _pixmapPos = TQPoint( mySize.width()-pw-bdSize-_pixmapSize.width(), 0 );
    painter.drawLine( pw, horLineY, _pixmapPos.x(), horLineY );
    painter.drawLine( mySize.width() - bdSize - pw, horLineY,
                      mySize.width(), horLineY);
    painter.drawPixmap( _pixmapPos, _up );
  }
  else {
    int maxH = TQMAX( _textSize.height(), _pixmapSize.height() );
    int offset = 0;
    horLineY = maxH/2;
    childY = maxH+bdSize;

    painter.drawLine(pw, horLineY, bdSize, horLineY);
    if ( _textSize.width() != 0 ) {
      offset += pw + 2*bdSize;

      painter.drawText(offset, horLineY-_textSize.height()/2,
                       bdSize+_textSize.width(), horLineY+_textSize.height()/2,
                       0, _content->title());
      offset += _textSize.width() + bdSize;
    }

    _pixmapPos = TQPoint( mySize.width()-pw-bdSize-_pixmapSize.width(),
                         horLineY - _pixmapSize.height()/2 );

    painter.drawLine( offset, horLineY, _pixmapPos.x(), horLineY );
    painter.drawPixmap( _pixmapPos, _down );

    painter.drawLine( mySize.width()-bdSize-pw, horLineY, mySize.width(), horLineY );
  }

  // draw rest frame
  painter.drawLine(0, horLineY, 0, mySize.height() );
  painter.drawLine( mySize.width()-pw, horLineY, mySize.width()-pw, mySize.height() );
  painter.drawLine( 0, mySize.height()-pw, mySize.width(), mySize.height()-pw );

  // place/size child
  if ( _hidden ) {
    _child->hide();
    painter.drawText( pw+bdSize, childY,
                      pw+bdSize+_textSize.width(), childY+_textSize.height(), 0,
                      _content->title() );
  }
  else {
    TQSize curSize = _child->size();
    TQSize newSize = TQSize( TQMAX( _child->sizeHint().width(), mySize.width()-2*pw),
                           _child->sizeHint().height());

    _child->move( pw, childY );
    if ( curSize != newSize ) {
      _child->resize(newSize);
      // I resized the child, so give it a chance to relect thus.
      _child->update();
    }

    _child->show();
  }

  RegExpWidget::paintEvent( e );
}

void CompoundWidget::slotConfigWindowClosed()
{
  _editorWindow->updateContent( 0 );
  update();
}

void CompoundWidget::slotConfigCanceled()
{
  TQDataStream stream( _backup, IO_ReadOnly );
  KWidgetStreamer streamer;
  streamer.fromStream( stream, TQT_TQOBJECT(_content) );
  repaint();
}

RegExp* CompoundWidget::regExp() const
{
  return new CompoundRegExp( isSelected(), _content->title(), _content->description(),
                             _hidden, _content->allowReplace(), _child->regExp() );
}

void CompoundWidget::mousePressEvent( TQMouseEvent* event )
{
  if ( event->button() == Qt::LeftButton &&
       TQRect( _pixmapPos, _pixmapSize ).contains( event->pos() ) ) {
    // Skip otherwise we will never see the mouse release
    // since it is eaten by Editor window.
  }
  else
    SingleContainerWidget::mousePressEvent( event );
}

void CompoundWidget::mouseReleaseEvent( TQMouseEvent* event)
{
  if ( event->button() == Qt::LeftButton &&
       TQRect( _pixmapPos, _pixmapSize ).contains( event->pos() ) ) {
    _hidden = !_hidden;
    _editorWindow->updateContent( 0 );
    repaint(); // is this necesary?
    _editorWindow->emitChange();
  }
  else
    SingleContainerWidget::mouseReleaseEvent( event );
}

bool CompoundWidget::updateSelection( bool parentSelected )
{
  if ( _hidden ) {
    bool changed = RegExpWidget::updateSelection( parentSelected );
    _child->selectWidget( _isSelected );
    if (changed)
      repaint();
    return changed;
  }
  else {
    return SingleContainerWidget::updateSelection( parentSelected );
  }
}

int CompoundWidget::edit()
{
  _configWindow->move(TQCursor::pos() - TQPoint(_configWindow->sizeHint().width()/2,
                                              _configWindow->sizeHint().height()/2)  );
  TQDataStream stream( _backup, IO_WriteOnly );
  KWidgetStreamer streamer;
  streamer.toStream( TQT_TQOBJECT(_content), stream );
  return _configWindow->exec();
}

int nextId()
{
  static int counter = 0;
  return ++counter;
}

TQPixmap CompoundWidget::getIcon( const TQString& name )
{
#ifdef TQT_ONLY
    TQPixmap pix;
    pix.convertFromImage( qembed_findImage(name) );
    return pix;
#else
    return SmallIcon( name );
#endif
}


