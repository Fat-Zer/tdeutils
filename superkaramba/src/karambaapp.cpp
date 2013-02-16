/***************************************************************************
 *   Copyright (C) 2003-2004 Adam Geitgey <adam@rootnode.org>              *
 *   Copyright (C) 2003 Hans Karlsson <karlsson.h@home.se>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqdir.h>
#include <tdefiledialog.h>
#include <tdecmdlineargs.h>
#include <fcntl.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kdebug.h>
#include <khelpmenu.h>

#include <tqtooltip.h>

#include "themesdlg.h"
#include "karambainterface.h"
#include "karambaapp.h"
#include "dcopinterface_stub.h"
#include "karamba.h"
#include "superkarambasettings.h"
#include "tqwidgetlist.h"

int KarambaApplication::fd = -1;

KarambaApplication::KarambaApplication() :
    m_helpMenu(0), iface(0), themeListWindow(0), dcopIfaceStub(0),
    karambaList(0), sysTrayIcon(0)
{
  iface = new KarambaIface();
  karambaList = new TQObjectList();
  // register ourselves as a dcop client
  dcopClient()->registerAs(name());
  dcopClient()->setDefaultObject(dcopIface()->objId());
}

KarambaApplication::~KarambaApplication()
{
  delete iface;
  delete karambaList;
  delete themeListWindow;
  delete dcopIfaceStub;
  //delete m_helpMenu;
}

void KarambaApplication::initDcopStub(TQCString app)
{
  if(app.isEmpty())
    app = dcopClient()->appId();
  dcopIfaceStub = new dcopIface_stub(app, iface->objId());
}

TQString KarambaApplication::getMainKaramba()
{
  TQStringList karambas = getKarambas();
  TQStringList::Iterator it;

  for (it = karambas.begin(); it != karambas.end(); ++it)
  {
    if((*it).ascii() == dcopClient()->appId())
      continue;
    dcopIface_stub dcop((*it).ascii(), iface->objId());
    if (dcop.isMainKaramba())
      return *it;
  }
  return TQString();
}

bool KarambaApplication::themeExists(TQString pretty_name)
{
  TQWidgetList  *list = TQApplication::allWidgets();
  TQWidgetListIt it( *list );         // iterate over the widgets
  TQWidget * w;
  while ( (w=it.current()) != 0 ) // for each widget...
  {
    ++it;
    if (TQString(w->name()).startsWith("karamba"))
    {
      karamba* k = (karamba*) w;
      if (k->getPrettyName() == pretty_name)
        return true;
    }
  }
  delete list; // delete the list, not the widgets
  return false;
}

TQStringList KarambaApplication::getKarambas()
{
  QCStringList applst = dcopClient()->registeredApplications();
  QCStringList::Iterator it;
  TQCString s;
  TQStringList result;

  for (it = applst.begin(); (s = *it) != 0; ++it)
  {
    if (s.left(strlen(name())) == name())
      result.append(s);
  }
  return result;
}

void KarambaApplication::checkSuperKarambaDir()
{
  // Create ~/.superkaramba if necessary
  TQDir configDir(TQDir::home().absPath() + "/.superkaramba");
  if (!configDir.exists())
  {
    tqWarning("~/.superkaramba doesn't exist");
    if(!configDir.mkdir(TQDir::home().absPath() + "/.superkaramba"))
    {
        tqWarning("Couldn't create Directory ~/.superkaramba");
    }
    else
    {
        tqWarning("created ~/.superkaramba");
    }
  }
}

void KarambaApplication::setUpSysTray(TDEAboutData* about)
{
  //kdDebug() << k_funcinfo << endl;
  TDEAction* action;

  //Create theme list window.
  //This will function as the main window for the tray icon
  themeListWindow = new ThemesDlg();

  //Set up systray icon
  sysTrayIcon = new KSystemTray(themeListWindow);

  TDEPopupMenu *menu = sysTrayIcon->contextMenu();
  menu->insertItem(SmallIconSet("superkaramba"),
                   i18n("Hide System Tray Icon"), this,
                   TQT_SLOT(globalHideSysTray()));
  menu->insertSeparator();

  m_helpMenu = new KHelpMenu(themeListWindow, about);
  action = KStdAction::help(m_helpMenu, TQT_SLOT(appHelpActivated()),
                            sysTrayIcon->actionCollection());
  action->plug(menu);
  action = KStdAction::aboutApp(m_helpMenu, TQT_SLOT(aboutApplication()),
                                sysTrayIcon->actionCollection());
  action->plug(menu);
  action = KStdAction::aboutKDE(m_helpMenu, TQT_SLOT(aboutKDE()),
                                sysTrayIcon->actionCollection());
  action->plug(menu);

  sysTrayIcon->setPixmap(sysTrayIcon->loadIcon("superkaramba"));
  setToolTip();

  if(SuperKarambaSettings::showSysTray())
    sysTrayIcon->show();
  else
    sysTrayIcon->hide();

  //Connect Systray icon's quit event
  TQObject::connect(sysTrayIcon, TQT_SIGNAL(quitSelected()),
                   this, TQT_SLOT(globalQuitSuperKaramba()));
}

void KarambaApplication::showKarambaMenuExtension(bool show)
{
  TQObject *k;

  if(show)
  {
    for (k = karambaList->first(); k; k = karambaList->next())
    {
      ((karamba*)k)->showMenuExtension();
    }
  }
  else
  {
    for (k = karambaList->first(); k; k = karambaList->next())
    {
      ((karamba*)k)->hideMenuExtension();
    }
  }
}

void KarambaApplication::setToolTip(const TQString &tip)
{
  TQToolTip::remove(sysTrayIcon);
  if(tip.isNull())
    TQToolTip::add(sysTrayIcon, i18n("SuperKaramba"));
  else
    TQToolTip::add(sysTrayIcon, tip);
}

void KarambaApplication::buildToolTip()
{
  if(!sysTrayIcon || !themeListWindow)
    return;

  TQStringList list = themeListWindow->runningThemes();

  if(list.isEmpty())
  {
    setToolTip();
    return;
  }

  TQString toolTip("<b><center>" + i18n("SuperKaramba") + "</center></b>");
  toolTip += "<table width=300>";

  bool firstRun = true;
  for(TQStringList::Iterator it = list.begin(); it != list.end(); ++it )
  {
    if(firstRun)
    {
      toolTip +=
          "<tr><td align=right>" +
          i18n("1 Running Theme:", "%n Running Themes:", list.count()) +
          "</td><td align=left>" + (*it) + "</td></tr>";
      firstRun = false;
    }
    else
    {
      toolTip += "<tr><td></td><td align=left>" + (*it) + "</td></tr>";
    }
  }

  toolTip += "</table>";

  setToolTip(toolTip);
}

void KarambaApplication::checkPreviousSession(TDEApplication &app,
                                              TQStringList &lst)
{
  /******
  Try to restore a previous session if applicable.
  */
  if (app.isSessionRestored())
  {
    TDEConfig* config = app.sessionConfig();
    config->setGroup("General Options");
    TQString restartThemes = config->readEntry("OpenThemes");

    //Get themes that were running
    lst = TQStringList::split(TQString(";"), restartThemes);
  }
}

void KarambaApplication::checkCommandLine(TDECmdLineArgs *args, TQStringList &lst)
{
  /******
    Not a saved session - check for themes given on command line
  */
  if(args->count() > 0)
  {
    for(int i = 0; i < (args->count()); i++)
    {
      if( args->arg(i) && *args->arg(i) )
      {
        KURL url = args->url(i);

        lst.push_back(url.path());
      }
    }
  }
}

bool KarambaApplication::startThemes(TQStringList &lst)
{
  bool result = false;

  for(TQStringList::Iterator it = lst.begin(); it != lst.end(); ++it )
  {
    karamba *mainWin = 0;

    mainWin = new karamba(*it , TQString());
    mainWin->show();
    result = true;
  }

  buildToolTip();
  return result;
}

void KarambaApplication::addKaramba(karamba* k, bool reloading)
{
  if(!reloading && karambaApp->dcopStub())
  {
    int instance = karambaApp->dcopStub()->themeAdded(
        karambaApp->dcopClient()->appId(), k->theme().file());
    k->setInstance(instance);
  }
  karambaList->append(TQT_TQOBJECT(k));
}

void KarambaApplication::deleteKaramba(karamba* k, bool reloading)
{
  if(!reloading && karambaApp->dcopStub())
    karambaApp->dcopStub()->themeClosed(
        karambaApp->dcopClient()->appId(), k->theme().file(), k->instance());
  karambaList->removeRef(TQT_TQOBJECT(k));
}

bool KarambaApplication::hasKaramba(karamba* k)
{
  return karambaList->containsRef(TQT_TQOBJECT(k)) > 0;
}

// XXX: I guess this should be made with mutex/semaphores
// but this is good for now...

bool KarambaApplication::lockKaramba()
{
  TQString file = TQDir::home().absPath() + "/.superkaramba/.lock";
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP;

  fd = open(file.ascii(), O_CREAT | O_RDWR | O_TRUNC, mode);
  if (fd < 0)
  {
    tqWarning("Open failed in lock.");
    return false;
  }
  //tqDebug("lock %d", getpid());
  if(lockf(fd, F_LOCK, 0))
  {
    tqWarning("Lock failed.");
    return false;
  }
  return true;
}

void KarambaApplication::unlockKaramba()
{
  if(fd > 0)
  {
    lockf(fd, F_ULOCK, 0);
    //tqDebug("Unlock %d", getpid());
    close(fd);
    fd = -1;
  }
}

void KarambaApplication::hideSysTray(bool hide)
{
  //kdDebug() << k_funcinfo << endl;
  if(hide)
  {
    if(sysTrayIcon)
    {
      KMessageBox::information(0,
          i18n("<qt>Hiding the system tray icon will keep SuperKaramba running "
              "in background. To show it again use the theme menu.</qt>"),
          i18n("Hiding System Tray Icon"), "hideIcon");
      sysTrayIcon->hide();
    }
    showKarambaMenuExtension();
  }
  else
  {
    showKarambaMenuExtension(false);
    if(sysTrayIcon)
      sysTrayIcon->show();
  }
}

void KarambaApplication::showThemeDialog()
{
  //kdDebug() << k_funcinfo << endl;
  if(themeListWindow)
    themeListWindow->show();
}

void KarambaApplication::quitSuperKaramba()
{
  if(themeListWindow)
    themeListWindow->saveUserAddedThemes();
  tqApp->closeAllWindows();
  tqApp->quit();
}

void KarambaApplication::globalQuitSuperKaramba()
{
  TQStringList apps = getKarambas();
  TQStringList::Iterator it;

  for (it = apps.begin(); it != apps.end(); ++it)
  {
    dcopIface_stub dcop((*it).ascii(), dcopIface()->objId());
    dcop.quit();
  }
}

void KarambaApplication::globalShowThemeDialog()
{
  TQStringList apps = getKarambas();
  TQStringList::Iterator it;

  for (it = apps.begin(); it != apps.end(); ++it)
  {
    dcopIface_stub dcop((*it).ascii(), dcopIface()->objId());
    dcop.showThemeDialog();
  }
}

void KarambaApplication::globalHideSysTray(bool hide)
{
  //kdDebug() << k_funcinfo << endl;
  TQStringList apps = getKarambas();
  TQStringList::Iterator it;

  SuperKarambaSettings::setShowSysTray(!hide);
  SuperKarambaSettings::writeConfig();

  for (it = apps.begin(); it != apps.end(); ++it)
  {
    dcopIface_stub dcop((*it).ascii(), dcopIface()->objId());
    dcop.hideSystemTray(hide);
  }
}

#include "karambaapp.moc"
