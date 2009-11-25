
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



#include <qtabwidget.h>
#include <qlayout.h>
#include <qtimer.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include <kpushbutton.h>
#include <kstdguiitem.h>
#include "kpcmcia.h"

#include "kpcmciainfo.h"


KPCMCIAInfo::KPCMCIAInfo(KPCMCIA *pcmcia, QWidget *parent, const char *name)
  : KDialog(parent, name, false), _pcmcia(pcmcia) {

  setMinimumSize(300,400);

  _mainGrid = new QGridLayout(this, 9, 5);

  _mainTab = new QTabWidget(this);
  _mainGrid->addMultiCellWidget(_mainTab, 0, 6, 0, 4);
  _mainGrid->setRowStretch(0, 1);
  _mainGrid->setRowStretch(1, 1);
  _mainGrid->setRowStretch(2, 1);
  _mainGrid->setRowStretch(3, 1);
  _mainGrid->setRowStretch(4, 1);
  _mainGrid->setRowStretch(5, 1);
  _mainGrid->setRowStretch(6, 1);

  setCaption(i18n("PCMCIA & CardBus Slots"));

  prepareCards();

  _mainTab->resize(KDialog::sizeHint());
  resize(KDialog::sizeHint());

  connect(_pcmcia, SIGNAL(cardUpdated(int)), this, SLOT(updateCard(int)));

  _sb = new KStatusBar(this);
  _sb->insertItem(i18n("Ready."), 0, 1, true);
  _sb->resize(KDialog::sizeHint());
  _mainGrid->addMultiCellWidget(_sb, 8, 8, 0, 4);
  _mainGrid->setRowStretch(8, 0);

  _updateButton = new QPushButton(i18n("&Update"), this);
  _mainGrid->addWidget(_updateButton, 7, 3);
  connect(_updateButton, SIGNAL(pressed()), this, SLOT(update()));
  _closeButton = new KPushButton(KStdGuiItem::close(), this);
  _mainGrid->addWidget(_closeButton, 7, 4);
  connect(_closeButton, SIGNAL(pressed()), this, SLOT(slotClose()));
  _mainGrid->setRowStretch(7, 0);

  show();
}



KPCMCIAInfo::~KPCMCIAInfo() {

}


void KPCMCIAInfo::showTab(int num) {
   _mainTab->showPage(_pages[num]);
}


void KPCMCIAInfo::slotResetStatus() {
  _sb->changeItem(i18n("Ready."), 0);
}


void KPCMCIAInfo::statusNotice(const QString& text, int life) {
   _sb->changeItem(text, 0);
   if (life > 0)
      QTimer::singleShot(life, this, SLOT(slotResetStatus()));
}



void KPCMCIAInfo::slotTabSetStatus(const QString& text) {
   statusNotice(text);
}


void KPCMCIAInfo::slotClose() {
   delete this;
}


void KPCMCIAInfo::update() {
  emit updateNow();
}


void KPCMCIAInfo::updateCard(int num) {
  _pages[num]->update();
}


void KPCMCIAInfo::prepareCards() {
  if (!_pcmcia) {
     // FIXME: display error
     return;
  }

  for (int i = 0; i < _pcmcia->getCardCount(); i++) {
     QString tabname = i18n("Card Slot %1");
     KPCMCIAInfoPage *tp = new KPCMCIAInfoPage(_pcmcia->getCard(i), _mainTab);
     connect(this, SIGNAL(updateNow()), tp, SLOT(update()));
     connect(tp, SIGNAL(setStatusBar(const QString&)), this, SLOT(slotTabSetStatus(const QString&)));
     tp->resize(_mainTab->sizeHint());
     _mainTab->addTab(tp, tabname.arg(i+1));
     _pages.insert(i, tp);
  }
}






///////////////////////////////////////////////////////////////////////////


KPCMCIAInfoPage::KPCMCIAInfoPage(KPCMCIACard *card, QWidget *parent, const char *name)
  : QFrame(parent, name), _card(card) {
  _mainGrid = new QGridLayout(this, 10, 10);
  if (!_card) {
      // display an error
  } else {
      _card_name = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_name, 0, 0, 0, 5);
      _card_type = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_type, 0, 0, 6, 9);
      _card_driver = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_driver, 1, 1, 0, 4);
      _card_irq = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_irq, 2, 2, 0, 3);
      _card_io = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_io, 3, 3, 0, 6);
      _card_dev = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_dev, 4, 4, 0, 4);
      _card_vcc = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_vcc, 5, 5, 0, 2);
      _card_vpp = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_vpp, 5, 5, 5, 9);
      _card_bus = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_bus, 6, 6, 0, 4);
      _card_cfgbase = new QLabel(this);
      _mainGrid->addMultiCellWidget(_card_cfgbase, 6, 6, 5, 9);

      _card_ej_ins = new QPushButton(i18n("&Eject"), this);
      _card_sus_res = new QPushButton(i18n("&Suspend"), this);
      _card_reset = new QPushButton(i18n("&Reset"), this);
      _mainGrid->addWidget(_card_ej_ins, 9, 5);
      _mainGrid->addWidget(_card_sus_res, 9, 6);
      _mainGrid->addWidget(_card_reset, 9, 7);
      connect(_card_reset, SIGNAL(pressed()), this, SLOT(slotResetCard()));
      connect(_card_sus_res, SIGNAL(pressed()), this, SLOT(slotSuspendResume()));
      connect(_card_ej_ins, SIGNAL(pressed()), this, SLOT(slotInsertEject()));

      update();
  }
}


KPCMCIAInfoPage::~KPCMCIAInfoPage() {

}



void KPCMCIAInfoPage::slotResetCard() {
  emit setStatusBar(i18n("Resetting card..."));
  _card->reset();
}


void KPCMCIAInfoPage::slotInsertEject() {
      if (!(_card->status() & (CARD_STATUS_READY|CARD_STATUS_SUSPEND))) {
         emit setStatusBar(i18n("Inserting new card..."));
         _card->insert();
         _card->reset();
      } else {
         emit setStatusBar(i18n("Ejecting card..."));
         if (_card->status() & CARD_STATUS_SUSPEND)
           _card->resume();
         _card->eject();
      }
}


void KPCMCIAInfoPage::slotSuspendResume() {
    if (!(_card->status() & CARD_STATUS_BUSY))
      if (!(_card->status() & CARD_STATUS_SUSPEND)) {
         emit setStatusBar(i18n("Suspending card..."));
        _card->suspend();
      } else {
         emit setStatusBar(i18n("Resuming card..."));
        _card->resume();
      }
}


void KPCMCIAInfoPage::update() {
   if (_card) {
      QString tmp;
      _card_name->setText(_card->name());
      _card_name->resize(_card_name->sizeHint());
      tmp = i18n("Card type: %1 ");
      _card_type->setText(tmp.arg(_card->type()));
      _card_type->resize(_card_type->sizeHint());
      tmp = i18n("Driver: %1");
      _card_driver->setText(tmp.arg(_card->driver()));
      _card_driver->resize(_card_driver->sizeHint());
      tmp = i18n("IRQ: %1%2");
      QString tmp2;
      switch (_card->intType()) {
      case 1:
        tmp2 = i18n(" (used for memory)");
       break;
      case 2:
        tmp2 = i18n(" (used for memory and I/O)");
       break;
      case 4:
        tmp2 = i18n(" (used for CardBus)");
       break;
      default:
       tmp2 = "";
      };
      if (_card->irq() <= 0)
         _card_irq->setText(tmp.arg(i18n("none")).arg(""));
      else _card_irq->setText(tmp.arg(_card->irq()).arg(tmp2));
      _card_irq->resize(_card_irq->sizeHint());
      tmp = i18n("I/O port(s): %1");
      if (_card->ports().isEmpty())
         _card_io->setText(tmp.arg(i18n("none")));
      else _card_io->setText(tmp.arg(_card->ports()));
      _card_io->resize(_card_io->sizeHint());
      tmp = i18n("Bus: %1 bit %2");
      if (_card->busWidth() == 0)
         _card_bus->setText(i18n("Bus: unknown"));
      else _card_bus->setText(tmp.arg(_card->busWidth()).arg(_card->busWidth() == 16 ? i18n("PC Card") : i18n("Cardbus")));
      _card_bus->resize(_card_bus->sizeHint());
      tmp = i18n("Device: %1");
      _card_dev->setText(tmp.arg(_card->device()));
      _card_dev->resize(_card_dev->sizeHint());
      tmp = i18n("Power: +%1V");
      _card_vcc->setText(tmp.arg(_card->vcc()/10));
      _card_vcc->resize(_card_vcc->sizeHint());
      tmp = i18n("Programming power: +%1V, +%2V");
      _card_vpp->setText(tmp.arg(_card->vpp()/10).arg(_card->vpp2()/10));
      _card_vpp->resize(_card_vpp->sizeHint());
      tmp = i18n("Configuration base: 0x%1");
      if (_card->configBase() == 0)
         _card_cfgbase->setText(i18n("Configuration base: none"));
      else _card_cfgbase->setText(tmp.arg(_card->configBase(), -1, 16));
      _card_cfgbase->resize(_card_cfgbase->sizeHint());

      if (!(_card->status() & (CARD_STATUS_READY|CARD_STATUS_SUSPEND))) {
         _card_ej_ins->setText(i18n("&Insert"));
      } else {
         _card_ej_ins->setText(i18n("&Eject"));
      }
      if (!(_card->status() & (CARD_STATUS_BUSY|CARD_STATUS_SUSPEND))) {
         _card_sus_res->setText(i18n("&Suspend"));
      } else {
         _card_sus_res->setText(i18n("Resu&me"));
      }
      if (!(_card->status() & (CARD_STATUS_READY|CARD_STATUS_SUSPEND))) {
         _card_sus_res->setEnabled(false);
         _card_reset->setEnabled(false);
      } else {
         _card_sus_res->setEnabled(true);
         if (!(_card->status() & CARD_STATUS_SUSPEND))
            _card_reset->setEnabled(true);
         else _card_reset->setEnabled(false);
      }
   }
}




#include "kpcmciainfo.moc"

