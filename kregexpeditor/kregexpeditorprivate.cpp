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
  #include <kiconloader.h>
  #include <kstandarddirs.h>
  #include <kmessagebox.h>
  #include "kregexpeditorprivate.moc"
#endif

#include <tqlineedit.h>
#include <tqtooltip.h>
#include <tqtoolbutton.h>
#include "kregexpeditorprivate.h"
#include "scrollededitorwindow.h"
#include "regexpbuttons.h"
//#include <unistd.h> // What do I need this for?
#include <stdio.h>
#include "infopage.h"
#include <tqsplitter.h>
#include <tqdockarea.h>
#include "userdefinedregexps.h"
#include "auxbuttons.h"
#include <tqaccel.h>
#include <tqtimer.h>
#include "verifier.h"
#include <tqfile.h>
#include "verifybuttons.h"
#include <tqwhatsthis.h>

KRegExpEditorPrivate::KRegExpEditorPrivate(TQWidget *parent, const char *name)
    : TQWidget(parent, name), _updating( false ), _autoVerify( true )
{
  setMinimumSize(730,300);
  TQDockArea* area = new TQDockArea(Qt::Horizontal, TQDockArea::Normal, this );
  area->setMinimumSize(2,2);
  TQDockArea* verArea1 = new TQDockArea(Qt::Vertical, TQDockArea::Normal, this );
  verArea1->setMinimumSize(2,2);
  TQDockArea* verArea2 = new TQDockArea(Qt::Vertical, TQDockArea::Reverse, this );
  verArea2->setMinimumSize(2,2);

  // The DockWindows.
  _regExpButtons = new RegExpButtons( area, "KRegExpEditorPrivate::regExpButton" );
  _verifyButtons = new VerifyButtons( area, "KRegExpEditorPrivate::VerifyButtons" );
  _auxButtons = new AuxButtons( area, "KRegExpEditorPrivate::AuxButtons" );
  _userRegExps = new UserDefinedRegExps( verArea1, "KRegExpEditorPrivate::userRegExps" );
  _userRegExps->setResizeEnabled( true );
  TQWhatsThis::add( _userRegExps, i18n( "In this window you will find predefined regular expressions. Both regular expressions "
                                       "you have developed and saved, and regular expressions shipped with the system." ));

  // Editor window
  _editor = new TQSplitter(Qt::Vertical, this, "KRegExpEditorPrivate::_editor" );

  _scrolledEditorWindow =
    new RegExpScrolledEditorWindow( _editor, "KRegExpEditorPrivate::_scrolledEditorWindow" );
  TQWhatsThis::add( _scrolledEditorWindow, i18n( "In this window you will develop your regular expressions. "
                                               "Select one of the actions from the action buttons above, and click the mouse in this "
                                               "window to insert the given action."));

  _info = new InfoPage( this, "_info" );
  _verifier = new Verifier( _editor, "KRegExpEditorPrivate::_verifier" );
  connect( _verifier, TQT_SIGNAL( textChanged() ), this, TQT_SLOT( maybeVerify() ) );
  TQWhatsThis::add( _verifier, i18n("Type in some text in this window, and see what the regular expression you have developed matches.<p>"
                                   "Each second match will be colored in red and each other match will be colored blue, simply so you "
                                   "can distinguish them from each other.<p>"
                                   "If you select part of the regular expression in the editor window, then this part will be "
                                   "highlighted - This allows you to <i>debug</i> your regular expressions") );

  _editor->hide();
  _editor->setSizes( TQValueList<int>() << _editor->height()/2 << _editor->height()/2 );

  TQVBoxLayout *topLayout = new TQVBoxLayout( this, 0, 6, "KRegExpEditorPrivate::topLayout" );
  topLayout->addWidget( area );
  TQHBoxLayout* rows = new TQHBoxLayout; // I need to cal addLayout explicit to get stretching right.
  topLayout->addLayout( rows, 1 );

  rows->addWidget( verArea1 );
  rows->addWidget( _editor, 1 );
  rows->addWidget( _info, 1 );
  rows->addWidget( verArea2 );

  // Connect the buttons
  connect( _regExpButtons, TQT_SIGNAL( clicked( int ) ),   _scrolledEditorWindow, TQT_SLOT( slotInsertRegExp( int ) ) );
  connect( _regExpButtons, TQT_SIGNAL( doSelect() ), _scrolledEditorWindow, TQT_SLOT( slotDoSelect() ) );
  connect( _userRegExps, TQT_SIGNAL( load( RegExp* ) ),    _scrolledEditorWindow, TQT_SLOT( slotInsertRegExp( RegExp*  ) ) );

  connect( _regExpButtons, TQT_SIGNAL( clicked( int ) ), _userRegExps,   TQT_SLOT( slotUnSelect() ) );
  connect( _regExpButtons, TQT_SIGNAL( doSelect() ),     _userRegExps,   TQT_SLOT( slotUnSelect() ) );
  connect( _userRegExps, TQT_SIGNAL( load( RegExp* ) ),  _regExpButtons, TQT_SLOT( slotUnSelect() ) );

  connect( _scrolledEditorWindow, TQT_SIGNAL( doneEditing() ), _regExpButtons, TQT_SLOT( slotSelectNewAction() ) );
  connect( _scrolledEditorWindow, TQT_SIGNAL( doneEditing() ), _userRegExps, TQT_SLOT( slotSelectNewAction() ) );

  connect( _regExpButtons, TQT_SIGNAL( clicked( int ) ), this, TQT_SLOT( slotShowEditor() ) );
  connect( _userRegExps, TQT_SIGNAL( load( RegExp* ) ), this, TQT_SLOT( slotShowEditor() ) );
  connect( _regExpButtons, TQT_SIGNAL( doSelect() ), this, TQT_SLOT( slotShowEditor() ) );

  connect( _scrolledEditorWindow, TQT_SIGNAL( savedRegexp() ), _userRegExps, TQT_SLOT( slotPopulateUserRegexps() ) );

  connect( _auxButtons, TQT_SIGNAL( undo() ), this, TQT_SLOT( slotUndo() ) );
  connect( _auxButtons, TQT_SIGNAL( redo() ), this, TQT_SLOT( slotRedo() ) );
  connect( _auxButtons, TQT_SIGNAL( cut() ), _scrolledEditorWindow, TQT_SLOT( slotCut() ) );
  connect( _auxButtons, TQT_SIGNAL( copy() ), _scrolledEditorWindow, TQT_SLOT( slotCopy() ) );
  connect( _auxButtons, TQT_SIGNAL( paste() ), _scrolledEditorWindow, TQT_SLOT( slotPaste() ) );
  connect( _auxButtons, TQT_SIGNAL( save() ), _scrolledEditorWindow, TQT_SLOT( slotSave() ) );
  connect( _verifyButtons, TQT_SIGNAL( autoVerify( bool ) ), this, TQT_SLOT( setAutoVerify( bool ) ) );
  connect( _verifyButtons, TQT_SIGNAL( verify() ), this, TQT_SLOT( doVerify() ) );
  connect( _verifyButtons, TQT_SIGNAL( changeSyntax( const TQString& ) ), this, TQT_SLOT( setSyntax( const TQString& ) ) );

  connect( this, TQT_SIGNAL( canUndo( bool ) ), _auxButtons, TQT_SLOT( slotCanUndo( bool ) ) );
  connect( this, TQT_SIGNAL( canRedo( bool ) ), _auxButtons, TQT_SLOT( slotCanRedo( bool ) ) );
  connect( _scrolledEditorWindow, TQT_SIGNAL( anythingSelected( bool ) ), _auxButtons, TQT_SLOT( slotCanCut( bool ) ) );
  connect( _scrolledEditorWindow, TQT_SIGNAL( anythingSelected( bool ) ), _auxButtons, TQT_SLOT( slotCanCopy( bool ) ) );
  connect( _scrolledEditorWindow, TQT_SIGNAL( anythingOnClipboard( bool ) ), _auxButtons, TQT_SLOT( slotCanPaste( bool ) ) );
  connect( _scrolledEditorWindow, TQT_SIGNAL( canSave( bool ) ), _auxButtons, TQT_SLOT( slotCanSave( bool ) ) );

  connect( _scrolledEditorWindow, TQT_SIGNAL( verifyRegExp() ), this, TQT_SLOT( maybeVerify() ) );

  connect( _verifyButtons, TQT_SIGNAL( loadVerifyText( const TQString& ) ), this, TQT_SLOT( setVerifyText( const TQString& ) ) );

  // connect( _verifier, TQT_SIGNAL( countChanged( int ) ), _verifyButtons, TQT_SLOT( setMatchCount( int ) ) );

  // TQt anchors do not work for <pre>...</pre>, thefore scrolling to next/prev match
  // do not work. Enable this when they work.
  // connect( _verifyButtons, TQT_SIGNAL( gotoFirst() ), _verifier, TQT_SLOT( gotoFirst() ) );
  // connect( _verifyButtons, TQT_SIGNAL( gotoPrev() ), _verifier, TQT_SLOT( gotoPrev() ) );
  // connect( _verifyButtons, TQT_SIGNAL( gotoNext() ), _verifier, TQT_SLOT( gotoNext() ) );
  // connect( _verifyButtons, TQT_SIGNAL( gotoLast() ), _verifier, TQT_SLOT( gotoLast() ) );
  // connect( _verifier, TQT_SIGNAL( goForwardPossible( bool ) ), _verifyButtons, TQT_SLOT( enableForwardButtons( bool ) ) );
  // connect( _verifier, TQT_SIGNAL( goBackwardPossible( bool ) ), _verifyButtons, TQT_SLOT( enableBackwardButtons( bool ) ) );

  _auxButtons->slotCanPaste( false );
  _auxButtons->slotCanCut( false );
  _auxButtons->slotCanCopy( false );
  _auxButtons->slotCanSave( false );


  // Line Edit
  TQHBoxLayout* tqlayout = new TQHBoxLayout( topLayout, 6 );
  TQLabel* label = new TQLabel( i18n("ASCII syntax:"), this );
  tqlayout->addWidget( label );
  clearButton = new TQToolButton( this );
  const TQString icon( TQString::fromLatin1( TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase" ) );
  TQIconSet clearIcon = SmallIconSet( icon );
  clearButton->setIconSet( clearIcon );
  tqlayout->addWidget( clearButton );
  TQToolTip::add( clearButton, i18n("Clear expression") );
  _regexpEdit = new TQLineEdit( this );
  tqlayout->addWidget( _regexpEdit );
  TQWhatsThis::add( _regexpEdit, i18n( "This is the regular expression in ASCII syntax. You are likely only "
				      "to be interested in this if you are a programmer, and need to "
				      "develop a regular expression using TQRegExp.<p>"
                                      "You may develop your regular expression both by using the graphical "
				      "editor, and by typing the regular expression in this line edit.") );

#ifdef TQT_ONLY
  TQPixmap pix( "icons/error.png" );
#else
  TQPixmap pix = KGlobal::iconLoader()->loadIcon(locate("data", TQString::fromLatin1("kregexpeditor/pics/error.png") ), KIcon::Toolbar );
#endif
  _error = new TQLabel( this );
  _error->setPixmap( pix );
  tqlayout->addWidget( _error );
  _error->hide();

  _timer = new TQTimer(this);

  connect( _scrolledEditorWindow, TQT_SIGNAL( change() ), this, TQT_SLOT( slotUpdateLineEdit() ) );
  connect( _regexpEdit, TQT_SIGNAL(textChanged( const TQString& ) ), this, TQT_SLOT( slotTriggerUpdate() ) );
  connect( _timer, TQT_SIGNAL( timeout() ), this, TQT_SLOT( slotTimeout() ) );
  connect( clearButton, TQT_SIGNAL( clicked() ), _regexpEdit, TQT_SLOT( clear() ) );

  // Push an initial empty element on the stack.
  _undoStack.push( _scrolledEditorWindow->regExp() );
  _redoStack.setAutoDelete( true );

  TQAccel* accel = new TQAccel( this );
  accel->connectItem( accel->insertItem( CTRL+Key_Z ), this, TQT_SLOT( slotUndo() ) );
  accel->connectItem( accel->insertItem( CTRL+Key_R ), this, TQT_SLOT( slotRedo() ) );

  setSyntax( TQString::fromLatin1( "TQt" ) );
}

TQString KRegExpEditorPrivate::regexp()
{
  RegExp* regexp = _scrolledEditorWindow->regExp();
  TQString res = RegExpConverter::current()->toStr( regexp, false );
  delete regexp;
  return res;
}

void KRegExpEditorPrivate::slotUpdateEditor( const TQString & txt)
{
  _updating = true;
  bool ok;
  if ( !RegExpConverter::current()->canParse() ) {
      // This can happend if the application set a text through the API.
  }
  else {
      RegExp* result = RegExpConverter::current()->parse( txt, &ok );
      if ( ok ) {
          TQPtrList<CompoundRegExp> list = _userRegExps->regExps();
          for ( TQPtrListIterator<CompoundRegExp> it( list ); *it; ++it ) {
              result->replacePart( *it );
          }

          _scrolledEditorWindow->slotSetRegExp( result );
          _error->hide();
          maybeVerify( );
          recordUndoInfo();
          result->check( _errorMap );
      }
      else {
          _error->show();
          if ( _autoVerify )
              _verifier->clearRegexp();
      }
      delete result;
  }
  _updating = false;
}

void KRegExpEditorPrivate::slotUpdateLineEdit()
{
  if ( _updating )
    return;
  _updating = true;

  RegExp* regexp = _scrolledEditorWindow->regExp();
  regexp->check( _errorMap );

  TQString str = RegExpConverter::current()->toStr( regexp, false );
  _regexpEdit->setText( str );
  delete regexp;

  recordUndoInfo();

  _updating = false;
}

void KRegExpEditorPrivate::recordUndoInfo()
{
  Q_ASSERT( _updating );

  // Update undo/redo stacks
  RegExp* regexp = _scrolledEditorWindow->regExp();
  if ( regexp->toXmlString() != _undoStack.top()->toXmlString() ) {
    _undoStack.push( regexp );
    _redoStack = TQPtrStack<RegExp>();
    emitUndoRedoSignals();
  }
}

void KRegExpEditorPrivate::slotRedo()
{
  if ( _redoStack.count() != 0 ) {
    _undoStack.push(_redoStack.pop());
    _scrolledEditorWindow->slotSetRegExp( _undoStack.top() );
    slotUpdateLineEdit();
    emitUndoRedoSignals();
    maybeVerify();
  }
}

void KRegExpEditorPrivate::slotUndo()
{
  if ( _undoStack.count() > 1 ) {
    _redoStack.push(_undoStack.pop());
    _scrolledEditorWindow->slotSetRegExp( _undoStack.top() );
    slotUpdateLineEdit();
    emitUndoRedoSignals();
    maybeVerify();
  }
}

void KRegExpEditorPrivate::slotShowEditor()
{
    _info->hide();
    _editor->show();
}

void KRegExpEditorPrivate::emitUndoRedoSignals()
{
  emit canUndo( _undoStack.count() > 1 );
  emit changes( _undoStack.count() > 1 );
  emit canRedo( _redoStack.count() > 0 );
}

void KRegExpEditorPrivate::slotSetRegexp( TQString regexp )
{
  _regexpEdit->setText( regexp );
}

void KRegExpEditorPrivate::slotTriggerUpdate()
{
  /* ### Guess this timeout value should be configurable somewhere, or (even
   * better: do some kind of benchmark each time the editor view gets updated
   * to measure how long it takes on the client system to render the editor
   * with the current complexity. That way we'd get good response times for
   * simple regexps, and flicker-free display for complex regexps.
   * - Frerich
   */
  if ( !_updating ) {
    _timer->start( 300, true );
    slotShowEditor();
  }
}

void KRegExpEditorPrivate::slotTimeout()
{
  slotUpdateEditor( _regexpEdit->text() );
}

void KRegExpEditorPrivate::setMatchText( const TQString& text )
{
    bool autoVerify = _autoVerify;
    _autoVerify = false;
    _verifier->setText( text );
    _autoVerify = autoVerify;
}

void KRegExpEditorPrivate::maybeVerify()
{
    if ( _autoVerify )
        doVerify();
    else
        _verifyButtons->setMatchCount(-1);
}

void KRegExpEditorPrivate::doVerify()
{
    bool autoVerify = _autoVerify; // prevent loop due to verify emit changed, which calls maybeVerify
    _autoVerify = false;
    RegExp* regexp = _scrolledEditorWindow->regExp();

    _verifier->verify( RegExpConverter::current()->toStr( regexp, true ) );
    delete regexp;
    _autoVerify = autoVerify;
}

void KRegExpEditorPrivate::setAutoVerify( bool on )
{
    _autoVerify = on;
    if ( !_autoVerify )
        _verifier->clearRegexp();
    else
        doVerify();
}

void KRegExpEditorPrivate::setVerifyText( const TQString& fileName )
{
    bool autoVerify = _autoVerify;
    _autoVerify = false;
    TQFile file( fileName );
    if ( !file.open( IO_ReadOnly ) ) {
        KMessageBox::sorry(0, i18n("Could not open file '%1' for reading").arg( fileName ) );
    }
    else {
        TQTextStream s( &file );
        TQString txt = s.read();
        file.close();
        RegExp* regexp = _scrolledEditorWindow->regExp();
        _verifier->setText( txt );
        _verifier->verify( RegExpConverter::current()->toStr( regexp, true ) );
        delete regexp;
    }
    _autoVerify = autoVerify;
}

void KRegExpEditorPrivate::setCaseSensitive( bool b )
{
    _verifier->setCaseSensitive( b );
}

void KRegExpEditorPrivate::setMinimal( bool b )
{
    _verifier->setMinimal( b );
}

void KRegExpEditorPrivate::setSyntax( const TQString& syntax )
{
    RegExpConverter* converter = _verifyButtons->setSyntax( syntax );
    RegExpConverter::setCurrent( converter );
    if ( converter->canParse() ) {
        _regexpEdit->setReadOnly( false );
        _regexpEdit->setBackgroundMode( TQt::PaletteBase );
    }
    else {
        _regexpEdit->setReadOnly( true );
        _regexpEdit->setBackgroundMode( TQt::PaletteBackground );
    }
    _regExpButtons->setFeatures( converter->features() );
    _verifier->setHighlighter( converter->highlighter(_verifier) );
    slotUpdateLineEdit();
}

void KRegExpEditorPrivate::showHelp()
{
    _info->show();
    _editor->hide();
}

void KRegExpEditorPrivate::setAllowNonTQtSyntax( bool b )
{
    _verifyButtons->setAllowNonTQtSyntax( b );
}
