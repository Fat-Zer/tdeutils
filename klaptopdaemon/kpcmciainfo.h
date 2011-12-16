
/* This file is part of the KDE project
 *
 * Copyright (C) 2001 George Staikos <staikos@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef _KPCMCIAINFO_H
#define _KPCMCIAINFO_H

#include <kdialog.h>
#include <tqframe.h>

class KPCMCIA;
class KPCMCIACard;
class KPCMCIAInfoPage;
class TQTabWidget;
class KStatusBar;
class TQGridLayout;
class TQPushButton;
class KPushButton;

#include <tqmap.h>

class KPCMCIAInfo : public KDialog {
Q_OBJECT
  TQ_OBJECT
public:

  KPCMCIAInfo(KPCMCIA *pcmcia, TQWidget *parent = NULL, const char *name = 0);
  virtual ~KPCMCIAInfo();

  void showTab(int num);
  void statusNotice(const TQString& text, int life = 1500);

public slots:
  void slotClose();
  void update();
  void updateCard(int num);
  void slotResettqStatus();
  void slotTabSettqStatus(const TQString& text);

signals:
  void updateNow();

private:
  TQFrame        *_mainFrame;
  TQTabWidget    *_mainTab;
  TQGridLayout   *_mainGrid;
  KPCMCIA       *_pcmcia;
  TQMap<int,KPCMCIAInfoPage*> _pages;
  KStatusBar    *_sb;
  KPushButton   *_closeButton;
  TQPushButton   *_updateButton;


  void prepareCards();
};


class TQLabel;


class KPCMCIAInfoPage : public TQFrame {
Q_OBJECT
  TQ_OBJECT
public:
  KPCMCIAInfoPage(KPCMCIACard *card, TQWidget *parent = NULL, const char *name = 0);
  virtual ~KPCMCIAInfoPage();

public slots:
  void update();
  void slotResetCard();
  void slotInsertEject();
  void slotSuspendResume();

signals:
  void setStatusBar(const TQString&);

private:

  KPCMCIACard   *_card;
  TQGridLayout   *_mainGrid;

  TQLabel        *_card_name;
  TQLabel        *_card_type;
  TQLabel        *_card_irq;
  TQLabel        *_card_io;
  TQLabel        *_card_dev;
  TQLabel        *_card_driver;
  TQLabel        *_card_vcc;
  TQLabel        *_card_vpp;
  TQLabel        *_card_cfgbase;
  TQLabel        *_card_bus;

  TQPushButton   *_card_ej_ins;
  TQPushButton   *_card_sus_res;
  TQPushButton   *_card_reset;

};

#endif

