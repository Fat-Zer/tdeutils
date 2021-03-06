/*
 * main.h
 *
 * Copyright (c) 2002 Paul Campbell paul@taniwha.com
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


#ifndef __MAIN_H__
#define __MAIN_H__


#include <tqtabwidget.h>


#include <tdecmodule.h>


class WarningConfig;
class BatteryConfig;
class PowerConfig;
class AcpiConfig;
class ApmConfig;
class SonyConfig;
class ProfileConfig;
class ButtonsConfig;

class LaptopModule : public TDECModule
{
  Q_OBJECT
  

public:

  LaptopModule(TQWidget *parent, const char *name);

  void load();
  void save();
  void defaults();
  TQString quickHelp() const;



protected slots:

  void moduleChanged(bool state);


private:

  TQTabWidget   *tab;

  WarningConfig 	*warning;
  WarningConfig 	*critical;
  BatteryConfig   	*battery;
  PowerConfig  		*power;
  AcpiConfig  		*acpi;
  ApmConfig  		*apm;
  ProfileConfig		*profile;
  SonyConfig  		*sony;
  ButtonsConfig		*buttons;
};


#endif


