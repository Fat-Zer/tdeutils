/*
 * apm.h
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


#ifndef __APMCONFIG_H__
#define __APMCONFIG_H__

#include <kcmodule.h>
#include <tqstring.h>

class TQWidget;
class TQSpinBox;
class TDEConfig;
class TQCheckBox;
class KIconLoader;
class KIconButton;
class TQPushButton;


class ApmConfig : public TDECModule
{
  Q_OBJECT
  
public:
  ApmConfig( TQWidget *parent=0, const char* name=0);
  ~ApmConfig( );     

  void save( void );
  void load();
  void load(bool useDefaults);
  void defaults();

  virtual TQString quickHelp() const;

private slots:

  void configChanged();
  void setupHelper();
  void setupHelper2();

private:
	TDEConfig *config;

	TQCheckBox *enableSuspend;
	TQCheckBox *enableStandby;
	TQCheckBox *enableSoftwareSuspendHibernate;
	bool enablestandby, enablesuspend, enablesoftwaresuspend;
	const char *apm_name;
};

#endif

