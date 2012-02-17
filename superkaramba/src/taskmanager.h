/*****************************************************************

Copyright (c) 2000-2001 Matthias Elter <elter@kde.org>
Copyright (c) 2001 Richard Moore <rich@kde.org>

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

#ifndef __taskmanager_h__
#define __taskmanager_h__

#include <sys/types.h>

#include <tqpoint.h>
#include <tqobject.h>
#include <tqvaluelist.h>
#include <tqptrlist.h>
#include <tqpixmap.h>

#include <dcopobject.h>
#include <twin.h>
#include <kstartupinfo.h>
#include "karambaapp.h"

class TaskManager;

/**
 * A dynamic interface to a task (main window).
 *
 * @see TaskManager
 * @see KWinModule
 */
class Task: public TQObject
{
    Q_OBJECT
  
    TQ_PROPERTY( TQString name READ name )
    TQ_PROPERTY( TQString visibleName READ visibleName )
    TQ_PROPERTY( TQString visibleNameWithState READ visibleNameWithState )
    TQ_PROPERTY( TQString iconName READ iconName )
    TQ_PROPERTY( TQString visibleIconName READ visibleIconName )
    TQ_PROPERTY( TQPixmap pixmap READ pixmap )
    TQ_PROPERTY( bool maximized READ isMaximized )
    TQ_PROPERTY( bool iconified READ isIconified )
    TQ_PROPERTY( bool shaded READ isShaded WRITE setShaded )
    TQ_PROPERTY( bool active READ isActive )
    TQ_PROPERTY( bool onCurrentDesktop READ isOnCurrentDesktop )
    TQ_PROPERTY( bool onAllDesktops READ isOnAllDesktops )
    TQ_PROPERTY( bool alwaysOnTop READ isAlwaysOnTop WRITE setAlwaysOnTop )
    TQ_PROPERTY( bool modified READ isModified )
    TQ_PROPERTY( int desktop READ desktop )
    TQ_PROPERTY( double thumbnailSize READ thumbnailSize WRITE setThumbnailSize )
    TQ_PROPERTY( bool hasThumbnail READ hasThumbnail )
    TQ_PROPERTY( TQPixmap thumbnail READ thumbnail )

public:
    Task( WId win, TaskManager * parent, const char *name = 0 );
    virtual ~Task();

    TaskManager* taskManager() const { return (TaskManager*) parent(); }

    WId window() const { return _win; }
#ifdef KDE_3_2
    TQString name() const { return _info.name(); }
    TQString visibleName() const { return _info.visibleName(); }
    /**
     * Returns the desktop on which this task's window resides.
     */
    int desktop() const { return _info.desktop(); }
#else
    TQString name() const { return _info.name; }
    TQString visibleName() const { return _info.visibleName; }
    /**
     * Returns the desktop on which this task's window resides.
     */
    int desktop() const { return _info.desktop; }
#endif
    TQString visibleNameWithState() const { return _info.visibleNameWithState(); }
    TQString iconName() const;
    TQString visibleIconName() const;
    TQString className();
    TQString classClass();

    /**
     * A list of the window ids of all transient windows (dialogs) associated
     * with this task.
     */
    TQValueList<WId> transients() const { return _transients; }

    /**
     * Returns a 16x16 (KIcon::Small) icon for the task. This method will
     * only fall back to a static icon if there is no icon of any size in
     * the WM hints.
     */
    TQPixmap pixmap() const { return _pixmap; }

    /**
     * Returns the best icon for any of the KIcon::StdSizes. If there is no
     * icon of the specified size specified in the WM hints, it will try to
     * get one using KIconLoader.
     *
     * <pre>
     *   bool gotStaticIcon;
     *   TQPixmap icon = myTask->icon( KIcon::SizeMedium, gotStaticIcon );
     * </pre>
     *
     * @param size Any of the constants in KIcon::StdSizes.
     * @param isStaticIcon Set to true if KIconLoader was used, false otherwise.
     * @see KIcon
     */
    TQPixmap bestIcon( int size, bool &isStaticIcon );

    /**
     * Tries to find an icon for the task with the specified size. If there
     * is no icon that matches then it will either resize the closest available
     * icon or return a null pixmap depending on the value of allowResize.
     *
     * Note that the last icon is cached, so a sequence of calls with the same
     * parameters will only query the NET properties if the icon has changed or
     * none was found.
     */
    TQPixmap icon( int width, int height, bool allowResize = false );

    /**
     * Returns true iff the windows with the specified ids should be grouped
     * together in the task list.
     */
    static bool idMatch(const TQString &, const TQString &);

    // state

    /**
     * Returns true if the task's window is maximized.
     */
    bool isMaximized() const;

    /**
     * Returns true if the task's window is iconified.
     */
    bool isIconified() const;

    /**
     * Returns true if the task's window is shaded.
     */
    bool isShaded() const;

    /**
     * Returns true if the task's window is the active window.
     */
    bool isActive() const;

    /**
     * Returns true if the task's window is the topmost non-iconified,
     * non-always-on-top window.
     */
    bool isOnTop() const;

    /**
     * Returns true if the task's window is on the current virtual desktop.
     */
    bool isOnCurrentDesktop() const;

    /**
     * Returns true if the task's window is on all virtual desktops.
     */
    bool isOnAllDesktops() const;

    /**
     * Returns true if the task's window will remain at the top of the
     * stacking order.
     */
    bool isAlwaysOnTop() const;

    /**
     * Returns true if the document the task is editing has been modified.
     * This is currently handled heuristically by looking for the string
     * '[i18n_modified]' in the window title where i18n_modified is the
     * word 'modified' in the current language.
     */
    bool isModified() const ;

    // internal

    //* @internal
    void refresh(bool icon = false);
    //* @internal
    void addTransient( WId w ) { _transients.append( w ); }
    //* @internal
    void removeTransient( WId w ) { _transients.remove( w ); }
    //* @internal
    bool hasTransient( WId w ) const { return _transients.contains( w ); }
    //* @internal
    void setActive(bool a);

    // For thumbnails

    /**
     * Returns the current thumbnail size.
     */
    double thumbnailSize() const { return _thumbSize; }

    /**
     * Sets the size for the window thumbnail. For example a size of
     * 0.2 indicates the thumbnail will be 20% of the original window
     * size.
     */
    void setThumbnailSize( double size ) { _thumbSize = size; }

    /**
     * Returns true if this task has a thumbnail. Note that this method
     * can only ever return true after a call to updateThumbnail().
     */
    bool hasThumbnail() const { return !_thumb.isNull(); }

    /**
     * Returns the thumbnail for this task (or a null image if there is
     * none).
     */
    const TQPixmap &thumbnail() const { return _thumb; }

public slots:
    // actions

    /**
     * Maximise the main window of this task.
     */
    void maximize();

    /**
     * Restore the main window of the task (if it was iconified).
     */
    void restore();

    /**
     * Iconify the task.
     */
    void iconify();

    /**
     * Activate the task's window.
     */
    void close();

    /**
     * Raise the task's window.
     */
    void raise();

    /**
     * Lower the task's window.
     */
    void lower();

   /**
     * Activate the task's window.
     */
    void activate();

    /**
     * Perform the action that is most appropriate for this task. If it
     * is not active, activate it. Else if it is not the top window, raise
     * it. Otherwise, iconify it.
     */
    void activateRaiseOrIconify();

    /**
     * If true, the task's window will remain at the top of the stacking order.
     */
    void setAlwaysOnTop(bool);
    void toggleAlwaysOnTop();

    /**
     * If true then the task's window will be shaded. Most window managers
     * represent this state by displaying on the window's title bar.
     */
    void setShaded(bool);
    void toggleShaded();

    /**
     * Moves the task's window to the specified virtual desktop.
     */
    void toDesktop(int);

    /**
     * Moves the task's window to the current virtual desktop.
     */
    void toCurrentDesktop();

    /**
     * This method informs the window manager of the location at which this
     * task will be displayed when iconised. It is used, for example by the
     * KWin inconify animation.
     */
    void publishIconGeometry(TQRect);

    /**
     * Tells the task to generate a new thumbnail. When the thumbnail is
     * ready the thumbnailChanged() signal will be emitted.
     */
    void updateThumbnail();

signals:
    /**
     * Indicates that this task has changed in some way.
     */
    void changed();

    /**
     * Indicates that the icon for this task has changed.
     */
    void iconChanged();

    /**
     * Indicates that this task is now the active task.
     */
    void activated();

    /**
     * Indicates that this task is no longer the active task.
     */
    void deactivated();

    /**
     * Indicates that the thumbnail for this task has changed.
     */
    void thumbnailChanged();

protected slots:
    //* @internal
    void generateThumbnail();

private:
    bool                _active;
    WId                 _win;
    TQPixmap             _pixmap;
#ifdef KDE_3_2
    KWin::WindowInfo    _info;
#else
    KWin::Info          _info;
#endif
    TQValueList<WId>     _transients;

    int                 _lastWidth;
    int                 _lastHeight;
    bool                _lastResize;
    TQPixmap             _lastIcon;

    double _thumbSize;
    TQPixmap _thumb;
    TQPixmap _grab;

    class TaskPrivate *d;
};

/**
 * Represents a task which is in the process of starting.
 *
 * @see TaskManager
 */
class Startup: public TQObject
{
    Q_OBJECT
  
    TQ_PROPERTY( TQString text READ text )
    TQ_PROPERTY( TQString bin READ bin )
    TQ_PROPERTY( TQString icon READ icon )

public:
    Startup( const KStartupInfoId& id, const KStartupInfoData& data, TQObject * parent,
        const char *name = 0);
    virtual ~Startup();

    /**
     * The name of the starting task (if known).
     */
    TQString text() const { return _data.findName(); }

    /**
     * The name of the executable of the starting task.
     */
    TQString bin() const { return _data.bin(); }

    /**
     * The name of the icon to be used for the starting task.
     */
    TQString icon() const { return _data.findIcon(); }
    void update( const KStartupInfoData& data );
    const KStartupInfoId& id() const { return _id; }

signals:
    /**
     * Indicates that this startup has changed in some way.
     */
    void changed();

private:
    KStartupInfoId _id;
    KStartupInfoData _data;
    class StartupPrivate *d;
};

typedef TQPtrList<Task> TaskList;
typedef TQPtrList<Startup> StartupList;


/**
 * A generic API for task managers. This class provides an easy way to
 * build NET compliant task managers. It provides support for startup
 * notification, virtual desktops and the full range of WM properties.
 *
 * @see Task
 * @see Startup
 * @see KWinModule
 * @version $Id: taskmanager.h,v 1.2 2004/11/17 10:16:47 kodaaja Exp $
 */
class TaskManager : public TQObject
{
    Q_OBJECT
  
    TQ_PROPERTY( int currentDesktop READ currentDesktop )
    TQ_PROPERTY( int numberOfDesktops READ numberOfDesktops )

public:
    TaskManager( TQObject *parent = 0, const char *name = 0 );
    virtual ~TaskManager();

    /**
     * Returns a list of all current tasks. Return type changed to
     * TQPtrList in KDE 3.
     */
    TaskList tasks() const { return _tasks; }

    /**
     * Returns a list of all current startups. Return type changed to
     * TQPtrList in KDE 3.
     */
    StartupList startups() const { return _startups; }

    /**
     * Returns the name of the nth desktop.
     */
    TQString desktopName(int n) const;

    /**
     * Returns the number of virtual desktops.
     */
    int numberOfDesktops() const;

    /**
     * Returns the number of the current desktop.
     */
    int currentDesktop() const;

    /**
     * Returns true if the specified task is on top.
     */
    bool isOnTop( const Task*);
signals:
    /**
     * Emitted when the active window changed.
     */
    void activeTaskChanged(Task*);

    /**
     * Emitted when a new task has started.
     */
    void taskAdded(Task*);

    /**
     * Emitted when a task has terminated.
     */
    void taskRemoved(Task*);

    /**
     * Emitted when a new task is expected.
     */
    void startupAdded(Startup*);

    /**
     * Emitted when a startup item should be removed. This could be because
     * the task has started, because it is known to have died, or simply
     * as a result of a timeout.
     */
    void startupRemoved(Startup*);

    /**
     * Emitted when the current desktop changes.
     */
    void desktopChanged(int desktop);

    /**
     * Emitted when a window changes desktop.
     */
    void windowChanged(WId);

protected slots:
    //* @internal
    void windowAdded(WId);
    //* @internal
    void windowRemoved(WId);
    //* @internal
    void windowChanged(WId, unsigned int);

    //* @internal
    void activeWindowChanged(WId);
    //* @internal
    void currentDesktopChanged(int);
    //* @internal
    void killStartup( const KStartupInfoId& );
    //* @internal
    void killStartup(Startup*);

    //* @internal
    void gotNewStartup( const KStartupInfoId&, const KStartupInfoData& );
    //* @internal
    void gotStartupChange( const KStartupInfoId&, const KStartupInfoData& );
    //* @internal
    void gotRemoveStartup( const KStartupInfoId& );

protected:
    /**
     * Returns the task for a given WId, or 0 if there is no such task.
     */
    Task* findTask(WId w);
    void configure_startup();

private:
    Task*               _active;
    TaskList           _tasks;
    TQValueList< WId >   _skiptaskbar_windows;
    StartupList         _startups;
    KStartupInfo*       _startup_info;

    class TaskManagerPrivate *d;
};

#endif
