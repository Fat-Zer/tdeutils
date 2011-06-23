/*
 * pcmcia.h
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


#ifndef __BUTTONSCONFIG_H__
#define __BUTTONSCONFIG_H__

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

class ButtonsConfig : public KCModule
{
  Q_OBJECT
  TQ_OBJECT
public:
  ButtonsConfig( TQWidget *tqparent=0, const char* name=0);
    ~ButtonsConfig();
  void save( void );
  void load();
  void load(bool useDefaults);
  void defaults();

  virtual TQString quickHelp() const;

private slots:

  void configChanged();


private:

  int  getPower();
  int  getLid();
  void setPower( int, int );

  TQButtonGroup *lidBox;
  TQRadioButton *lidStandby, *lidSuspend, *lidOff, *lidHibernate, *lidShutdown, *lidLogout;
  TQCheckBox *lidBrightness;
  TQSlider *lidValBrightness;
  TQCheckBox *lidThrottle;
  KComboBox *lidValThrottle;
  TQCheckBox *lidPerformance;
  KComboBox *lidValPerformance;
  TQButtonGroup *powerBox;
  TQRadioButton *powerStandby, *powerSuspend, *powerOff, *powerHibernate, *powerShutdown, *powerLogout;
  TQCheckBox *powerBrightness;
  TQSlider *powerValBrightness;
  TQCheckBox *powerThrottle;
  KComboBox *powerValThrottle;
  TQCheckBox *powerPerformance;
  KComboBox *powerValPerformance;
  int power_bright_val, lid_bright_val;
  bool lid_bright_enabled, power_bright_enabled;
  bool lid_throttle_enabled, power_throttle_enabled;
  TQString lid_throttle_val, power_throttle_val;
  bool lid_performance_enabled, power_performance_enabled;
  TQString lid_performance_val, power_performance_val;

  KConfig *config;
  int power, lid, apm;
};

#endif

