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
  #include <klocale.h>
  #include <kmessagebox.h>
//   #include <kfiledialog.h>
  #include <kstandarddirs.h>
  #include <kiconloader.h>
  #include "editorwindow.moc"
  #include <klineeditdlg.h>
#endif

#include "editorwindow.h"
#include "concwidget.h"
#include <tqlayout.h>
#include <tqpainter.h>
#include <tqaccel.h>
#include <tqcursor.h>
#include <tqclipboard.h>
#include <tqpopupmenu.h>
#include "regexp.h"
#include "userdefinedregexps.h"
#include <tqfileinfo.h>

RegExpEditorWindow::RegExpEditorWindow( TQWidget *parent, const char *name)
    : TQWidget(parent, name, TQt::WPaintUnclipped)
{
    _top = new ConcWidget(this, this);
    _layout = new TQHBoxLayout( this);
    _layout->addWidget(_top);
    _top->setToplevel();
    _undrawSelection = false;
    _menu = 0;
    _insertInAction = false;
    _pasteInAction = false;
    _pasteData = 0;

    TQAccel* accel = new TQAccel( this );
    accel->connectItem( accel->insertItem( CTRL+Key_C ), this, TQT_SLOT( slotCopy() ) );
    accel->connectItem( accel->insertItem( CTRL+Key_X ), this, TQT_SLOT( slotCut() ) );
    accel->connectItem( accel->insertItem( Key_Delete ), this, TQT_SLOT( slotCut() ) );
    accel->connectItem( accel->insertItem( Key_BackSpace ), this, TQT_SLOT( slotCut() ) );
    accel->connectItem( accel->insertItem( CTRL+Key_V ), this, TQT_SLOT( slotStartPasteAction() ) );
    accel->connectItem( accel->insertItem( Key_Escape ), this, TQT_SLOT( slotEndActions() ) );
    accel->connectItem( accel->insertItem( CTRL+Key_S ), this, TQT_SLOT( slotSave() ) );

    connect( this, TQT_SIGNAL( change() ), this, TQT_SLOT( emitVerifyRegExp() ) );
}

RegExp* RegExpEditorWindow::regExp() const
{
	return _top->regExp();
}

void  RegExpEditorWindow::mousePressEvent ( TQMouseEvent* event )
{
    setFocus();
    updateContent( 0 );

    _start = event->pos();
    _lastPoint = TQPoint(0,0);

    if ( pointSelected( event->globalPos() ) ) {
        _isDndOperation = true;
    }
    else {
        _isDndOperation = false;
        _selection = TQRect();
        _top->updateSelection( false );

        TQWidget::mousePressEvent( event );
    }
    grabMouse();
}

bool RegExpEditorWindow::pointSelected( TQPoint p ) const
{
    TQRect rect = _top->selectionRect();
    return rect.contains(p);
}

void RegExpEditorWindow::mouseMoveEvent ( TQMouseEvent* event )
{
    if ( _isDndOperation ) {
        if ( ( _start - event->pos() ).manhattanLength() > TQApplication::startDragDistance() ) {
            RegExp* regexp = _top->selection();
            if ( !regexp )
                return;
            TQDragObject *d = new RegExpWidgetDrag( regexp, this );
            delete regexp;

            bool del = d->drag();
            if ( del )
                slotDeleteSelection();
            else {
                clearSelection( true );
            }
            releaseMouse();
            emit change();
            emit canSave( _top->hasAnyChildren() );
        }
    }
    else {
        TQPainter p( this );
        p.setRasterOp( TQt::NotROP );
        p.setPen( TQt::DotLine );

        // remove last selection rectangle
        if ( ! _lastPoint.isNull() && _undrawSelection ) {
            p.drawRect(TQRect(_start, _lastPoint));
        }

        // Note this line must come after the old rect has been removed
        // and before the new one is draw otherwise the update event which this
        // line invokes, will remove a line, which later will be drawn instead of
        // removed during NotROP.
        _top->updateSelection( false );
        emit scrolling( event->pos() );

        p.drawRect(TQRect(_start, event->pos()));
        _undrawSelection = true;
        _lastPoint = event->pos();

        _selection = TQRect(mapToGlobal(_start), mapToGlobal(_lastPoint)).normalize();
    }
}

void RegExpEditorWindow::mouseReleaseEvent( TQMouseEvent *event)
{
    releaseMouse();
    TQWidget::mouseReleaseEvent( event);

    // remove last selection rectangle
    TQPainter p( this );
    p.setRasterOp( TQt::NotROP );
    p.setPen( TQt::DotLine );
    if ( ! _lastPoint.isNull() ) {
        p.drawRect(TQRect(_start, _lastPoint));
    }
    _top->validateSelection();
    _top->updateAll();
    emit anythingSelected( hasSelection() );
    if ( hasSelection() ) {
        emit verifyRegExp();
    }
}

bool RegExpEditorWindow::selectionOverlap( TQPoint pos, TQSize size ) const
{
    TQRect child(pos, size);

    return (_selection.intersects(child) && ! child.contains(_selection));
}

bool RegExpEditorWindow::hasSelection() const
{
    return _top->hasSelection();
}

void RegExpEditorWindow::clearSelection( bool update )
{
    _top->clearSelection();
    if ( update )
        _top->updateAll();
    emit anythingSelected(false);
}

void RegExpEditorWindow::slotInsertRegExp( RegExpType type )
{
    _insertInAction = true;
    _insertTp = type;

    updateCursorUnderPoint();
    setFocus();
}

void RegExpEditorWindow::slotInsertRegExp( RegExp* regexp )
{
    if ( _pasteData )
        delete _pasteData;

    _pasteData = regexp->clone();
    _pasteInAction = true;
    updateCursorUnderPoint();
    setFocus();
}

void RegExpEditorWindow::slotDoSelect()
{
    _pasteInAction = false;
    _insertInAction = false;

    // I need to update the cursor recursively, as a tqrepaint may not have been issued yet
    // when this method is invoked. This means that when the tqrepaint comes, the cursor may
    // move to an other widget.
    _top->updateCursorRecursively();
}

void RegExpEditorWindow::slotDeleteSelection()
{
    if ( ! hasSelection() ) {
        KMessageBox::information(this, i18n( "There is no selection."), i18n("Missing Selection") );
    }
    else {
        _top->deleteSelection();
    }
    updateContent( 0 );
}

void RegExpEditorWindow::updateContent( TQWidget* focusChild)
{
    TQPoint p(0,0);
    if ( focusChild )
        p = focusChild->mapTo( this, TQPoint(0,0) );

    _top->update();
    emit contentChanged( p );
}

TQSize RegExpEditorWindow::tqsizeHint() const
{
    return _top->tqsizeHint();
}

void RegExpEditorWindow::paintEvent( TQPaintEvent* event )
{
    TQWidget::paintEvent( event );
    _undrawSelection = false;
}

void RegExpEditorWindow::slotCut()
{
    cut( TQCursor::pos() );
    emit change();
    emit canSave( _top->hasAnyChildren() );
}

void RegExpEditorWindow::cut( TQPoint pos )
{
    cutCopyAux( pos );
    slotDeleteSelection();
}

void RegExpEditorWindow::slotCopy()
{
    copy( TQCursor::pos() );
}

void RegExpEditorWindow::copy( TQPoint pos )
{
    cutCopyAux( pos );
    clearSelection( true );
}


void RegExpEditorWindow::cutCopyAux( TQPoint pos )
{
    if ( !hasSelection() ) {
        RegExpWidget* widget = _top->widgetUnderPoint( pos, true );
        if ( !widget ) {
            KMessageBox::information(this, i18n("There is no widget under cursor."), i18n("Invalid Operation") );
            return;
        }
        else {
            widget->updateSelection( true ); // HACK!
        }
    }

    RegExp* regexp = _top->selection();
    RegExpWidgetDrag *clipboardData = new RegExpWidgetDrag( regexp, this );
    delete regexp;

    TQClipboard* clipboard = tqApp->tqclipboard();
    clipboard->setData( clipboardData );
    emit anythingOnClipboard( true );
    emit canSave( _top->hasAnyChildren() );
}


void RegExpEditorWindow::slotStartPasteAction()
{
    TQByteArray data = tqApp->tqclipboard()->data()->tqencodedData( "KRegExpEditor/widgetdrag" );
    TQTextStream stream( data, IO_ReadOnly );
    TQString str = stream.read();

    RegExp* regexp = WidgetFactory::createRegExp( str );
    if ( regexp )
        slotInsertRegExp( regexp );
}

void RegExpEditorWindow::slotEndActions() {
    emit doneEditing();
    emit change();
    emit canSave( _top->hasAnyChildren() );
}

void RegExpEditorWindow::showRMBMenu( bool enableCutCopy )
{
    enum CHOICES { CUT, COPY, PASTE, SAVE, EDIT };

    if ( !_menu ) {
        _menu = new TQPopupMenu( 0 );
        _menu->insertItem(getIcon(TQString::fromLocal8Bit("editcut")),
                          i18n("C&ut"), CUT);
        _menu->insertItem(getIcon(TQString::fromLocal8Bit("editcopy")),
                          i18n("&Copy"), COPY);
        _menu->insertItem(getIcon(TQString::fromLocal8Bit("editpaste")),
                          i18n("&Paste"), PASTE);
        _menu->insertSeparator();
        _menu->insertItem(getIcon(TQString::fromLocal8Bit("edit")),
                          i18n("&Edit"), EDIT);
        _menu->insertItem(getIcon(TQString::fromLocal8Bit("filesave")),
                          i18n("&Save Regular Expression..."), SAVE);
    }

    _menu->setItemEnabled( CUT, enableCutCopy );
    _menu->setItemEnabled( COPY, enableCutCopy );

    if ( ! tqApp->tqclipboard()->data()->provides( "KRegExpEditor/widgetdrag" ) )
        _menu->setItemEnabled( PASTE, false );
    else
        _menu->setItemEnabled( PASTE, true );

    _menu->setItemEnabled( SAVE, _top->hasAnyChildren() );

    RegExpWidget* editWidget = _top->findWidgetToEdit( TQCursor::pos() );

    _menu->setItemEnabled( EDIT, editWidget  );

    TQPoint pos = TQCursor::pos();
    int choice = _menu->exec( pos );
    switch ( choice ) {
    case COPY: copy( pos ); break;
    case CUT: cut( pos ); break;
    case PASTE: slotStartPasteAction(); break;
    case SAVE: slotSave(); break;
    case EDIT: editWidget->edit(); break;
    }
    emit change();
    emit canSave( _top->hasAnyChildren() );
}

void RegExpEditorWindow::applyRegExpToSelection( RegExpType tp )
{
    _top->applyRegExpToSelection( tp );
}

void RegExpEditorWindow::slotSave()
{
    TQString dir = WidgetWinItem::path();
    TQString txt;

#ifdef TQT_ONLY
    txt = TQInputDialog::getText( tr("Name for regexp"), tr("Enter name:") );
    if ( txt.isNull() )
        return;
#else
    KLineEditDlg dlg(i18n("Enter name:"), TQString(), this);
    dlg.setCaption(i18n("Name for Regular Expression"));
    if (!dlg.exec()) return;
    txt = dlg.text();
#endif

    TQString fileName = dir + TQString::fromLocal8Bit("/") + txt + TQString::fromLocal8Bit(".regexp");
    TQFileInfo finfo( fileName );
    if ( finfo.exists() ) {
        int answer = KMessageBox::warningContinueCancel( this, i18n("<p>Overwrite named regular expression <b>%1</b></p>").tqarg(txt), TQString(), i18n("Overwrite"));
        if ( answer != KMessageBox::Continue )
            return;
    }

    TQFile file( fileName );
    if ( ! file.open(IO_WriteOnly) ) {
        KMessageBox::sorry( this, i18n("Could not open file for writing: %1").tqarg(fileName) );
        return;
    }

    // Convert to XML.
    RegExp* regexp = _top->regExp();
    TQString xml = regexp->toXmlString();
    delete regexp;

    TQTextStream stream(&file);
    stream << xml;

    file.close();
    emit savedRegexp();
}


void RegExpEditorWindow::slotSetRegExp( RegExp* regexp )
{
    // I have no clue why the following line is necesarry, but if it is not here
    // then the editor area is messed up when calling slotSetRegExp before starting the eventloop.
    tqApp->processEvents();

    delete _top;
    RegExpWidget* widget = WidgetFactory::createWidget( regexp, this, this );
    if ( ! (_top = dynamic_cast<ConcWidget*>( widget ) ) ) {
        // It was not a ConcWidget
        _top = new ConcWidget( this, widget, this );
    }
    _top->setToplevel();

    _top->show();
    _layout->addWidget( _top );
    clearSelection( true ); // HACK?
    emit canSave( _top->hasAnyChildren() );
}

void RegExpEditorWindow::updateCursorUnderPoint()
{
    RegExpWidget* widget = _top->widgetUnderPoint( TQCursor::pos(), false );
    if ( widget )
        widget->updatetqCursorShape();
}

void RegExpEditorWindow::emitVerifyRegExp()
{
    emit verifyRegExp();
}


TQIconSet RegExpEditorWindow::getIcon( const TQString& name )
{
#ifdef TQT_ONLY
    TQPixmap pix;
    pix.convertFromImage( qembed_findImage( name ) );
    return pix;
#else
        return SmallIconSet( name );
#endif
}

