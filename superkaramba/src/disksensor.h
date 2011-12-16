/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef DISKSENSOR_H
#define DISKSENSOR_H
#include "sensor.h"
#include <tqmap.h>
#include <tqstring.h>
#include <tqtextcodec.h>
#include <tqregexp.h>
#include <tqstringlist.h>
#include <kprocess.h>
class DiskSensor :  public Sensor
{
Q_OBJECT
  TQ_OBJECT
public:
  DiskSensor(int msec );
  ~DiskSensor();
  void update();
  void setMaxValue( SensorParams *sp );

private:
  long getFreeSpace(TQString mntPt) const;
  long getUsedSpace(TQString mntPt) const;
  long getTotalSpace(TQString mntPt) const;
  int getPercentUsed(TQString mntPt) const;
  int getPercentFree(TQString mntPt) const;

  KShellProcess ksp;
  TQString sensorResult;

  TQMap<TQString,TQString> mntMap;
  TQStringList stringList;

  int init;

private slots:
  void receivedStdout(KProcess *, char *buffer, int);
  void processExited(KProcess *);

signals:
  void initComplete();

};
#endif // DISKSENSOR_H
