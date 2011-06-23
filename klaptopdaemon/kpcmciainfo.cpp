
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



#include <tqtabwidget.h>
#include <tqlayout.h>
#include <tqtimer.h>
#include <kstatusbar.h>
#include <klocale.h>
#include <kpushbutton.h>
#include <kstdguiitem.h>

#include <kpushbutton.h>
#include <kstdguiitem.h>
#include "kpcmcia.h"

#include "kpcmciainfo.h"


KPCMCIAInfo::KPCMCIAInfo(KPCMCIA *pcmcia, TQWidget *tqparent, const char *name)
  : KDialog(tqparent, name, false), _pcmcia(pcmcia) {

  setMinimumSize(300,400);

  _mainGrid = new TQGridLayout(this, 9, 5);

  _mainTab = new TQTabWidget(this);
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

  _mainTab->resize(KDialog::tqsizeHint());
  resize(KDialog::tqsizeHint());

  connect(_pcmcia, TQT_SIGNAL(cardUpdated(int)), this, TQT_SLOT(updateCard(int)));

  _sb = new KStatusBar(this);
  _sb->insertItem(i18n("Ready."), 0, 1, true);
  _sb->resize(KDialog::tqsizeHint());
  _mainGrid->addMultiCellWidget(_sb, 8, 8, 0, 4);
  _mainGrid->setRowStretch(8, 0);

  _updateButton = new TQPushButton(i18n("&Update"), this);
  _mainGrid->addWidget(_updateButton, 7, 3);
  connect(_updateButton, TQT_SIGNAL(pressed()), this, TQT_SLOT(update()));
  _closeButton = new KPushButton(KStdGuiItem::close(), this);
  _mainGrid->addWidget(_closeButton, 7, 4);
  connect(_closeButton, TQT_SIGNAL(pressed()), this, TQT_SLOT(slotClose()));
  _mainGrid->setRowStretch(7, 0);

  show();
}



KPCMCIAInfo::~KPCMCIAInfo() {

}


void KPCMCIAInfo::showTab(int num) {
   _mainTab->showPage(_pages[num]);
}


void KPCMCIAInfo::slotResettqStatus() {
  _sb->changeItem(i18n("Ready."), 0);
}


void KPCMCIAInfo::statusNotice(const TQString& text, int life) {
   _sb->changeItem(text, 0);
   if (life > 0)
      TQTimer::singleShot(life, this, TQT_SLOT(slotResettqStatus()));
}



void KPCMCIAInfo::slotTabSettqStatus(const TQString& text) {
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
     TQString tabname = i18n("Card Slot %1");
     KPCMCIAInfoPage *tp = new KPCMCIAInfoPage(_pcmcia->getCard(i), _mainTab);
     connect(this, TQT_SIGNAL(updateNow()), tp, TQT_SLOT(update()));
     connect(tp, TQT_SIGNAL(setStatusBar(const TQString&)), this, TQT_SLOT(slotTabSettqStatus(const TQString&)));
     tp->resize(_mainTab->tqsizeHint());
     _mainTab->addTab(tp, tabname.tqarg(i+1));
     _pages.insert(i, tp);
  }
}






///////////////////////////////////////////////////////////////////////////


KPCMCIAInfoPage::KPCMCIAInfoPage(KPCMCIACard *card, TQWidget *tqparent, const char *name)
  : TQFrame(tqparent, name), _card(card) {
  _mainGrid = new TQGridLayout(this, 10, 10);
  if (!_card) {
      // display an error
  } else {
      _card_name = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_name, 0, 0, 0, 5);
      _card_type = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_type, 0, 0, 6, 9);
      _card_driver = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_driver, 1, 1, 0, 4);
      _card_irq = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_irq, 2, 2, 0, 3);
      _card_io = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_io, 3, 3, 0, 6);
      _card_dev = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_dev, 4, 4, 0, 4);
      _card_vcc = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_vcc, 5, 5, 0, 2);
      _card_vpp = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_vpp, 5, 5, 5, 9);
      _card_bus = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_bus, 6, 6, 0, 4);
      _card_cfgbase = new TQLabel(this);
      _mainGrid->addMultiCellWidget(_card_cfgbase, 6, 6, 5, 9);

      _card_ej_ins = new TQPushButton(i18n("&Eject"), this);
      _card_sus_res = new TQPushButton(i18n("&Suspend"), this);
      _card_reset = new TQPushButton(i18n("&Reset"), this);
      _mainGrid->addWidget(_card_ej_ins, 9, 5);
      _mainGrid->addWidget(_card_sus_res, 9, 6);
      _mainGrid->addWidget(_card_reset, 9, 7);
      connect(_card_reset, TQT_SIGNAL(pressed()), this, TQT_SLOT(slotResetCard()));
      connect(_card_sus_res, TQT_SIGNAL(pressed()), this, TQT_SLOT(slotSuspendResume()));
      connect(_card_ej_ins, TQT_SIGNAL(pressed()), this, TQT_SLOT(slotInsertEject()));

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
      TQString tmp;
      _card_name->setText(_card->name());
      _card_name->resize(_card_name->tqsizeHint());
      tmp = i18n("Card type: %1 ");
      _card_type->setText(tmp.tqarg(_card->type()));
      _card_type->resize(_card_type->tqsizeHint());
      tmp = i18n("Driver: %1");
      _card_driver->setText(tmp.tqarg(_card->driver()));
      _card_driver->resize(_card_driver->tqsizeHint());
      tmp = i18n("IRQ: %1%2");
      TQString tmp2;
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
         _card_irq->setText(tmp.tqarg(i18n("none")).tqarg(""));
      else _card_irq->setText(tmp.tqarg(_card->irq()).tqarg(tmp2));
      _card_irq->resize(_card_irq->tqsizeHint());
      tmp = i18n("I/O port(s): %1");
      if (_card->ports().isEmpty())
         _card_io->setText(tmp.tqarg(i18n("none")));
      else _card_io->setText(tmp.tqarg(_card->ports()));
      _card_io->resize(_card_io->tqsizeHint());
      tmp = i18n("Bus: %1 bit %2");
      if (_card->busWidth() == 0)
         _card_bus->setText(i18n("Bus: unknown"));
      else _card_bus->setText(tmp.tqarg(_card->busWidth()).tqarg(_card->busWidth() == 16 ? i18n("PC Card") : i18n("Cardbus")));
      _card_bus->resize(_card_bus->tqsizeHint());
      tmp = i18n("Device: %1");
      _card_dev->setText(tmp.tqarg(_card->device()));
      _card_dev->resize(_card_dev->tqsizeHint());
      tmp = i18n("Power: +%1V");
      _card_vcc->setText(tmp.tqarg(_card->vcc()/10));
      _card_vcc->resize(_card_vcc->tqsizeHint());
      tmp = i18n("Programming power: +%1V, +%2V");
      _card_vpp->setText(tmp.tqarg(_card->vpp()/10).tqarg(_card->vpp2()/10));
      _card_vpp->resize(_card_vpp->tqsizeHint());
      tmp = i18n("Configuration base: 0x%1");
      if (_card->configBase() == 0)
         _card_cfgbase->setText(i18n("Configuration base: none"));
      else _card_cfgbase->setText(tmp.tqarg(_card->configBase(), -1, 16));
      _card_cfgbase->resize(_card_cfgbase->tqsizeHint());

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

