/*
 * battery.h
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (c) 2002 Marc Mutz <mutz@kde.org>
 * Copyright (c) 2006 Flavio Castelli <flavio.castelli@gmail.com>
 *
 * Requires the TQt widget libraries, available at no cost at
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


#ifndef __BATTERYCONFIG_H__
#define __BATTERYCONFIG_H__

#include <tdecmodule.h>
#include <tqstring.h>
#include <tqpixmap.h>
#include <tqptrlist.h>

class TQWidget;
class TQSpinBox;
class TDEConfig;
class TQCheckBox;
class TDEIconLoader;
class TDEIconButton;
class TQPushButton;
class TQLabel;


class BatteryConfig : public TDECModule
{
  Q_OBJECT
  
public:
  BatteryConfig( TQWidget *parent=0, const char* name=0);
  ~BatteryConfig( );     

  void save( void );
  void load();
  void load(bool useDefaults);
  void defaults();

  virtual TQString quickHelp() const;

private slots:

  void configChanged();
  void runMonitorChanged();
  void slotStartMonitor();
  void iconChanged();
  void BatteryStateUpdate();

private:
	void timerEvent(TQTimerEvent *);
	void ConvertIcon(int percent, TQPixmap &pm, TQPixmap &result);
	TDEConfig *config;

        TQSpinBox* editPoll;
        TQCheckBox* runMonitor;
        TQCheckBox* showLevel;
        TQCheckBox* notifyMe;
	TQCheckBox* blankSaver;
        bool            enablemonitor;
        bool            showlevel;
        bool            enablequitmenu;
	bool		useblanksaver;
	bool		notifyme;

	TDEIconLoader *iconloader;

        TDEIconButton *buttonNoBattery;
        TDEIconButton *buttonNoCharge;
        TDEIconButton *buttonCharge;
        TQString nobattery, nochargebattery, chargebattery;
	TQPushButton *startMonitor;
        bool    apm;
        int poll_time;       

	TQPtrList<TQLabel> batt_label_1, batt_label_2, batt_label_3;
	TQPixmap battery_pm, battery_nopm;

    	TDEInstance *instance;
};

#endif

