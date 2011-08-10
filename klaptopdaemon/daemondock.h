#ifndef DAEMONDOCK
#define DAEMONDOCK 1
/*
 * daemondock.h
 * Copyright (C) 1999 Paul Campbell <paul@taniwha.com>
 * Copyright (C) 2006 Flavio Castelli <flavio.castelli@gmail.com>
 *
 * This file contains the docked widget for the laptop battery monitor
 *
 * $Id$
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


#include "laptop_daemon.h"
#include <ksystemtray.h>

class laptop_daemon;
class KPCMCIA;
class KPCMCIACard;
class TQPopupMenu;
class TQVBox;
class TQSlider;
class KPopupMenu;

#include <tqmap.h>

class laptop_dock : public KSystemTray {

  Q_OBJECT
  TQ_OBJECT

public:
  laptop_dock(laptop_daemon* parent);
  ~laptop_dock();
  void displayPixmap();

  void mousePressEvent( TQMouseEvent * ); 
  void mouseReleaseEvent( TQMouseEvent * ); 
  void showEvent( TQShowEvent * );  
  void reload_icon();

  inline void setPCMCIA(KPCMCIA *p) { _pcmcia = p; }

  void SetupPopup();

private slots:
  void invokeStandby();
  void invokeSuspend();
  void invokeLockSuspend();
  void invokeHibernation();
  void invokeLockHibernation();
  void invokeSetup();
  void invokeBrightness();
  void invokeBrightnessSlider(int v);

  void slotEjectAction(int id);
  void slotResumeAction(int id);
  void slotSuspendAction(int id);
  void slotInsertAction(int id);
  void slotResetAction(int id);
  void slotDisplayAction(int id);

  void activate_throttle(int ind);
  void fill_throttle();
  void activate_performance(int ind);
  void fill_performance();


  void rootExited(KProcess *p);
  void slotGoRoot(int id);
  void slotQuit();
  void slotHide();

private:
  int brightness;
  TQVBox *brightness_widget;
  TQSlider *brightness_slider;
  laptop_daemon *pdaemon;
  TQPixmap	pm;
  TQPopupMenu	*performance_popup, *throttle_popup;
  int	 	current_code;
  KPCMCIA       *_pcmcia;
  TQMap<int,KPCMCIACard *> _ejectActions,
                          _suspendActions,
                          _resumeActions,
                          _resetActions,
                          _displayActions,
                          _insertActions;

  KInstance 	*instance; 	// handle so we get our pretty icons right
  KPopupMenu 	*rightPopup;

}; 
#endif 

