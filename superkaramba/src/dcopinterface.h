/***************************************************************************
 *   Copyright (C) 2004 by Petri Damsten                                   *
 *   petri.damsten@iki.fi                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef DCOPINTERFACE_H
#define DCOPINTERFACE_H

#include <dcopobject.h>

class dcopIface : virtual public DCOPObject
{
  K_DCOP
public:

k_dcop:
    virtual ASYNC openTheme(TQString file) = 0;
    virtual ASYNC openNamedTheme(TQString file, TQString name, bool is_sub_theme) = 0;
    virtual ASYNC closeTheme(TQString name) = 0;
    virtual ASYNC quit() = 0;
    virtual ASYNC hideSystemTray(bool hide) = 0;
    virtual ASYNC showThemeDialog() = 0;

    virtual int themeAdded(TQString appId, TQString file) = 0;
    virtual ASYNC themeClosed(TQString appId, TQString file, int instance) = 0;
    virtual ASYNC themeNotify(TQString name, TQString text) = 0;
    virtual ASYNC setIncomingData(TQString name, TQString obj) = 0;
    virtual bool isMainKaramba() = 0;

};

#endif // DCOPINTERFACE_H
