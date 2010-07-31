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

#ifndef GENERAL_H
#define GENERAL_H

#include <tqwidget.h>
#include <tqiconset.h>

class QVBoxLayout;
class QHBoxLayout;
class QGridLayout;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QTabWidget;
class KComboBox;
class QPopupMenu;
class QPushButton;
class KIntSpinBox;

namespace KSim
{
  class Config;

  class GeneralPrefs : public QWidget
  {
    Q_OBJECT
    public:
      GeneralPrefs(TQWidget *parent, const char *name=0);
      ~GeneralPrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private:
      TQGridLayout *m_mainLayout;
      TQGroupBox *m_sizeBox;
      TQLabel *m_sizeHLabel;
      KIntSpinBox *m_sizeHSpin;
      TQLabel *m_sizeWLabel;
      KIntSpinBox *m_sizeWSpin;
      TQCheckBox *m_displayFqdn;
      TQCheckBox *m_recolourThemes;
  };

  class ClockPrefs : public QWidget
  {
    Q_OBJECT
    public:
      ClockPrefs(TQWidget *parent, const char *name=0);
      ~ClockPrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private:
      TQVBoxLayout *m_mainLayout;
      TQCheckBox *m_timeCheck;
      TQCheckBox *m_dateCheck;
  };

  class UptimePrefs : public QWidget
  {
    Q_OBJECT
    public:
      UptimePrefs(TQWidget *parent, const char *name=0);
      ~UptimePrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private slots:
      void uptimeContextMenu(TQPopupMenu *);
      void insertUptimeItem();
      void removeUptimeItem();

    private:
      TQVBoxLayout *m_mainLayout;
      TQHBoxLayout *m_subLayout;
      TQVBoxLayout *m_boxLayout;
      KComboBox *m_uptimeCombo;
      TQPushButton *m_uptimeAdd;
      TQCheckBox *m_uptimeCheck;
      TQLabel *m_formatLabel;
      TQLabel *m_uptimeInfo;
      TQGroupBox *m_uptimeBox;
      TQLabel *m_udLabel;
      TQLabel *m_uhLabel;
      TQLabel *m_umLabel;
      TQLabel *m_usLabel;
      TQIconSet m_addIcon;
      TQIconSet m_removeIcon;
  };

  class MemoryPrefs : public QWidget
  {
    Q_OBJECT
    public:
      MemoryPrefs(TQWidget *parent, const char *name=0);
      ~MemoryPrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private slots:
      void memoryContextMenu(TQPopupMenu *);
      void insertMemoryItem();
      void removeMemoryItem();

    private:
      TQVBoxLayout *m_mainLayout;
      TQHBoxLayout *m_subLayout;
      TQVBoxLayout *m_boxLayout;
      TQCheckBox *m_memCheck;
      TQLabel *m_memFormat;
      KComboBox *m_memCombo;
      TQLabel *m_memInfo;
      TQGroupBox *m_memBox;
      TQLabel *m_mtLabel;
      TQLabel *m_mfLabel;
      TQLabel *m_muLabel;
      TQLabel *m_mcLabel;
      TQLabel *m_mbLabel;
      TQLabel *m_msLabel;
      TQPushButton *m_memoryAdd;
      TQIconSet m_addIcon;
      TQIconSet m_removeIcon;
  };

  class SwapPrefs : public QWidget
  {
    Q_OBJECT
    public:
      SwapPrefs(TQWidget *parent, const char *name=0);
      ~SwapPrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private slots:
      void swapContextMenu(TQPopupMenu *);
      void insertSwapItem();
      void removeSwapItem();

    private:
      TQVBoxLayout *m_mainLayout;
      TQHBoxLayout *m_subLayout;
      TQVBoxLayout *m_boxLayout;
      TQCheckBox *m_swapCheck;
      TQLabel *m_swapFormat;
      KComboBox *m_swapCombo;
      TQLabel *m_swapInfo;
      TQGroupBox *m_swapBox;
      TQLabel *m_stLabel;
      TQLabel *m_sfLabel;
      TQLabel *m_suLabel;
      TQPushButton *m_swapAdd;
      TQIconSet m_addIcon;
      TQIconSet m_removeIcon;
  };
}
#endif
