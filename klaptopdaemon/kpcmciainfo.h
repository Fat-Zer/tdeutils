
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
#include <qframe.h>

class KPCMCIA;
class KPCMCIACard;
class KPCMCIAInfoPage;
class QTabWidget;
class KStatusBar;
class QGridLayout;
class QPushButton;
class KPushButton;

#include <qmap.h>

class KPCMCIAInfo : public KDialog {
Q_OBJECT
public:

  KPCMCIAInfo(KPCMCIA *pcmcia, QWidget *parent = NULL, const char *name = 0);
  virtual ~KPCMCIAInfo();

  void showTab(int num);
  void statusNotice(const QString& text, int life = 1500);

public slots:
  void slotClose();
  void update();
  void updateCard(int num);
  void slotResetStatus();
  void slotTabSetStatus(const QString& text);

signals:
  void updateNow();

private:
  QFrame        *_mainFrame;
  QTabWidget    *_mainTab;
  QGridLayout   *_mainGrid;
  KPCMCIA       *_pcmcia;
  QMap<int,KPCMCIAInfoPage*> _pages;
  KStatusBar    *_sb;
  KPushButton   *_closeButton;
  QPushButton   *_updateButton;


  void prepareCards();
};


class QLabel;


class KPCMCIAInfoPage : public QFrame {
Q_OBJECT
public:
  KPCMCIAInfoPage(KPCMCIACard *card, QWidget *parent = NULL, const char *name = 0);
  virtual ~KPCMCIAInfoPage();

public slots:
  void update();
  void slotResetCard();
  void slotInsertEject();
  void slotSuspendResume();

signals:
  void setStatusBar(const QString&);

private:

  KPCMCIACard   *_card;
  QGridLayout   *_mainGrid;

  QLabel        *_card_name;
  QLabel        *_card_type;
  QLabel        *_card_irq;
  QLabel        *_card_io;
  QLabel        *_card_dev;
  QLabel        *_card_driver;
  QLabel        *_card_vcc;
  QLabel        *_card_vpp;
  QLabel        *_card_cfgbase;
  QLabel        *_card_bus;

  QPushButton   *_card_ej_ins;
  QPushButton   *_card_sus_res;
  QPushButton   *_card_reset;

};

#endif

