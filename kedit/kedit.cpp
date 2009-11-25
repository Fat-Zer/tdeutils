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

#include <qlayout.h>
#include <qtimer.h>
#include <qpaintdevicemetrics.h>
#include <qpainter.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kconfigdialog.h>
#include <kcursor.h>
#include <keditcl.h>
#include <kfileitem.h>
#include <kfontdialog.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kprinter.h>
#include <ksavefile.h>
#include <kstatusbar.h>
#include <kspell.h>
#include <kurldrag.h>

#include "ktextfiledlg.h"
#include "kedit.h"
#include "misc.h"
#include "color.h"
#include "prefs.h"

#include <kdebug.h>


QPtrList<TopLevel> *TopLevel::windowList = 0;

int default_open =  TopLevel::OPEN_READWRITE;

TopLevel::TopLevel (QWidget *, const char *name)
  : KMainWindow ( 0,name ), kspellconfigOptions(0),
  eframe(0), newWindow(false), kspell(0)
{
  if (!windowList)
  {
     windowList = new QPtrList<TopLevel>;
     windowList->setAutoDelete( FALSE );
  }
  windowList->append( this );

  statusbar_timer = new QTimer(this);
  connect(statusbar_timer, SIGNAL(timeout()),this,SLOT(timer_slot()));

  connect(kapp,SIGNAL(kdisplayPaletteChanged()),this,SLOT(set_colors()));

  setupStatusBar();
  setupActions();

  readSettings();

  setupEditWidget();

  if (!initialGeometrySet())
    resize( QSize(550, 400).expandedTo(minimumSizeHint()));
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

    connect(eframe, SIGNAL(CursorPositionChanged()),this,
           SLOT(statusbar_slot()));
    connect(eframe, SIGNAL(toggle_overwrite_signal()),this,
           SLOT(toggle_overwrite()));
    connect(eframe, SIGNAL(gotUrlDrop(QDropEvent*)), this,
           SLOT(urlDrop_slot(QDropEvent*)));
    connect(eframe, SIGNAL(undoAvailable(bool)),undoAction,
           SLOT(setEnabled(bool)));
    connect(eframe, SIGNAL(redoAvailable(bool)),redoAction,
           SLOT(setEnabled(bool)));
    connect(eframe, SIGNAL(copyAvailable(bool)),cutAction,
           SLOT(setEnabled(bool)));
    connect(eframe, SIGNAL(copyAvailable(bool)),copyAction,
           SLOT(setEnabled(bool)));
    connect( eframe, SIGNAL(selectionChanged()),this,
           SLOT(slotSelectionChanged()));
    connect( eframe, SIGNAL(modificationChanged( bool)),
           SLOT(setFileCaption()));

    undoAction->setEnabled(false);
    redoAction->setEnabled(false);
    cutAction->setEnabled(false);
    copyAction->setEnabled(false);

    setCentralWidget(eframe);
    eframe->setMinimumSize(200, 100);
  }

  if( Prefs::wrapMode() == Prefs::EnumWrapMode::FixedColumnWrap )
  {
    eframe->setWordWrap(QMultiLineEdit::FixedColumnWidth);
    eframe->setWrapColumnOrWidth(Prefs::wrapColumn());
  }
  else if( Prefs::wrapMode() == Prefs::EnumWrapMode::SoftWrap )
  {
    eframe->setWordWrap(QMultiLineEdit::WidgetWidth);
  }
  else
  {
    eframe->setWordWrap(QMultiLineEdit::NoWrap);
  }

  eframe->setFont( Prefs::font() );
  int w = eframe->fontMetrics().width("M");
  eframe->setTabStopWidth(8*w);

  eframe->setModified(false);

  setSensitivity();

  eframe->setFocus();

  set_colors();

  /*
  right_mouse_button = new QPopupMenu;

  right_mouse_button->insertItem (i18n("Open..."),
				  this, 	SLOT(file_open()));
  right_mouse_button->insertItem (SmallIcon("filesave"),i18n("Save"),
				  this, 	SLOT(file_save()));
  right_mouse_button->insertItem (SmallIcon("filesaveas"),i18n("Save As..."),
				  this, SLOT(file_save_as()));
  right_mouse_button->insertSeparator(-1);
  right_mouse_button->insertItem(i18n("Copy"),
				 this, SLOT(copy()));
  right_mouse_button->insertItem(i18n("Paste"),
				 this, SLOT(paste()));
  right_mouse_button->insertItem(i18n("Cut"),
				 this, SLOT(cut()));
  right_mouse_button->insertItem(i18n("Select All"),
				 this, SLOT(select_all()));
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
    KStdAction::openNew(this, SLOT(file_new()), actionCollection());
    KStdAction::open(this, SLOT(file_open()), actionCollection());
    recent = KStdAction::openRecent(this, SLOT(openRecent(const KURL&)),
                                          actionCollection());
    KStdAction::save(this, SLOT(file_save()), actionCollection());
    KStdAction::saveAs(this, SLOT(file_save_as()), actionCollection());
    KStdAction::close(this, SLOT(file_close()), actionCollection());
    KStdAction::print(this, SLOT(print()), actionCollection());
    KStdAction::mail(this, SLOT(mail()), actionCollection());
    KStdAction::quit(this, SLOT(close()), actionCollection());

    // setup edit menu
    undoAction = KStdAction::undo(this, SLOT(undo()), actionCollection());
    redoAction = KStdAction::redo(this, SLOT(redo()), actionCollection());
    cutAction = KStdAction::cut(this, SLOT(cut()), actionCollection());
    copyAction = KStdAction::copy(this, SLOT(copy()), actionCollection());
    KStdAction::pasteText(this, SLOT(paste()), actionCollection());
    KStdAction::selectAll(this, SLOT(select_all()), actionCollection());
    KStdAction::find(this, SLOT(search()), actionCollection());
    KStdAction::findNext(this, SLOT(search_again()), actionCollection());
    KStdAction::replace(this, SLOT(replace()), actionCollection());

    (void)new KAction(i18n("&Insert File..."), 0, this, SLOT(file_insert()),
                      actionCollection(), "insert_file");
    (void)new KAction(i18n("In&sert Date"), 0, this, SLOT(insertDate()),
                      actionCollection(), "insert_date");
    (void)new KAction(i18n("Cl&ean Spaces"), 0, this, SLOT(clean_space()),
                      actionCollection(), "clean_spaces");

    // setup Tools menu
    KStdAction::spelling(this, SLOT(spellcheck()), actionCollection());

    // setup Go menu
    KStdAction::gotoLine(this, SLOT(gotoLine()), actionCollection());

    KStdAction::preferences(this, SLOT(showSettings()), actionCollection());
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


void TopLevel::saveProperties(KConfig* config)
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
    QString name = m_url.url();
    if (name.isEmpty())
       name = QString("kedit%1-%2").arg(getpid()).arg((long)this);
    QString tmplocation = kapp->tempSaveName(m_url.url());
    config->writeEntry("saved_to",tmplocation);
    saveFile(tmplocation, false, m_url.fileEncoding());
  }
}


void TopLevel::readProperties(KConfig* config){
    KURL url = config->readPathEntry("url");
    QString filename = config->readPathEntry("saved_to");

    QString encoding = url.fileEncoding();
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

  QString string;
  QDate dt = QDate::currentDate();
  string = KGlobal::locale()->formatDate(dt);

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

  if (kspell) return; // In progress

  statusBar()->changeItem(i18n("Spellcheck:  Started."), ID_GENERAL);

  initSpellConfig();
  kspell = new KSpell(this, i18n("Spellcheck"), this,
	SLOT( spell_started(KSpell *)), kspellconfigOptions);

  connect (kspell, SIGNAL ( death()),
          this, SLOT ( spell_finished( )));

  connect (kspell, SIGNAL (progress (unsigned int)),
          this, SLOT (spell_progress (unsigned int)));
  connect (kspell, SIGNAL (misspelling (const QString &, const QStringList &, unsigned int)),
          eframe, SLOT (misspelling (const QString &, const QStringList &, unsigned int)));
  connect (kspell, SIGNAL (corrected (const QString &, const QString &, unsigned int)),
          eframe, SLOT (corrected (const QString &, const QString &, unsigned int)));
  connect (kspell, SIGNAL (done(const QString&)),
		 this, SLOT (spell_done(const QString&)));
}


void TopLevel::spell_started( KSpell *)
{
   eframe->spellcheck_start();
   kspell->setProgressResolution(2);
   kspell->check(eframe->text());
}


void TopLevel::spell_progress (unsigned int percent)
{
  QString s;
  s = i18n("Spellcheck:  %1% complete").arg(percent);

  statusBar()->changeItem (s, ID_GENERAL);
}


void TopLevel::spell_done(const QString& newtext)
{
  eframe->spellcheck_stop();
  if (kspell->dlgResult() == 0)
  {
     eframe->setText( newtext);
     statusBar()->changeItem (i18n("Spellcheck:  Aborted."), ID_GENERAL);
  }
  else
  {
     statusBar()->changeItem (i18n("Spellcheck:  Complete."), ID_GENERAL);
  }
  kspell->cleanUp();
}

// Replace ISpell with the name of the actual spell checker.
// TODO: Use %1 in the original string instead when string freeze is over.
QString TopLevel::replaceISpell(QString msg, int client)
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
  KSpell::spellStatus status = kspell->status();
  int client = kspellconfigOptions->client();
  delete kspell;
  kspell = 0;
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
		QString::null, QString::null, this,
		i18n("Open File"));
    if( url.isEmpty() )
    {
      return;
    }

    KIO::UDSEntry entry;
    KIO::NetAccess::stat(url, entry, this);
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

    QString tmpfile;
    KIO::NetAccess::download( url, tmpfile, toplevel );

    int result = toplevel->openFile( tmpfile, 0, url.fileEncoding());
    KIO::NetAccess::removeTempFile( tmpfile );

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
        QString::null, QString::null, this,
	i18n("Insert File"), "", KStdGuiItem::insert().text());
    if( url.isEmpty() )
    {
      return;
    }

    QString tmpfile;
    KIO::NetAccess::download( url, tmpfile, this );
    int result = openFile( tmpfile, OPEN_INSERT, url.fileEncoding(), true );
    KIO::NetAccess::removeTempFile( tmpfile );

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

  QString msg = i18n(""
        "This document has been modified.\n"
        "Would you like to save it?" );
  switch( KMessageBox::warningYesNoCancel( this, msg, QString::null,
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
                    switch( KMessageBox::warningContinueCancel( this, msg, QString::null, KStdGuiItem::quit() ) )
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
    QString msg = i18n("This document has been modified.\n"
                       "Would you like to save it?" );
    switch( KMessageBox::warningYesNoCancel( this, msg, QString::null,
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
     QString string;
     string = i18n("Wrote: %1").arg(m_caption);
     setGeneralStatusField(string);
  }
}


void TopLevel::setGeneralStatusField(const QString &text){

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
                 m_url.url(), QString::null, this,
                 i18n("Save File As"),
                 m_url.fileEncoding());

     if (u.isEmpty())
        return;

     if ( KIO::NetAccess::exists(u, false, this) )
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
      QString string = i18n("Saved as: %1").arg(m_caption);
      setGeneralStatusField(string);
      recent->addURL( u );
    }
}



void TopLevel::mail()
{
  //
  // Default subject string
  //
  QString defaultsubject = name();
  int index = defaultsubject.findRev('/');
  if( index != -1)
    defaultsubject = defaultsubject.right(defaultsubject.length() - index - 1 );

  kapp->invokeMailer( QString::null, QString::null, QString::null,
       defaultsubject, eframe->text() );
}

/*
void TopLevel::fancyprint(){

  QPrinter prt;
  char buf[200];
  if ( prt.setup(0) ) {

    int y =10;
    QPainter p;
    p.begin( &prt );
    p.setFont( eframe->font() );
    QFontMetrics fm = p.fontMetrics();

    int numlines = eframe->numLines();
    for(int i = 0; i< numlines; i++){
      y += fm.ascent();
      QString line;
      line = eframe->textLine(i);
      line.replace( QRegExp("\t"), "        " );
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
  if(KConfigDialog::showDialog("settings"))
    return;

  initSpellConfig();
  KConfigDialog* dialog = new SettingsDialog(this, "settings", Prefs::self(), kspellconfigOptions);

  connect(dialog, SIGNAL(settingsChanged()), this, SLOT(updateSettings()));
  dialog->show();
}

void TopLevel::initSpellConfig()
{
  if (!kspellconfigOptions)
     kspellconfigOptions = new KSpellConfig(0 , "SpellingSettings", 0, false );
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
         if (QDir::currentDirPath() == m_url.directory())
            m_caption = m_url.fileName();
         else
            m_caption = m_url.path();
     }
     else
     {
         KURL url(m_url);
         url.setQuery(QString::null);
         m_caption = url.prettyURL();
     }
     QString encoding = m_url.fileEncoding();
     if (!encoding.isEmpty())
        m_caption += QString(" (%1)").arg(encoding);
  }
  setCaption(m_caption, eframe->isModified());
}


void TopLevel::gotoLine() {
	eframe->doGotoLine();
}

void TopLevel::statusbar_slot(){

  QString linenumber;

  linenumber = i18n("Line: %1 Col: %2")
		     .arg(eframe->currentLine() + 1)
		     .arg(eframe->currentColumn() +1);

  statusBar()->changeItem(linenumber,ID_LINE_COLUMN);
}

void TopLevel::print()
{
    bool aborted = false;
    QString headerLeft = i18n("Date: %1").arg(KGlobal::locale()->formatDate(QDate::currentDate(),true));
    QString headerMid = i18n("File: %1").arg(m_caption);
    QString headerRight;

    QFont printFont = eframe->font();
    QFont headerFont(printFont);
    headerFont.setBold(true);

    QFontMetrics printFontMetrics(printFont);
    QFontMetrics headerFontMetrics(headerFont);

    KPrinter *printer = new KPrinter;
    if(printer->setup(this, i18n("Print %1").arg(m_caption))) {
        // set up KPrinter
        printer->setFullPage(false);
        printer->setCreator("KEdit");
        if ( !m_caption.isEmpty() )
	    printer->setDocName(m_caption);

        QPainter *p = new QPainter;
        p->begin( printer );

        QPaintDeviceMetrics metrics( printer );

        int dy = 0;

	p->setFont(headerFont);
        int w = printFontMetrics.width("M");
        p->setTabStops(8*w);

        int page = 1;
        int lineCount = 0;
        int maxLineCount = eframe->numLines();


        while(true) {
           headerRight = QString("#%1").arg(page);
           dy = headerFontMetrics.lineSpacing();
           QRect body( 0, dy*2,  metrics.width(), metrics.height()-dy*2);

           p->drawText(0, 0, metrics.width(), dy, Qt::AlignLeft, headerLeft);
           p->drawText(0, 0, metrics.width(), dy, Qt::AlignHCenter, headerMid);
           p->drawText(0, 0, metrics.width(), dy, Qt::AlignRight, headerRight);

           QPen pen;
           pen.setWidth(3);
           p->setPen(pen);

           p->drawLine(0, dy+dy/2, metrics.width(), dy+dy/2);

           int y = dy*2;
           while(lineCount < maxLineCount) {
              QString text = eframe->textLine(lineCount);
              if( text.isEmpty() )
                text = " ";	// don't ignore empty lines
              QRect r = p->boundingRect(0, y, body.width(), body.height(),
			QPainter::ExpandTabs | QPainter::WordBreak, text);

              dy = r.height();

              if (y+dy > metrics.height()) break;

              p->drawText(0, y, metrics.width(), metrics.height() - y,
                        QPainter::ExpandTabs | QPainter::WordBreak, text);

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

    if (KIO::NetAccess::upload( tmpFile.name(), _url, this ) == false)
    {
      KMessageBox::error(this, "Could not save remote file");
      return KEDIT_RETRY;
    }

    return true;
}


int TopLevel::openFile( const QString& _filename, int _mode, const QString &encoding, bool _undoAction )
{
  QFileInfo info(_filename);

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

  QFile file(_filename);

  if(!file.open(IO_ReadOnly))
  {
     KMessageBox::sorry(this, i18n("You do not have read permission to this file."));
     return KEDIT_RETRY;
  }

  QTextStream stream(&file);
  QTextCodec *codec;
  if (!encoding.isEmpty())
    codec = QTextCodec::codecForName(encoding.latin1());
  else
    codec = QTextCodec::codecForLocale();
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


int TopLevel::saveFile( const QString& _filename, bool backup, const QString& encoding )
{
  QFileInfo info(_filename);
  bool bSoftWrap = (Prefs::wrapMode() == Prefs::EnumWrapMode::SoftWrap);

  if(info.isDir())
  {
     KMessageBox::sorry(this, i18n("You have specified a folder"));
     return KEDIT_RETRY;
  }

  if (backup && Prefs::backupCopies() && QFile::exists(_filename))
  {
     if (!KSaveFile::backupFile(_filename))
     {
        KMessageBox::sorry(this, i18n("Unable to make a backup of the original file."));
     }
  }

  // WABA: We don't use KSaveFile because it doesn't preserve hard/soft
  // links when saving. Most applications don't care about this, but an
  // editor is supposed to preserve such things.

  QFile file(_filename);
  if(!file.open(IO_WriteOnly))
  {
     KMessageBox::sorry(this, i18n("Unable to write to file."));
     return KEDIT_RETRY;
  }

  QTextStream textStream(&file);
  QTextCodec *codec;
  if (!encoding.isEmpty())
    codec = QTextCodec::codecForName(encoding.latin1());
  else
    codec = QTextCodec::codecForLocale();
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
        QString string;
        string = i18n( "Malformed URL\n%1").arg(_url.url());

        KMessageBox::sorry(this, string);
        return;
    }

    QString target;
    int result = KEDIT_OK;
    if (KIO::NetAccess::download(_url, target, this))
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

void TopLevel::urlDrop_slot(QDropEvent* e) {

  dropEvent(e);
}

void TopLevel::dragEnterEvent(QDragEnterEvent* e)
{
  e->accept(KURLDrag::canDecode(e));
}

void TopLevel::dropEvent(QDropEvent* e)
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
  QPalette mypalette = (eframe->palette()).copy();

  QColorGroup ncgrp( mypalette.active() );

  if (Prefs::customColor())
  {
     ncgrp.setColor(QColorGroup::Text, Prefs::textColor());
     ncgrp.setColor(QColorGroup::Base, Prefs::backgroundColor());
  }
  else
  {
     ncgrp.setColor(QColorGroup::Text, KGlobalSettings::textColor());
     ncgrp.setColor(QColorGroup::Base, KGlobalSettings::baseColor());
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

static const char description[] = I18N_NOOP("KDE text editor");

static const KCmdLineOptions options[] =
{
	{ "encoding <encoding>", I18N_NOOP("Encoding to use for the following documents"), 0 },
	{ "+file", I18N_NOOP("File or URL to open"), 0 },
	KCmdLineLastOption
};

extern "C" KDE_EXPORT int kdemain (int argc, char **argv)
{
	bool have_top_window = false;

	KAboutData aboutData( "kedit", I18N_NOOP("KEdit"),
		KEDITVERSION, description, KAboutData::License_GPL,
		"(c) 1997-2000, Bernd Johannes Wuebben");
	aboutData.addAuthor("Bernd Johannes Wuebben",0, "wuebben@kde.org");
	KCmdLineArgs::init( argc, argv, &aboutData );
	KCmdLineArgs::addCmdLineOptions( options );

	KApplication a;
	//CT KIO::Job::initStatic();
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
		KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

		const QString encoding = args->getOption("encoding");
		const bool doEncoding = args->isSet("encoding") &&
		                        QTextCodec::codecForName(encoding.latin1());

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

SettingsDialog::SettingsDialog(QWidget *parent, const char *name,KConfigSkeleton *config, KSpellConfig *_spellConfig)
 : KConfigDialog(parent, name, config),
 spellConfig(_spellConfig), spellConfigChanged(false)
{
  // Font
  QWidget *font = new QWidget(0, "FontSetting");
  QVBoxLayout *topLayout = new QVBoxLayout(font, 0, KDialog::spacingHint());
  KFontChooser *mFontChooser = new KFontChooser(font, "kcfg_Font", false, QStringList(), false, 6);
  topLayout->addWidget(mFontChooser);
  addPage(font, i18n("Font"), "fonts", i18n("Editor Font"));

  // Color
  Color *color = new Color(0, "ColorSettings");
  addPage(color, i18n("Color"), "colorize", i18n("Text Color in Editor Area"));

  // Spelling
  addPage(spellConfig, i18n("Spelling"),
  	  "spellcheck", i18n("Spelling Checker"));
  connect(spellConfig, SIGNAL(configChanged()), this, SLOT(slotSpellConfigChanged()));

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

