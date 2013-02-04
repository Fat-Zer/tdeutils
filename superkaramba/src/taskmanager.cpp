/*****************************************************************

Copyright (c) 2000 Matthias Elter <elter@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <kglobal.h>
#include <klocale.h>
#include <kdebug.h>
#include <tdeconfig.h>
#include <kiconloader.h>
#include <twinmodule.h>
#include <netwm.h>
#include <tqtimer.h>
#include <tqimage.h>

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

#include "taskmanager.h"
#include "taskmanager.moc"

template class TQPtrList<Task>;

// Hack: create a global KWinModule without a parent. We
// can't make it a child of TaskManager because more than one
// TaskManager might be created. We can't make it a class
// variable without changing Task, which also uses it.
// So, we'll leak a little memory, but it's better than crashing.
// The real problem is that KWinModule should be a singleton.
KWinModule* twin_module = 0;

TaskManager::TaskManager(TQObject *parent, const char *name)
    : TQObject(parent, name), _active(0), _startup_info( NULL )
{

    twin_module = new KWinModule();

//    TDEGlobal::locale()->insertCatalogue("libtaskmanager");
    connect(twin_module, TQT_SIGNAL(windowAdded(WId)), TQT_SLOT(windowAdded(WId)));
    connect(twin_module, TQT_SIGNAL(windowRemoved(WId)), TQT_SLOT(windowRemoved(WId)));
    connect(twin_module, TQT_SIGNAL(activeWindowChanged(WId)), TQT_SLOT(activeWindowChanged(WId)));
    connect(twin_module, TQT_SIGNAL(currentDesktopChanged(int)), TQT_SLOT(currentDesktopChanged(int)));
    connect(twin_module, TQT_SIGNAL(windowChanged(WId,unsigned int)), TQT_SLOT(windowChanged(WId,unsigned int)));

    // register existing windows
    const TQValueList<WId> windows = twin_module->windows();
    TQValueList<WId>::ConstIterator end( windows.end() );
    for (TQValueList<WId>::ConstIterator it = windows.begin(); it != end; ++it )
  windowAdded(*it);

    // set active window
    WId win = twin_module->activeWindow();
    activeWindowChanged(win);

    configure_startup();
}

TaskManager::~TaskManager()
{
}

void TaskManager::configure_startup()
{
    TDEConfig c("tdelaunchrc", true);
    c.setGroup("FeedbackStyle");
    if (!c.readBoolEntry("TaskbarButton", true))
        return;
    _startup_info = new TDEStartupInfo( true, this );
    connect( _startup_info,
        TQT_SIGNAL( gotNewStartup( const TDEStartupInfoId&, const TDEStartupInfoData& )),
        TQT_SLOT( gotNewStartup( const TDEStartupInfoId&, const TDEStartupInfoData& )));
    connect( _startup_info,
        TQT_SIGNAL( gotStartupChange( const TDEStartupInfoId&, const TDEStartupInfoData& )),
        TQT_SLOT( gotStartupChange( const TDEStartupInfoId&, const TDEStartupInfoData& )));
    connect( _startup_info,
        TQT_SIGNAL( gotRemoveStartup( const TDEStartupInfoId&, const TDEStartupInfoData& )),
        TQT_SLOT( gotRemoveStartup( const TDEStartupInfoId& )));
    c.setGroup( "TaskbarButtonSettings" );
    _startup_info->setTimeout( c.readUnsignedNumEntry( "Timeout", 30 ));
}

Task* TaskManager::findTask(WId w)
{
    for (Task* t = _tasks.first(); t != 0; t = _tasks.next())
        if (t->window() == w  || t->hasTransient(w))
            return t;
    return 0;
}

#ifdef KDE_3_3
#define NET_ALL_TYPES_MASK (NET::AllTypesMask)
#else
#define NET_ALL_TYPES_MASK (-1LU)
#endif

void TaskManager::windowAdded(WId w )
{
  NETWinInfo info(tqt_xdisplay(),  w, tqt_xrootwin(),
                  NET::WMWindowType | NET::WMPid | NET::WMState );
  #ifdef KDE_3_2
  NET::WindowType windowType = info.windowType(NET_ALL_TYPES_MASK);
  #else
  NET::WindowType windowType = info.windowType();
  #endif
  // ignore NET::Tool and other special window types
  if (windowType != NET::Normal && windowType != NET::Override
      && windowType != NET::Unknown && windowType != NET::Dialog)
    return;
  // ignore windows that want to be ignored by the taskbar
  if ((info.state() & NET::SkipTaskbar) != 0)
  {
      _skiptaskbar_windows.push_front( w ); // remember them though
    return;
  }

  Window transient_for_tmp;
  if (XGetTransientForHint(tqt_xdisplay(), (Window) w, &transient_for_tmp))
  {
    WId transient_for = (WId) transient_for_tmp;

    // check if it's transient for a skiptaskbar window
    if (_skiptaskbar_windows.contains(transient_for))
      return;

    // lets see if this is a transient for an existing task
    if (transient_for != tqt_xrootwin() && transient_for != 0 )
    {
      Task* t = findTask(transient_for);
      if (t)
      {
        if (t->window() != w)
        {
          t->addTransient(w);
          // kdDebug() << "TM: Transient " << w << " added for Task: " << t->window() << endl;
        }
        return;
      }
    }
  }
  Task* t = new Task(w, this);
  _tasks.append(t);

  // kdDebug() << "TM: Task added for WId: " << w << endl;
  emit taskAdded(t);
}

void TaskManager::windowRemoved(WId w )
{
    _skiptaskbar_windows.remove( w );
    // find task
    Task* t = findTask(w);
    if (!t) return;

    if (t->window() == w) {
        _tasks.removeRef(t);

        emit taskRemoved(t);

        if(t == _active) _active = 0;
        delete t;
        //kdDebug() << "TM: Task for WId " << w << " removed." << endl;
    }
    else {
        t->removeTransient( w );
        //kdDebug() << "TM: Transient " << w << " for Task " << t->window() << " removed." << endl;
    }
}

void TaskManager::windowChanged(WId w, unsigned int dirty)
{
    if( dirty & NET::WMState ) {
        NETWinInfo info ( tqt_xdisplay(),  w, tqt_xrootwin(), NET::WMState );
        if ( (info.state() & NET::SkipTaskbar) != 0 ) {
            windowRemoved( w );
            _skiptaskbar_windows.push_front( w );
            return;
        }
        else {
            _skiptaskbar_windows.remove( w );
            if( !findTask( w ))
                windowAdded( w ); // skipTaskBar state was removed, so add this window
        }
    }

    // check if any state we are interested in is marked dirty
    if(!(dirty & (NET::WMVisibleName|NET::WMName|NET::WMState|NET::WMIcon|NET::XAWMState|NET::WMDesktop)) )
        return;

    // find task
    Task* t = findTask( w );
    if (!t) return;

    //kdDebug() << "TaskManager::windowChanged " << w << " " << dirty << endl;


    // refresh icon pixmap if necessary
    if (dirty & NET::WMIcon)
        t->refresh(true);
    else
        t->refresh();

    if(dirty & (NET::WMDesktop|NET::WMState|NET::XAWMState))
        emit windowChanged(w); // moved to different desktop or is on all or change in iconification/withdrawnnes
}

void TaskManager::activeWindowChanged(WId w )
{
    //kdDebug() << "TaskManager::activeWindowChanged" << endl;

    Task* t = findTask( w );
    if (!t) {
        if (_active) {
            _active->setActive(false);
            _active = 0;

            // there is no active window at the moment
            emit activeTaskChanged(0);
        }
    }
    else {
        if (_active)
            _active->setActive(false);

        _active = t;
        _active->setActive(true);

        emit activeTaskChanged(_active);
    }
}

void TaskManager::currentDesktopChanged(int desktop)
{
    emit desktopChanged(desktop);
}

void TaskManager::gotNewStartup( const TDEStartupInfoId& id, const TDEStartupInfoData& data )
{
    Startup* s = new Startup( id, data, this );
    _startups.append(s);

    emit startupAdded(s);
}

void TaskManager::gotStartupChange( const TDEStartupInfoId& id, const TDEStartupInfoData& data )
{
    for( Startup* s = _startups.first(); s != 0; s = _startups.next()) {
        if ( s->id() == id ) {
            s->update( data );
            return;
        }
    }
}

void TaskManager::gotRemoveStartup( const TDEStartupInfoId& id )
{
    killStartup( id );
}

void TaskManager::killStartup( const TDEStartupInfoId& id )
{
    Startup* s = 0;
    for(s = _startups.first(); s != 0; s = _startups.next()) {
        if (s->id() == id)
            break;
    }
    if (s == 0) return;

    _startups.removeRef(s);
    emit startupRemoved(s);
    delete s;
}

void TaskManager::killStartup(Startup* s)
{
    if (s == 0) return;

    _startups.removeRef(s);
    emit startupRemoved(s);
    delete s;
}

TQString TaskManager::desktopName(int desk) const
{
    return twin_module->desktopName(desk);
}

int TaskManager::numberOfDesktops() const
{
    return twin_module->numberOfDesktops();
}

bool TaskManager::isOnTop(const Task* task)
{
    if(!task) return false;

    for (TQValueList<WId>::ConstIterator it = twin_module->stackingOrder().fromLast();
         it != twin_module->stackingOrder().end(); --it ) {
  for (Task* t = _tasks.first(); t != 0; t = _tasks.next() ) {
      if ( (*it) == t->window() ) {
    if ( t == task )
        return true;
    if ( !t->isIconified() && (t->isAlwaysOnTop() == task->isAlwaysOnTop()) )
        return false;
    break;
      }
  }
    }
    return false;
}


Task::Task(WId win, TaskManager * parent, const char *name) :
  TQObject(parent, name),
  _active(false), _win(win),
  _lastWidth(0), _lastHeight(0), _lastResize(false), _lastIcon(),
  _thumbSize(0.2), _thumb(), _grab()
{
#ifdef KDE_3_2
  _info = KWin::windowInfo(_win, 0, 0);
#else
  _info = KWin::info(_win);
#endif
  // try to load icon via net_wm
  _pixmap = KWin::icon(_win, 16, 16, true);

  // try to guess the icon from the classhint
  if(_pixmap.isNull())
    TDEGlobal::instance()->iconLoader()->loadIcon(className().lower(),
            TDEIcon::Small,TDEIcon::Small,
            TDEIcon::DefaultState, 0, true);

  // load xapp icon
  if (_pixmap.isNull())
    _pixmap = SmallIcon("kcmx");
}

Task::~Task()
{
}

void Task::refresh(bool icon)
{
#ifdef KDE_3_2
  _info = KWin::windowInfo(_win, 0, 0);
#else
  _info = KWin::info(_win);
#endif
  if (icon)
  {
    // try to load icon via net_wm
    _pixmap = KWin::icon(_win, 16, 16, true);

    // try to guess the icon from the classhint
    if(_pixmap.isNull())
    {
        TDEGlobal::instance()->iconLoader()->loadIcon(className().lower(),
            TDEIcon::Small, TDEIcon::Small, TDEIcon::DefaultState, 0, true);
    }

    // load xapp icon
    if (_pixmap.isNull())
      _pixmap = SmallIcon("kcmx");

    _lastIcon.resize(0,0);
    emit iconChanged();
  }
  emit changed();
}

void Task::setActive(bool a)
{
    _active = a;
    emit changed();
    if ( a )
      emit activated();
    else
      emit deactivated();
}

bool Task::isMaximized() const
{
#ifdef KDE_3_2
  return(_info.state() & NET::Max);
#else
  return(_info.state & NET::Max);
#endif
}

bool Task::isIconified() const
{
#ifdef KDE_3_2
  return (_info.mappingState() == NET::Iconic);
#else
  return (_info.mappingState == NET::Iconic);
#endif
}

bool Task::isAlwaysOnTop() const
{
#ifdef KDE_3_2
  return (_info.state() & NET::StaysOnTop);
#else
  return (_info.state & NET::StaysOnTop);
#endif
}

bool Task::isShaded() const
{
#ifdef KDE_3_2
  return (_info.state() & NET::Shaded);
#else
  return (_info.state & NET::Shaded);
#endif
}

bool Task::isOnCurrentDesktop() const
{
#ifdef KDE_3_2
  return (_info.onAllDesktops() || _info.desktop() == twin_module->currentDesktop());
#else
  return (_info.onAllDesktops || _info.desktop == twin_module->currentDesktop());
#endif
}

bool Task::isOnAllDesktops() const
{
#ifdef KDE_3_2
  return _info.onAllDesktops();
#else
  return _info.onAllDesktops;
#endif
}

bool Task::isActive() const
{
  return _active;
}

bool Task::isOnTop() const
{
  return taskManager()->isOnTop( this );
}

bool Task::isModified() const
{
  static TQString modStr = TQString::fromUtf8("[") + i18n("modified") + TQString::fromUtf8("]");
#ifdef KDE_3_2
  int modStrPos = _info.visibleName().find(modStr);
#else
  int modStrPos = _info.visibleName.find(modStr);
#endif

  return ( modStrPos != -1 );
}

TQString Task::iconName() const
{
    NETWinInfo ni( tqt_xdisplay(),  _win, tqt_xrootwin(), NET::WMIconName);
    return TQString::fromUtf8(ni.iconName());
}
TQString Task::visibleIconName() const
{
    NETWinInfo ni( tqt_xdisplay(),  _win, tqt_xrootwin(), NET::WMVisibleIconName);
    return TQString::fromUtf8(ni.visibleIconName());
}

TQString Task::className()
{
    XClassHint hint;
    if(XGetClassHint(tqt_xdisplay(), _win, &hint)) {
  TQString nh( hint.res_name );
  XFree( hint.res_name );
  XFree( hint.res_class );
  return nh;
    }
    return TQString();
}

TQString Task::classClass()
{
    XClassHint hint;
    if(XGetClassHint(tqt_xdisplay(), _win, &hint)) {
  TQString ch( hint.res_class );
  XFree( hint.res_name );
  XFree( hint.res_class );
  return ch;
    }
    return TQString();
}

TQPixmap Task::icon( int width, int height, bool allowResize )
{
  if ( (width == _lastWidth)
       && (height == _lastHeight)
       && (allowResize == _lastResize )
       && (!_lastIcon.isNull()) )
    return _lastIcon;

  TQPixmap newIcon = KWin::icon( _win, width, height, allowResize );
  if ( !newIcon.isNull() ) {
    _lastIcon = newIcon;
    _lastWidth = width;
    _lastHeight = height;
    _lastResize = allowResize;
  }

  return newIcon;
}

TQPixmap Task::bestIcon( int size, bool &isStaticIcon )
{
  TQPixmap pixmap;
  isStaticIcon = false;

  switch( size ) {
  case TDEIcon::SizeSmall:
    {
      pixmap = icon( 16, 16, true  );

      // Icon of last resort
      if( pixmap.isNull() ) {
  pixmap = TDEGlobal::iconLoader()->loadIcon( "go",
              TDEIcon::NoGroup,
              TDEIcon::SizeSmall );
  isStaticIcon = true;
      }
    }
    break;
  case TDEIcon::SizeMedium:
    {
      //
      // Try 34x34 first for KDE 2.1 icons with shadows, if we don't
      // get one then try 32x32.
      //
      pixmap = icon( 34, 34, false  );

      if ( ( pixmap.width() != 34 ) || ( pixmap.height() != 34 ) ) {
  if ( ( pixmap.width() != 32 ) || ( pixmap.height() != 32 ) ) {
    pixmap = icon( 32, 32, true  );
  }
      }

      // Icon of last resort
      if( pixmap.isNull() ) {
  pixmap = TDEGlobal::iconLoader()->loadIcon( "go",
              TDEIcon::NoGroup,
              TDEIcon::SizeMedium );
  isStaticIcon = true;
      }
    }
    break;
  case TDEIcon::SizeLarge:
    {
      // If there's a 48x48 icon in the hints then use it
      pixmap = icon( size, size, false  );

      // If not, try to get one from the classname
      if ( pixmap.isNull() || pixmap.width() != size || pixmap.height() != size ) {
  pixmap = TDEGlobal::iconLoader()->loadIcon( className(),
              TDEIcon::NoGroup,
              size,
              TDEIcon::DefaultState,
              0L,
              true );
  isStaticIcon = true;
      }

      // If we still don't have an icon then scale the one in the hints
      if ( pixmap.isNull() || ( pixmap.width() != size ) || ( pixmap.height() != size ) ) {
  pixmap = icon( size, size, true  );
  isStaticIcon = false;
      }

      // Icon of last resort
      if( pixmap.isNull() ) {
  pixmap = TDEGlobal::iconLoader()->loadIcon( "go",
              TDEIcon::NoGroup,
              size );
  isStaticIcon = true;
      }
    }
  }

  return pixmap;
}

bool Task::idMatch( const TQString& id1, const TQString& id2 )
{
  if ( id1.isEmpty() || id2.isEmpty() )
    return false;

  if ( id1.contains( id2 ) > 0 )
    return true;

  if ( id2.contains( id1 ) > 0 )
    return true;

  return false;
}


void Task::maximize()
{
  NETWinInfo ni( tqt_xdisplay(),  _win, tqt_xrootwin(), NET::WMState);
  ni.setState( NET::Max, NET::Max );

#ifdef KDE_3_2
  if (_info.mappingState() == NET::Iconic)
#else
  if (_info.mappingState == NET::Iconic)
#endif
    activate();
}

void Task::restore()
{
  NETWinInfo ni( tqt_xdisplay(),  _win, tqt_xrootwin(), NET::WMState);
  ni.setState( 0, NET::Max );
#ifdef KDE_3_2
  if (_info.mappingState() == NET::Iconic)
#else
  if (_info.mappingState == NET::Iconic)
#endif
    activate();
}

void Task::iconify()
{
  XIconifyWindow( tqt_xdisplay(), _win, tqt_xscreen() );
}

void Task::close()
{
    NETRootInfo ri( tqt_xdisplay(),  NET::CloseWindow );
    ri.closeWindowRequest( _win );
}

void Task::raise()
{
//    kdDebug(1210) << "Task::raise(): " << name() << endl;
    XRaiseWindow( tqt_xdisplay(), _win );
}

void Task::lower()
{
//    kdDebug(1210) << "Task::lower(): " << name() << endl;
    XLowerWindow( tqt_xdisplay(), _win );
}

void Task::activate()
{
//    kdDebug(1210) << "Task::activate():" << name() << endl;
    NETRootInfo ri( tqt_xdisplay(), 0 );
    ri.setActiveWindow( _win );
}

void Task::activateRaiseOrIconify()
{
    if ( !isActive() || isIconified() ) {
        activate();
    } else if ( !isOnTop() ) {
       raise();
    } else {
       iconify();
    }
}

void Task::toDesktop(int desk)
{
  NETWinInfo ni(tqt_xdisplay(), _win, tqt_xrootwin(), NET::WMDesktop);
  if (desk == 0)
  {
#ifdef KDE_3_2
    if (_info.onAllDesktops())
    {
        ni.setDesktop(twin_module->currentDesktop());
        KWin::forceActiveWindow(_win);
    }
#else
    if (_info.onAllDesktops)
    {
        ni.setDesktop(twin_module->currentDesktop());
        KWin::setActiveWindow(_win);
    }
#endif
    else
        ni.setDesktop(NETWinInfo::OnAllDesktops);
    return;
  }
  ni.setDesktop(desk);
  if (desk == twin_module->currentDesktop())
#ifdef KDE_3_2
    KWin::forceActiveWindow(_win);
#else
    KWin::setActiveWindow(_win);
#endif
}

void Task::toCurrentDesktop()
{
    toDesktop(twin_module->currentDesktop());
}

void Task::setAlwaysOnTop(bool stay)
{
    NETWinInfo ni( tqt_xdisplay(),  _win, tqt_xrootwin(), NET::WMState);
    if(stay)
        ni.setState( NET::StaysOnTop, NET::StaysOnTop );
    else
        ni.setState( 0, NET::StaysOnTop );
}

void Task::toggleAlwaysOnTop()
{
    setAlwaysOnTop( !isAlwaysOnTop() );
}

void Task::setShaded(bool shade)
{
    NETWinInfo ni( tqt_xdisplay(),  _win, tqt_xrootwin(), NET::WMState);
    if(shade)
        ni.setState( NET::Shaded, NET::Shaded );
    else
        ni.setState( 0, NET::Shaded );
}

void Task::toggleShaded()
{
    setShaded( !isShaded() );
}

void Task::publishIconGeometry(TQRect rect)
{
    NETWinInfo ni( tqt_xdisplay(),  _win, tqt_xrootwin(), NET::WMIconGeometry);
    NETRect r;
    r.pos.x = rect.x();
    r.pos.y = rect.y();
    r.size.width = rect.width();
    r.size.height = rect.height();
    ni.setIconGeometry(r);
}

void Task::updateThumbnail()
{
  if ( !isOnCurrentDesktop() )
    return;
  if ( !isActive() )
    return;
  if ( !_grab.isNull() ) // We're already processing one...
    return;

   //
   // We do this as a two stage process to remove the delay caused
   // by the thumbnail generation. This makes things much smoother
   // on slower machines.
   //
  TQWidget *rootWin = TQT_TQWIDGET(tqApp->desktop());
  TQRect geom = _info.geometry();
   _grab = TQPixmap::grabWindow( rootWin->winId(),
        geom.x(), geom.y(),
        geom.width(), geom.height() );

   if ( !_grab.isNull() )
     TQTimer::singleShot( 200, this, TQT_SLOT( generateThumbnail() ) );
}

void Task::generateThumbnail()
{
   if ( _grab.isNull() )
      return;

   TQImage img = _grab.convertToImage();

   double width = img.width();
   double height = img.height();
   width = width * _thumbSize;
   height = height * _thumbSize;

   img = img.smoothScale( (int) width, (int) height );
   _thumb = img;
   _grab.resize( 0, 0 ); // Makes grab a null image.

   emit thumbnailChanged();
}

Startup::Startup( const TDEStartupInfoId& id, const TDEStartupInfoData& data,
    TQObject * parent, const char *name)
    : TQObject(parent, name), _id( id ), _data( data )
{
}

Startup::~Startup()
{

}

void Startup::update( const TDEStartupInfoData& data )
{
    _data.update( data );
    emit changed();
}

int TaskManager::currentDesktop() const
{
    return twin_module->currentDesktop();
}
