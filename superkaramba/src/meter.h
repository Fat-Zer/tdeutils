/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef METER_H
#define METER_H

#include <tqpixmap.h>
#include <tqpainter.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqobject.h>

#include <tqfileinfo.h>

class karamba;

class Meter : public TQObject
{
Q_OBJECT
  TQ_OBJECT
public:

  Meter(karamba* k, int ix,int iy,int iw,int ih);
  Meter(karamba* k);
  virtual ~Meter();
  virtual int getX();
  virtual int getY();
  virtual int getWidth();
  virtual int getHeight();
  virtual void setX(int);
  virtual void setY(int);
  virtual void setWidth(int);
  virtual void setHeight(int);

  virtual void setSize(int ix, int iy, int iw, int ih);

  virtual void setMax(long max) { maxValue = max; };
  virtual void setMin(long min) { minValue = min; };
  virtual long getMax() { return minValue; };
  virtual long getMin() { return maxValue; };

  void setThemePath( TQString );

  virtual void mUpdate(TQPainter *)=0 ;

  virtual void setValue(long) {};
  virtual long getValue() { return -1; };
  virtual void setValue(TQString) {};
  virtual TQString getStringValue() const { return TQString(); };
  virtual void recalculateValue() {};

  virtual void setColor(TQColor clr) { color = clr; };
  virtual TQColor getColor() { return color; };

  virtual void show() { hidden = 0; };
  virtual void hide() { hidden = 1; };

  TQRect getBoundingBox();

  // true when given coordinate point is inside the meters
  // active reagion and meter is enabled
  virtual bool insideActiveArea(int, int);

  // returns true when callback meterClicked should be called.
  virtual bool click( TQMouseEvent* );

  void setEnabled(bool);
  bool isEnabled();

  /*
  void setOnClick( TQString );
  void setOnMiddleClick( TQString );
  */

protected: // Protected attributes
  TQString themePath;

  TQRect boundingBox;

  // Actions to execute when clicked on meter
  TQString leftButtonAction;
  TQString middleButtonAction;
  TQString rightButtonAction;

  bool clickable;
  int hidden;
  long minValue;
  long maxValue;

  TQColor color;
  karamba* m_karamba;
};

#endif // METER_H
