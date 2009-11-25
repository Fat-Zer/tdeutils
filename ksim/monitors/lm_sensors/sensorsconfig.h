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

#ifndef SENSORSIMPL_H
#define SENSORSIMPL_H

#include <pluginmodule.h>

class QPushButton;
class QCheckListItem;
class QGridLayout;
class QLabel;
class KListView;
class QListViewItem;
class QPopupMenu;
class KIntSpinBox;
class QCheckBox;
class QTimer;

class SensorsConfig : public KSim::PluginPage
{
  Q_OBJECT
  public:
    SensorsConfig(KSim::PluginObject *parent, const char *name);
    ~SensorsConfig();

    void saveConfig();
    void readConfig();

  private slots:
    void menu(KListView *, QListViewItem *, const QPoint &);
    void initSensors();
    void selectAll();
    void unSelectAll();
    void invertSelect();

    void modify( QListViewItem * );
    void modify();

  protected:
    void showEvent(QShowEvent *);

  private:
    bool m_neverShown;
    QLabel *m_updateLabel;
    KIntSpinBox *m_sensorSlider;
    KListView *m_sensorView;
    QCheckBox *m_fahrenBox;
    QGridLayout *m_layout;
    QPopupMenu *m_popupMenu;
    QPushButton * m_modify;
};
#endif
