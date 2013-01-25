/*
 * sony.h
 *
 * Copyright (c) 2002 Paul Campbell <paul@taniwha.com>
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


#ifndef __SONYCONFIG_H__
#define __SONYCONFIG_H__

#include <kcmodule.h>
#include <tqstring.h>

class TQWidget;
class TQSpinBox;
class TDEConfig;
class TQCheckBox;
class TQSlider;
class KIconLoader;
class KIconButton;
class TQPushButton;


class SonyConfig : public TDECModule
{
  Q_OBJECT
  
public:
  SonyConfig( TQWidget *parent=0, const char* name=0);
  ~SonyConfig( );     

  void save( void );
  void load();
  void load(bool useDefaults);
  void defaults();

  virtual TQString quickHelp() const;

private slots:

  void configChanged();
  void slotStartMonitor();
  void setupHelper();

private:
	TDEConfig *config;

	TQCheckBox *enableScrollBar;
	bool enablescrollbar;
	TQCheckBox *enableMiddleEmulation;
	bool middleemulation;
};

#endif

