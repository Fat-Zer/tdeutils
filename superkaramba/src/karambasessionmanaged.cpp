/****************************************************************************
*  karambasessionmanaged.cpp  -  Karamba session management
*
*  Copyright (C) 2004 -
*
*  This file is part of SuperKaramba.
*
*  SuperKaramba is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2 of the License, or
*  (at your option) any later version.
*
*  SuperKaramba is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with SuperKaramba; if not, write to the Free Software
*  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
****************************************************************************/

#include <tdeapplication.h>
#include <tdeconfig.h>
#include "karambasessionmanaged.h"
#include "karamba.h"
#include "tqwidgetlist.h"

bool KarambaSessionManaged::saveState(TQSessionManager&)
{
  TDEConfig* config = kapp->sessionConfig();

  config->setGroup("General Options");

  TQString openThemes="";

  TQWidgetList  *list = TQApplication::allWidgets();
  TQWidgetListIt it( *list );         // iterate over the widgets
  TQWidget * w;
  while ( (w=it.current()) != 0 ) // for each widget...
  {
    ++it;
    if (TQString(w->name()).startsWith("karamba"))
    {
      karamba* k = (karamba*) w;
      if (k->isSubTheme())
        continue;
      openThemes += TQFileInfo(k->theme().file()).absFilePath();
      k->writeConfigData();
      openThemes += ";";
    }
  }
  delete list;                      // delete the list, not the widgets

  tqDebug("Open themes %s", openThemes.ascii());
  config->writeEntry("OpenThemes", openThemes);
  return true;
}

bool KarambaSessionManaged::commitData(TQSessionManager&)
{
  return true;
}
