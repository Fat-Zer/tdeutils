/*
 * warning.h
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (c) 2002 Marc Mutz <mutz@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
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


#ifndef __WARNINGCONFIG_H__
#define __WARNINGCONFIG_H__

#include <kcmodule.h>
#include <tqstring.h>

class QWidget;
class KConfig;
class KURLRequester;
class QCheckBox;
class QRadioButton;
class QSpinBox;
class QSlider;
class KComboBox;

class WarningConfig : public KCModule
{
  Q_OBJECT
public:
  WarningConfig(int x, TQWidget *parent=0, const char* name=0);
  ~WarningConfig();
  void save( void );
  void load();
  void load(bool useDefaults);
  void defaults();

  virtual TQString quickHelp() const;

private slots:
  void configChanged();
  void brightness_changed(bool v);
  void throttle_changed(bool v);
  void performance_changed(bool v);
  void checkLowTimeChanged(bool state);
  void checkLowPercentChanged(bool state);
  void checkCriticalTimeChanged(bool state);
  void checkCriticalPercentChanged(bool state);
    
private:
  void my_load(int x, bool useDefaults=false );

  KConfig *config;
 
  KURLRequester* editRunCommand;
  KURLRequester* editPlaySound;
  TQCheckBox* checkLowTime;
  TQCheckBox* checkLowPercent;
  TQCheckBox* checkCriticalTime;
  TQCheckBox* checkCriticalPercent;
  TQSpinBox* editLowTime;
  TQSpinBox* editLowPercent;
  TQSpinBox* editCriticalTime;
  TQSpinBox* editCriticalPercent;
 
  TQCheckBox *checkRunCommand;
  TQCheckBox *checkPlaySound;
  TQCheckBox *checkBeep;
  TQCheckBox *checkNotify;
  TQCheckBox *checkBrightness;
  TQSlider   *valueBrightness;
  TQCheckBox *performance, *throttle;
  KComboBox *performance_val, *throttle_val;
  TQRadioButton *checkNone;
  TQRadioButton *checkShutdown;
  TQRadioButton *checkLogout;
  TQRadioButton *checkSuspend;
  TQRadioButton *checkStandby;
  TQRadioButton *checkHibernate;
 
  bool    apm, runcommand, playsound, beep, notify, do_suspend, do_standby, do_hibernate, logout, shutdown, do_brightness;
  bool	  do_performance, do_throttle;
  bool    time_based_action_low, time_based_action_critical;
  TQString val_performance, val_throttle;
  int 	  val_brightness;
  TQString runcommand_val, sound_val;
  int     low_val_time, low_val_percent, critical_val_time, critical_val_percent;
  int     have_time, type;
};

#endif

