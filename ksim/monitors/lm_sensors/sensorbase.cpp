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

#include "sensorbase.h"
#include "sensorbase.moc"
#include <ksimconfig.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <tqglobal.h>
#include <klibloader.h>
#include <kstandarddirs.h>
#include <tdeconfig.h>

#include <tqtimer.h>
#include <tqfile.h>
#include <tqregexp.h>

#include <stdio.h>

#include <X11/Xlib.h>

#include "NVCtrl.h"
#include "NVCtrlLib.h"

SensorBase *SensorBase::m_self = 0;
SensorBase *SensorBase::self()
{
  if (!m_self) {
    m_self = new SensorBase;
    tqAddPostRoutine(cleanup);
  }

  return m_self;
}

SensorBase::SensorBase() : TQObject()
{
  KSim::Config::config()->setGroup("Sensors");
  TQCString sensorsName("libsensors.so");

  TQStringList sensorLocations = KSim::Config::config()->readListEntry("sensorLocations");

  TQStringList::ConstIterator it;
  for (it = sensorLocations.begin(); it != sensorLocations.end(); ++it) {
    if (TQFile::exists((*it).local8Bit() + sensorsName)) {
      m_libLocation = (*it).local8Bit() + sensorsName;
      break;
    }
  }

  m_library = KLibLoader::self()->library(m_libLocation);
  kdDebug(2003) << sensorsName << " location = " << m_libLocation << endl;
  m_loaded = init();

  {
      int eventBase;
      int errorBase;
      m_hasNVControl = XNVCTRLQueryExtension(tqt_xdisplay(), &eventBase, &errorBase) == True;
  }

  m_updateTimer = new TQTimer(this);
  connect(m_updateTimer, TQT_SIGNAL(timeout()), TQT_SLOT(update()));
}

SensorBase::~SensorBase()
{
  if (!m_libLocation.isNull()) {
    if (m_cleanup) m_cleanup();
    KLibLoader::self()->unloadLibrary(m_libLocation);
  }
}

void SensorBase::setUpdateSpeed(uint speed)
{
  if (m_updateTimer->isActive())
    m_updateTimer->stop();
    
  update(); // just to make sure the display is updated ASAP

  if (speed == 0)
    return;

  m_updateTimer->start(speed);
}

void SensorBase::update()
{
  if (!m_loaded)
    return;

  m_sensorList.clear();
  const ChipName *chip;
  int currentSensor = 0;
  int sensor = 0;

  while ((chip = m_detectedChips(&sensor)) != NULL) {
    int first = 0, second = 0;
    const FeatureData *sFeature = 0;
    while ((sFeature = m_allFeatures(*chip, &first, &second)) != NULL) {
      if (sFeature->mapping == NoMapping) {
        char *name;
        double value;
        m_label(*chip, sFeature->number, &name);
        m_feature(*chip, sFeature->number, &value);

        float returnValue = formatValue(TQString::fromUtf8(name), float(value));
        TQString label = formatString(TQString::fromUtf8(name), returnValue);
        TQString chipset = chipsetString(chip);

        m_sensorList.append(SensorInfo(currentSensor++, label,
           TQString::fromUtf8(name), TQString::fromUtf8(chip->prefix),
           chipset, sensorType(TQString::fromLatin1(name))));
      }
    }
  }

  if (m_hasNVControl) {
      int temp = 0;
      if (XNVCTRLQueryAttribute(tqt_xdisplay(), tqt_xscreen(), 0 /* not used? */, NV_CTRL_GPU_CORE_TEMPERATURE, &temp)) {
          TQString name = TQString::fromLatin1("GPU Temp");
          m_sensorList.append(SensorInfo(currentSensor++, TQString::number(temp),
                                         name, TQString(), TQString(), sensorType(name)));
      }

      if (XNVCTRLQueryAttribute(tqt_xdisplay(), tqt_xscreen(), 0 /* not used? */, NV_CTRL_AMBIENT_TEMPERATURE, &temp)) {
          TQString name = TQString::fromLatin1("GPU Ambient Temp");
          m_sensorList.append(SensorInfo(currentSensor++, TQString::number(temp),
                                         name, TQString(), TQString(), sensorType(name)));
      }

  }

  emit updateSensors(m_sensorList);
}

void SensorBase::cleanup()
{
  if (!m_self)
    return;

  delete m_self;
  m_self = 0;
}

bool SensorBase::init()
{
  if (m_libLocation.isNull()) {
    kdError() << "Unable to find libsensors.so" << endl;
    return false;
  }

  m_init = (Init)m_library->symbol("sensors_init");
  if (!m_init)
    return false;

  m_error = (Error)m_library->symbol("sensors_strerror");
  if (!m_error)
    return false;

  m_detectedChips = (GetDetectedChips *)m_library->symbol("sensors_get_detected_chips");
  m_allFeatures = (GetAllFeatures *)m_library->symbol("sensors_get_all_features");
  m_label = (Label)m_library->symbol("sensors_get_label");
  m_feature = (Feature)m_library->symbol("sensors_get_feature");

  if (!m_detectedChips || !m_allFeatures || !m_label || !m_feature)
    return false;

  m_cleanup = (Cleanup)m_library->symbol("sensors_cleanup");
  if (!m_cleanup)
    return false;

  int res;
  FILE *input;

  if ((input = fopen("/etc/sensors.conf", "r")) == 0)
    return false;

  if ((res = m_init(input))) {
    if (res == ProcError) {
      kdError() << "There was an error reading the sensor information\n"
         << "Make sure sensors.o and i2c-proc.o are loaded" << endl;
    }
    else {
      kdError() << m_error(res) << endl;
    }

    fclose(input);
    return false;
  }

  fclose(input);

  return true;
}

TQString SensorBase::sensorType(const TQString &name)
{
  if (name.findRev("fan", -1, false) != -1)
    return i18n("Rounds per minute", " RPM");

  if (name.findRev("temp", -1, false) != -1)
    if (SensorBase::fahrenheit())
      return TQString::fromLatin1("�F");
    else
      return TQString::fromLatin1("�C");

  if (name.findRev(TQRegExp("[^\\+]?[^\\-]?V$")) != -1)
    return i18n("Volt", "V");

  return TQString();
}

TQString SensorBase::chipsetString(const ChipName *c)
{
  TQString data = TQString::fromUtf8(c->prefix);

  if (c->bus == BusISA)
    return TQString().sprintf("%s-isa-%04x", data.utf8().data(), c->addr);

  return TQString().sprintf("%s-i2c-%d-%02x", data.utf8().data(), c->bus, c->addr);
}

float SensorBase::formatValue(const TQString &label, float value)
{
  if (label.findRev("temp", -1, false) != -1)
      return toFahrenheit(value);

 return value;
}

TQString SensorBase::formatString(const TQString &label, float value)
{
  if (label.findRev("fan", -1, false) != -1)
    return TQString::number(value);

  return TQString::number(value,'f',2);
}
