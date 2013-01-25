/*
 * Copyright (C) 2003 Hans Karlsson <karlsson.h@home.se>
 * Copyright (C) 2004,2005 Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 * Copyright (C) 2003-2004 Adam Geitgey <adam@rootnode.org>
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

#ifndef _KARAMBA_H_
#define _KARAMBA_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <tqwidget.h>
#include <kapplication.h>

#include <twinmodule.h>
#include <twin.h>

#include <tqfile.h>
#include <kfile.h>
#include <tqfileinfo.h>
#include <kaction.h>
#include <tqtimer.h>
#include <tqpixmap.h>
#include <tqpainter.h>

//#include <krootpixmap.h>

#include <tqregexp.h>
#include <tqlabel.h>
#include <tqobjectlist.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <ksharedpixmap.h>
#include <tqvaluestack.h>
#include <dcopclient.h>
#include <kpopupmenu.h>
#include <tqcursor.h>
#include <netwm.h>
#include <kiconloader.h>
#include <kfiledialog.h>
#include <tqmap.h>
#include <kurl.h>
#include <krun.h>
#include <tqdatetime.h>
#include <tqbitmap.h>
#include <kconfig.h>
#include  <kprocess.h>
#include <tqdragobject.h>

#include "karambarootpixmap.h"

#include "bar.h"
#include "textlabel.h"
#include "imagelabel.h"
#include "graph.h"
#include "input.h"

#include "clickarea.h"

#include "sensorparams.h"
#include "memsensor.h"
#include "datesensor.h"
#include "uptimesensor.h"
#include "memsensor.h"
#include "cpusensor.h"
#include "networksensor.h"
#include "xmmssensor.h"
#include "noatunsensor.h"
#include "programsensor.h"
#include "disksensor.h"
#include "sensorsensor.h"
#include "textfilesensor.h"

#include "clickmap.h"
#include "rsssensor.h"
//#include "clickable.h"
#include "taskmanager.h"
#include "showdesktop.h"
#include "systemtray.h"
#include "themefile.h"

/**
 * @short Application Main Window
 * @author Adam Geitgey <adam@rootnode.org>
 * @author Hans Karlsson <karlsson.h@home.se>
 * @author Luke Kenneth Casson Leighton <lkcl@lkcl.net>
 * @version 0.26
 */

class KarambaPython;
class LineParser;

class karamba :  public TQWidget
{
    Q_OBJECT
  

public:
    karamba(TQString fn, TQString name, bool reloading = false,
            int instance = -1, bool sub_theme = false);
    TQObjectList *menuList;

    virtual ~karamba();
    const ThemeFile& theme() const { return m_theme; };

    TQObjectList *meterList;
    TQObjectList *imageList;
    TQObjectList *clickList;
    void setSensor(const LineParser& lineParser, Meter* meter);
    TQString getSensor(Meter* meter);
    TQString findSensorFromMap(Sensor* sensor);
    void deleteMeterFromSensors(Meter* meter);
    Sensor* findSensorFromList(Meter* meter);
    KPopupMenu* keditpop;
    KPopupMenu *kpop;
    TQBitmap* widgetMask;
    KarambaRootPixmap *kroot;
    TaskManager taskManager;
    Systemtray* systray;
    TDEProcess* currProcess;
    bool useSmoothTransforms();

    void changeInterval(int interval);
    void setWidgetUpdate(bool wu) { widgetUpdate = wu; };
    bool getWidgetUpdate() { return widgetUpdate; };
    bool hasMeter(Meter* meter) { return meterList->containsRef(meter) > 0; };
    char getTempUnit() { return tempUnit; };
    void addMenuConfigOption(TQString key, TQString name);
    bool setMenuConfigOption(TQString key, bool value);
    bool readMenuConfigOption(TQString key);
    void writeConfigData();
    TextField* getDefaultTextProps() { return defaultTextField; };
    int instance() const { return m_instance; };
    void setInstance(int instance) { m_instance = instance; };
    void closeTheme(bool reloading = false);
    void keyPressed(const TQString& s, const Meter* meter);

    int numberOfConfMenuItems;
    TDEConfig* config;
    TQString prettyName;
    bool m_sub_theme;
    bool isSubTheme() { return m_sub_theme; }

    void toggleWidgetUpdate( bool );

    KWinModule*    kWinModule;

    TQString incomingData;
    TQString getIncomingData() { return incomingData; }
    void _setIncomingData(TQString data) { incomingData = data; }
    void setIncomingData(TQString theme, TQString data);

    void themeNotify(TQString theme, TQString txt);
    void callTheme(TQString theme, TQString txt);

    double getUpdateTime() { return update_time; }
    void setUpdateTime(double time) { update_time = time; }

    void makeActive();
    void makePassive();

    void showMenuExtension();
    void hideMenuExtension();

protected:
    void mousePressEvent( TQMouseEvent *);
    void wheelEvent( TQWheelEvent *);
    void mouseReleaseEvent( TQMouseEvent *);
    void mouseDoubleClickEvent( TQMouseEvent *);
    void mouseMoveEvent( TQMouseEvent *);
    void keyPressEvent ( TQKeyEvent * e );
    void closeEvent ( TQCloseEvent *);
    void paintEvent ( TQPaintEvent *);
    void saveProperties(TDEConfig *);
    void readProperties(TDEConfig *);
    void dragEnterEvent(TQDragEnterEvent* event);
    void dropEvent(TQDropEvent* event);

private:
    bool widgetUpdate;
    bool repaintInProgress;
    //bool reloading;
    bool want_right_button;
    bool want_meter_wheel_event;

    NETWinInfo* info;
    bool onTop;
    bool managed;
    bool fixedPosition;
    bool haveUpdated;
    char tempUnit;
    double update_time;
    int m_instance;

    bool parseConfig();

    void passClick( TQMouseEvent* );
    void passWheelClick( TQWheelEvent* );
    void meterClicked(TQMouseEvent*, Meter*);

    TQMap<TQString, Sensor*> sensorMap;
    TQObjectList *sensorList;
    TQObjectList *timeList;

    TQTime lowerTimer;
    // use only the first occurance of KARAMBA in a config file
    bool foundKaramba;

    KPopupMenu* themeConfMenu;
    KPopupMenu* toDesktopMenu;
    KPopupMenu* kglobal;

    DCOPClient *client;
    TQCString appId;

    TQPixmap pm;
    TQPixmap background;
    TQPainter p;

    TQPoint clickPos;
    KActionCollection* accColl;
    KActionCollection* menuAccColl;
    KToggleAction *toggleLocked;
    // use highquality scale and rotate algorithms
    KToggleAction *toggleFastTransforms;

    // Python module references
    KarambaPython* pythonIface;
    TextField *defaultTextField;

    int  desktop;
    ThemeFile m_theme;

  int trayMenuSeperatorId;
  int trayMenuQuitId;
  int trayMenuToggleId;
  int trayMenuThemeId;
  void start();

public slots:
    void step();
    void externalStep();
    void widgetClosed();
    void updateSensors();
    void currentDesktopChanged(int);
    void currentWallpaperChanged(int);
    void slotToggleConfigOption(TQString key, bool);
    void updateBackground(KSharedPixmap*);
    void passMenuOptionChanged(TQString key, bool);
    void passMenuItemClicked(int);
    void processExited (TDEProcess *proc);
    void receivedStdout (TDEProcess *proc, char *buffer, int buflen);
    void toDesktop(int desktopid, int menuid);
    const char *getPrettyName() { return prettyName.ascii(); }

    // Systray
    void systrayUpdated();

    // Task Manager
    void startupAdded(Startup*);
    void startupRemoved(Startup*);

    void taskAdded(Task*);
    void taskRemoved(Task*);
    void activeTaskChanged(Task*);
    void reloadConfig();

    void setAlwaysOnTop(bool stay);

    /**
     * If true, then when a right button is pressed on the theme,
     * the theme's python widgetMouseMoved function is called.
     */
    void setWantRightButton(bool yesno) { want_right_button = yesno; }

    void setWantMeterWheelEvent(bool yesno) { want_meter_wheel_event = yesno; }

    /**
     * can be used to fire up the karamba management popup menu
     */
    void management_popup( void );

private:
    bool m_reloading;
    bool themeStarted;
    TQTimer *m_sysTimer;
    int m_interval;

private slots:
    void initPythonInterface();
    void killWidget();
    void editConfig();
    void editScript();
    void slotToggleLocked();
    void slotToggleFastTransforms();
    void popupNotify(int);
    void slotFileChanged( const TQString & );

    void slotToggleSystemTray();
    void slotQuit();
    void slotShowTheme();
};

/*
 * Slot to receive the event of moving the karamba object
 * to a new desktop. Generated by karamba::toDesktopMenu items
 */
class DesktopChangeSlot : public TQObject
{
  Q_OBJECT
  

  public:
  DesktopChangeSlot(TQObject *parent, int desktop_id);
  /* Parent should be the karamba object
   * desktop id of 0 indicates all desktops */
  void setMenuId(int id);
  int menuId();

  public slots:
      void receive();

 protected:
  int desktopid;
  int menuid;
};

/** SignalBridge is an ungulate that lives in the forests of wild Wisconsin. */
class SignalBridge : public TQObject
{
  Q_OBJECT
  

  public:
    SignalBridge(TQObject* parent, TQString, KActionCollection*);

  signals:
    void enabled(TQString, bool);

  public slots:
    void receive();

  private:
    KActionCollection* collection;
};

#endif // _KARAMBA_H_
