/*
 * Copyright (C) 2003-2004 Adam Geitgey <adam@rootnode.org>
 * Copyright (C) 2003 Hans Karlsson <karlsson.h@home.se>
 * Copyright (C) 2004,2005 Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 * Copyright (c) 2005 Ryan Nickell <p0z3r@earthlink.net>
 *
 * This file is part of SuperKaramba.
 *
 *  SuperKaramba is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  SuperKaramba is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with SuperKaramba; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 ****************************************************************************/

#include "karamba_python.h"
#include "dcopinterface_stub.h"
#include "richtextlabel.h"
#include "karamba.h"
#include "karambaapp.h"
#include "themesdlg.h"
#include "lineparser.h"
#include "themelocale.h"
#include "superkarambasettings.h"

#include <kdebug.h>
#include <kmessagebox.h>
#include <krun.h>
#include <klocale.h>
#include <twin.h>
#include <kdeversion.h>
#include <kdirwatch.h>

#include <kparts/componentfactory.h>
#include <kparts/part.h>

#include <tqdir.h>
#include <tqwidgetlist.h>

// Menu IDs
#define EDITSCRIPT 1
#define THEMECONF  2

karamba::karamba(TQString fn, TQString name, bool reloading, int instance,
    bool sub_theme):
    TQWidget(0,"karamba", TQt::WGroupLeader | WStyle_Customize |
            WRepaintNoErase| WStyle_NoBorder | WDestructiveClose  ),
    meterList(0), imageList(0), clickList(0), kpop(0), widgetMask(0),
    config(0), kWinModule(0), tempUnit('C'), m_instance(instance),
    sensorList(0), timeList(0),
    themeConfMenu(0), toDesktopMenu(0), kglobal(0), clickPos(0, 0), accColl(0),
    menuAccColl(0), toggleLocked(0), pythonIface(0), defaultTextField(0),
    trayMenuSeperatorId(-1), trayMenuQuitId(-1), trayMenuToggleId(-1),
    trayMenuThemeId(-1),
    m_sysTimer(NULL)
{
  themeStarted = false;
  want_right_button = false;
  want_meter_wheel_event = false;
  prettyName = name;
  m_sub_theme = sub_theme;

  KURL url;

  if(fn.find('/') == -1)
    url.setFileName(fn);
  else
    url = fn;
  if(!m_theme.set(url))
  {
    setFixedSize(0, 0);
    TQTimer::singleShot(100, this, TQT_SLOT(killWidget()));
    return;
  }
  // Add self to list of open themes
  // This also updates instance number
  karambaApp->addKaramba(this, reloading);

  if(prettyName.isEmpty())
    prettyName = TQString("%1 - %2").tqarg(m_theme.name()).tqarg(m_instance);

  kdDebug() << "Starting theme: " << m_theme.name()
            << " pretty name: " << prettyName << endl;
  TQString qName = "karamba - " + prettyName;
  setName(qName.ascii());

  KDirWatch *dirWatch = KDirWatch::self();
  connect(dirWatch, TQT_SIGNAL( dirty( const TQString & ) ),
          TQT_SLOT( slotFileChanged( const TQString & ) ) );

  if(!dirWatch->contains(m_theme.file()))
    dirWatch->addFile(m_theme.file());

  if(!m_theme.isZipTheme() && m_theme.pythonModuleExists())
  {
    TQString pythonFile = m_theme.path() + "/" + m_theme.pythonModule() + ".py";
    if(!dirWatch->contains(pythonFile))
      dirWatch->addFile(pythonFile);
  }

  widgetUpdate = true;

  // Creates KConfig Object
  TQString instanceString;
  if(m_instance > 1)
    instanceString = TQString("-%1").tqarg(m_instance);
  TQString cfg = TQDir::home().absPath() + "/.superkaramba/"
      + m_theme.id() + instanceString + ".rc";
  kdDebug() << cfg << endl;
  TQFile themeConfigFile(cfg);
  // Tests if config file Exists
  if (!TQFileInfo(themeConfigFile).exists())
  {
    // Create config file
    themeConfigFile.open(IO_ReadWrite);
    themeConfigFile.close();
  }

  config = new KConfig(cfg, false, false);
  config -> sync();
  config -> setGroup("internal");

  m_reloading = reloading;
  if(m_theme.pythonModuleExists())
  {
    kdDebug() << "Loading python module: " << m_theme.pythonModule() << endl;
    TQTimer::singleShot(0, this, TQT_SLOT(initPythonInterface()));
  }

  widgetMask = 0;
  info = new NETWinInfo( qt_xdisplay(), winId(), qt_xrootwin(), NET::WMState );

  // could be replaced with TaskManager
  kWinModule = new KWinModule();
  desktop = 0;

  connect( kWinModule,TQT_SIGNAL(currentDesktopChanged(int)), this,
           TQT_SLOT(currentDesktopChanged(int)) );
  connect( kapp, TQT_SIGNAL(backgroundChanged(int)), this,
           TQT_SLOT(currentWallpaperChanged(int)));

  // Setup of the Task Manager Callbacks
  connect(&taskManager, TQT_SIGNAL(activeTaskChanged(Task*)), this,
           TQT_SLOT(activeTaskChanged(Task*)) );
  connect(&taskManager, TQT_SIGNAL(taskAdded(Task*)), this,
           TQT_SLOT(taskAdded(Task*)) );
  connect(&taskManager, TQT_SIGNAL(taskRemoved(Task*)), this,
           TQT_SLOT(taskRemoved(Task*)) );
  connect(&taskManager, TQT_SIGNAL(startupAdded(Startup*)), this,
           TQT_SLOT(startupAdded(Startup*)) );
  connect(&taskManager, TQT_SIGNAL(startupRemoved(Startup*)), this,
           TQT_SLOT(startupRemoved(Startup*)) );

  themeConfMenu = new KPopupMenu( this);
  themeConfMenu -> setCheckable(true);

  /* XXX - need to be able to delete all these DesktopChangeSlot objects */
  DesktopChangeSlot *dslot;

  int mid;

  toDesktopMenu = new KPopupMenu (this);
  toDesktopMenu -> setCheckable(true);
  mid = toDesktopMenu -> insertItem (i18n("&All Desktops"),
                                     dslot = new DesktopChangeSlot(TQT_TQOBJECT(this),0),
                                     TQT_SLOT(receive()));
  dslot->setMenuId(mid);

  toDesktopMenu -> insertSeparator();
  for (int ndesktop=1; ndesktop <= kWinModule->numberOfDesktops(); ndesktop++)
  {
    TQString name = i18n("Desktop &");
    name += ('0' + ndesktop);

    mid = toDesktopMenu -> insertItem (name,
        dslot = new DesktopChangeSlot(TQT_TQOBJECT(this), ndesktop), TQT_SLOT(receive()));
    dslot->setMenuId(mid);
  }


  kpop = new KPopupMenu( this );
  kpop -> setCheckable(true);

  accColl = new KActionCollection( this );
  menuAccColl = new KActionCollection( this );

  kpop->insertItem( SmallIconSet("reload"),i18n("Update"), this,
                    TQT_SLOT(updateSensors()), Key_F5 );
  toggleLocked = new KToggleAction (  i18n("Toggle &Locked Position"),
                                      SmallIconSet("locked"),
                                      CTRL+Key_L, TQT_TQOBJECT(this),
                                      TQT_SLOT( slotToggleLocked() ),
                                      accColl, "Locked position" );
  accColl->insert(toggleLocked);
  toggleLocked -> setChecked(true);

  toggleLocked->plug(kpop);

  toggleFastTransforms = new KToggleAction(i18n("Use &Fast Image Scaling"),
                         CTRL+Key_F, TQT_TQOBJECT(this),
                         TQT_SLOT( slotToggleFastTransforms() ),
                         accColl, "Fast transformations");

  accColl->insert(toggleFastTransforms);
  toggleFastTransforms -> setChecked(true);

  toggleFastTransforms -> plug(kpop);

  kpop->insertSeparator();

  kpop->insertItem(i18n("Configure &Theme"), themeConfMenu, THEMECONF);
  kpop->setItemEnabled(THEMECONF, false);
  kpop->insertItem(i18n("To Des&ktop"), toDesktopMenu);

  kpop->insertItem( SmallIconSet("reload3"),i18n("&Reload Theme"),this,
                    TQT_SLOT(reloadConfig()), CTRL+Key_R );
  kpop->insertItem( SmallIconSet("fileclose"),i18n("&Close This Theme"), this,
                    TQT_SLOT(killWidget()), CTRL+Key_C );

  if(!SuperKarambaSettings::showSysTray())
    showMenuExtension();

  kpop->polish();

  numberOfConfMenuItems = 0;

  systray = 0;
  foundKaramba = false;
  onTop = false;
  managed = false;
  fixedPosition = false;
  defaultTextField = new TextField();

  meterList = new TQObjectList();
  meterList->setAutoDelete( true );
  sensorList = new TQObjectList();
  sensorList->setAutoDelete( true );
  clickList = new TQObjectList();
  timeList = new TQObjectList();
  imageList = new TQObjectList();
  menuList = new TQObjectList();
  menuList->setAutoDelete( true );

  client = kapp->dcopClient();
  if (!client->isAttached())
    client->attach();
  appId = client->registerAs(tqApp->name());


  setBackgroundMode( NoBackground);
  if( !(onTop || managed))
    KWin::lowerWindow( winId() );

  if( !parseConfig() )
  {
    setFixedSize(0,0);
    TQTimer::singleShot( 100, this, TQT_SLOT(killWidget()) );
    qWarning("Could not read config file.");
  }
  else
  {
    kroot = new KarambaRootPixmap((TQWidget*)this);
    kroot->start();
  }

  // Karamba specific Config Entries
  bool locked = toggleLocked->isChecked();
  locked = config->readBoolEntry("lockedPosition", locked);
  toggleLocked->setChecked(locked);
  slotToggleLocked();

  if (!config -> readBoolEntry("fastTransforms", true))
  {
    toggleFastTransforms -> setChecked(false);
    slotToggleFastTransforms();
  }

  desktop = config -> readNumEntry("desktop", desktop);
  if (desktop > kWinModule->numberOfDesktops())
  {
    desktop = 0;
  }

  if (desktop)
  {
    info->setDesktop( desktop );
  }
  else
    info->setDesktop( NETWinInfo::OnAllDesktops);

  // Read Themespecific Config Entries
  config -> setGroup("theme");
  if (config -> hasKey("widgetPosX") && config -> hasKey("widgetPosY"))
  {
    int xpos = config -> readNumEntry("widgetPosX");
    int ypos = config -> readNumEntry("widgetPosY");

    if (xpos < 0)
      xpos = 0;
    if (ypos < 0)
      ypos = 0;
    move(xpos, ypos);
  }

  haveUpdated = 0;
  this->setMouseTracking(true);


  setFocusPolicy(TQ_StrongFocus);
}

karamba::~karamba()
{
  //qDebug("karamba::~karamba");
  //Remove self from list of open themes
  karambaApp->deleteKaramba(this, m_reloading);

  widgetClosed();
  if(m_theme.isValid())
    writeConfigData();

  delete config;

  if(meterList != 0)
  {
    meterList->clear();
    delete meterList;
  }

  if( sensorList != 0 )
  {
    sensorList->clear();
    delete sensorList;
  }

  if( imageList != 0 )
  {
    imageList->clear();
    delete imageList;
  }

  if( clickList != 0 )
  {
    clickList->clear();
    delete clickList;
  }

  if( timeList != 0 )
  {
    timeList->clear();
    delete timeList;
  }

  delete toggleLocked;
  delete accColl;
  delete menuAccColl;
  delete themeConfMenu;
  delete kpop;
  delete widgetMask;
  delete kWinModule;
  delete defaultTextField;
  if (pythonIface != NULL)
    delete pythonIface;
}

bool karamba::parseConfig()
{
  //qDebug("karamba::parseConfig");
  bool passive = true;

  if(m_theme.open())
  {
    TQValueStack<TQPoint> offsetStack;
    LineParser lineParser;
    int x=0;
    int y=0;
    int w=0;
    int h=0;

    offsetStack.push(TQPoint(0,0));

    while(m_theme.nextLine(lineParser))
    {
      x = lineParser.getInt("X") + offsetStack.top().x();
      y = lineParser.getInt("Y") + offsetStack.top().y();
      w = lineParser.getInt("W");
      h = lineParser.getInt("H");

      if(lineParser.meter() == "KARAMBA" && !foundKaramba )
      {
        //qDebug("karamba found");
        toggleLocked->setChecked(lineParser.getBoolean("LOCKED"));
        slotToggleLocked();

        x = ( x < 0 ) ? 0:x;
        y = ( y < 0 ) ? 0:y;

        if( w == 0 ||  h == 0)
        {
          w = 300;
          h = 300;
        }
        setFixedSize(w,h);

        if(lineParser.getBoolean("RIGHT"))
        {
          TQDesktopWidget *d = TQApplication::desktop();
          x = d->width() - w;
        }
        else if(lineParser.getBoolean("LEFT"))
        {
          x = 0;
        }

        if(lineParser.getBoolean("BOTTOM"))
        {
          TQDesktopWidget *d = TQApplication::desktop();
          y = d->height() - h;
        }
        else if(lineParser.getBoolean("TOP"))
        {
          y = 0;
        }

        move(x,y);
        //pm = TQPixmap(size());

        if(lineParser.getBoolean("ONTOP"))
        {
          onTop = true;
          KWin::setState( winId(), NET::StaysOnTop );
        }

        if(lineParser.getBoolean("MANAGED"))
        {
          managed = true;
          reparent(0, TQt::WType_Dialog | WStyle_Customize | WStyle_NormalBorder
                      |  WRepaintNoErase | WDestructiveClose, pos());
        }
        else
        {
          info->setState( NETWinInfo::SkipTaskbar
              | NETWinInfo::SkipPager,NETWinInfo::SkipTaskbar
              | NETWinInfo::SkipPager );
          if (onTop)
          {
            KWin::setState( winId(), NET::StaysOnTop );

          }
        }

        if (lineParser.getBoolean("ONALLDESKTOPS"))
        {
          desktop = 200; // ugly
        }


        bool dfound=false;
        //int desktop = lineParser.getInt("DESKTOP", line, dfound);
        if (dfound)
        {
          info->setDesktop( dfound );
        }
        if(lineParser.getBoolean("TOPBAR"))
        {
          move(x,0);
          KWin::setStrut( winId(), 0, 0, h, 0 );
          toggleLocked->setChecked( true );
          slotToggleLocked();
          toggleLocked->setEnabled(false);
        }

        if(lineParser.getBoolean("BOTTOMBAR"))
        {
          int dh = TQApplication::desktop()->height();
          move( x, dh - h );
          KWin::setStrut( winId(), 0, 0, 0, h );
          toggleLocked->setChecked( true );
          slotToggleLocked();
          toggleLocked->setEnabled(false);
        }

        if(lineParser.getBoolean("RIGHTBAR"))
        {
          int dw = TQApplication::desktop()->width();
          move( dw - w, y );
          KWin::setStrut( winId(), 0, w, 0, 0 );
          toggleLocked->setChecked( true );
          slotToggleLocked();
          toggleLocked->setEnabled(false);
        }

        if(lineParser.getBoolean("LEFTBAR"))
        {
          move( 0, y );
          KWin::setStrut( winId(), w, 0, 0, 0 );
          toggleLocked->setChecked( true );
          slotToggleLocked();
          toggleLocked->setEnabled(false);
        }

        TQString path = lineParser.getString("MASK");

        TQFileInfo info(path);
        TQString absPath;
        TQBitmap bmMask;
        TQByteArray ba;
        if( info.isRelative() )
        {
          absPath.setAscii(m_theme.path().ascii());
          absPath.append(path.ascii());
          ba = m_theme.readThemeFile(path);
        }
        else
        {
          absPath.setAscii(path.ascii());
          ba = m_theme.readThemeFile(info.fileName());
        }
        if(m_theme.isZipTheme())
        {
          bmMask.loadFromData(ba);
        }
        else
        {
          bmMask.load(absPath);
        }
        setMask(bmMask);

        m_interval = lineParser.getInt("INTERVAL");
        m_interval = (m_interval == 0) ? 1000 : m_interval;

        TQString temp = lineParser.getString("TEMPUNIT", "C").upper();
        tempUnit = temp.ascii()[0];
        foundKaramba = true;
      }

      if(lineParser.meter() == "THEME")
      {
        TQString path = lineParser.getString("PATH");
        TQFileInfo info(path);
        if( info.isRelative())
          path = m_theme.path() +"/" + path;
        (new karamba( path, TQString() ))->show();
      }

      if(lineParser.meter() == "<GROUP>")
      {
        int offsetX = offsetStack.top().x();
        int offsetY = offsetStack.top().y();
        offsetStack.push( TQPoint( offsetX + lineParser.getInt("X"),
                                  offsetY + lineParser.getInt("Y")));
      }

      if(lineParser.meter() == "</GROUP>")
      {
        offsetStack.pop();
      }

      if(lineParser.meter() == "CLICKAREA")
      {
        if( !hasMouseTracking() )
          setMouseTracking(true);
        ClickArea *tmp = new ClickArea(this, x, y, w, h );
        tmp->setOnClick(lineParser.getString("ONCLICK"));

        setSensor(lineParser, (Meter*)tmp);
        clickList->append( tmp );
        if( lineParser.getBoolean("PREVIEW"))
          meterList->append( tmp );
      }

      // program sensor without a meter
      if(lineParser.meter() == "SENSOR=PROGRAM")
      {
        setSensor(lineParser, 0 );
      }

      if(lineParser.meter() == "IMAGE")
      {
        TQString file = lineParser.getString("PATH");
        TQString file_roll = lineParser.getString("PATHROLL");
        int xon = lineParser.getInt("XROLL");
        int yon = lineParser.getInt("YROLL");
        TQString tiptext = lineParser.getString("TOOLTIP");
        TQString name = lineParser.getString("NAME");
        bool bg = lineParser.getBoolean("BACKGROUND");
        xon = ( xon <= 0 ) ? x:xon;
        yon = ( yon <= 0 ) ? y:yon;

        ImageLabel *tmp = new ImageLabel(this, x, y, 0, 0);
        tmp->setValue(file);
        if(!file_roll.isEmpty())
          tmp->parseImages(file, file_roll, x,y, xon, yon);
        tmp->setBackground(bg);
        if (!name.isEmpty())
          tmp->setName(name.ascii());
        if (!tiptext.isEmpty())
          tmp->setTooltip(tiptext);

        connect(tmp, TQT_SIGNAL(pixmapLoaded()), this, TQT_SLOT(externalStep()));
        setSensor(lineParser, (Meter*) tmp );
        meterList->append (tmp );
        imageList->append (tmp );
      }

      if(lineParser.meter() == "DEFAULTFONT" )
      {
        delete defaultTextField;
        defaultTextField = new TextField( );

        defaultTextField->setColor(lineParser.getColor("COLOR",
                                   TQColor("black")));
        defaultTextField->setBGColor(lineParser.getColor("BGCOLOR",
                                     TQColor("white")));
        defaultTextField->setFont(lineParser.getString("FONT", "Helvetica"));
        defaultTextField->setFontSize(lineParser.getInt("FONTSIZE", 12));
        defaultTextField->tqsetAlignment(lineParser.getString("ALIGN",
                                       "LEFT"));
        defaultTextField->setFixedPitch(lineParser.getBoolean("FIXEDPITCH",
                                        false));
        defaultTextField->setShadow(lineParser.getInt("SHADOW", 0));
      }

      if(lineParser.meter() == "TEXT" ||
         lineParser.meter() == "CLICKMAP" ||
         lineParser.meter() == "RICHTEXT" ||
         lineParser.meter() == "INPUT")
      {
        TextField defTxt;

        if(defaultTextField)
          defTxt = *defaultTextField;

        TextField* tmpText = new TextField();

        tmpText->setColor(lineParser.getColor("COLOR", defTxt.getColor()));
        tmpText->setBGColor(lineParser.getColor("BGCOLOR",
                            defTxt.getBGColor()));
        tmpText->setFont(lineParser.getString("FONT", defTxt.getFont()));
        tmpText->setFontSize(lineParser.getInt("FONTSIZE",
                             defTxt.getFontSize()));
        tmpText->tqsetAlignment(lineParser.getString("ALIGN",
                              defTxt.getAlignmentAsString()));
        tmpText->setFixedPitch(lineParser.getInt("FIXEDPITCH",
                               defTxt.getFixedPitch()));

        tmpText->setShadow(lineParser.getInt("SHADOW", defTxt.getShadow()));

        // ////////////////////////////////////////////////////
        // Now handle the specifics
        if(lineParser.meter() == "TEXT")
        {

          TextLabel *tmp = new TextLabel(this, x, y, w, h );
          tmp->setTextProps(tmpText);
          tmp->setValue(
              m_theme.locale()->translate(lineParser.getString("VALUE")));

          TQString name = lineParser.getString("NAME");
          if (!name.isEmpty())
            tmp->setName(name.ascii());

          setSensor(lineParser, (Meter*)tmp);
          meterList->append ( tmp );
        }

        if(lineParser.meter() == "CLICKMAP")
        {
          if( !hasMouseTracking() )
            setMouseTracking(true);
          ClickMap *tmp = new ClickMap(this, x, y, w, h);
          tmp->setTextProps( tmpText );

          setSensor(lineParser, (Meter*)tmp);
          // set all params
          clickList -> append(tmp);
          meterList->append( tmp );

        }

        if(lineParser.meter() == "RICHTEXT")
        {
          RichTextLabel *tmp = new RichTextLabel(this, x, y, w, h);

          bool dUl = lineParser.getBoolean("UNDERLINE");

          tmp->setText(
              m_theme.locale()->translate(lineParser.getString("VALUE").ascii()), dUl);
          tmp->setTextProps( tmpText );
          tmp->setWidth(w);
          tmp->setHeight(h);

          TQString name = lineParser.getString("NAME");
          if (!name.isEmpty())
            tmp->setName(name.ascii());

          setSensor(lineParser, (Meter*)tmp);
          clickList -> append(tmp);
          meterList->append ( tmp );
        }

        if(lineParser.meter() == "INPUT")
        {
          Input *tmp = new Input(this, x, y, w, h);

          TQString name = lineParser.getString("NAME");
          if (!name.isEmpty())
            tmp->setName(name.ascii());

          tmp->setTextProps(tmpText);
          tmp->setValue(
              m_theme.locale()->translate(lineParser.getString("VALUE").ascii()));

          meterList->append(tmp);
          passive = false;
        }
      }

      if(lineParser.meter() == "BAR")
      {
        Bar *tmp = new Bar(this, x, y, w, h );
        tmp->setImage(lineParser.getString("PATH").ascii());
        tmp->setVertical(lineParser.getBoolean("VERTICAL"));
        tmp->setMax(lineParser.getInt("MAX", 100));
        tmp->setMin(lineParser.getInt("MIN", 0));
        tmp->setValue(lineParser.getInt("VALUE"));
        TQString name = lineParser.getString("NAME");
        if (!name.isEmpty())
          tmp->setName(name.ascii());
        setSensor(lineParser, (Meter*)tmp );
        meterList->append ( tmp );
      }

      if(lineParser.meter() == "GRAPH")
      {
        int points = lineParser.getInt("POINTS");

        Graph *tmp = new Graph(this, x, y, w, h, points);
        tmp->setMax(lineParser.getInt("MAX", 100));
        tmp->setMin(lineParser.getInt("MIN", 0));
        TQString name = lineParser.getString("NAME");
        if (!name.isEmpty())
          tmp->setName(name.ascii());

        tmp->setColor(lineParser.getColor("COLOR"));

        setSensor(lineParser, (Graph*)tmp);
        meterList->append ( tmp );
      }
    }

    if(passive && !managed)
    {
      // Matthew Kay: set window type to "dock"
      // (plays better with taskbar themes this way)
      KWin::setType(winId(), NET::Dock);
      #if defined(KDE_MAKE_VERSION)
        #if TDE_VERSION >= KDE_MAKE_VERSION(3,1,9)
          //KDE 3.2 addition for the always on top issues
          KWin::setState(winId(), NET::KeepBelow);
        #endif
      #endif
    }

    m_theme.close();
  }
  //qDebug("parseConfig ok: %d", foundKaramba);
  if( !foundKaramba )
  {
    //  interval = initKaramba( "", 0, 0, 0, 0 );
    //   this->close(true);
    //delete this;
    return false;
  }
  else
  {
    return true;
  }
}

void karamba::start()
{
  m_sysTimer = new TQTimer(this);

  connect(m_sysTimer, TQT_SIGNAL(timeout()), TQT_SLOT(step()));

  m_sysTimer->start(m_interval);

    //Start the widget running
  TQTimer::singleShot( 0, this, TQT_SLOT(step()) );

  if( !(onTop || managed) )
    lowerTimer.start();
}

void karamba::makeActive()
{
  KWin::setType(winId(), NET::Normal);

  #if defined(KDE_MAKE_VERSION)
    #if TDE_VERSION >= KDE_MAKE_VERSION(3,1,9)
      //KDE 3.2 addition for the always on top issues
      KWin::setState(winId(), NET::Modal);
    #endif
  #endif
}

void karamba::makePassive()
{
  if(managed)
    return;

  TQObject *meter;
  for (meter = meterList->first(); meter; meter = meterList->next())
  {
    if((meter)->isA("Input"))
      return;
  }

  // Matthew Kay: set window type to "dock" (plays better with taskbar themes
  // this way)
  KWin::setType(winId(), NET::Dock);
  #if defined(KDE_MAKE_VERSION)
    #if TDE_VERSION >= KDE_MAKE_VERSION(3,1,9)
      //KDE 3.2 addition for the always on top issues
      KWin::setState(winId(), NET::KeepBelow);
    #endif
  #endif
}

void karamba::popupNotify(int)
{
  //qDebug("karamba::popupNotify");
}

void karamba::reloadConfig()
{
  //qDebug("karamba::reloadConfig: %s", m_theme.file().ascii());
  writeConfigData();
  if(m_theme.exists())
  {
    TQFileInfo fileInfo( m_theme.file() );
    (new karamba(m_theme.file(), fileInfo.baseName(), true, m_instance))->show();
  }
  closeTheme(true);
}

void karamba::closeTheme(bool reloading)
{
  m_reloading = reloading;
  close();
}

void karamba::killWidget()
{
  closeTheme();
}

void karamba::initPythonInterface()
{
  pythonIface = new KarambaPython(m_theme, m_reloading);
}

void karamba::editConfig()
{
  //qDebug("karamba::editConfig");
  TQFileInfo fileInfo( m_theme.file() );
  TQString path;

  if( fileInfo.isRelative() )
  {
    path = m_theme.path() + "/" + m_theme.file();
  }
  else
  {
    path = m_theme.file();
  }

  KRun::runURL( KURL( path ), "text/plain" );
}

void karamba::editScript()
{
  //qDebug("karamba::editScript");
  TQFileInfo fileInfo( m_theme.file() );
  TQString path;

  if( fileInfo.isRelative() )
  {
      path = m_theme.path() + "/" + m_theme.name() + ".py";
  }
  else
  {
      path = TQFileInfo(m_theme.file()).dirPath() + "/" + m_theme.name() + ".py";
  }
  KRun::runURL( KURL( path ), "text/plain" );
}

TQString karamba::findSensorFromMap(Sensor* sensor)
{
  //qDebug("karamba::findSensorFromMap");
  TQMap<TQString,Sensor*>::ConstIterator it;
  TQMap<TQString,Sensor*>::ConstIterator end( sensorMap.end() );
  for ( it = sensorMap.begin(); it != end; ++it )
  {
    if (it.data() == sensor)
      return it.key();
  }
  return "";
}

Sensor* karamba::findSensorFromList(Meter* meter)
{
  //qDebug("karamba::findSensorFromList");
  TQObjectListIt it( *sensorList ); // iterate over meters

  while ( it != 0 )
  {
    if (((Sensor*) *it)->hasMeter(meter))
      return ((Sensor*)*it);
    ++it;
  }
  return NULL;
}

TQString karamba::getSensor(Meter* meter)
{
  //qDebug("karamba::getSensor");
  TQString s;
  Sensor* sensor = findSensorFromList(meter);
  if (sensor)
    s = findSensorFromMap(sensor);
  return s;
}

void karamba::deleteMeterFromSensors(Meter* meter)
{
  //qDebug("karamba::deleteMeterFromSensors");
  Sensor* sensor = findSensorFromList(meter);

  if (sensor)
  {
    sensor->deleteMeter(meter);
    if (sensor->isEmpty())
    {
      TQString s = findSensorFromMap(sensor);
      sensorMap.erase(s);
      sensorList->removeRef(sensor);
    }
  }
}

void karamba::setSensor(const LineParser& lineParser, Meter* meter)
{
  //qDebug("karamba::setSensor");
  Sensor* sensor = 0;

  deleteMeterFromSensors(meter);

  TQString sens = lineParser.getString("SENSOR").upper();

  if( sens == "CPU" )
  {
    TQString cpuNbr = lineParser.getString("CPU");
    sensor = sensorMap["CPU"+cpuNbr];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = (interval == 0)?1000:interval;
      sensor = ( sensorMap["CPU"+cpuNbr] = new CPUSensor( cpuNbr, interval ) );
      sensorList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("FORMAT",
                 m_theme.locale()->translate(lineParser.getString("FORMAT").ascii()));
    sp->addParam("DECIMALS",lineParser.getString("DECIMALS"));

    sensor->addMeter(sp);
    sensor->setMaxValue(sp);

  }

  if( sens == "MEMORY" )
  {
    sensor = sensorMap["MEMORY"];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = (interval == 0)?3000:interval;
      sensor = ( sensorMap["MEMORY"] = new MemSensor( interval ) );
      sensorList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("FORMAT",
        m_theme.locale()->translate(lineParser.getString("FORMAT").ascii()));

    sensor->addMeter(sp);
    sensor->setMaxValue(sp);
  }


  if( sens == "DISK" )
  {
    sensor = sensorMap["DISK"];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = (interval == 0)?5000:interval;
      sensor = ( sensorMap["DISK"] = new DiskSensor( interval ) );
      connect( sensor, TQT_SIGNAL(initComplete()), this, TQT_SLOT(externalStep()) );
      sensorList->append( sensor );
    }
    // meter->setMax( ((DiskSensor*)sensor)->getTotalSpace(mntPt)/1024 );
    SensorParams *sp = new SensorParams(meter);
    TQString mntPt = lineParser.getString("MOUNTPOINT");
    if( mntPt.isEmpty()  )
    {
        mntPt = "/";
    }
    // remove any trailing '/' from mount points in the .theme config, our
    // mntMap doesn't like trailing '/'s for matching in DiskSensor
    if( mntPt.length() > 1 && mntPt.endsWith("/") )
    {
        mntPt.remove( mntPt.length()-1, 1 );
    }
    sp->addParam("MOUNTPOINT",mntPt);
    sp->addParam("FORMAT",
                 m_theme.locale()->translate(lineParser.getString("FORMAT").ascii()));
    sensor->addMeter(sp);
    sensor->setMaxValue(sp);
  }

  if( sens == "NETWORK")
  {
    int interval = lineParser.getInt("INTERVAL");
    interval = (interval == 0)?2000:interval;
    TQString device = lineParser.getString("DEVICE");
    sensor = sensorMap["NETWORK"+device];
    if (sensor == 0)
    {
      sensor = ( sensorMap["NETWORK"+device] =
          new NetworkSensor(device, interval));
      sensorList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("FORMAT",
                 m_theme.locale()->translate(lineParser.getString("FORMAT").ascii()));
    sp->addParam("DECIMALS", lineParser.getString("DECIMALS"));
    sensor->addMeter(sp);
  }

  if( sens == "UPTIME" )
  {
    sensor = sensorMap["UPTIME"];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = (interval == 0)?60000:interval;
      sensor = ( sensorMap["UPTIME"] = new UptimeSensor( interval ));
      sensorList->append( sensor );

    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("FORMAT",
                 m_theme.locale()->translate(lineParser.getString("FORMAT").ascii()));
    sensor->addMeter(sp);
  }

  if( sens == "SENSOR" )
  {
    sensor = sensorMap["SENSOR"];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = (interval == 0)?30000:interval;
      sensor = (sensorMap["SENSOR"] = new SensorSensor(interval, tempUnit));
      sensorList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("FORMAT",
                 m_theme.locale()->translate(lineParser.getString("FORMAT").ascii()));
    sp->addParam("TYPE", lineParser.getString("TYPE"));
    sensor->addMeter(sp);
  }


  if( sens == "TEXTFILE" )
  {
    TQString path = lineParser.getString("PATH");
    bool rdf = lineParser.getBoolean("RDF");
    sensor = sensorMap["TEXTFILE"+path];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = ( interval == 0 )?60000:interval;
      TQString encoding = lineParser.getString("ENCODING");

      sensor = ( sensorMap["TEXTFILE"+path] =
                   new TextFileSensor( path, rdf, interval, encoding ) );
      sensorList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("LINE",TQString::number(lineParser.getInt("LINE")));
    sensor->addMeter(sp);
  }


  if( sens == "TIME")
  {
    sensor = sensorMap["DATE"];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = (interval == 0)?60000:interval;
      sensor = ( sensorMap["DATE"] = new DateSensor( interval ) );
      sensorList->append( sensor );
      timeList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("FORMAT",
                 m_theme.locale()->translate(lineParser.getString("FORMAT").ascii()));
    sp->addParam("CALWIDTH",lineParser.getString("CALWIDTH"));
    sp->addParam("CALHEIGHT",lineParser.getString("CALHEIGHT"));
    sensor->addMeter(sp);
  }

#ifdef HAVE_XMMS

  if( sens == "XMMS" )
  {
    sensor = sensorMap["XMMS"];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = (interval == 0)?1000:interval;
      TQString encoding = lineParser.getString("ENCODING");

      sensor = ( sensorMap["XMMS"] = new XMMSSensor( interval, encoding ) );
      sensorList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("FORMAT",
                 m_theme.locale()->translate(lineParser.getString("FORMAT").ascii()));
    sensor->addMeter(sp);
    sensor->setMaxValue(sp);
  }
#endif // HAVE_XMMS


  if( sens == "NOATUN" )
  {
    sensor = sensorMap["NOATUN"];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = (interval == 0)?1000:interval;
      sensor = ( sensorMap["NOATUN"] = new NoatunSensor( interval, client ) );
      sensorList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("FORMAT",
                 m_theme.locale()->translate(lineParser.getString("FORMAT").ascii()));
    sensor->addMeter(sp);
    sensor->setMaxValue(sp);
  }

  if( sens == "PROGRAM")
  {
    TQString progName = lineParser.getString("PROGRAM");
    sensor = sensorMap["PROGRAM"+progName];
    if (sensor == 0)
    {
      int interval = lineParser.getInt("INTERVAL");
      interval = (interval == 0)?3600000:interval;
      TQString encoding = lineParser.getString("ENCODING");

      sensor = (sensorMap["PROGRAM"+progName] =
                  new ProgramSensor( progName, interval, encoding ) );
      sensorList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam( "LINE", TQString::number(lineParser.getInt("LINE")));
    sp->addParam( "THEMAPATH", m_theme.path() );
    sensor->addMeter(sp);
  }

  if( sens == "RSS" )
  {
    TQString source = lineParser.getString("SOURCE");
    TQString format =
        m_theme.locale()->translate(lineParser.getString("FORMAT").ascii());

    sensor = sensorMap["RSS"+source];
    if (sensor == 0)
    {
      int interval = lineParser.getInt( "INTERVAL");
      interval = ( interval == 0 )?60000:interval;
      TQString encoding = lineParser.getString("ENCODING");

      sensor = ( sensorMap["RSS"+source] =
                   new RssSensor( source, interval, format, encoding ) );
      sensorList->append( sensor );
    }
    SensorParams *sp = new SensorParams(meter);
    sp->addParam("SOURCE",lineParser.getString("SOURCE"));
    sensor->addMeter(sp);
  }

  if (sensor != 0)
  {
    TQTimer::singleShot( 0, sensor, TQT_SLOT(update()) );
    sensor->start();
  }
}

void karamba::slotFileChanged( const TQString & file)
{
  //kdDebug() << "fileChanged: " << file << endl;

  TQString pythonFile = m_theme.path() + "/" + m_theme.pythonModule() + ".py";

  if(file == m_theme.file() || file == pythonFile)
    reloadConfig();
}

void karamba::passMenuOptionChanged(TQString key, bool value)
{
  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->menuOptionChanged(this, key, value);
}

void karamba::setIncomingData(TQString theme, TQString obj)
{
  KarambaApplication* app = (KarambaApplication*)tqApp;

  kdDebug() << "karamba::setIncomingData " << theme << obj << endl;
   //TQByteArray data;
   //TQDataStream dataStream( data, IO_WriteOnly );
   //dataStream << theme;
   //dataStream << txt;

   //kapp->dcopClient()->send( app->dcopClient()->appId(), "KarambaIface", "themeNotify(TQString,TQString)", data );

  DCOPClient *c = kapp->dcopClient();
  if (!c->isAttached())
    c->attach();

  if(app->dcopStub())
    app->dcopStub()->setIncomingData(theme, obj);
}

void karamba::callTheme(TQString theme, TQString txt)
{
  KarambaApplication* app = (KarambaApplication*)tqApp;
  kdDebug() << "karamba::callTheme " << theme << txt << endl;
  //qWarning("karamba::callTheme");
   //TQByteArray data;
   //TQDataStream dataStream( data, IO_WriteOnly );
   //dataStream << theme;
   //dataStream << txt;

   //kapp->dcopClient()->send( app->dcopClient()->appId(), "KarambaIface", "themeNotify(TQString,TQString)", data );

  DCOPClient *c = kapp->dcopClient();
  if (!c->isAttached())
    c->attach();

  if(app->dcopStub())
    app->dcopStub()->themeNotify(theme, txt);
}

void karamba::themeNotify(TQString theme, TQString txt)
{
  kdDebug() << "karamba::themeNotify" << theme << txt << endl;
  if (pythonIface->isExtensionLoaded())
  {
      pythonIface->themeNotify(this, theme.ascii(), txt.ascii());
  }
}

void karamba::meterClicked(TQMouseEvent* e, Meter* meter)
{
  //qWarning("karamba::meterClicked");
  if (pythonIface && pythonIface->isExtensionLoaded() && haveUpdated)
  {
    int button = 0;

    if( e->button() == Qt::LeftButton )
      button = 1;
    else if( e->button() == Qt::MidButton )
      button = 2;
    else if( e->button() == Qt::RightButton )
      button = 3;

    if (RichTextLabel* richText = dynamic_cast<RichTextLabel*>(meter))
    {
      pythonIface->meterClicked(this, richText->anchorAt(e->x(), e->y()),
                                button);
    }
    else
    {
      pythonIface->meterClicked(this, meter, button);
    }
  }
}

void karamba::changeInterval(int interval)
{
  if (m_sysTimer != NULL)
    m_sysTimer->changeInterval(interval);
}

void karamba::passClick(TQMouseEvent *e)
{
  //qDebug("karamba::passClick");
  TQObjectListIt it2( *timeList ); // iterate over meters
  while ( it2 != 0 )
  {
    (( DateSensor* ) *it2)->toggleCalendar( e );
    ++it2;
  }


  // We create a temporary click list here because original
  // can change during the loop (infinite loop Bug 994359)
  TQObjectList clickListTmp(*clickList);
  TQObjectListIt it(clickListTmp);
  while (it != 0)
  {
    Meter* meter = (Meter*)(*it);
    // Check if meter is still in list
    if (clickList->containsRef(meter) && meter->click(e))
    {
      // callback
      meterClicked(e, meter);
    }
    ++it;
  }

  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded() && haveUpdated)
  {
    int button = 0;

    if( e->button() == Qt::LeftButton )
      button = 1;
    else if( e->button() == Qt::MidButton )
      button = 2;
    else if( e->button() == Qt::RightButton )
      button = 3;

    pythonIface->widgetClicked(this, e->x(), e->y(), button);
  }
}

void karamba::passWheelClick( TQWheelEvent *e )
{
  //qDebug("karamba::passWheelClick");
  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded() && haveUpdated)
  {
    int button = 0;

    if( e->delta() > 0 )
      button = 4;
    else
      button = 5;

    // We create a temporary click list here because original
    // can change during the loop (infinite loop Bug 994359)
    if (want_meter_wheel_event)
    {
      TQObjectList clickListTmp(*clickList);
      TQObjectListIt it(clickListTmp);

      TQMouseEvent fakeEvent(TQEvent::MouseButtonPress, e->pos(), e->globalPos(), button, e->state());

      while (it != 0)
      {
        Meter* meter = (Meter*)(*it);
        // Check if meter is still in list
        if (clickList->containsRef(meter) && meter->click(&fakeEvent))
        {
          if (RichTextLabel* richText = dynamic_cast<RichTextLabel*>(meter))
          {
            pythonIface->meterClicked(this, richText->anchorAt(fakeEvent.x(), fakeEvent.y()),
                                    button);
          }
          else
          {
            pythonIface->meterClicked(this, meter, button);
          }
        }
        ++it;
      }
    }

    pythonIface->widgetClicked(this, e->x(), e->y(), button);
  }
}

void karamba::management_popup( void )
{
  kpop->popup(TQCursor::pos());
}

void karamba::mousePressEvent( TQMouseEvent *e )
{
  //qDebug("karamba::mousePressEvent");
  if( e->button() == Qt::RightButton && !want_right_button )
  {
    management_popup();
  }
  else
  {
    clickPos = e->pos();
    if( toggleLocked -> isChecked() )
      passClick( e );
    if( !(onTop || managed))
      KWin::lowerWindow( winId() );
  }
}

void karamba::wheelEvent( TQWheelEvent *e )
{
  //qDebug("karamba::wheelEvent");
  passWheelClick( e );
}

void karamba::mouseReleaseEvent( TQMouseEvent *e )
{
  //qDebug("karamba::mouseReleaseEvent");
  clickPos = e->pos();
}

void karamba::mouseDoubleClickEvent( TQMouseEvent *e )
{
  //qDebug("karamba::mouseDoubleClickEvent");
  if( !toggleLocked -> isChecked() )
  {
    passClick( e );
  }
}

void karamba::keyPressEvent(TQKeyEvent *e)
{
  //qDebug("karamba::keyPressEvent");
  keyPressed(e->text(), 0);
}

void karamba::keyPressed(const TQString& s, const Meter* meter)
{
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->keyPressed(this, meter, s);
}

void karamba::mouseMoveEvent( TQMouseEvent *e )
{
  //qDebug("karamba::mouseMoveEvent");
  if( e->state() !=  0 && e->state() < 16 && !toggleLocked -> isChecked() )
  {
    move( e->globalPos() - clickPos );
  }
  else
  {
    // Change cursor over ClickArea
    TQObjectListIt it(*clickList);
    bool insideArea = false;

    while (it != 0)
    {
      insideArea = ((Meter*)(*it)) -> insideActiveArea(e -> x(), e ->y());
      if (insideArea)
      {
         break;
      }
      ++it;
    }

    if(insideArea)
    {
      if( cursor().shape() != PointingHandCursor )
        setCursor( PointingHandCursor );
    }
    else
    {
      if( cursor().shape() != ArrowCursor )
        setCursor( ArrowCursor );
    }

    TQObjectListIt image_it( *imageList);        // iterate over image sensors
    while ( image_it != 0 )
    {
      ((ImageLabel*) *image_it)->rolloverImage(e);
      ++image_it;
    }
  }

  if (pythonIface && pythonIface->isExtensionLoaded())
  {
    int button = 0;

    //Modified by Ryan Nickell (p0z3r@mail.com) 03/16/2004
    // This will work now, but only when you move at least
    // one pixel in any direction with your mouse.
    //if( e->button() == Qt::LeftButton )
    if( e->state() == Qt::LeftButton)
      button = 1;
    //else if( e->button() == Qt::MidButton )
    else if( e->state() == Qt::MidButton )
      button = 2;
    //else if( e->button() == Qt::RightButton )
    else if( e->state() == Qt::RightButton )
      button = 3;

    pythonIface->widgetMouseMoved(this, e->x(), e->y(), button);
  }
}

void karamba::closeEvent ( TQCloseEvent *  qc)
{
  //qDebug("karamba::closeEvent");
  qc->accept();
  //  close(true);
  //  delete this;
}

void karamba::paintEvent ( TQPaintEvent *e)
{
  //kdDebug() << k_funcinfo << pm.size() << endl;
  if(pm.width() == 0)
    return;
  if( !(onTop || managed))
  {
    if( lowerTimer.elapsed() > 100 )
    {
      KWin::lowerWindow( winId() );
      lowerTimer.restart();
    }
  }
  TQRect rect = e->rect();
  bitBlt(this,rect.topLeft(),&pm,rect,TQt::CopyROP);
}

void karamba::updateSensors()
{
  //qDebug("karamba::updateSensors");
  TQObjectListIt it( *sensorList ); // iterate over meters
  while ( it != 0 )
  {
    ((Sensor*) *it)->update();
    ++it;
  }
  TQTimer::singleShot( 500, this, TQT_SLOT(step()) );
}

void karamba::updateBackground(KSharedPixmap* kpm)
{
  //kdDebug() << k_funcinfo << pm.size() << endl;
  // if pm width == 0 this is the first time we come here and we should start
  // the theme. This is because we need the background before starting.
  //if(pm.width() == 0)
  if( !themeStarted )
  {
    themeStarted = true;
    start();
  }
  background = TQPixmap(*kpm);

  TQPixmap buffer = TQPixmap(size());

  pm = TQPixmap(size());
  buffer.fill(TQt::black);

  TQObjectListIt it( *imageList ); // iterate over meters
  p.begin(&buffer);
  bitBlt(&buffer,0,0,&background,0,TQt::CopyROP);

  while ( it != 0 )
  {
    if (((ImageLabel*) *it)->background == 1)
    {
      ((ImageLabel*) *it)->mUpdate(&p, 1);
    }
    ++it;
  }
  p.end();

  bitBlt(&pm,0,0,&buffer,0,TQt::CopyROP);
  background = pm;

  TQPixmap buffer2 = TQPixmap(size());

  pm = TQPixmap(size());
  buffer2.fill(TQt::black);

  TQObjectListIt it2( *meterList ); // iterate over meters
  p.begin(&buffer2);
  bitBlt(&buffer2,0,0,&background,0,TQt::CopyROP);

  while ( it2 != 0 )
  {
    ((Meter*) *it2)->mUpdate(&p);
    ++it2;
  }
  p.end();

  bitBlt(&pm,0,0,&buffer2,0,TQt::CopyROP);
  if (systray != 0)
  {
    systray->updateBackgroundPixmap(buffer2);
  }
  tqrepaint();
}

void karamba::currentDesktopChanged( int i )
{
  //qDebug("karamba::currentDesktopChanged");
  kroot->tqrepaint( true );
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->desktopChanged(this, i);
}

void karamba::currentWallpaperChanged(int i )
{
  //qDebug("karamba::currentWallpaperChanged");
  kroot->tqrepaint( true );
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->wallpaperChanged(this, i);
}

void karamba::externalStep()
{
  //kdDebug() << k_funcinfo << pm.size() << endl;
  if (widgetUpdate)
  {
    TQPixmap buffer = TQPixmap(size());

    pm = TQPixmap(size());
    buffer.fill(TQt::black);

    TQObjectListIt it( *meterList ); // iterate over meters
    p.begin(&buffer);
    bitBlt(&buffer,0,0,&background,0,TQt::CopyROP);

    while ( it != 0 )
    {
      ((Meter*) *it)->mUpdate(&p);
      ++it;
    }
    p.end();

    bitBlt(&pm,0,0,&buffer,0,TQt::CopyROP);
    tqrepaint();
  }
}

void karamba::step()
{
  //kdDebug() << k_funcinfo << pm.size() << endl;
  if (widgetUpdate && haveUpdated)
  {
    pm = TQPixmap(size());
    TQPixmap buffer = TQPixmap(size());
    buffer.fill(TQt::black);

    TQObjectListIt it( *meterList ); // iterate over meters
    p.begin(&buffer);

    bitBlt(&buffer,0,0,&background,0,TQt::CopyROP);

    while (it != 0)
    {
      ((Meter*) *it)->mUpdate(&p);
      ++it;
    }
    p.end();

    bitBlt(&pm,0,0,&buffer,0,TQt::CopyROP);
    update();
  }

  if (pythonIface && pythonIface->isExtensionLoaded())
  {
    if (haveUpdated == 0)
      pythonIface->initWidget(this);
    else
      pythonIface->widgetUpdated(this);
  }

  if (haveUpdated == 0)
    haveUpdated = 1;
}

void karamba::widgetClosed()
{
  //qDebug("karamba::widgetClosed");
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->widgetClosed(this);
}

void karamba::slotToggleLocked()
{
  //qDebug("karamba::slotToggleLocked");
  if(toggleLocked->isChecked())
  {
    toggleLocked->setIconSet(SmallIconSet("lock"));
  }
  else
  {
    toggleLocked->setIconSet(SmallIconSet("move"));
  }
}

void karamba::slotToggleFastTransforms()
{
  //qDebug("karamba::slotToggleFastTransforms");
  //    bool fastTransforms = toggleFastTransforms -> isChecked();
  //    if (toggleFastTransforms -> isChecked())
  //    {
  //     toggleFastTransforms -> setIconSet(SmallIconSet("ok"));
  //    }
  //    else
  //    {
  //     TQPixmap ok_disabled;
  //            toggleFastTransforms -> setIconSet(ok_disabled);
  //    }
  //config.setGroup("internal");
  //config.writeEntry("fastTransforms", toggleFastTransforms -> isChecked());
}


bool karamba::useSmoothTransforms()
{
  //qDebug("karamba::useSmoothTransforms");
  return !toggleFastTransforms -> isChecked();
}

void karamba::writeConfigData()
{
  //qDebug("karamba::writeConfigData");
  config -> setGroup("internal");
  config -> writeEntry("lockedPosition", toggleLocked -> isChecked() );
  config -> writeEntry("fastTransforms", toggleFastTransforms -> isChecked() );
  config -> writeEntry("desktop", desktop );
  config -> setGroup("theme");
  // Widget Position
  config -> writeEntry("widgetPosX", x());
  config -> writeEntry("widgetPosY", y());
  // Widget Size
  config -> writeEntry("widgetWidth", width());
  config -> writeEntry("widgetHeight", height());

  // write changes to DiskSensor
  config -> sync();
  //qWarning("Config File ~/.superkaramba/%s.rc written.",
  //         m_theme.name().ascii());
}

void karamba::slotToggleConfigOption(TQString key, bool value)
{
  //qDebug("karamba::slotToggleConfigOption");
  config -> setGroup("config menu");
  config -> writeEntry(key, value);
  passMenuOptionChanged(key, value);
}

void karamba::addMenuConfigOption(TQString key, TQString name)
{
  //qDebug("karamba::addMenuConfigOption");
  kpop -> setItemEnabled(THEMECONF, true);

  SignalBridge* action = new SignalBridge(TQT_TQOBJECT(this), key, menuAccColl);
  KToggleAction* confItem = new KToggleAction (name, KShortcut::null(),
                                               action, TQT_SLOT(receive()),
                                               menuAccColl, key.ascii());
  confItem -> setName(key.ascii());

  menuAccColl -> insert(confItem);

  connect(action, TQT_SIGNAL( enabled(TQString, bool) ),
          this, TQT_SLOT( slotToggleConfigOption(TQString, bool) ));

  config -> setGroup("config menu");
  confItem -> setChecked(config -> readBoolEntry(key));

  confItem -> plug(themeConfMenu);

  numberOfConfMenuItems++;
}

bool karamba::setMenuConfigOption(TQString key, bool value)
{
  //qDebug("karamba::setMenuConfigOption");
  KToggleAction* menuAction = ((KToggleAction*)menuAccColl -> action(key.ascii()));
  if (menuAction == NULL)
  {
    qWarning("Menu action %s not found.", key.ascii());
    return false;
  }
  else
  {
    menuAction -> setChecked(value);
    return true;
  }
}

bool karamba::readMenuConfigOption(TQString key)
{
  //qDebug("karamba::readMenuConfigOption");
  KToggleAction* menuAction = ((KToggleAction*)menuAccColl -> action(key.ascii()));
  if (menuAction == NULL)
  {
    qWarning("Menu action %s not found.", key.ascii());
    return false;
  }
  else
  {
    return menuAction -> isChecked();
  }
}

void karamba::passMenuItemClicked(int id)
{
  //qDebug("karamba::passMenuItemClicked");
  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded())
  {
    KPopupMenu* menu = 0;
    for(int i = 0; i < (int)menuList->count(); i++)
    {
      KPopupMenu* tmp;
      if(i==0)
      {
        tmp = (KPopupMenu*) menuList->first();
      }
      else
      {
        tmp = (KPopupMenu*) menuList->next();
      }
      if(tmp != 0)
      {
        if(tmp->isItemVisible(id))
        {
          menu = tmp;
          break;
        }
      }
    }
    pythonIface->menuItemClicked(this, menu, id);
  }
}

void karamba::activeTaskChanged(Task* t)
{
  //qDebug("karamba::activeTaskChanged");
  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->activeTaskChanged(this, t);
}

void karamba::taskAdded(Task* t)
{
  //qDebug("karamba::taskAdded");
  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->taskAdded(this, t);
}

void karamba::taskRemoved(Task* t)
{
  //qDebug("karamba::taskRemoved");
  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->taskRemoved(this, t);
}

void karamba::startupAdded(Startup* t)
{
  //qDebug("karamba::startupAdded");
  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->startupAdded(this, t);
}

void karamba::startupRemoved(Startup* t)
{
  //qDebug("karamba::startupRemoved");
  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->startupRemoved(this, t);
}

void  karamba::processExited (KProcess* proc)
{
  //qDebug("karamba::processExited");
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->commandFinished(this, (int)proc->pid());
}

void  karamba::receivedStdout (KProcess *proc, char *buffer, int)
{
  //qDebug("karamba::receivedStdout");
  //Everything below is to call the python callback function
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->commandOutput(this, (int)proc->pid(), buffer);
}

//For KDE session management
void karamba::saveProperties(KConfig* config)
{
  //qDebug("karamba::saveProperties");
  config->setGroup("session");
  config->writeEntry("theme", m_theme.file());
  writeConfigData();
}

//For KDE session management
void karamba::readProperties(KConfig* config)
{
  //qDebug("karamba::readProperties");
  config->setGroup("session");
  TQString atheme = config->readEntry("theme");
}

//Register types of events that can be dragged on our widget
void karamba::dragEnterEvent(TQDragEnterEvent* event)
{
  //qDebug("karamba::dragEnterEvent");
  event->accept(TQTextDrag::canDecode(event));
}

//Handle the drop part of a drag and drop event.
void karamba::dropEvent(TQDropEvent* event)
{
  //qDebug("karamba::dropEvent");
  TQString text;

  if ( TQTextDrag::decode(event, text) )
  {
    //Everything below is to call the python callback function
    if (pythonIface && pythonIface->isExtensionLoaded())
    {
      const TQPoint &p = event->pos();
      pythonIface->itemDropped(this, text, p.x(), p.y());
    }
  }
}

void karamba::toDesktop(int id, int menuid)
{
  //qDebug("karamba::toDesktop");
  int i;

  desktop = id;
  for (i=0; ; i++)
  {
    int mid = toDesktopMenu->idAt(i);
    if (mid == -1)
      break;

    toDesktopMenu->setItemChecked(mid, false);
  }
  toDesktopMenu->setItemChecked(menuid, true);

  if (desktop)
    info->setDesktop( desktop);
  else
    info->setDesktop( NETWinInfo::OnAllDesktops );
}

void karamba::systrayUpdated()
{
  //qDebug("karamba::systrayUpdated");
  if (pythonIface && pythonIface->isExtensionLoaded())
    pythonIface->systrayUpdated(this);
}

void karamba::toggleWidgetUpdate( bool b)
{
  //qDebug("karamba::toggleWidgetUpdate");
  if (pythonIface && pythonIface->isExtensionLoaded())
    widgetUpdate = b;
}

SignalBridge::SignalBridge(TQObject* parent, TQString name, KActionCollection* ac)
  : TQObject(parent, name.ascii()), collection(ac)
{
  setName(name.ascii());
}

void SignalBridge::receive()
{
  emit enabled(name(), ((KToggleAction*)collection -> action(name())) ->
isChecked());
}

DesktopChangeSlot::DesktopChangeSlot(TQObject *parent, int id)
    : TQObject(parent, "")
{
  desktopid = id;
}

void DesktopChangeSlot::receive()
{
  karamba *k = (karamba *)parent();

  // XXX - check type cast

  k->toDesktop(desktopid, menuid);
}

void DesktopChangeSlot::setMenuId(int id)
{
  menuid = id;
}

int DesktopChangeSlot::menuId()
{
  return menuid;
}

void karamba::showMenuExtension()
{
  kglobal = new KPopupMenu(this);

  trayMenuToggleId = kglobal->insertItem(SmallIconSet("superkaramba"),
                                         i18n("Show System Tray Icon"), this,
                                         TQT_SLOT(slotToggleSystemTray()),
                                         CTRL+Key_S);

  trayMenuThemeId = kglobal->insertItem(SmallIconSet("knewstuff"),
                                        i18n("&Manage Themes..."), this,
                                        TQT_SLOT(slotShowTheme()), CTRL+Key_M);

  trayMenuQuitId = kglobal->insertItem(SmallIconSet("exit"),
                                       i18n("&Quit SuperKaramba"), this,
                                       TQT_SLOT(slotQuit()), CTRL+Key_Q);

  kglobal->polish();

  trayMenuSeperatorId = kpop->insertSeparator();
  kpop->insertItem("SuperKaramba", kglobal);
}

void karamba::hideMenuExtension()
{
  if(kglobal)
  {
    kpop->removeItem(trayMenuSeperatorId);
    kglobal->removeItem(trayMenuToggleId);
    kglobal->removeItem(trayMenuThemeId);
    kglobal->removeItem(trayMenuQuitId);

    delete kglobal;
    kglobal = 0;
  }
}

void karamba::slotToggleSystemTray()
{
  karambaApp->globalHideSysTray(false);
}

void karamba::slotQuit()
{
  karambaApp->globalQuitSuperKaramba();
}

void karamba::slotShowTheme()
{
  karambaApp->globalShowThemeDialog();
}

void karamba::setAlwaysOnTop(bool stay)
{
    if(stay)
    {
        onTop = true;
        KWin::setState( winId(), NET::KeepAbove );
    }
    else
    {
        onTop = false;
        KWin::setState( winId(), NET::KeepBelow );
    }
}

#include "karamba.moc"
