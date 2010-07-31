/***************************************************************************
 *   Copyright (C) 2004 by Petri Damsten                                   *
 *   petri.damsten@iki.fi                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef KARAMBAINTERFACE_H
#define KARAMBAINTERFACE_H

#include "dcopinterface.h"

class karamba;
class ThemeListWindow;

class KarambaIface: virtual public dcopIface
{
public:
  KarambaIface();
  ~KarambaIface();
  karamba* getKaramba(TQString name);
  ThemesDlg* getThemeWnd();

public slots:
  virtual void openTheme(TQString filename);
  virtual void openNamedTheme(TQString filename, TQString name, bool is_sub_theme);
  virtual void closeTheme(TQString name);
  virtual void quit();
  virtual void hideSystemTray(bool show);
  virtual void showThemeDialog();

  virtual int themeAdded(TQString appId, TQString file);
  virtual void themeClosed(TQString appId, TQString file, int instance);
  virtual void themeNotify(TQString name, TQString text);
  virtual void setIncomingData(TQString name, TQString text);
  virtual bool isMainKaramba();
};

#endif // KARAMBAINTERFACE_H
