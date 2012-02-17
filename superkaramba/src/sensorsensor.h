
#ifndef SENSORSENSOR_H
#define SENSORSENSOR_H

#include <tqstring.h>
#include <tqtextcodec.h>
#include <tqmap.h>
#include <tqstringlist.h>
#include <tqregexp.h>
#include <kprocess.h>
#include <kprocio.h>


#include "sensor.h"

/**
 *
 * Hans Karlsson
 **/
class SensorSensor : public Sensor
{
    Q_OBJECT
  
public:
    SensorSensor(int interval, char tempUnit);

    ~SensorSensor();

    void update();


private:
    KShellProcess ksp;
    TQString extraParams;

    TQMap<TQString,TQString> sensorMap;
#ifdef __FreeBSD__
    TQMap<TQString,TQString> sensorMapBSD;
#endif
    TQString sensorResult;

private slots:
    void receivedStdout(KProcess *, char *buffer, int);
    void processExited(KProcess *);



};

#endif
