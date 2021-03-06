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

#include "ksimsensors.h"
#include "ksimsensors.moc"

#include <kdebug.h>
#include <tdeapplication.h>
#include <tdelocale.h>
#include <tdeaboutapplication.h>
#include <tdeaboutdata.h>
#include <tdeconfig.h>

#include <tqlayout.h>

#include <label.h>
#include <themetypes.h>
#include "sensorsconfig.h"
#include "sensorbase.h"

KSIM_INIT_PLUGIN(PluginModule)

PluginModule::PluginModule(const char *name)
           : KSim::PluginObject(name)
{
  setConfigFileName(instanceName());
}

PluginModule::~PluginModule()
{
}

KSim::PluginView *PluginModule::createView(const char *className)
{
  return new SensorsView(this, className);
}

KSim::PluginPage *PluginModule::createConfigPage(const char *className)
{
  return new SensorsConfig(this, className);
}

void PluginModule::showAbout()  
{
  TQString version = kapp->aboutData()->version();

  TDEAboutData aboutData(instanceName(),
     I18N_NOOP("KSim Sensors Plugin"), version.latin1(),
     I18N_NOOP("An lm_sensors plugin for KSim"),
     TDEAboutData::License_GPL, "(C) 2001 Robbie Ward");

  aboutData.addAuthor("Robbie Ward", I18N_NOOP("Author"),
     "linuxphreak@gmx.co.uk");

  TDEAboutApplication(&aboutData).exec();
}

SensorsView::SensorsView(KSim::PluginObject *parent, const char *name)
   : DCOPObject("sensors"), KSim::PluginView(parent, name)
{
  config()->setGroup("Sensors");
  (new TQVBoxLayout(this))->setAutoAdd(true);

  connect(SensorBase::self(), TQT_SIGNAL(updateSensors(const SensorList &)),
     this, TQT_SLOT(updateSensors(const SensorList &)));

  insertSensors();
}

SensorsView::~SensorsView()
{
}

void SensorsView::reparseConfig()
{
  config()->setGroup("Sensors");
  bool displayFahrenheit = config()->readBoolEntry("displayFahrenheit", false);
  int updateVal = config()->readNumEntry("sensorUpdateValue", 5);
  SensorBase::self()->setDisplayFahrenheit(displayFahrenheit);
  SensorBase::self()->setUpdateSpeed(updateVal * 1000);

  TQString label;
  TQStringList names;
  SensorItemList sensorItemList;
  const SensorList &list = SensorBase::self()->sensorsList();

  SensorList::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    config()->setGroup("Sensors");
    label = (*it).sensorType() + "/" + (*it).sensorName();
    names = TQStringList::split(':', config()->readEntry(label));
    if (names[0] == "1")
      sensorItemList.append(SensorItem((*it).sensorId(), names[1]));
  }

  if (sensorItemList == m_sensorItemList)
    return;

  m_sensorItemList.clear();
  m_sensorItemList = sensorItemList;
  insertSensors(false);
}

void SensorsView::insertSensors(bool createList)
{
  const SensorList &list = SensorBase::self()->sensorsList();

  if (createList) {
    TQString label;
    TQStringList names;

    config()->setGroup("Sensors");
    bool displayFahrenheit = config()->readBoolEntry("displayFahrenheit", false);
    int updateVal = config()->readNumEntry("sensorUpdateValue", 5);
    SensorBase::self()->setDisplayFahrenheit(displayFahrenheit);
    SensorBase::self()->setUpdateSpeed(updateVal * 1000);

    SensorList::ConstIterator it;
    for (it = list.begin(); it != list.end(); ++it) {
      label = (*it).sensorType() + "/" + (*it).sensorName();
      names = TQStringList::split(':', config()->readEntry(label));
      if (names[0] == "1")
        m_sensorItemList.append(SensorItem((*it).sensorId(), names[1]));
    }
  }

  SensorItemList::Iterator item;
  for (item = m_sensorItemList.begin(); item != m_sensorItemList.end(); ++item)
    (*item).setLabel(new KSim::Label(KSim::Types::None, this));

  updateSensors(list);
}

void SensorsView::updateSensors(const SensorList &sensorList)
{
  if (sensorList.isEmpty() || m_sensorItemList.isEmpty())
    return;

  SensorList::ConstIterator it;
  for (it = sensorList.begin(); it != sensorList.end(); ++it) {
    SensorItemList::Iterator item;
    for (item = m_sensorItemList.begin(); item != m_sensorItemList.end(); ++item) {
      if ((*item).id == (*it).sensorId()) {
        if (!(*item).label->isVisible())
          (*item).label->show();

        (*item).label->setText((*item).name + ": " +
           (*it).sensorValue() + (*it).sensorUnit());
      }
    }
  }
}

TQString SensorsView::sensorValue(const TQString &sensor,
    const TQString &label1)
{
  const SensorList &list = SensorBase::self()->sensorsList();
  config()->setGroup("Sensors");
  TQStringList names = TQStringList::split(':',
     config()->readEntry(sensor + "/" + label1));

  if (names[0] == "0" || list.isEmpty())
    return i18n("Sensor specified not found.");

  SensorList::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    if (sensor == (*it).sensorType() && label1 == (*it).sensorName()) {
      return names[1] + TQString(": ") + (*it).sensorValue() + (*it).sensorUnit();
    }
  }

  return i18n("Sensor specified not found.");
}
