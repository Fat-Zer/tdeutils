/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef DATESENSOR_H
#define DATESENSOR_H
#include "sensor.h"
#include "sensorparams.h"
#include <tqdatetime.h>
#include <kdatepicker.h>
#include <tqvbox.h>

class DatePicker : public TQVBox
{
public:
	DatePicker(TQWidget*);
private:
	KDatePicker *picker;
	void keyReleaseEvent(TQKeyEvent *e);
};

class DateSensor :  public Sensor
{
Q_OBJECT
  
public:
  DateSensor( int interval );
  ~DateSensor();

  void toggleCalendar(TQMouseEvent *ev);
  void mousePressEvent(TQMouseEvent *ev);
  void update();

protected slots:
  void slotCalendarDeleted();

private:
  bool hidden;
  DatePicker* cal;

};

#endif // SENSOR_H
