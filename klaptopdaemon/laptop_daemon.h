#ifndef LAPTOPDAEMON
#define LAPTOPDAEMON 1
/*
 * laptop_daemon.h
 * Copyright (C) 1999 Paul Campbell <paul@taniwha.com>
 *
 * This file contains the implementation of the main laptop battery monitoring daemon
 *
 * $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "daemon_state.h"

#include <qdir.h>
#include <unistd.h>
#include <time.h>
#include <qmovie.h>
#include <qptrlist.h>
#include <qfileinfo.h>
#include <qimage.h>

#include <kiconloader.h>
#include <kprocess.h>
//#include <kaudio.h>
#include <qtooltip.h>

#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>

#include <qsocketnotifier.h>



#include <kdebug.h>
#include <qthread.h>

#include "kpcmcia.h"

#include "daemondock.h"
#include "xautolock.h"
#include <kdedmodule.h>


class laptop_daemon;
class XWidget;

class ButtonThread : public QThread {
public:
	ButtonThread() { quitting = 0; }
	void sethandle(laptop_daemon *h) { handle = h; }
	void quit() { quitting = 1; }				// make it quit
	void done() { while (!finished()) msleep(100); quitting = 0; }// wait 'till it's done
	virtual void run();
private:
	bool quitting;
	laptop_daemon *handle;
};

class laptop_dock;
class laptop_daemon: public KDEDModule
{
	Q_OBJECT
	K_DCOP
public:
    	laptop_daemon(const QCString& obj);
    	~laptop_daemon();
        void 	setPollInterval(const int poll=60);
	void 	SetBrightness(bool blank, int v);	// routine to do it
	int	GetBrightness() { return brightness; }
	void    SetThrottle(QString v);
	void    SetPerformance(QString v);
	void	ButtonThreadInternals();
k_dcop:
	void	restart();
	void	quit();
signals:
        void 	signal_checkBattery();
protected:
	 void 	timerEvent(QTimerEvent *);
protected slots:
	void 	checkBatteryNow();
	void 	timerDone();
	void	dock_quit();
        void    updatePCMCIA(int num);
	void 	sonyDataReceived();
	void	BackoffRestart();
	void	WakeCheck();
private:
	void 	haveBatteryLow(int t, const int num, const int type);
	int	calcBatteryTime(int percent, long time, bool restart);
	void 	start_monitor();
	void 	invokeStandby();
	void 	invokeSuspend();
	void 	invokeHibernate();
	void 	invokeShutdown();
	void 	invokeLogout();
	void	displayPixmap();
	void	setBlankSaver(bool);
        void setLoadAverage(bool enable, float value) {mLavEnabled=enable&&(value>=0.0); mLav=value;}

	laptop_dock *dock_widget;

	// Capability
	bool    hasAudio;
	//KAudio  audioServer;

	// General settings
public:
	int	val;
	int	powered;
	int	left;
	bool	x11Event(XEvent *event);
	bool	exists() { return s.exists; }
	QString noBatteryIcon() { return s.noBatteryIcon; }
	QString chargeIcon() { return s.chargeIcon; }
	QString noChargeIcon() { return s.noChargeIcon; }
protected:
	int	triggered[2];

	int	oldval, oldexists, oldpowered, oldleft, knownFullyCharged;

	int	changed;

	int     oldTimer;
	bool	timer;		// autolock timer is active

	int	power_time;

        // PCMCIA related
        KPCMCIA *_pcmcia;

	// sony jog-bar support
	
	int  sony_fd;			// file desc form open /dev/sonypi
	Display *sony_disp;		// X display
	QSocketNotifier *sony_notifier; // how we know data is waiting

	// brightness 
	
	int  brightness;		// actual brightness, -1 if not known

	bool lid_state;
	bool power_state;
	ButtonThread	buttonThread;


	//
	XAutoLock	autoLock;		// timer/UI maint
        float   mLav;
        bool    mLavEnabled;
	bool	need_wait;			// undo sleep stuff
	bool	saved_brightness, saved_throttle, saved_performance;
	int	saved_brightness_val;
	QString saved_performance_val, saved_throttle_val;
	QTimer  *wake_timer;		// the timer for the above running
	QPoint	wakepos;		// the mouse pos at the beginning of time
	void	WakeUpAuto();
	QTimer  *backoffTimer;		// backoff

	bool	power_button_off;			// imagined state of the power button
	bool 	button_bright_saved;			// saved button state
	int 	button_bright_val;
	bool	button_saved_performance;
	QString button_saved_performance_val;
	bool 	button_saved_throttle;
	QString button_saved_throttle_val;

	daemon_state	s;			// saved daemon state from config file

	XWidget 	*xwidget;
};
#endif
