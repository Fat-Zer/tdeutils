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

#ifndef KSIM__CHART_H
#define KSIM__CHART_H

#include <qwidget.h>
#include "common.h"

#include <kdemacros.h>

class QColor;

namespace KSim
{
  /**
   * provides a graph displaying data onscreen using gkrellm themes
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT Chart : public QWidget, public KSim::Base
  {
    Q_OBJECT
    public:
      enum DataType { DataIn = 0, DataOut };
      enum LabelType { Label = 0, Progress, Led };
      /**
       * Constructs a KSim::Chart.
       *
       * @param displayMeter is if the krellbar should be shown
       * @param minValue is the minimum value to show
       * @param maxValue is the maximum value to show
       * @param title is the title off the krell bar (if enabled)
       * @param parent is the parent widget
       */
      Chart(bool displayMeter, int maxValue,
         const QString &title, QWidget *parent,
         const char *name = 0, WFlags fl = 0);
      /**
       * Constructs a KSim::Chart.
       *
       * @param showKrell is if the krellbar should be shown
       * @param minValue is the minimum value to show
       * @param maxValue is the maximum value to show
       * @param parent is the parent widget
       */
      Chart(bool displayMeter, int maxValue,
         QWidget *parent, const char *name = 0,
         WFlags fl = 0);
      /**
       * destructs KSim::Chart
       */
      virtual ~Chart();

      /**
       * returns the current text of the chart
       */
      const QString &text(DataType type) const;
      /**
       * @return the title of the chart
       */
      const QString &title() const;
      /**
       * @return true if the meter is enabled
       */
      bool displayMeter() const;
      /**
       * @return the minimum value
       */
      int minValue() const;
      /**
       * @return the minimum value
       */
      int maxValue() const;
      /**
       * @return the current value
       */
      int value(DataType dataType) const;
      /**
       * sets the type of the label that will appear under the graph
       */
      void setLabelType(LabelType labelType);
      /**
       * @return the current label type, one of: Label, Progress, Led
       */
      LabelType labelType() const;

      /**
       * reimplemented for internal reasons
       */
      virtual void configureObject(bool repaintWidget = true);

      /**
       * reimplemented for internal reasons
       */
      QSize sizeHint() const;
      /**
       * reimplemented for internal reasons
       */
      QSize minimumSizeHint() const;

      void disableAutomaticUpdates();

    public slots:
      /**
       * Cleats the graphs contents
       */
      void clear();
      /**
       * sets the title of the chart, this function returns if
       * the optional krell widget is disabled
       */
      void setTitle(const QString &);
      /**
       * turns the meter on and off
       */
      void setDisplayMeter(bool);
      /**
       * sets the current text that apears in the top left hand corner
       */
      void setText(const QString &in, const QString &out = QString::null);
      /**
       * Sets the minimum value the graph will display
       */
      void setMinValue(int);
      /**
       * Sets the maximum value the graph will display
       */
      void setMaxValue(int);
      /**
       * sets the value of the graph
       */
      void setValue(int valueIn, int valueOut = 0);

    protected:
      /**
       * Set the config values depending on the chart type
       */
      void setConfigValues();
      /**
       * reimplemented for internal reasons
       */
      virtual void extraTypeCall();

      QColor chartColour(const DataType &dataType, int x = 1, int y = 1) const;
      virtual void paintEvent(QPaintEvent *);
      virtual void fontChange(const QFont &);
      virtual void resizeEvent(QResizeEvent *re);

    protected slots:
      void updateDisplay();

    private:
      void buildPixmaps();
      int yLocation(int) const;
      void drawChart();
      QSize chartSize() const;
      QBitmap drawMask(QPainter *);
      void drawIn(QPainter *, int, int, bool = false);
      void drawOut(QPainter *, int, int, bool = false);
      int range(int) const;
      void init(bool, int, const QString &);

      class Timer;
      class Private;
      Private *d;
  };
}
#endif
