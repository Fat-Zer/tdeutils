  /*

    $Id$

    Copyright (C) 1997 Bernd Johannes Wuebben
                       wuebben@math.cornell.edu

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

    */

#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>

#include <tqlayout.h>
#include <tqtimer.h>
#include <tqpaintdevicemetrics.h>
#include <tqpainter.h>

#include <kaboutdata.h>
#include <tdeaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <tdeconfigdialog.h>
#include <kcursor.h>
#include <keditcl.h>
#include <tdefileitem.h>
#include <tdefontdialog.h>
#include <tdeio/netaccess.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <ksavefile.h>
#include <kstatusbar.h>
#include <tdespell.h>
#include <kurldrag.h>

#include "ktextfiledlg.h"
#include "kedit.h"
#include "misc.h"
#include "color.h"
#include "prefs.h"

#include <kdebug.h>


TQPtrList<TopLevel> *TopLevel::windowList = 0;

int default_open =  TopLevel::OPEN_READWRITE;

TopLevel::TopLevel (TQWidget *, const char *name)
  : TDEMainWindow ( 0,name ), tdespellconfigOptions(0),
  eframe(0), newWindow(false), tdespell(0)
{
  if (!windowList)
  {
     windowList = new TQPtrList<TopLevel>;
     windowList->setAutoDelete( FALSE );
  }
  windowList->append( this );

  statusbar_timer = new TQTimer(this);
  connect(statusbar_timer, TQT_SIGNAL(timeout()),this,TQT_SLOT(timer_slot()));

  connect(kapp,TQT_SIGNAL(kdisplayPaletteChanged()),this,TQT_SLOT(set_colors()));

  setupStatusBar();
  setupActions();

  readSettings();

  setupEditWidget();

  if (!initialGeometrySet())
    resize( TQSize(550, 400).expandedTo(minimumSizeHint()));
  setupGUI(ToolBar | Keys | StatusBar | Create);
  setAutoSaveSettings();

  setAcceptDrops(true);

  setFileCaption();
}


TopLevel::~TopLevel ()
{
  windowList->remove( this );
}


void TopLevel::setupEditWidget()
{
  if (!eframe)
  {
    eframe = new KEdit (this, "eframe");
    eframe->setOverwriteEnabled(true);

    connect(eframe, TQT_SIGNAL(CursorPositionChanged()),this,
           TQT_SLOT(statusbar_slot()));
    connect(eframe, TQT_SIGNAL(toggle_overwrite_signal()),this,
           TQT_SLOT(toggle_overwrite()));
    connect(eframe, TQT_SIGNAL(gotUrlDrop(TQDropEvent*)), this,
           TQT_SLOT(urlDrop_slot(TQDropEvent*)));
    connect(eframe, TQT_SIGNAL(undoAvailable(bool)),undoAction,
           TQT_SLOT(setEnabled(bool)));
    connect(eframe, TQT_SIGNAL(redoAvailable(bool)),redoAction,
           TQT_SLOT(setEnabled(bool)));
    connect(eframe, TQT_SIGNAL(copyAvailable(bool)),cutAction,
           TQT_SLOT(setEnabled(bool)));
    connect(eframe, TQT_SIGNAL(copyAvailable(bool)),copyAction,
           TQT_SLOT(setEnabled(bool)));
    connect( eframe, TQT_SIGNAL(selectionChanged()),this,
           TQT_SLOT(slotSelectionChanged()));
    connect( eframe, TQT_SIGNAL(modificationChanged( bool)),
           TQT_SLOT(setFileCaption()));

    undoAction->setEnabled(false);
    redoAction->setEnabled(false);
    cutAction->setEnabled(false);
    copyAction->setEnabled(false);

    setCentralWidget(eframe);
    eframe->setMinimumSize(200, 100);
  }

  if( Prefs::wrapMode() == Prefs::EnumWrapMode::FixedColumnWrap )
  {
    eframe->setWordWrap(TQMultiLineEdit::FixedColumnWidth);
    eframe->setWrapColumnOrWidth(Prefs::wrapColumn());
  }
  else if( Prefs::wrapMode() == Prefs::EnumWrapMode::SoftWrap )
  {
    eframe->setWordWrap(TQMultiLineEdit::WidgetWidth);
  }
  else
  {
    eframe->setWordWrap(TQMultiLineEdit::NoWrap);
  }

  eframe->setFont( Prefs::font() );
  int w = eframe->fontMetrics().width("M");
  eframe->setTabStopWidth(8*w);

  eframe->setModified(false);

  setSensitivity();

  eframe->setFocus();

  set_colors();

  /*
  right_mouse_button = new TQPopupMenu;

  right_mouse_button->insertItem (i18n("Open..."),
				  this, 	TQT_SLOT(file_open()));
  right_mouse_button->insertItem (SmallIcon("filesave"),i18n("Save"),
				  this, 	TQT_SLOT(file_save()));
  right_mouse_button->insertItem (SmallIcon("filesaveas"),i18n("Save As..."),
				  this, TQT_SLOT(file_save_as()));
  right_mouse_button->insertSeparator(-1);
  right_mouse_button->insertItem(i18n("Copy"),
				 this, TQT_SLOT(copy()));
  right_mouse_button->insertItem(i18n("Paste"),
				 this, TQT_SLOT(paste()));
  right_mouse_button->insertItem(i18n("Cut"),
				 this, TQT_SLOT(cut()));
  right_mouse_button->insertItem(i18n("Select All"),
				 this, TQT_SLOT(select_all()));
  eframe->installRBPopup(right_mouse_button);
  */
}


void TopLevel::slotSelectionChanged()
{
  bool state = eframe->hasSelectedText();
  cutAction->setEnabled( state );
  copyAction->setEnabled( state );
}


void TopLevel::setupActions()
{
    // setup File menu
    KStdAction::openNew(TQT_TQOBJECT(this), TQT_SLOT(file_new()), actionCollection());
    KStdAction::open(TQT_TQOBJECT(this), TQT_SLOT(file_open()), actionCollection());
    recent = KStdAction::openRecent(TQT_TQOBJECT(this), TQT_SLOT(openRecent(const KURL&)),
                                          actionCollection());
    KStdAction::save(TQT_TQOBJECT(this), TQT_SLOT(file_save()), actionCollection());
    KStdAction::saveAs(TQT_TQOBJECT(this), TQT_SLOT(file_save_as()), actionCollection());
    KStdAction::close(TQT_TQOBJECT(this), TQT_SLOT(file_close()), actionCollection());
    KStdAction::print(TQT_TQOBJECT(this), TQT_SLOT(print()), actionCollection());
    KStdAction::mail(TQT_TQOBJECT(this), TQT_SLOT(mail()), actionCollection());
    KStdAction::quit(TQT_TQOBJECT(this), TQT_SLOT(close()), actionCollection());

    // setup edit menu
    undoAction = KStdAction::undo(TQT_TQOBJECT(this), TQT_SLOT(undo()), actionCollection());
    redoAction = KStdAction::redo(TQT_TQOBJECT(this), TQT_SLOT(redo()), actionCollection());
    cutAction = KStdAction::cut(TQT_TQOBJECT(this), TQT_SLOT(cut()), actionCollection());
    copyAction = KStdAction::copy(TQT_TQOBJECT(this), TQT_SLOT(copy()), actionCollection());
    KStdAction::pasteText(TQT_TQOBJECT(this), TQT_SLOT(paste()), actionCollection());
    KStdAction::selectAll(TQT_TQOBJECT(this), TQT_SLOT(select_all()), actionCollection());
    KStdAction::find(TQT_TQOBJECT(this), TQT_SLOT(search()), actionCollection());
    KStdAction::findNext(TQT_TQOBJECT(this), TQT_SLOT(search_again()), actionCollection());
    KStdAction::replace(TQT_TQOBJECT(this), TQT_SLOT(replace()), actionCollection());

    (void)new TDEAction(i18n("&Insert File..."), 0, TQT_TQOBJECT(this), TQT_SLOT(file_insert()),
                      actionCollection(), "insert_file");
    (void)new TDEAction(i18n("In&sert Date"), 0, TQT_TQOBJECT(this), TQT_SLOT(insertDate()),
                      actionCollection(), "insert_date");
    (void)new TDEAction(i18n("Cl&ean Spaces"), 0, TQT_TQOBJECT(this), TQT_SLOT(clean_space()),
                      actionCollection(), "clean_spaces");

    // setup Tools menu
    KStdAction::spelling(TQT_TQOBJECT(this), TQT_SLOT(spellcheck()), actionCollection());

    // setup Go menu
    KStdAction::gotoLine(TQT_TQOBJECT(this), TQT_SLOT(gotoLine()), actionCollection());

    KStdAction::preferences(TQT_TQOBJECT(this), TQT_SLOT(showSettings()), actionCollection());
}

void TopLevel::setupStatusBar()
{
  statusBar()->insertItem("", ID_GENERAL, 10 );
  statusBar()->insertFixedItem( i18n("OVR"), ID_INS_OVR );
  statusBar()->insertFixedItem( i18n("Line:000000 Col: 000"), ID_LINE_COLUMN );

  statusBar()->setItemAlignment( ID_GENERAL, AlignLeft|AlignVCenter );
  statusBar()->setItemAlignment( ID_LINE_COLUMN, AlignLeft|AlignVCenter );
  statusBar()->setItemAlignment( ID_INS_OVR, AlignLeft|AlignVCenter );

  statusBar()->changeItem( i18n("Line: 1 Col: 1"), ID_LINE_COLUMN );
  statusBar()->changeItem( i18n("INS"), ID_INS_OVR );
}


void TopLevel::saveProperties(TDEConfig* config)
{
  if(m_url.isEmpty() && !eframe->isModified())
    return;

  config->writeEntry("url",m_url.url());
  config->writeEntry("modified",eframe->isModified());

  int line, col;
  eframe->getCursorPosition(&line, &col);
  config->writeEntry("current_line", line);
  config->writeEntry("current_column", col);

  if(eframe->isModified())
  {
    TQString name = m_url.url();
    if (name.isEmpty())
       name = TQString("kedit%1-%2").arg(getpid()).arg((long)this);
    TQString tmplocation = kapp->tempSaveName(m_url.url());
    config->writeEntry("saved_to",tmplocation);
    saveFile(tmplocation, false, m_url.fileEncoding());
  }
}


void TopLevel::readProperties(TDEConfig* config){
    KURL url = config->readPathEntry("url");
    TQString filename = config->readPathEntry("saved_to");

    TQString encoding = url.fileEncoding();
    int modified = config->readNumEntry("modified",0);
    int line = config->readNumEntry("current_line", 0);
    int col = config->readNumEntry("current_column", 0);
    int result = KEDIT_RETRY;

    if(!filename.isEmpty())
    {
        if (modified)
        {
            result = openFile(filename, OPEN_READWRITE, url.fileEncoding());
        }
        else
        {
            result = openFile(filename, OPEN_READWRITE, url.fileEncoding());
        }
    }
    else
    {
        openURL(url, OPEN_READWRITE);
        modified = false;
        result = KEDIT_OK;
    }

    if (result == KEDIT_OK)
    {
        m_url = url;
        eframe->setModified(modified);
        eframe->setCursorPosition(line, col);
        setFileCaption();
        statusbar_slot();
    }
}


void TopLevel::undo()
{
  eframe->undo();
}


void TopLevel::redo()
{
  eframe->redo();
}


void TopLevel::copy()
{
  eframe->copy();
}


void TopLevel::select_all()
{
  eframe->selectAll();
}


void TopLevel::insertDate(){

  int line, column;

  TQString string;
  TQDate dt = TQDate::currentDate();
  string = TDEGlobal::locale()->formatDate(dt);

  eframe->getCursorPosition(&line,&column);
  eframe->insertAt(string,line,column);
  eframe->setModified(TRUE);

  statusbar_slot();
}

void TopLevel::paste(){

 eframe->paste();
 eframe->setModified(TRUE);

 statusbar_slot();
}


void TopLevel::cut(){

 eframe->cut();
 eframe->setModified(TRUE);
 statusbar_slot();

}


void TopLevel::file_new()
{
  TopLevel *t = new TopLevel ();
  t->show();
  return;
}

void TopLevel::clean_space()
{
   eframe->cleanWhiteSpace();
}

void TopLevel::spellcheck()
{
  if (!eframe) return;

  if (tdespell) return; // In progress

  statusBar()->changeItem(i18n("Spellcheck:  Started."), ID_GENERAL);

  initSpellConfig();
  tdespell = new KSpell(this, i18n("Spellcheck"), TQT_TQOBJECT(this),
	TQT_SLOT( spell_started(KSpell *)), tdespellconfigOptions);

  connect (tdespell, TQT_SIGNAL ( death()),
          this, TQT_SLOT ( spell_finished( )));

  connect (tdespell, TQT_SIGNAL (progress (unsigned int)),
          this, TQT_SLOT (spell_progress (unsigned int)));
  connect (tdespell, TQT_SIGNAL (misspelling (const TQString &, const TQStringList &, unsigned int)),
          eframe, TQT_SLOT (misspelling (const TQString &, const TQStringList &, unsigned int)));
  connect (tdespell, TQT_SIGNAL (corrected (const TQString &, const TQString &, unsigned int)),
          eframe, TQT_SLOT (corrected (const TQString &, const TQString &, unsigned int)));
  connect (tdespell, TQT_SIGNAL (done(const TQString&)),
		 this, TQT_SLOT (spell_done(const TQString&)));
}


void TopLevel::spell_started( KSpell *)
{
   eframe->spellcheck_start();
   tdespell->setProgressResolution(2);
   tdespell->check(eframe->text());
}


void TopLevel::spell_progress (unsigned int percent)
{
  TQString s;
  s = i18n("Spellcheck:  %1% complete").arg(percent);

  statusBar()->changeItem (s, ID_GENERAL);
}


void TopLevel::spell_done(const TQString& newtext)
{
  eframe->spellcheck_stop();
  if (tdespell->dlgResult() == 0)
  {
     eframe->setText( newtext);
     statusBar()->changeItem (i18n("Spellcheck:  Aborted."), ID_GENERAL);
  }
  else
  {
     statusBar()->changeItem (i18n("Spellcheck:  Complete."), ID_GENERAL);
  }
  tdespell->cleanUp();
}

// Replace ISpell with the name of the actual spell checker.
// TODO: Use %1 in the original string instead when string freeze is over.
TQString TopLevel::replaceISpell(TQString msg, int client)
{
  switch(client)
  {
    case KS_CLIENT_ISPELL: msg.replace("ISpell", "<b>ispell</b>"); break;
    case KS_CLIENT_ASPELL: msg.replace("ISpell", "<b>aspell</b>"); break;
    case KS_CLIENT_HSPELL: msg.replace("ISpell", "<b>hspell</b>"); break;
  }
  msg.replace("\n", "<p>");
  return "<qt>"+msg+"</qt>";
}

void TopLevel::spell_finished( )
{
  KSpell::spellStatus status = tdespell->status();
  int client = tdespellconfigOptions->client();
  delete tdespell;
  tdespell = 0;
  if (status == KSpell::Error)
  {
     KMessageBox::sorry(this, replaceISpell(i18n("ISpell could not be started.\n"
     "Please make sure you have ISpell properly configured and in your PATH."), client));
  }
  else if (status == KSpell::Crashed)
  {
     eframe->spellcheck_stop();
     statusBar()->changeItem (i18n("Spellcheck:  Crashed."), ID_GENERAL);
     KMessageBox::sorry(this, replaceISpell(i18n("ISpell seems to have crashed."), client));
  }
}


void TopLevel::file_open( void )
{

    while( 1 )
  {
    KURL url = KTextFileDialog::getOpenURLwithEncoding(
		TQString(), TQString(), this,
		i18n("Open File"));
    if( url.isEmpty() )
    {
      return;
    }

    TDEIO::UDSEntry entry;
    TDEIO::NetAccess::stat(url, entry, this);
    KFileItem fileInfo(entry, url);
    if (fileInfo.size() > 2097152 && // 2MB large/small enough?
        KMessageBox::warningContinueCancel(this,
                         i18n("The file you have requested is larger than KEdit is designed for. "
                              "Please ensure you have enough system resources available to safely load this file, "
                              "or consider using a program that is designed to handle large files such as KWrite."),
                        i18n("Attempting to Open Large File"),
                        KStdGuiItem::cont(), "attemptingToOpenLargeFile") == KMessageBox::Cancel)
    {
        return;
    }

    TopLevel *toplevel;
    if( !m_url.isEmpty() || eframe->isModified() )
    {
      toplevel = new TopLevel();
      if( toplevel == 0 )
      {
	return;
      }
    }
    else
    {
      toplevel = this;
    }

    TQString tmpfile;
    TDEIO::NetAccess::download( url, tmpfile, toplevel );

    int result = toplevel->openFile( tmpfile, 0, url.fileEncoding());
    TDEIO::NetAccess::removeTempFile( tmpfile );

    if( result == KEDIT_OK )
    {
      if( toplevel != this ) { toplevel->show(); }
      toplevel->m_url = url;
      toplevel->setFileCaption();
      recent->addURL( url );
      toplevel->eframe->setModified(false);
      toplevel->setGeneralStatusField(i18n("Done"));
      toplevel->statusbar_slot();
      break;
    }
    else if( result == KEDIT_RETRY )
    {
      if( toplevel != this ) { delete toplevel; }
    }
    else
    {
      if( toplevel != this ) { delete toplevel; }
      break;
    }
  }

}

void TopLevel::file_insert()
{
  while( 1 )
  {
    KURL url = KTextFileDialog::getOpenURLwithEncoding(
        TQString(), TQString(), this,
	i18n("Insert File"), "", KStdGuiItem::insert().text());
    if( url.isEmpty() )
    {
      return;
    }

    TQString tmpfile;
    TDEIO::NetAccess::download( url, tmpfile, this );
    int result = openFile( tmpfile, OPEN_INSERT, url.fileEncoding(), true );
    TDEIO::NetAccess::removeTempFile( tmpfile );

    if( result == KEDIT_OK )
    {
      recent->addURL( url );
      eframe->setModified(true);
      setGeneralStatusField(i18n("Done"));
      statusbar_slot();
    }
    else if( result == KEDIT_RETRY )
    {
      continue;
    }
    return;
  }
}

bool TopLevel::queryExit()
{
  // save recent files menu
  config = kapp->config();
  recent->saveEntries( config );
  config->sync();

  return true;
}

bool TopLevel::queryClose()
{
  queryExit();
  int result;

  if ( !eframe->isModified() )
     return true;

  TQString msg = i18n(""
        "This document has been modified.\n"
        "Would you like to save it?" );
  switch( KMessageBox::warningYesNoCancel( this, msg, TQString(),
                       KStdGuiItem::save(), KStdGuiItem::discard() ) )
  {
     case KMessageBox::Yes: // Save, then exit
              if ( m_url.isEmpty())
              {
                 file_save_as();
                 if ( eframe->isModified() )
                     return false; // Still modified? Don't exit!
              }
              else
              {
                 result = saveURL(m_url);
                 if ( result == KEDIT_USER_CANCEL )
                    return false; // Don't exit.

                 if ( result != KEDIT_OK)
                 {
                    msg = i18n(""
	                "Could not save the file.\n"
	                "Exit anyways?");
                    switch( KMessageBox::warningContinueCancel( this, msg, TQString(), KStdGuiItem::quit() ) )
                    {
                     case KMessageBox::Continue:
                              return true; // Exit.
                     case KMessageBox::Cancel:
                     default:
                              return false; // Don't exit.
                    }
                 }
              }

     case KMessageBox::No: // Don't save but exit.
          return true;

     case KMessageBox::Cancel: // Don't save and don't exit.
     default:
	  return false; // Don't exit...
  }

  return true; // Exit.
}


void TopLevel::openRecent(const KURL& url)
{
  if (!m_url.isEmpty() || eframe->isModified())
  {
     TopLevel *t = new TopLevel (0,0);
     t->show();
     t->openRecent(url);
     return;
  }
  openURL( url, OPEN_READWRITE );
}


void TopLevel::file_close()
{
  if( eframe->isModified() )
  {
    TQString msg = i18n("This document has been modified.\n"
                       "Would you like to save it?" );
    switch( KMessageBox::warningYesNoCancel( this, msg, TQString(),
                           KStdGuiItem::save(), KStdGuiItem::discard() ) )
    {
      case KMessageBox::Yes: // Save, then close
        file_save();
        if (eframe->isModified())
           return; // Error during save.
      break;

      case KMessageBox::No: // Don't save but close.
      break;

      case KMessageBox::Cancel: // Don't save and don't close.
	return;
      break;
    }
  }
  eframe->clear();
  eframe->setModified(false);
  m_url = KURL();
  setFileCaption();
  statusbar_slot();
}


void TopLevel::file_save()
{
  if (m_url.isEmpty())
  {
     file_save_as();
     return;
  }

  int result = KEDIT_OK;

  result =  saveURL(m_url); // error messages are handled by saveFile

  if ( result == KEDIT_OK ){
     TQString string;
     string = i18n("Wrote: %1").arg(m_caption);
     setGeneralStatusField(string);
  }
}


void TopLevel::setGeneralStatusField(const TQString &text){

    statusbar_timer->stop();

    statusBar()->changeItem(text,ID_GENERAL);
    statusbar_timer->start(10000,TRUE); // single shot

}


void TopLevel::file_save_as()
{
  KURL u;
  while(true)
  {
     u = KTextFileDialog::getSaveURLwithEncoding(
                 m_url.url(), TQString(), this,
                 i18n("Save File As"),
                 m_url.fileEncoding());

     if (u.isEmpty())
        return;

     if ( TDEIO::NetAccess::exists(u, false, this) )
     {
        int result = KMessageBox::warningContinueCancel( this,
           i18n( "A file named \"%1\" already exists. "
                 "Are you sure you want to overwrite it?" ).arg( u.prettyURL() ),
           i18n( "Overwrite File?" ),
           i18n( "Overwrite" ) );

        if (result != KMessageBox::Continue)
           continue;
     }
     break;
  }

  int result = saveURL(u); // error messages are handled by saveFile

  if ( result == KEDIT_OK )
    {
      m_url = u;
      setFileCaption();
      TQString string = i18n("Saved as: %1").arg(m_caption);
      setGeneralStatusField(string);
      recent->addURL( u );
    }
}



void TopLevel::mail()
{
  //
  // Default subject string
  //
  TQString defaultsubject = name();
  int index = defaultsubject.findRev('/');
  if( index != -1)
    defaultsubject = defaultsubject.right(defaultsubject.length() - index - 1 );

  kapp->invokeMailer( TQString(), TQString(), TQString(),
       defaultsubject, eframe->text() );
}

/*
void TopLevel::fancyprint(){

  TQPrinter prt;
  char buf[200];
  if ( prt.setup(0) ) {

    int y =10;
    TQPainter p;
    p.begin( &prt );
    p.setFont( eframe->font() );
    TQFontMetrics fm = p.fontMetrics();

    int numlines = eframe->numLines();
    for(int i = 0; i< numlines; i++){
      y += fm.ascent();
      TQString line;
      line = eframe->textLine(i);
      line.replace( TQRegExp("\t"), "        " );
      strncpy(buf,line.local8Bit(),160);
      for (int j = 0 ; j <150; j++){
	if (!isprint(buf[j]))
	    buf[j] = ' ';
      }
      buf[line.length()] = '\0';
      p.drawText( 10, y, buf );
      y += fm.descent();
    }

    p.end();
  }
  return ;
}
*/

void TopLevel::helpselected(){

  kapp->invokeHelp( );

}


void TopLevel::search(){

  eframe->search();
  statusbar_slot();
}

void TopLevel::replace(){
  eframe->replace();
  statusbar_slot();
}



void TopLevel::showSettings()
{
  if(TDEConfigDialog::showDialog("settings"))
    return;

  initSpellConfig();
  TDEConfigDialog* dialog = new SettingsDialog(this, "settings", Prefs::self(), tdespellconfigOptions);

  connect(dialog, TQT_SIGNAL(settingsChanged()), this, TQT_SLOT(updateSettings()));
  dialog->show();
}

void TopLevel::initSpellConfig()
{
  if (!tdespellconfigOptions)
     tdespellconfigOptions = new KSpellConfig(0 , "SpellingSettings", 0, false );
}

void TopLevel::search_again()
{
  eframe->repeatSearch();
  statusbar_slot();
}

void TopLevel::setFileCaption()
{
  if (m_url.isEmpty())
  {
     m_caption = i18n("[New Document]");
  }
  else
  {
     if (m_url.isLocalFile())
     {
         if (TQDir::currentDirPath() == m_url.directory())
            m_caption = m_url.fileName();
         else
            m_caption = m_url.path();
     }
     else
     {
         KURL url(m_url);
         url.setQuery(TQString());
         m_caption = url.prettyURL();
     }
     TQString encoding = m_url.fileEncoding();
     if (!encoding.isEmpty())
        m_caption += TQString(" (%1)").arg(encoding);
  }
  setCaption(m_caption, eframe->isModified());
}


void TopLevel::gotoLine() {
	eframe->doGotoLine();
}

void TopLevel::statusbar_slot(){

  TQString linenumber;

  linenumber = i18n("Line: %1 Col: %2")
		     .arg(eframe->currentLine() + 1)
		     .arg(eframe->currentColumn() +1);

  statusBar()->changeItem(linenumber,ID_LINE_COLUMN);
}

void TopLevel::print()
{
    bool aborted = false;
    TQString headerLeft = i18n("Date: %1").arg(TDEGlobal::locale()->formatDate(TQDate::currentDate(),true));
    TQString headerMid = i18n("File: %1").arg(m_caption);
    TQString headerRight;

    TQFont printFont = eframe->font();
    TQFont headerFont(printFont);
    headerFont.setBold(true);

    TQFontMetrics printFontMetrics(printFont);
    TQFontMetrics headerFontMetrics(headerFont);

    KPrinter *printer = new KPrinter;
    if(printer->setup(this, i18n("Print %1").arg(m_caption))) {
        // set up KPrinter
        printer->setFullPage(false);
        printer->setCreator("KEdit");
        if ( !m_caption.isEmpty() )
	    printer->setDocName(m_caption);

        TQPainter *p = new TQPainter;
        p->begin( printer );

        TQPaintDeviceMetrics metrics( printer );

        int dy = 0;

	p->setFont(headerFont);
        int w = printFontMetrics.width("M");
        p->setTabStops(8*w);

        int page = 1;
        int lineCount = 0;
        int maxLineCount = eframe->numLines();


        while(true) {
           headerRight = TQString("#%1").arg(page);
           dy = headerFontMetrics.lineSpacing();
           TQRect body( 0, dy*2,  metrics.width(), metrics.height()-dy*2);

           p->drawText(0, 0, metrics.width(), dy, TQt::AlignLeft, headerLeft);
           p->drawText(0, 0, metrics.width(), dy, TQt::AlignHCenter, headerMid);
           p->drawText(0, 0, metrics.width(), dy, TQt::AlignRight, headerRight);

           TQPen pen;
           pen.setWidth(3);
           p->setPen(pen);

           p->drawLine(0, dy+dy/2, metrics.width(), dy+dy/2);

           int y = dy*2;
           while(lineCount < maxLineCount) {
              TQString text = eframe->textLine(lineCount);
              if( text.isEmpty() )
                text = " ";	// don't ignore empty lines
              TQRect r = p->boundingRect(0, y, body.width(), body.height(),
			TQPainter::ExpandTabs | TQPainter::WordBreak, text);

              dy = r.height();

              if (y+dy > metrics.height()) break;

              p->drawText(0, y, metrics.width(), metrics.height() - y,
                        TQPainter::ExpandTabs | TQPainter::WordBreak, text);

              y += dy;
              lineCount++;
           }
           if (lineCount >= maxLineCount)
              break;

           printer->newPage();
           page++;
        }

        p->end();
        delete p;
    }
    delete printer;
    if (aborted)
      setGeneralStatusField(i18n("Printing aborted."));
    else
      setGeneralStatusField(i18n("Printing complete."));
}



void TopLevel::setSensitivity (){

}


int TopLevel::saveURL( const KURL& _url )
{
    if ( !_url.isValid() )
    {
        KMessageBox::sorry(this, i18n("Malformed URL"));
        return KEDIT_RETRY;
    }

    // Just a usual file ?
    if ( _url.isLocalFile() )
    {
        return saveFile( _url.path(), true, _url.fileEncoding() );
    }

    KTempFile tmpFile;
    tmpFile.setAutoDelete(true);
    eframe->setModified( true );
    saveFile( tmpFile.name(), false, _url.fileEncoding() );

    if (TDEIO::NetAccess::upload( tmpFile.name(), _url, this ) == false)
    {
      KMessageBox::error(this, "Could not save remote file");
      return KEDIT_RETRY;
    }

    return true;
}


int TopLevel::openFile( const TQString& _filename, int _mode, const TQString &encoding, bool _undoAction )
{
  TQFileInfo info(_filename);

  if(info.isDir())
  {
     KMessageBox::sorry(this, i18n("You have specified a folder"));
     return KEDIT_RETRY;
  }

  if(!info.exists() || !info.isFile())
  {
     if ((_mode & OPEN_NEW) != 0)
        return KEDIT_OK;
     KMessageBox::sorry(this, i18n("The specified file does not exist"));
     return KEDIT_RETRY;
  }

  TQFile file(_filename);

  if(!file.open(IO_ReadOnly))
  {
     KMessageBox::sorry(this, i18n("You do not have read permission to this file."));
     return KEDIT_RETRY;
  }

  TQTextStream stream(&file);
  TQTextCodec *codec;
  if (!encoding.isEmpty())
    codec = TQTextCodec::codecForName(encoding.latin1());
  else
    codec = TQTextCodec::codecForLocale();
  stream.setCodec(codec);

  if ((_mode & OPEN_INSERT) == 0)
  {
     eframe->clear();
  }
  if ( ! _undoAction )
      eframe->setUndoRedoEnabled(false);

  eframe->insertText( &stream );
  eframe->setModified(false);

  if ( !_undoAction)
      eframe->setUndoRedoEnabled(true);

  return KEDIT_OK;

}


int TopLevel::saveFile( const TQString& _filename, bool backup, const TQString& encoding )
{
  TQFileInfo info(_filename);
  bool bSoftWrap = (Prefs::wrapMode() == Prefs::EnumWrapMode::SoftWrap);

  if(info.isDir())
  {
     KMessageBox::sorry(this, i18n("You have specified a folder"));
     return KEDIT_RETRY;
  }

  if (backup && Prefs::backupCopies() && TQFile::exists(_filename))
  {
     if (!KSaveFile::backupFile(_filename))
     {
        KMessageBox::sorry(this, i18n("Unable to make a backup of the original file."));
     }
  }

  // WABA: We don't use KSaveFile because it doesn't preserve hard/soft
  // links when saving. Most applications don't care about this, but an
  // editor is supposed to preserve such things.

  TQFile file(_filename);
  if(!file.open(IO_WriteOnly))
  {
     KMessageBox::sorry(this, i18n("Unable to write to file."));
     return KEDIT_RETRY;
  }

  TQTextStream textStream(&file);
  TQTextCodec *codec;
  if (!encoding.isEmpty())
    codec = TQTextCodec::codecForName(encoding.latin1());
  else
    codec = TQTextCodec::codecForLocale();
  textStream.setCodec(codec);

  eframe->saveText( &textStream, bSoftWrap );
  file.close();

  if(file.status())
  {
     KMessageBox::sorry(this, i18n("Could not save file."));
     return KEDIT_RETRY;
  }
  eframe->setModified(false);
  return KEDIT_OK;
}


void TopLevel::openURL( const KURL& _url, int _mode )
{
    if ( !_url.isValid() )
    {
        TQString string;
        string = i18n( "Malformed URL\n%1").arg(_url.url());

        KMessageBox::sorry(this, string);
        return;
    }

    TQString target;
    int result = KEDIT_OK;
    if (TDEIO::NetAccess::download(_url, target, this))
    {
        result = openFile(target, _mode, _url.fileEncoding());
    }
    else
    {
      if ((_mode & OPEN_NEW) == 0)
      {
         KMessageBox::error(this, i18n("Cannot download file."));
         return;
      }
    }
    if (result == KEDIT_OK)
    {
        m_url = _url;
        setFileCaption();
        recent->addURL(_url);
        eframe->setModified(false);
        setGeneralStatusField(i18n("Done"));
    }
}

void TopLevel::urlDrop_slot(TQDropEvent* e) {

  dropEvent(e);
}

void TopLevel::dragEnterEvent(TQDragEnterEvent* e)
{
  e->accept(KURLDrag::canDecode(e));
}

void TopLevel::dropEvent(TQDropEvent* e)
{

    KURL::List list;

    // This should never happen, but anyway...
    if(!KURLDrag::decode(e, list))
        return;

    bool first = true;
    for ( KURL::List::ConstIterator it = list.begin(); it != list.end(); ++it)
    {
	// Load the first file in this window
	if ( first && !eframe->isModified() )
	{
            openURL( *it, OPEN_READWRITE );
	}
	else
	{
	    setGeneralStatusField(i18n("New Window"));
	    TopLevel *t = new TopLevel ();
	    t->show ();
	    setGeneralStatusField(i18n("New Window Created"));
	    t->openURL( *it, OPEN_READWRITE );
	    setGeneralStatusField(i18n("Load Command Done"));
	}
	first = false;
    }
}

void TopLevel::timer_slot(){

  statusBar()->changeItem("",ID_GENERAL);

}


void TopLevel::set_colors()
{
  TQPalette mypalette = TQPalette((eframe->palette()));

  TQColorGroup ncgrp( mypalette.active() );

  if (Prefs::customColor())
  {
     ncgrp.setColor(TQColorGroup::Text, Prefs::textColor());
     ncgrp.setColor(TQColorGroup::Base, Prefs::backgroundColor());
  }
  else
  {
     ncgrp.setColor(TQColorGroup::Text, TDEGlobalSettings::textColor());
     ncgrp.setColor(TQColorGroup::Base, TDEGlobalSettings::baseColor());
  }

  mypalette.setActive(ncgrp);
  mypalette.setDisabled(ncgrp);
  mypalette.setInactive(ncgrp);

  eframe->setPalette(mypalette);
}


void TopLevel::updateSettings( void )
{
  readSettings();
  setupEditWidget();
}

void TopLevel::readSettings( void )
{
  recent->loadEntries( kapp->config() );
}


void TopLevel::toggle_overwrite(){

  if(eframe->isOverwriteMode()){
    statusBar()->changeItem("OVR",ID_INS_OVR);
  }
  else{
    statusBar()->changeItem("INS",ID_INS_OVR);
  }

}

static const char description[] = I18N_NOOP("TDE text editor");

static const KCmdLineOptions options[] =
{
	{ "encoding <encoding>", I18N_NOOP("Encoding to use for the following documents"), 0 },
	{ "+file", I18N_NOOP("File or URL to open"), 0 },
	KCmdLineLastOption
};

extern "C" KDE_EXPORT int kdemain (int argc, char **argv)
{
	bool have_top_window = false;

	TDEAboutData aboutData( "kedit", I18N_NOOP("KEdit"),
		KEDITVERSION, description, TDEAboutData::License_GPL,
		"(c) 1997-2000, Bernd Johannes Wuebben");
	aboutData.addAuthor("Bernd Johannes Wuebben",0, "wuebben@kde.org");
	TDECmdLineArgs::init( argc, argv, &aboutData );
	TDECmdLineArgs::addCmdLineOptions( options );

	TDEApplication a;
	//CT TDEIO::Job::initStatic();
	if ( a.isRestored() )
	{
		int n = 1;
		while (TopLevel::canBeRestored(n))
		{
			TopLevel *tl = new TopLevel();
			tl->restore(n);
			n++;
			have_top_window = true;
		}
	}
	else
	{
		have_top_window = false;
		TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

		const TQString encoding = args->getOption("encoding");
		const bool doEncoding = args->isSet("encoding") &&
		                        TQTextCodec::codecForName(encoding.latin1());

		for(int i = 0; i < args->count(); i++)
		{
			TopLevel *t = new TopLevel;
			t->show ();
			have_top_window = true;

			KURL url = args->url(i);

			if(doEncoding)
				url.setFileEncoding(encoding);

			t->openURL( url, default_open | TopLevel::OPEN_NEW );
		}
		args->clear();
	}

	if(!have_top_window)
	{
		TopLevel *t = new TopLevel ();
		t->show ();
	}

	return a.exec ();
}

SettingsDialog::SettingsDialog(TQWidget *parent, const char *name,TDEConfigSkeleton *config, KSpellConfig *_spellConfig)
 : TDEConfigDialog(parent, name, config),
 spellConfig(_spellConfig), spellConfigChanged(false)
{
  // Font
  TQWidget *font = new TQWidget(0, "FontSetting");
  TQVBoxLayout *topLayout = new TQVBoxLayout(font, 0, KDialog::spacingHint());
  TDEFontChooser *mFontChooser = new TDEFontChooser(font, "kcfg_Font", false, TQStringList(), false, 6);
  topLayout->addWidget(mFontChooser);
  addPage(font, i18n("Font"), "fonts", i18n("Editor Font"));

  // Color
  Color *color = new Color(0, "ColorSettings");
  addPage(color, i18n("Color"), "colorize", i18n("Text Color in Editor Area"));

  // Spelling
  addPage(spellConfig, i18n("Spelling"),
  	  "spellcheck", i18n("Spelling Checker"));
  connect(spellConfig, TQT_SIGNAL(configChanged()), this, TQT_SLOT(slotSpellConfigChanged()));

  // Miscellaneous
  Misc *miscOptions = new Misc(0, "MiscSettings");
  addPage(miscOptions, i18n("Miscellaneous"), "misc");
}

void SettingsDialog::updateSettings()
{
  spellConfig->writeGlobalSettings();
}

void SettingsDialog::updateWidgets()
{
  spellConfig->readGlobalSettings();
  spellConfigChanged = false;
}

void SettingsDialog::updateWidgetsDefault()
{
}

bool SettingsDialog::hasChanged()
{
  return spellConfigChanged;
}

bool SettingsDialog::isDefault()
{
  return true;
}

void SettingsDialog::slotSpellConfigChanged()
{
  spellConfigChanged = true;
  updateButtons();
}

#include "kedit.moc"

