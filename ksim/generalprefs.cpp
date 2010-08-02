/*  ksim - a system monitor for kde
 *
 *  Copyright (C) 2001  Robbie Ward <linuxphreak@gmx.co.uk>
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

#include "generalprefs.h"
#include "generalprefs.moc"

#include <tqcombobox.h>
#include <tqcheckbox.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqpushbutton.h>
#include <tqtabwidget.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqpopupmenu.h>

#include <klocale.h>
#include <kapplication.h>
#include <kcombobox.h>
#include <kdebug.h>
#include <kiconloader.h>
#include <knuminput.h>

#include <ksimconfig.h>

KSim::GeneralPrefs::GeneralPrefs(TQWidget *parent, const char *name)
   : TQWidget(parent, name)
{
  m_mainLayout = new TQGridLayout(this);
  m_mainLayout->setSpacing(6);

  m_sizeBox = new TQGroupBox(this);
  m_sizeBox->setTitle(i18n("Graph Size"));
  m_sizeBox->setColumnLayout(0, Qt::Horizontal);

  TQGridLayout *sizeBoxLayout = new TQGridLayout(m_sizeBox->layout());
  sizeBoxLayout->setSpacing(6);

  m_sizeHLabel = new TQLabel(m_sizeBox);
  m_sizeHLabel->setText(i18n("Graph height:"));
  sizeBoxLayout->addWidget(m_sizeHLabel, 0, 0);

  m_sizeHSpin = new KIntSpinBox(m_sizeBox);
  m_sizeHSpin->setValue(40);
  m_sizeHSpin->setMinValue(40);
  m_sizeHSpin->setMaxValue(200);
  m_sizeHSpin->setSizePolicy(TQSizePolicy(TQSizePolicy::Preferred,
     TQSizePolicy::Fixed));
  sizeBoxLayout->addWidget(m_sizeHSpin, 0, 1);

  TQSpacerItem *sizeHSpacer = new TQSpacerItem(20, 20,
     TQSizePolicy::Expanding, TQSizePolicy::Minimum);
  sizeBoxLayout->addItem(sizeHSpacer, 0, 2);

  m_sizeWLabel = new TQLabel(m_sizeBox);
  m_sizeWLabel->setText(i18n("Graph width:"));
  sizeBoxLayout->addWidget(m_sizeWLabel, 1, 0);

  m_sizeWSpin = new KIntSpinBox(m_sizeBox);
  m_sizeWSpin->setValue(58);
  m_sizeWSpin->setMinValue(58);
  m_sizeWSpin->setMaxValue(200);
  m_sizeWSpin->setSizePolicy(TQSizePolicy(TQSizePolicy::Preferred,
     TQSizePolicy::Fixed));
  sizeBoxLayout->addWidget(m_sizeWSpin, 1, 1);

  TQSpacerItem *sizeWSpacer = new TQSpacerItem(20, 20, TQSizePolicy::Expanding, TQSizePolicy::Minimum);
  sizeBoxLayout->addItem(sizeWSpacer, 1, 2);

  m_mainLayout->addWidget(m_sizeBox, 0, 0);
  TQSpacerItem *boxSpacer = new TQSpacerItem(20, 20,
     TQSizePolicy::Expanding, TQSizePolicy::Minimum);
  m_mainLayout->addItem(boxSpacer, 0, 1);

  m_displayFqdn = new TQCheckBox(this);
  m_displayFqdn->setText(i18n("Display fully qualified domain name"));
  m_mainLayout->addMultiCellWidget(m_displayFqdn, 1, 1, 0, 1);

  m_recolourThemes = new TQCheckBox(this);
  m_recolourThemes->setText(i18n("Recolor themes to the current color scheme"));
  m_mainLayout->addMultiCellWidget(m_recolourThemes, 2, 2, 0, 1);

  TQSpacerItem *spacer = new TQSpacerItem(20, 20,
     TQSizePolicy::Minimum, TQSizePolicy::Expanding);
  m_mainLayout->addItem(spacer, 3, 0);
}

KSim::GeneralPrefs::~GeneralPrefs()
{
}

void KSim::GeneralPrefs::saveConfig(KSim::Config *config)
{
  config->setGraphSize(TQSize(m_sizeWSpin->value(), m_sizeHSpin->value()));
  config->setDisplayFqdn(m_displayFqdn->isChecked());
  config->setReColourThemes(m_recolourThemes->isChecked());
}

void KSim::GeneralPrefs::readConfig(KSim::Config *config)
{
  TQSize size = config->graphSize();
  m_sizeHSpin->setValue(size.height());
  m_sizeWSpin->setValue(size.width());
  m_displayFqdn->setChecked(config->displayFqdn());
  m_recolourThemes->setChecked(config->reColourThemes());
}

KSim::ClockPrefs::ClockPrefs(TQWidget *parent, const char *name)
   : TQWidget(parent, name)
{
  m_mainLayout = new TQVBoxLayout(this);
  m_mainLayout->setSpacing(6);

  m_timeCheck = new TQCheckBox(i18n("Show time"),this);
  m_mainLayout->addWidget(m_timeCheck);
  m_timeCheck->setChecked(true);

  m_dateCheck = new TQCheckBox(i18n("Show date"),this);
  m_mainLayout->addWidget(m_dateCheck);
  m_dateCheck->setChecked(true);

  TQSpacerItem *spacer = new TQSpacerItem(20, 20,
     TQSizePolicy::Minimum, TQSizePolicy::Expanding);
  m_mainLayout->addItem(spacer);
}

KSim::ClockPrefs::~ClockPrefs()
{
}

void KSim::ClockPrefs::saveConfig(KSim::Config *config)
{
  config->setShowTime(m_timeCheck->isChecked());
  config->setShowDate(m_dateCheck->isChecked());
}

void KSim::ClockPrefs::readConfig(KSim::Config *config)
{
  m_timeCheck->setChecked(config->showTime());
  m_dateCheck->setChecked(config->showDate());
}

KSim::UptimePrefs::UptimePrefs(TQWidget *parent, const char *name)
   : TQWidget(parent, name)
{
  m_addIcon = SmallIconSet("filenew");
  m_removeIcon = SmallIconSet("editdelete");

  m_mainLayout = new TQVBoxLayout(this);
  m_mainLayout->setSpacing(6);

  m_uptimeCombo = new KComboBox(true, this);
  m_uptimeCombo->setDuplicatesEnabled(false);
  m_uptimeCombo->setSizePolicy(TQSizePolicy(TQSizePolicy::Expanding,
     TQSizePolicy::Fixed));
  m_uptimeCombo->insertItem(i18n("%hh:%mm:%ss"));
  m_uptimeCombo->insertItem(i18n("%dd %h:%m"));
  m_uptimeCombo->insertItem(i18n("Uptime: %h:%m:%s"));
  connect(m_uptimeCombo,
     TQT_SIGNAL(aboutToShowContextMenu(TQPopupMenu *)),
     TQT_SLOT(uptimeContextMenu(TQPopupMenu *)));

  m_uptimeAdd = new TQPushButton(this);
  m_uptimeAdd->setPixmap(SmallIcon("down"));
  connect(m_uptimeAdd, TQT_SIGNAL(clicked()), TQT_SLOT(insertUptimeItem()));
  TQToolTip::add(m_uptimeAdd, i18n("Insert item"));

  m_uptimeCheck = new TQCheckBox(this);
  m_uptimeCheck->setText(i18n("Show uptime"));
  m_uptimeCheck->setChecked(true);
  connect(m_uptimeCheck, TQT_SIGNAL(toggled(bool)),
     m_uptimeCombo, TQT_SLOT(setEnabled(bool)));
  connect(m_uptimeCheck, TQT_SIGNAL(toggled(bool)),
     m_uptimeAdd, TQT_SLOT(setEnabled(bool)));
  m_mainLayout->addWidget(m_uptimeCheck);

  m_subLayout = new TQHBoxLayout;
  m_subLayout->setSpacing(6);

  m_formatLabel = new TQLabel(this);
  m_formatLabel->setText(i18n("Uptime format:"));
  m_subLayout->addWidget(m_formatLabel);

  m_subLayout->addWidget(m_uptimeCombo);
  m_subLayout->addWidget(m_uptimeAdd);
  m_mainLayout->addLayout(m_subLayout);

  m_uptimeInfo = new TQLabel(this);
  m_uptimeInfo->setText(i18n("The text in the edit box will be "
        "what is displayed as \nthe uptime except the % items "
        "will be replaced with \nthe legend"));
  m_mainLayout->addWidget(m_uptimeInfo);

  m_uptimeBox = new TQGroupBox(this);
  m_uptimeBox->setTitle(i18n("Uptime Legend"));
  m_uptimeBox->setColumnLayout(0, Qt::Vertical);
  m_uptimeBox->layout()->setSpacing(0);
  m_uptimeBox->layout()->setMargin(0);
  m_boxLayout = new TQVBoxLayout(m_uptimeBox->layout());
  m_boxLayout->setAlignment(Qt::AlignTop);
  m_boxLayout->setSpacing(6);
  m_boxLayout->setMargin(11);

  m_udLabel = new TQLabel(m_uptimeBox);
  m_udLabel->setText(i18n("%d - Total days uptime"));
  m_boxLayout->addWidget(m_udLabel);

  m_uhLabel = new TQLabel(m_uptimeBox);
  m_uhLabel->setText(i18n("%h - Total hours uptime"));
  m_boxLayout->addWidget(m_uhLabel);

  m_umLabel = new TQLabel(m_uptimeBox);
  m_umLabel->setText(i18n("%m - Total minutes uptime"));
  m_boxLayout->addWidget(m_umLabel);

  m_usLabel = new TQLabel(m_uptimeBox);
  m_usLabel->setText(i18n("%s - Total seconds uptime"));
  m_boxLayout->addWidget(m_usLabel);
  m_mainLayout->addWidget(m_uptimeBox);

  TQSpacerItem *spacer = new TQSpacerItem(20, 20,
     TQSizePolicy::Minimum, TQSizePolicy::Expanding);
  m_mainLayout->addItem(spacer);
}

KSim::UptimePrefs::~UptimePrefs()
{
}

void KSim::UptimePrefs::saveConfig(KSim::Config *config)
{
  config->setUptimeItem(m_uptimeCombo->currentItem());
  config->setShowUptime(m_uptimeCheck->isChecked());

  TQStringList comboItems;
  for (int i = 0; i < m_uptimeCombo->count(); i++)
    comboItems.append(m_uptimeCombo->text(i));

  config->setUptimeFormat(comboItems);
}

void KSim::UptimePrefs::readConfig(KSim::Config *config)
{
  m_uptimeCheck->setChecked(config->showUptime());

  TQStringList::ConstIterator it;
  TQStringList items = config->uptimeFormatList();
  for (it = items.begin(); it != items.end(); ++it)
    if (!m_uptimeCombo->contains(*it))
      m_uptimeCombo->insertItem(*it);

  m_uptimeCombo->setCurrentItem(config->uptimeItem());
}

void KSim::UptimePrefs::uptimeContextMenu(TQPopupMenu *menu)
{
  menu->insertSeparator();
  menu->insertItem(m_addIcon, i18n("Insert Item"),
     this, TQT_SLOT(insertUptimeItem()));
  menu->insertItem(m_removeIcon, i18n("Remove Item"),
     this, TQT_SLOT(removeUptimeItem()));
}

void KSim::UptimePrefs::insertUptimeItem()
{
  TQString text = m_uptimeCombo->lineEdit()->text();
  if (!m_uptimeCombo->contains(text)) {
    m_uptimeCombo->insertItem(text);
    m_uptimeCombo->setCurrentItem(m_uptimeCombo->count() - 1);
  }
}

void KSim::UptimePrefs::removeUptimeItem()
{
  int currentItem = m_uptimeCombo->currentItem();
  m_uptimeCombo->removeItem(currentItem);
  m_uptimeCombo->setCurrentItem(currentItem - 1);
}

KSim::MemoryPrefs::MemoryPrefs(TQWidget *parent, const char *name)
   : TQWidget(parent, name)
{
  m_addIcon = SmallIconSet("filenew");
  m_removeIcon = SmallIconSet("editdelete");

  m_mainLayout = new TQVBoxLayout(this);
  m_mainLayout->setSpacing(6);

  m_memCombo = new KComboBox(true, this);
  m_memCombo->setDuplicatesEnabled(false);
  m_memCombo->setSizePolicy(TQSizePolicy(TQSizePolicy::Expanding,
     TQSizePolicy::Fixed));
  //m_memCombo->insertItem(i18n("%tM - %fM free"));
  //m_memCombo->insertItem(i18n("%tM - %uM used"));
  connect(m_memCombo,
     TQT_SIGNAL(aboutToShowContextMenu(TQPopupMenu *)),
     TQT_SLOT(memoryContextMenu(TQPopupMenu *)));

  m_memoryAdd = new TQPushButton(this);
  m_memoryAdd->setPixmap(SmallIcon("down"));
  connect(m_memoryAdd, TQT_SIGNAL(clicked()), TQT_SLOT(insertMemoryItem()));
  TQToolTip::add(m_memoryAdd, i18n("Insert item"));

  m_memCheck = new TQCheckBox(this);
  m_memCheck->setText(i18n("Show memory and free memory"));
  m_memCheck->setChecked(false);
  connect(m_memCheck, TQT_SIGNAL(toggled(bool)),
     m_memCombo, TQT_SLOT(setEnabled(bool)));
  connect(m_memCheck, TQT_SIGNAL(toggled(bool)),
     m_memoryAdd, TQT_SLOT(setEnabled(bool)));
  m_mainLayout->addWidget(m_memCheck);

  m_subLayout = new TQHBoxLayout;
  m_subLayout->setSpacing(6);

  m_memFormat = new TQLabel(this);
  m_memFormat->setText(i18n("Mem format:"));
  m_subLayout->addWidget(m_memFormat);

  m_subLayout->addWidget(m_memCombo);
  m_subLayout->addWidget(m_memoryAdd);
  m_mainLayout->addLayout(m_subLayout);

  m_memInfo = new TQLabel(this);
  m_memInfo->setText(i18n("The text in the edit box will be what "
           "is displayed as \nthe memory & free memory except the "
           "% items will be \nreplaced with the legend"));
  m_mainLayout->addWidget(m_memInfo);

  m_memBox = new TQGroupBox(this);
  m_memBox->setTitle(i18n("Memory Legend"));
  m_memBox->setColumnLayout(0, Qt::Vertical);
  m_memBox->layout()->setSpacing(0);
  m_memBox->layout()->setMargin(0);
  m_boxLayout = new TQVBoxLayout(m_memBox->layout());
  m_boxLayout->setAlignment(Qt::AlignTop);
  m_boxLayout->setSpacing(6);
  m_boxLayout->setMargin(11);

  m_mtLabel = new TQLabel(m_memBox);
  m_mtLabel->setText(i18n("%t - Total memory"));
  m_boxLayout->addWidget(m_mtLabel);

  m_mfLabel = new TQLabel(m_memBox);
  m_mfLabel->setText(i18n("%F - Total free memory including cached and buffered"));
  m_boxLayout->addWidget(m_mfLabel);

  m_mfLabel = new TQLabel(m_memBox);
  m_mfLabel->setText(i18n("%f - Total free memory"));
  m_boxLayout->addWidget(m_mfLabel);

  m_muLabel = new TQLabel(m_memBox);
  m_muLabel->setText(i18n("%u - Total used memory"));
  m_boxLayout->addWidget(m_muLabel);

  m_mcLabel = new TQLabel(m_memBox);
  m_mcLabel->setText(i18n("%c - Total cached memory"));
  m_boxLayout->addWidget(m_mcLabel);

  m_mbLabel = new TQLabel(m_memBox);
  m_mbLabel->setText(i18n("%b - Total buffered memory"));
  m_boxLayout->addWidget(m_mbLabel);

  m_msLabel = new TQLabel(m_memBox);
  m_msLabel->setText(i18n("%s - Total shared memory"));
  m_boxLayout->addWidget(m_msLabel);
  m_mainLayout->addWidget(m_memBox);

  TQSpacerItem *spacer = new TQSpacerItem(20, 20,
     TQSizePolicy::Minimum, TQSizePolicy::Expanding);
  m_mainLayout->addItem(spacer);
}

KSim::MemoryPrefs::~MemoryPrefs()
{
}

void KSim::MemoryPrefs::saveConfig(KSim::Config *config)
{
  config->setMemoryItem(m_memCombo->currentItem());
  config->setShowMemory(m_memCheck->isChecked());

  TQStringList comboItems;
  for (int i = 0; i < m_memCombo->count(); i++)
    comboItems.append(m_memCombo->text(i));

  config->setMemoryFormat(comboItems);
}

void KSim::MemoryPrefs::readConfig(KSim::Config *config)
{
  m_memCheck->setChecked(config->showMemory());

  TQStringList::ConstIterator it;
  TQStringList items = config->memoryFormatList();
  for (it = items.begin(); it != items.end(); ++it)
    if (!m_memCombo->contains(*it))
      m_memCombo->insertItem(*it);

  m_memCombo->setCurrentItem(config->memoryItem());
}

void KSim::MemoryPrefs::memoryContextMenu(TQPopupMenu *menu)
{
  menu->insertSeparator();
  menu->insertItem(m_addIcon, i18n("Insert Item"),
     this, TQT_SLOT(insertMemoryItem()));
  menu->insertItem(m_removeIcon, i18n("Remove Item"),
     this, TQT_SLOT(removeMemoryItem()));
}

void KSim::MemoryPrefs::insertMemoryItem()
{
  TQString text = m_memCombo->lineEdit()->text();
  if (!m_memCombo->contains(text)) {
    m_memCombo->insertItem(text);
    m_memCombo->setCurrentItem(m_memCombo->count() - 1);
  }
}

void KSim::MemoryPrefs::removeMemoryItem()
{
  int currentItem = m_memCombo->currentItem();
  m_memCombo->removeItem(currentItem);
  m_memCombo->setCurrentItem(currentItem - 1);
}

KSim::SwapPrefs::SwapPrefs(TQWidget *parent, const char *name)
   : TQWidget(parent, name)
{
  m_addIcon = SmallIconSet("filenew");
  m_removeIcon = SmallIconSet("editdelete");

  m_mainLayout = new TQVBoxLayout(this);
  m_mainLayout->setSpacing(6);

  m_swapCheck = new TQCheckBox(this);
  m_swapCheck->setText(i18n("Show swap and free swap"));
  m_swapCheck->setChecked(true);
  m_mainLayout->addWidget(m_swapCheck);

  m_swapCombo = new KComboBox(true, this);
  m_swapCombo->setDuplicatesEnabled(false);
  m_swapCombo->setSizePolicy(TQSizePolicy(TQSizePolicy::Expanding,
     TQSizePolicy::Fixed));
  //m_swapCombo->insertItem(i18n("%tM - %fM free"));
  //m_swapCombo->insertItem(i18n("%tM - %uM used"));
  
  connect(m_swapCombo,
     TQT_SIGNAL(aboutToShowContextMenu(TQPopupMenu *)),
     TQT_SLOT(swapContextMenu(TQPopupMenu *)));

  m_swapAdd = new TQPushButton(this);
  m_swapAdd->setPixmap(SmallIcon("down"));
  connect(m_swapAdd, TQT_SIGNAL(clicked()), TQT_SLOT(insertSwapItem()));
  TQToolTip::add(m_swapAdd, i18n("Insert item"));

  m_subLayout = new TQHBoxLayout;
  m_subLayout->setSpacing(6);
  connect(m_swapCheck, TQT_SIGNAL(toggled(bool)),
     m_swapCombo, TQT_SLOT(setEnabled(bool)));
  connect(m_swapCheck, TQT_SIGNAL(toggled(bool)),
     m_swapAdd, TQT_SLOT(setEnabled(bool)));

  m_swapFormat = new TQLabel(this);
  m_swapFormat->setText(i18n("Swap format:"));
  m_subLayout->addWidget(m_swapFormat);

  m_subLayout->addWidget(m_swapCombo);
  m_subLayout->addWidget(m_swapAdd);
  m_mainLayout->addLayout(m_subLayout);

  m_swapInfo = new TQLabel(this);
  m_swapInfo->setText(i18n("The text in the edit box will be what is "
             "displayed as \nthe swap & free swap except the % items "
             "will be \nreplaced with the legend"));
  m_mainLayout->addWidget(m_swapInfo);

  m_swapBox = new TQGroupBox(this);
  m_swapBox->setTitle(i18n("Swap Legend"));
  m_swapBox->setColumnLayout(0, Qt::Vertical);
  m_swapBox->layout()->setSpacing(0);
  m_swapBox->layout()->setMargin(0);
  m_boxLayout = new TQVBoxLayout(m_swapBox->layout());
  m_boxLayout->setAlignment(Qt::AlignTop);
  m_boxLayout->setSpacing(6);
  m_boxLayout->setMargin(11);

  m_stLabel = new TQLabel(m_swapBox);
  m_stLabel->setText(i18n("%t - Total swap"));
  m_boxLayout->addWidget(m_stLabel);

  m_sfLabel = new TQLabel(m_swapBox);
  m_sfLabel->setText(i18n("%f - Total free swap"));
  m_boxLayout->addWidget(m_sfLabel);

  m_suLabel = new TQLabel(m_swapBox);
  m_suLabel->setText(i18n("%u - Total used swap"));
  m_boxLayout->addWidget(m_suLabel);
  m_mainLayout->addWidget(m_swapBox);

  TQSpacerItem *spacer = new TQSpacerItem(20, 20,
     TQSizePolicy::Minimum, TQSizePolicy::Expanding);
  m_mainLayout->addItem(spacer);
}

KSim::SwapPrefs::~SwapPrefs()
{
}

void KSim::SwapPrefs::saveConfig(KSim::Config *config)
{
  config->setSwapItem(m_swapCombo->currentItem());

  TQStringList comboItems;
  for (int i = 0; i < m_swapCombo->count(); i++)
    comboItems.append(m_swapCombo->text(i));

  config->setSwapFormat(comboItems);
  config->setShowSwap(m_swapCheck->isChecked());
}

void KSim::SwapPrefs::readConfig(KSim::Config *config)
{
  m_swapCheck->setChecked(config->showSwap());

  TQStringList::ConstIterator it;
  TQStringList items = config->swapFormatList();
  for (it = items.begin(); it != items.end(); ++it)
    if (!m_swapCombo->contains(*it))
      m_swapCombo->insertItem(*it);

  m_swapCombo->setCurrentItem(config->swapItem());
}

void KSim::SwapPrefs::swapContextMenu(TQPopupMenu *menu)
{
  menu->insertSeparator();
  menu->insertItem(m_addIcon, i18n("Insert Item"), this, TQT_SLOT(insertSwapItem()));
  menu->insertItem(m_removeIcon, i18n("Remove Item"), this, TQT_SLOT(removeSwapItem()));
}

void KSim::SwapPrefs::insertSwapItem()
{
  TQString text = m_swapCombo->lineEdit()->text();
  if (!m_swapCombo->contains(text)) {
    m_swapCombo->insertItem(text);
    m_swapCombo->setCurrentItem(m_swapCombo->count() - 1);
  }
}

void KSim::SwapPrefs::removeSwapItem()
{
  int currentItem = m_swapCombo->currentItem();
  m_swapCombo->removeItem(currentItem);
  m_swapCombo->setCurrentItem(currentItem - 1);
}
