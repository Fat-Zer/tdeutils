/*
 * laptop_daemon.cpp
 * Copyright (C) 1999 Paul Campbell <paul@taniwha.com>
 *
 * this replaces kcmlaptop - there are 2 parts - one is the panels that
 *	put the setup configuration in the "kcmlaptop" configm, the other
 *	is the laptop_daemon (this guy) who watches the battery state
 *	and does stuff as a result
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

#include <stdlib.h>
#include <fcntl.h>

#include <qtimer.h>

#include <kconfig.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include "laptop_daemon.h"
#include "portable.h"
#include <kaboutdata.h>
#include <kaudioplayer.h>
#include <kapplication.h>
#include <sys/ioctl.h>
#include <kmessagebox.h>
#include <kpassivepopup.h>
#include <dcopclient.h>
#include <qsocketnotifier.h>
#include <qcursor.h>

#include <unistd.h>
#include <sys/time.h>

extern "C"
{
       KDE_EXPORT KDEDModule *create_klaptopdaemon(const QCString& name) {
		return new laptop_daemon(name);
        }
}


class XWidget: public QWidget {
public:
	XWidget(laptop_daemon *p):QWidget(0) { pd = p; }
private:
	bool x11Event(XEvent *event);
	laptop_daemon *pd;
};
bool XWidget::x11Event(XEvent *event) { return pd->x11Event(event); }

laptop_daemon::laptop_daemon(const QCString& obj): KDEDModule(obj)
{
	xwidget = new XWidget(this);
	xwidget->hide();
	kapp->installX11EventFilter(xwidget);

        mLavEnabled = false;
	backoffTimer = 0;
	need_wait = 0;
	saved_brightness = 0;
       	saved_throttle = 0;
       	saved_performance = 0;
	wake_timer = 0;
	button_bright_saved=0;
	button_bright_val=0;
	button_saved_performance = 0;
	button_saved_throttle = 0;
	power_button_off = 0;

	if (laptop_portable::has_brightness()) {
		brightness = laptop_portable::get_brightness();
	} else {
		brightness = 0;
	}
	buttonThread.sethandle(this);
        triggered[0] = 0;
        triggered[1] = 0;
	timer = 0;
	dock_widget = 0;
	oldTimer = 0;
	sony_fd = -1;
	sony_notifier = 0;
	knownFullyCharged = 0;
	sony_disp = 0;
        connect(this, SIGNAL(signal_checkBattery()), SLOT(checkBatteryNow()));

	//hasAudio = (audioServer.serverStatus() == 0) ? true : false;

        // FIXME: make these configurable.  Some system definitely don't
        //        use /var/run/stab
	if (!access("/var/run/stab", R_OK|F_OK))
	        _pcmcia = new KPCMCIA(8, "/var/run/stab");
	else if (!access("/var/lib/pcmcia/stab", R_OK|F_OK))
		_pcmcia = new KPCMCIA(8, "/var/lib/pcmcia/stab");
	else _pcmcia = NULL;

	if (_pcmcia)
	        connect(_pcmcia, SIGNAL(cardUpdated(int)), this, SLOT(updatePCMCIA(int)));
	connect( &autoLock, SIGNAL(timeout()), this, SLOT(timerDone()) );

}

laptop_daemon::~laptop_daemon()
{
	delete xwidget;
	delete _pcmcia;
        delete dock_widget;
        delete sony_notifier;
	if (sony_disp)
		XCloseDisplay(sony_disp);
}

void
laptop_daemon::quit()
{
	deleteLater();
}

void laptop_daemon::restart()
{
        if (oldTimer > 0) {
                killTimer(oldTimer);
		oldTimer=0;
	}
	if (timer) {
	    autoLock.stop();
	    timer = 0;
	}
	s.load();
	if (s.has_brightness) {
		brightness = laptop_portable::get_brightness();
	} else {
		brightness = 0;
	}

	if (!s.need_to_run()) {
		quit();
		return;
	}

	if (sony_fd < 0)
		sony_fd = ::open("/dev/sonypi", O_RDONLY|O_NONBLOCK);
	if (s.sony_enablescrollbar||s.sony_middleemulation) {
		if (sony_disp == 0 && sony_fd >= 0)
			sony_disp = XOpenDisplay(NULL);
		if (sony_fd < 0 || sony_disp == 0) {
			s.sony_enablescrollbar = 0;
			s.sony_middleemulation = 0;
		}
	}

	// change the dock state if necessary

	if (s.enabled) {
		if (!dock_widget) {
			dock_widget = new laptop_dock(this);
			dock_widget->setPCMCIA(_pcmcia);
  			dock_widget->show();
		}
		dock_widget->reload_icon();
		dock_widget->SetupPopup();
	} else {
		if (dock_widget) {
			delete dock_widget;
			dock_widget = 0;
		}
	}

	if (s.enable_lid_button && (lid_state = laptop_portable::get_button(laptop_portable::LidButton))) {
		if (s.button_lid_bright_enabled)
			SetBrightness(0, s.button_lid_bright_val);
		if (s.button_lid_performance_enabled)
			SetPerformance(s.button_lid_performance_val);
		if (s.button_lid_throttle_enabled)
			SetThrottle(s.button_lid_throttle_val);
		switch (s.button_lid) {
		case 1: invokeStandby();
			break;
		case 2: invokeSuspend();
			break;
		case 3: invokeHibernate();
			break;
		case 4: invokeLogout();
			break;
		case 5: invokeShutdown();
			break;
		}
	}
	if (s.enable_power_button && (power_state = laptop_portable::get_button(laptop_portable::PowerButton))) {
		if (s.button_power_bright_enabled)
			SetBrightness(0, s.button_power_bright_val);
		if (s.button_power_performance_enabled)
			SetPerformance(s.button_power_performance_val);
		if (s.button_power_throttle_enabled)
			SetThrottle(s.button_power_throttle_val);
		switch (s.button_power) {
		case 1: invokeStandby();
			break;
		case 2: invokeSuspend();
			break;
		case 3: invokeHibernate();
			break;
		case 4: invokeLogout();
			break;
		case 5: invokeShutdown();
			break;
		}
	}
	if (s.button_power_bright_enabled || s.button_power_performance_enabled || s.button_power_throttle_enabled ||
	    s.button_lid_bright_enabled || s.button_lid_performance_enabled || s.button_lid_throttle_enabled ||
	    s.button_lid != 0 || s.button_power != 0) { // need a fast thread to poll every sec
		if (!buttonThread.running()) {
			buttonThread.start();
		}
	} else {
		if (buttonThread.running()) {
			buttonThread.quit();
			buttonThread.done();
		}
	}


	// Do setup
	struct power_result p = laptop_portable::poll_battery_state();
	powered = p.powered;

	need_wait = 0;
	saved_brightness = 0;
       	saved_throttle = 0;
       	saved_performance = 0;
	if (s.power_action[0] || s.power_action[1] || s.power_brightness_enabled[0] || s.power_brightness_enabled[0] ||
	    s.power_performance_enabled[0] || s.power_performance_enabled[1] || s.power_throttle_enabled[0] || s.power_throttle_enabled[1]) {
		power_time = s.power_wait[powered?0:1];
		timer = 1;
		setLoadAverage(s.lav_enabled[powered?0:1], s.lav_val[powered?0:1]);
		autoLock.setTimeout(power_time);
		autoLock.start();
	} else {
		timer = 0;
	}


	if (s.useBlankSaver) {
		setBlankSaver(!p.powered);
	}

	start_monitor();

	// brightness control

	if (s.has_brightness) {
		if (s.bright_pon && powered) {
			SetBrightness(0, s.bright_son);
		} else
		if (s.bright_poff && !powered) {
			SetBrightness(0, s.bright_soff);
		}
	}
	if (s.has_performance) {
		if (s.performance_pon && powered) {
			SetPerformance(s.performance_val_on);
		} else
		if (s.performance_poff && !powered) {
			SetPerformance(s.performance_val_off);
		}
	}
	if (s.has_throttle) {
		if (s.throttle_pon && powered) {
			SetThrottle(s.throttle_val_on);
		} else
		if (s.throttle_poff && !powered) {
			SetThrottle(s.throttle_val_off);
		}
	}

	// sony support

	if (s.sony_enablescrollbar||s.sony_middleemulation) {
		if (sony_notifier == 0) {
			sony_notifier = new QSocketNotifier( sony_fd, QSocketNotifier::Read, this );
			if (sony_notifier)
				QObject::connect( sony_notifier, SIGNAL(activated(int)),
						                          this, SLOT(sonyDataReceived()) );
		}
	} else {
		if (sony_notifier) {
			delete sony_notifier;
			sony_notifier = 0;
		}
	}
}


void laptop_daemon::setBlankSaver(bool blanked)
{
	QByteArray ba;
	QDataStream ds(ba, IO_WriteOnly);
	ds << bool(blanked);
	// can't use kapp->dcopClient() because it breaks KUniqueApplication
	DCOPClient c;
	c.attach();
	c.send("kdesktop", "KScreensaverIface", "setBlankOnly(bool)", ba);
	c.detach();
}


void laptop_daemon::timerDone()
{
	if (mLavEnabled && laptop_portable::get_load_average() >= mLav) {
            autoLock.postpone(); // will call timerDone() again later
            return;
        }
	int action;

	timer = 0;
	autoLock.stop();		// stop - see the note below about backoff
	if (powered) {
		action = s.power_action[0];
	} else {
		action = s.power_action[1];
	}
	switch (action) {
	case 1: invokeStandby();
		break;
	case 2: invokeSuspend();
		break;
	case 3: invokeHibernate();
		break;
	}
	if ((powered?s.power_brightness_enabled[0]:s.power_brightness_enabled[1])) {
		need_wait = 1;
		if (!saved_brightness) {
			saved_brightness = 1;
			saved_brightness_val = brightness;
		}
		SetBrightness(1, powered?s.power_brightness[0]:s.power_brightness[1]);
	}
	if ((powered?s.power_performance_enabled[0]:s.power_performance_enabled[1])) {
		need_wait = 1;
		if (!saved_performance) {
			QStringList profile_list;
    			int current_profile;
			bool *active_list;
    			if (laptop_portable::get_system_performance(1, current_profile, profile_list, active_list)) {
				saved_performance = 1;
				saved_performance_val = profile_list[current_profile];
			}
		}
		SetPerformance(powered?s.power_performance[0]:s.power_performance[1]);
	}
	if ((powered?s.power_throttle_enabled[0]:s.power_throttle_enabled[1])) {
		need_wait = 1;
		if (!saved_throttle) {
			QStringList profile_list;
    			int current_profile;
			bool *active_list;
    			if (laptop_portable::get_system_throttling(1, current_profile, profile_list, active_list)) {
				saved_throttle = 1;
				saved_throttle_val = profile_list[current_profile];
			}
		}
		SetPerformance(powered?s.power_throttle[0]:s.power_throttle[1]);
	}
	//
	// we must give ourself enough time to handle any necessary evil before we start looking again
	// 	(many of the above things cause unexpected time discontinuities)
	//
	if (need_wait) {
		wakepos.setX(QCursor::pos().x());
		wakepos.setY(QCursor::pos().y());
		if (!wake_timer) {
			wake_timer = new QTimer(this);
        		connect(wake_timer, SIGNAL(timeout()), this, SLOT(WakeCheck()));
			wake_timer->start(1*1000, 1);
		}
	} else {
		if (!backoffTimer) {
			backoffTimer = new QTimer(this);
        		connect(backoffTimer, SIGNAL(timeout()), this, SLOT(BackoffRestart()));
			backoffTimer->start(60*1000, 1);
		}
	}
}

void
laptop_daemon::BackoffRestart()
{
    delete backoffTimer;
    backoffTimer = 0;
    if (!timer) {
        timer = 1;
        autoLock.start();
    }
}

void
laptop_daemon::WakeCheck()
{
	if (!wake_timer)
		return;
	if (!need_wait) {
		wake_timer->stop();
		delete wake_timer;
		wake_timer = 0;
		return;
	}
	if (wakepos.x() != QCursor::pos().x() || wakepos.y() != QCursor::pos().y()) {
		wake_timer->stop();
		delete wake_timer;
		wake_timer = 0;
		WakeUpAuto();
	}
}

void
laptop_daemon::WakeUpAuto()
{
	if (!need_wait)
		return;
	need_wait = 0;
	if (saved_brightness) {
		SetBrightness(0, saved_brightness_val);
		saved_brightness = 0;
	}
	if (saved_throttle) {
		SetThrottle(saved_throttle_val);
		saved_throttle = 0;
	}
	if (saved_performance) {
		SetPerformance(saved_performance_val);
		saved_performance = 0;
	}
	if (!timer) {
		timer = 1;
		autoLock.start();
	}
}

bool
laptop_daemon::x11Event(XEvent *event)
{
    switch (event->type) {
    case KeyPress:
    case ButtonPress:
	    if (need_wait)
	        WakeUpAuto();
	    break;

    }
    return(0);
}

void laptop_daemon::dock_quit()
{
	if (dock_widget)
		delete dock_widget;
	dock_widget = 0;
}


void laptop_daemon::updatePCMCIA(int num)
{
  Q_UNUSED(num);
  //kdDebug() << "PCMCIA card " << num << " was updated." << endl;

  // Two things we do here.  We provide notifications for cards
  // being inserted / cards going to sleep / cards waking up
  // and cards being safe to eject.
  // The second thing we do is provide the desktop icon actions
  // via dcop.
}


void laptop_daemon::haveBatteryLow(int t, const int num, const int type)
{
	displayPixmap();

	// beep if we are allowed to
	if (s.systemBeep[t]) {
		//KNotifyClient::beep();
		(void)kapp->beep();
	}

	// run a command if we have to
	if (s.runCommand[t]) {
		// make sure the command exists
		if (!s.runCommandPath[t].isEmpty()) {
			KProcess command;
			command << s.runCommandPath[t];
			command.start(KProcess::DontCare);
		}
	}

	if (s.do_brightness[t])
		SetBrightness(0, s.val_brightness[t]);
	if (s.do_throttle[t])
		SetThrottle(s.val_throttle[t]);
	if (s.do_performance[t])
		SetPerformance(s.val_performance[t]);

	// play a sound if we have to
	if (s.playSound[t]) {
		 KAudioPlayer::play(s.playSoundPath[t]);
	}


	if (s.do_hibernate[t])
		invokeHibernate();
	if (s.do_suspend[t])
		invokeSuspend();
	if (s.do_standby[t])
		invokeStandby();
	if (s.logout[t])
		invokeLogout();
	if (s.shutdown[t])
		invokeShutdown();
	// notify if we must (must be last since it's synchronous)
	if (s.notify[t]) {
            if (type) {
                if (s.time_based_action_critical) {
                    KPassivePopup::message(i18n("Battery power is running out."),
                                           i18n("%1 % charge left.").arg(num),
                                           BarIcon("laptop_battery"), dock_widget,
                                           0, 20000);
                } else {
                    // Will this ever be reached?
                    KPassivePopup::message(i18n("Battery power is running out."),
                                           i18n("%1 % charge left.").arg(num),
                                           BarIcon("laptop_battery"), dock_widget,
                                           0, 20000);
                }
            }
            else {
                if (s.time_based_action_low) {
                    KPassivePopup::message(i18n("Battery power is running out."),
                                           i18n("1 minute left.","%n minutes left.", num),
                                           BarIcon("laptop_battery"), dock_widget,
                                           0, 20000);
                } else {
                    KPassivePopup::message(i18n("Battery power is running out."),
                                           i18n("1% left.", "%n percent left.", num),
                                           BarIcon("laptop_battery"), dock_widget,
                                           0, 20000);
                }
            }
	}
}

int laptop_daemon::calcBatteryTime(int percent, long time, bool restart)
{
#define MAX_SAMPLES_USED 3
  static int percents[MAX_SAMPLES_USED];
  static long times[MAX_SAMPLES_USED];
  static int lastused=-1;
  int r=-1;

  if ( (lastused==-1) || restart )
  {
     percents[0]=percent;
     times[0]=time;
     lastused=0;
  }
  else
  {
     // Add the % and time to the arrays
     // (or just update the time if the % hasn't changed)
     if (percents[lastused]!=percent)
	if (lastused!=MAX_SAMPLES_USED-1)
	{
	   lastused++;
	   percents[lastused]=percent;
	   times[lastused]=time;
	}
	else
	{
	  for (int i=1;i<MAX_SAMPLES_USED;i++)
	  {  percents[i-1]=percents[i]; times[i-1]=times[i]; };
	}
     percents[lastused]=percent;
     times[lastused]=time;

     //Now let's do the real calculations

     if (lastused==0) return -1;

     // Copy the data to temporary variables
     double tp[MAX_SAMPLES_USED];
     double tt[MAX_SAMPLES_USED];

     for (int i=0;i<=lastused;i++)
     { tp[i]=percents[i]; tt[i]=times[i]; };

     for (int c=lastused; c>1; c--)
     {
       for (int i=0; i<c-1; i++)
       {
          tp[i]=(tp[i]+tp[i+1])/2;
          tt[i]=(tt[i]+tt[i+1])/2;
       }
     }

     // Now we've reduced all the samples to an approximation with just a line

     if (tp[1]-tp[0]!=0)
        r=static_cast<int>(tt[0]-(tp[0]/(tp[1]-tp[0]))*(tt[1]-tt[0])-time);


  }

  return r;
}

void laptop_daemon::checkBatteryNow()
{
	struct power_result p;

	p = laptop_portable::poll_battery_state();

	if (s.useBlankSaver && oldpowered != p.powered) {
		setBlankSaver(!p.powered);
	}

	powered = p.powered;
	left = p.time;
	val = p.percentage;

	if (oldpowered != powered && s.has_brightness) {
		if (s.bright_pon && powered) {
			SetBrightness(0, s.bright_son);
		} else
		if (s.bright_poff && !powered) {
			SetBrightness(0, s.bright_soff);
		}
		if (s.performance_pon && powered) {
			SetPerformance(s.performance_val_on);
		} else
		if (s.performance_poff && !powered) {
			SetPerformance(s.performance_val_off);
		}
		if (s.throttle_pon && powered) {
			SetThrottle(s.throttle_val_on);
		} else
		if (s.throttle_poff && !powered) {
			SetThrottle(s.throttle_val_off);
		}

	}

        if (left==-1)  // Let's try to calculate the expected battery time left
        {
	   timeval tv;
	   gettimeofday(&tv, 0);
	   left=calcBatteryTime(((powered)?100-val:val), tv.tv_sec, oldpowered!=powered );
        }

	if (timer && oldpowered != powered) {
		need_wait = 0;
		saved_brightness = 0;
       		saved_throttle = 0;
       		saved_performance = 0;
		setLoadAverage(s.lav_enabled[powered?0:1], s.lav_val[powered?0:1]);
		if (power_time != s.power_wait[powered?0:1]) {
			power_time = s.power_wait[powered?0:1];
			autoLock.stop();
			autoLock.setTimeout(power_time);
			autoLock.start();
		}
	}
	if (!knownFullyCharged) {
		knownFullyCharged = 1;
	} else
	if (s.notifyMeWhenFull && oldval != val && val == 100)
		KMessageBox::queuedMessageBox(0, KMessageBox::Information, i18n("Your battery is now fully charged."), i18n("Laptop Battery"));
	changed =  oldpowered != powered||oldexists != s.exists||oldval != val || oldleft!=left;
	oldpowered = powered;
	oldexists = s.exists;
	oldval = val;
	oldleft = left;
	if (changed)
		displayPixmap();
}

void laptop_daemon::start_monitor()
{
	checkBatteryNow();
	displayPixmap();
	oldTimer = startTimer(s.poll * 1000);
}

void laptop_daemon::setPollInterval(const int interval)
{
        s.poll = interval;

        // Kill any old timers that may be running
        if (oldTimer > 0) {
                killTimer(oldTimer);

                // Start a new timer will the specified time
                oldTimer = startTimer(interval * 1000);

                emit(signal_checkBattery());
        }
}

void laptop_daemon::timerEvent(QTimerEvent *)
{
        emit(signal_checkBattery());
}

void laptop_daemon::displayPixmap()
{
	if (s.have_time == 2 && s.exists && !powered) {		// in some circumstances
		s.have_time = (val < 0 ? 0 : 1);			// the battery is not charging
		KConfig *config = new KConfig("kcmlaptoprc");
		if (config) {
			config->setGroup("BatteryLow");			// we can;t figure this out 'till
			config->writeEntry("HaveTime", s.have_time);
			config->sync();
			delete config;
		}
	}

	if (dock_widget)
		dock_widget->displayPixmap();
            
	if (left >= 0) {
            if (!triggered[0]) {
                if (s.time_based_action_low) {
                    if (s.exists && !powered && left <= s.low[0]) {
                        triggered[0] = 1;
                        haveBatteryLow(0, left, 0);
                    }
                } else {
                    if (s.exists && !powered && val <= s.low[1]) {
                        triggered[0] = 1;
                        haveBatteryLow(0, val, 0);
                    }
                }
            }
            if (!triggered[1]) {
                if (s.time_based_action_critical) {
                    if (s.exists && !powered && left <= s.low[2]) {
                        triggered[1] = 1;
                        haveBatteryLow(1, left, 0);
                    }
                } else {
                    if (s.exists && !powered && val <= s.low[3]) {
                        triggered[1] = 1;
                        haveBatteryLow(1, val, 0);
                    }
                }
            }
        }

        if (s.time_based_action_low || s.time_based_action_critical) {
            if (left > (s.low[2]+1))
                triggered[1] = 0;
            if (left > s.low[0])
                triggered[0] = 0;
        } else {
            if (val > (s.low[3]+1))
                triggered[1] = 0;
            if (val > s.low[1])
                triggered[0] = 0;
        }

        if (s.have_time != 1) {
            if (!triggered[0]) {
                if (s.exists && !powered && val <= s.low[0]) {
                    triggered[0] = 1;
                    haveBatteryLow(0, val, 1);
                }
            } else {
                if (!triggered[1]) {
                    if (s.exists && !powered && val <= s.low[1]) {
                        triggered[1] = 1;
                        haveBatteryLow(1, val, 1);
                    }
                }
                if (val > (s.low[1]+1))
                    triggered[1] = 0;
                if (val > s.low[0])
                    triggered[0] = 0;
            }
        }
}

void laptop_daemon::invokeStandby()
{
	laptop_portable::invoke_standby();
}

void laptop_daemon::invokeSuspend()
{
	laptop_portable::invoke_suspend();
}

void laptop_daemon::invokeHibernate()
{
	laptop_portable::invoke_hibernation();
}

void laptop_daemon::invokeLogout()
{
	bool rc = kapp->requestShutDown(KApplication::ShutdownConfirmNo, KApplication::ShutdownTypeNone, KApplication::ShutdownModeForceNow);
	if (!rc)
		KMessageBox::sorry(0, i18n("Logout failed."));
}

void laptop_daemon::invokeShutdown()
{
	bool rc = kapp->requestShutDown(KApplication::ShutdownConfirmNo, KApplication::ShutdownTypeHalt, KApplication::ShutdownModeForceNow);
	if (!rc)
		KMessageBox::sorry(0, i18n("Shutdown failed."));
}

/*
 * Portions of the following code borrowed with thanks from:
 *
 * Sony Programmable I/O Control Device driver for VAIO.
 * Userspace X11 Daemon Utility
 *
 * Copyright 2001 Stelian Pop, Alcove
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */


static void simulateButton(Display *disp, int button) {
       XTestGrabControl(disp, True);
       XTestFakeButtonEvent(disp, button, True, 0);
       XTestFakeButtonEvent(disp, button, False, 0);
       XSync(disp, False);
       XTestGrabControl(disp, False);
}

static void simulateButtonDown(Display *disp, int button) {
       XTestGrabControl(disp, True);
       XTestFakeButtonEvent(disp, button, True, 0);
       XSync(disp, False);
       XTestGrabControl(disp, False);
}

static void simulateButtonUp(Display *disp, int button) {
        XTestGrabControl(disp, True);
	XTestFakeButtonEvent(disp, button, False, 0);
	XSync(disp, False);
	XTestGrabControl(disp, False);
}

#define SONYPI_EVENT_JOGDIAL_DOWN                1
#define SONYPI_EVENT_JOGDIAL_UP                  2
#define SONYPI_EVENT_JOGDIAL_DOWN_PRESSED        3
#define SONYPI_EVENT_JOGDIAL_UP_PRESSED          4
#define SONYPI_EVENT_JOGDIAL_PRESSED             5
#define SONYPI_EVENT_JOGDIAL_RELEASED            6

void laptop_daemon::sonyDataReceived()
{
	unsigned char event;

	if (::read(sony_fd, &event, sizeof(event)) != sizeof(event))
			return;
	switch(event) {
	case SONYPI_EVENT_JOGDIAL_UP:
		if (sony_disp && s.sony_enablescrollbar) {
			simulateButton(sony_disp, 4);
		}
		break;
	case SONYPI_EVENT_JOGDIAL_DOWN:
		if (sony_disp && s.sony_enablescrollbar) {
			simulateButton(sony_disp, 5);
		}
		break;
	case SONYPI_EVENT_JOGDIAL_PRESSED:
		if (sony_disp && s.sony_middleemulation) {
			simulateButtonDown(sony_disp, 2);
		}
		break;
	case SONYPI_EVENT_JOGDIAL_RELEASED:
		if (sony_disp && s.sony_middleemulation) {
			simulateButtonUp(sony_disp, 2);
		}
		break;
	default:
		break;
	}
}

void laptop_daemon::SetBrightness(bool blank, int v)
{
	if (v < 0)
		return;
	brightness = v;
	laptop_portable::set_brightness(blank, v);
}

void laptop_daemon::SetThrottle(QString v)
{
	laptop_portable::set_system_throttling(v);
}

void laptop_daemon::SetPerformance(QString v)
{
	laptop_portable::set_system_performance(v);
}

void
ButtonThread::run()
{
	while (!quitting) {
		handle->ButtonThreadInternals();
		msleep(500);		// have to run fast because if the power button is held down for too long
	}				// the system actually gets powered off
}

void
laptop_daemon::ButtonThreadInternals()
{
	//
	// the lid button turns stuff on when it's down and back off again when it's raised
	// 	(kind of like the fridge door light)
	//
	if (s.enable_lid_button && lid_state != laptop_portable::get_button(laptop_portable::LidButton)) {
		lid_state = !lid_state;
		if (lid_state) {
			if (s.button_lid_bright_enabled) {
				if (!button_bright_val)
					button_bright_val = brightness;
				button_bright_saved = 1;
				SetBrightness(1, s.button_lid_bright_val);
			}
			if (s.button_lid_performance_enabled) {
				if (!button_saved_performance) {
					QStringList profile_list;
    					int current_profile;
					bool *active_list;
    					if (laptop_portable::get_system_performance(1, current_profile, profile_list, active_list)) {
						button_saved_performance = 1;
						button_saved_performance_val = profile_list[current_profile];
					}
				}
				SetPerformance(s.button_lid_performance_val);
			}
			if (s.button_lid_throttle_enabled) {
				if (!button_saved_throttle) {
					QStringList profile_list;
    					int current_profile;
					bool *active_list;
    					if (laptop_portable::get_system_throttling(1, current_profile, profile_list, active_list)) {
						button_saved_throttle = 1;
						button_saved_throttle_val = profile_list[current_profile];
					}
				}
				SetThrottle(s.button_lid_throttle_val);
			}
			switch (s.button_lid) {
			case 1: invokeStandby();
				break;
			case 2: invokeSuspend();
				break;
			case 3: invokeHibernate();
				break;
			case 4: invokeLogout();
				break;
			case 5: invokeShutdown();
				break;
			}
		} else {
			if (button_bright_saved) {
				SetBrightness(0, button_bright_val);
				button_bright_saved = 0;
			}
			if (button_saved_performance) {
				button_saved_performance = 0;
				SetPerformance(	button_saved_performance_val);
			}
			if (button_saved_throttle) {
				button_saved_throttle = 0;
				SetThrottle(button_saved_throttle_val);
			}
		}
	}
	//
	// the power button on the other hand is an off/on switch for non-suspend type ops
	//
	if (s.enable_power_button && power_state != laptop_portable::get_button(laptop_portable::PowerButton)) {
		power_state = !power_state;
		if (power_state) {
			if (power_button_off) {
				if (button_bright_saved) {
					SetBrightness(0, button_bright_val);
					button_bright_saved = 0;
				}
				if (button_saved_performance) {
					button_saved_performance = 0;
					SetPerformance(	button_saved_performance_val);
				}
				if (button_saved_throttle) {
					button_saved_throttle = 0;
					SetThrottle(button_saved_throttle_val);
				}
			} else {
				if (s.button_power_bright_enabled) {
					if (!button_bright_val)
						button_bright_val = brightness;
					button_bright_saved = 1;
					SetBrightness(1, s.button_power_bright_val);
				}
				if (s.button_power_performance_enabled) {
					if (!button_saved_performance) {
						QStringList profile_list;
    						int current_profile;
						bool *active_list;
    						if (laptop_portable::get_system_performance(1, current_profile, profile_list, active_list)) {
							button_saved_performance = 1;
							button_saved_performance_val = profile_list[current_profile];
						}
					}
					SetPerformance(s.button_power_performance_val);
				}
				if (s.button_power_throttle_enabled) {
					if (!button_saved_throttle) {
						QStringList profile_list;
    						int current_profile;
						bool *active_list;
    						if (laptop_portable::get_system_throttling(1, current_profile, profile_list, active_list)) {
							button_saved_throttle = 1;
							button_saved_throttle_val = profile_list[current_profile];
						}
					}
					SetThrottle(s.button_power_throttle_val);
				}
			}
			switch (s.button_power) {
			case 1: invokeStandby();
				break;
			case 2: invokeSuspend();
				break;
			case 3: invokeHibernate();
				break;
			case 4: invokeLogout();
				break;
			case 5: invokeShutdown();
				break;
			}
			power_button_off = !power_button_off;
		}
	}
}

#include "laptop_daemon.moc"
