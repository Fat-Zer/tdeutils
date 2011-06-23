/***************************************************************************
 *   Copyright (C) 2003 by Hans Karlsson                                   *
 *   karlsson.h@home.se                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/

#ifndef GRAPH_H
#define GRAPH_H

#include "meter.h"
#include <tqpainter.h>
#include <tqcolor.h>

class Graph : public Meter
{
Q_OBJECT
  TQ_OBJECT

public:
  Graph(karamba* k, int ix, int iy, int iw, int ih, int nbrPoints);
  Graph();
  ~Graph();

  void setValue( long );
  long getValue() { return lastValue; };
  void setValue( TQString );
  void mUpdate( TQPainter * );

private:
  int lastValue;
  int* values;
  int nbrPoints;
  int ptPtr;  // points to the array position
};

#endif // GRAPH_H
