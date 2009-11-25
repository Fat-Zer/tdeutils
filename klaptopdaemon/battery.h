/*
 * battery.h
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (c) 2002 Marc Mutz <mutz@kde.org>
 * Copyright (c) 2006 Flavio Castelli <flavio.castelli@gmail.com>
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


#ifndef __BATTERYCONFIG_H__
#define __BATTERYCONFIG_H__

#include <kcmodule.h>
#include <qstring.h>
#include <qpixmap.h>
#include <qptrlist.h>

class QWidget;
class QSpinBox;
class KConfig;
class QCheckBox;
class KIconLoader;
class KIconButton;
class QPushButton;
class QLabel;


class BatteryConfig : public KCModule
{
  Q_OBJECT
public:
  BatteryConfig( QWidget *parent=0, const char* name=0);
  ~BatteryConfig( );     

  void save( void );
  void load();
  void load(bool useDefaults);
  void defaults();

  virtual QString quickHelp() const;

private slots:

  void configChanged();
  void runMonitorChanged();
  void slotStartMonitor();
  void iconChanged();
  void BatteryStateUpdate();

private:
	void timerEvent(QTimerEvent *);
	void ConvertIcon(int percent, QPixmap &pm, QPixmap &result);
	KConfig *config;

        QSpinBox* editPoll;
        QCheckBox* runMonitor;
        QCheckBox* showLevel;
        QCheckBox* notifyMe;
	QCheckBox* blankSaver;
        bool            enablemonitor;
        bool            showlevel;
        bool            enablequitmenu;
	bool		useblanksaver;
	bool		notifyme;

	KIconLoader *iconloader;

        KIconButton *buttonNoBattery;
        KIconButton *buttonNoCharge;
        KIconButton *buttonCharge;
        QString nobattery, nochargebattery, chargebattery;
	QPushButton *startMonitor;
        bool    apm;
        int poll_time;       

	QPtrList<QLabel> batt_label_1, batt_label_2, batt_label_3;
	QPixmap battery_pm, battery_nopm;

    	KInstance *instance;
};

#endif

