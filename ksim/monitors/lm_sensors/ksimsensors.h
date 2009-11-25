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

#ifndef SENSORS_H
#define SENSORS_H

#include <pluginmodule.h>
#include <label.h>
#include "ksimsensorsiface.h"

class KConfig;
class SensorList;
class KInstance;

/**
 * This class is the lm_sensors monitor plugin
 *
 * @author Robbie Ward <linuxphreak@gmx.co.uk>
 */
class PluginModule : public KSim::PluginObject
{
  public:
    PluginModule(const char *name);
    ~PluginModule();

    virtual KSim::PluginView *createView(const char *);
    virtual KSim::PluginPage *createConfigPage(const char *);

    virtual void showAbout();
};

class SensorsView : public KSim::PluginView, public KSimSensorsIface
{
  Q_OBJECT
  public:
    SensorsView(KSim::PluginObject *parent, const char *name);
    ~SensorsView();

    virtual void reparseConfig();
    virtual QString sensorValue(const QString &, const QString &);

  private slots:
    void updateSensors(const SensorList &);

  private:
    void insertSensors(bool createList = true);

    class SensorItem
    {
      public:
        SensorItem() : id(-1), label(0) {}
        SensorItem(int i, const QString &na)
           : id(i), name(na), label(0) {}
        ~SensorItem() { delete label; }

        bool operator==(const SensorItem &rhs) const
        {
          return (id == rhs.id && name == rhs.name);
        }

        void setLabel(KSim::Label *lab)
        {
          delete label;
          label = lab;
        }

        int id;
        QString name;
        KSim::Label *label;
    };

    typedef QValueList<SensorItem> SensorItemList;
    SensorItemList m_sensorItemList;
};
#endif
