/*
 * power.h
 *
 * Copyright (c) 1999 Paul Campbell <paul@taniwha.com>
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


#ifndef __POWERCONFIG_H__
#define __POWERCONFIG_H__

#include <kcmodule.h>
#include <tqstring.h>

class TQWidget;
class TQSlider;
class TQButtonGroup;
class TQRadioButton;
class TQSpinBox;
class KConfig;
class TQCheckBox;
class KComboBox;
class KDoubleSpinBox;

class PowerConfig : public KCModule
{
  Q_OBJECT
  
public:
  PowerConfig( TQWidget *parent=0, const char* name=0);
    ~PowerConfig();
  void save( void );
  void load();
  void load(bool useDefaults);
  void defaults();

  virtual TQString quickHelp() const;


private:

  int  getPower();
  int  getNoPower();
  void setPower( int, int );

  TQButtonGroup *nopowerBox;
  TQRadioButton *nopowerStandby, *nopowerSuspend, *nopowerOff, *nopowerHibernate;
  TQCheckBox *nopowerBrightness;
  TQSlider *nopowerValBrightness;
  TQCheckBox *nopowerThrottle;
  KComboBox *nopowerValThrottle;
  TQCheckBox *nopowerPerformance;
  KComboBox *nopowerValPerformance;
  TQButtonGroup *powerBox;
  TQRadioButton *powerStandby, *powerSuspend, *powerOff, *powerHibernate;
  TQCheckBox *powerBrightness;
  TQSlider *powerValBrightness;
  TQCheckBox *powerThrottle;
  KComboBox *powerValThrottle;
  TQCheckBox *powerPerformance;
  KComboBox *powerValPerformance;
  TQSpinBox *noeditwait;
  TQSpinBox *editwait;
  TQCheckBox *enablelav;
  TQCheckBox *noenablelav;
  KDoubleSpinBox *noeditlav;
  KDoubleSpinBox *editlav;
  int edit_wait, noedit_wait;
  int power_bright_val, nopower_bright_val;
  bool nopower_bright_enabled, power_bright_enabled;
  bool nopower_throttle_enabled, power_throttle_enabled;
  TQString nopower_throttle_val, power_throttle_val;
  bool nopower_performance_enabled, power_performance_enabled;
  bool lav_enabled, nolav_enabled;
  float edit_lav, noedit_lav;
  TQString nopower_performance_val, power_performance_val;

  KConfig *config;
  int power, nopower, apm;
};

#endif

