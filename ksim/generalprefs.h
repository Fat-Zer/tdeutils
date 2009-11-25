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

#include <qwidget.h>
#include <qiconset.h>

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
      GeneralPrefs(QWidget *parent, const char *name=0);
      ~GeneralPrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private:
      QGridLayout *m_mainLayout;
      QGroupBox *m_sizeBox;
      QLabel *m_sizeHLabel;
      KIntSpinBox *m_sizeHSpin;
      QLabel *m_sizeWLabel;
      KIntSpinBox *m_sizeWSpin;
      QCheckBox *m_displayFqdn;
      QCheckBox *m_recolourThemes;
  };

  class ClockPrefs : public QWidget
  {
    Q_OBJECT
    public:
      ClockPrefs(QWidget *parent, const char *name=0);
      ~ClockPrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private:
      QVBoxLayout *m_mainLayout;
      QCheckBox *m_timeCheck;
      QCheckBox *m_dateCheck;
  };

  class UptimePrefs : public QWidget
  {
    Q_OBJECT
    public:
      UptimePrefs(QWidget *parent, const char *name=0);
      ~UptimePrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private slots:
      void uptimeContextMenu(QPopupMenu *);
      void insertUptimeItem();
      void removeUptimeItem();

    private:
      QVBoxLayout *m_mainLayout;
      QHBoxLayout *m_subLayout;
      QVBoxLayout *m_boxLayout;
      KComboBox *m_uptimeCombo;
      QPushButton *m_uptimeAdd;
      QCheckBox *m_uptimeCheck;
      QLabel *m_formatLabel;
      QLabel *m_uptimeInfo;
      QGroupBox *m_uptimeBox;
      QLabel *m_udLabel;
      QLabel *m_uhLabel;
      QLabel *m_umLabel;
      QLabel *m_usLabel;
      QIconSet m_addIcon;
      QIconSet m_removeIcon;
  };

  class MemoryPrefs : public QWidget
  {
    Q_OBJECT
    public:
      MemoryPrefs(QWidget *parent, const char *name=0);
      ~MemoryPrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private slots:
      void memoryContextMenu(QPopupMenu *);
      void insertMemoryItem();
      void removeMemoryItem();

    private:
      QVBoxLayout *m_mainLayout;
      QHBoxLayout *m_subLayout;
      QVBoxLayout *m_boxLayout;
      QCheckBox *m_memCheck;
      QLabel *m_memFormat;
      KComboBox *m_memCombo;
      QLabel *m_memInfo;
      QGroupBox *m_memBox;
      QLabel *m_mtLabel;
      QLabel *m_mfLabel;
      QLabel *m_muLabel;
      QLabel *m_mcLabel;
      QLabel *m_mbLabel;
      QLabel *m_msLabel;
      QPushButton *m_memoryAdd;
      QIconSet m_addIcon;
      QIconSet m_removeIcon;
  };

  class SwapPrefs : public QWidget
  {
    Q_OBJECT
    public:
      SwapPrefs(QWidget *parent, const char *name=0);
      ~SwapPrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private slots:
      void swapContextMenu(QPopupMenu *);
      void insertSwapItem();
      void removeSwapItem();

    private:
      QVBoxLayout *m_mainLayout;
      QHBoxLayout *m_subLayout;
      QVBoxLayout *m_boxLayout;
      QCheckBox *m_swapCheck;
      QLabel *m_swapFormat;
      KComboBox *m_swapCombo;
      QLabel *m_swapInfo;
      QGroupBox *m_swapBox;
      QLabel *m_stLabel;
      QLabel *m_sfLabel;
      QLabel *m_suLabel;
      QPushButton *m_swapAdd;
      QIconSet m_addIcon;
      QIconSet m_removeIcon;
  };
}
#endif
