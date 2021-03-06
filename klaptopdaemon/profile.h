/*
 * brightness.h
 *
 * Copyright (c) 2003 Paul Campbell <paul@taniwha.com>
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


#ifndef __BRIGHTNESSCONFIG_H__
#define __BRIGHTNESSCONFIG_H__

#include <tdecmodule.h>
#include <tqstring.h>

class TQWidget;
class TQSpinBox;
class TDEConfig;
class TQCheckBox;
class TQSlider;
class TDEIconLoader;
class TDEIconButton;
class TQPushButton;
class KComboBox;


class ProfileConfig : public TDECModule
{
  Q_OBJECT
  
public:
  ProfileConfig( TQWidget *parent=0, const char* name=0);
  ~ProfileConfig( );     

  void save( void );
  void load();
  void load(bool useDefaults);
  void defaults();

  virtual TQString quickHelp() const;

private slots:

  void configChanged();
  void slotStartMonitor();
  void poff_changed(bool);
  void pon_changed(bool);
  void throttle_off_changed(bool);
  void throttle_on_changed(bool);
  void performance_off_changed(bool);
  void performance_on_changed(bool);


private:
	TDEConfig *config;

	TQCheckBox *pon, *performance_on, *throttle_on;
	TQSlider *son;
	KComboBox *performance_val_on, *throttle_val_on;
	TQCheckBox *poff, *performance_off, *throttle_off;
	TQSlider *soff;
	KComboBox *performance_val_off, *throttle_val_off;


};

#endif

