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

#ifndef SENSORBASE_H
#define SENSORBASE_H

#include <qobject.h>
#include <qvaluelist.h>
#include <qcstring.h>

class QTimer;
class KLibrary;

class SensorInfo
{
  public:
    SensorInfo() {}
    SensorInfo(int id, 
       const QString &sensorValue, 
       const QString &sensorName, 
       const QString &sensorType,
       const QString &chipsetName, 
       const QString &sensorUnit)
       : m_id(id), m_sensor(sensorValue),
       m_sensorName(sensorName),
       m_sensorType(sensorType), 
       m_chipsetName(chipsetName),
       m_sensorUnit(sensorUnit) {}

    /**
     * @return the sensor Id
     */
    int sensorId() const { return m_id; }
    /**
     * @return the current value, eg: 5000
     */
    const QString &sensorValue() const { return m_sensor; }
    /**
     * @return the sensor name, eg: temp1, fan2
     */
    const QString &sensorName() const { return m_sensorName; }
    /**
     * @return the sensor type name, eg: w83782d
     */
    const QString &sensorType() const { return m_sensorType; }
    /**
     * @return the chipset name, eg: w83782d-i2c-0-2d
     */
    const QString &chipsetName() const { return m_chipsetName; }
    /**
     * @return the unit name, eg: RPM, °C or °F if display fahrenheit is enabled
     */
    const QString &sensorUnit() const { return m_sensorUnit; }

  private:
    int m_id;
    QString m_sensor;
    QString m_sensorName;
    QString m_sensorType;
    QString m_chipsetName;
    QString m_sensorUnit;
};

class SensorList : public QValueList<SensorInfo>
{
  public:
    SensorList() {}
    SensorList(const SensorList &sensorList) 
       : QValueList<SensorInfo>(sensorList) {}
    SensorList(const QValueList<SensorInfo> &sensorList) 
       : QValueList<SensorInfo>(sensorList) {}
    SensorList(const SensorInfo &sensor) { append(sensor); }
};

class SensorBase : public QObject
{
  Q_OBJECT
  public:
    static SensorBase *self();

    const SensorList &sensorsList() const { return m_sensorList; }
    bool fahrenheit() { return m_fahrenheit; }

  signals:
    void updateSensors(const SensorList&);

  public slots:
    void setUpdateSpeed(uint);
    void setDisplayFahrenheit(bool fah) { m_fahrenheit = fah; }

  private slots:
    void update();

  protected:
    SensorBase();
    ~SensorBase();

  private:
    SensorBase(const SensorBase &);
    SensorBase &operator=(const SensorBase &);

    static void cleanup();
    struct ChipName
    {
      char *prefix;
      int bus;
      int addr;
      char *busname;
    };

    struct FeatureData
    {
      int number;
      const char *name;
      int mapping;
      int unused;
      int mode;
    };

    enum { ProcError=4, NoMapping=-1, BusISA=-1 };
    typedef void (*Cleanup)();
    typedef int (*Init)(FILE *);
    typedef const char *(*Error)(int);
    typedef const ChipName *(GetDetectedChips)(int *);
    typedef const FeatureData *(GetAllFeatures)(ChipName, int *, int *);
    typedef int (*Label)(ChipName, int, char **);
    typedef int (*Feature)(ChipName, int, double *);

    bool init();
    float toFahrenheit(float value);
    QString sensorType(const QString &name);
    QString chipsetString(const ChipName *c);
    float formatValue(const QString &label, float value);
    QString formatString(const QString &label, float value);

    SensorList m_sensorList;
    QTimer *m_updateTimer;
    KLibrary *m_library;
    QCString m_libLocation;
    bool m_loaded;
    bool m_fahrenheit;

    bool m_hasNVControl;
    
    Init m_init;
    Error m_error;
    GetDetectedChips *m_detectedChips;
    GetAllFeatures *m_allFeatures;
    Label m_label;
    Feature m_feature;
    Cleanup m_cleanup;
    static SensorBase *m_self;
};

inline float SensorBase::toFahrenheit(float value)
{
  return fahrenheit() ? (value * (9.0F / 5.0F) + 32.0F) : value;
}
#endif // SENSORBASE_H
