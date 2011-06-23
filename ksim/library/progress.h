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

#ifndef KSIM__PROGRESS_H
#define KSIM__PROGRESS_H

#include "label.h"

#include <kdemacros.h>

namespace KSim
{
  /**
   * provides a label with a progress bar meter
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT Progress : public KSim::Label
  {
    Q_OBJECT
  TQ_OBJECT
    public:
      enum ProgressType { Panel, Meter };
      /**
       * constructs a KSim::Progress
       *
       * @param maxValue is the maximum value
       * that the progress bar will show
       * @param label is the text that will be displayed
       * @param tqparent is the tqparent widget
       */
      Progress(int maxValue, TQWidget *tqparent,
         const char *name = 0, WFlags fl = 0);
      /**
       * constructs a KSim::Progress
       *
       * @param maxValue is the maximum value
       * that the progress bar will show
       * @param type is the theme type
       * @param label is the text that will be displayed
       * @param tqparent is the tqparent widget
       */
      Progress(int maxValue, int type, const TQString &label,
           TQWidget *tqparent, const char *name = 0, WFlags fl = 0);
      /**
       * constructs a KSim::Progress
       *
       * @param maxValue is the maximum value
       * that the progress bar will show
       * @param type is the theme type
       * @param label is the text that will be displayed
       * @param value is the initial value to be displayed
       * @param tqparent is the tqparent widget
       */
      Progress(int maxValue, int type, const TQString &label,
           int value, TQWidget *tqparent, const char *name = 0,
           WFlags fl = 0);
      /**
       * constructs a KSim::Progress
       *
       * @param maxValue is the maximum value
       * that the progress bar will show
       * @param type is the theme type
       * @param tqparent is the tqparent widget
       */
      Progress(int maxValue, int type, TQWidget *tqparent,
           const char *name = 0, WFlags fl = 0);
      /**
       * constructs a KSim::Progress
       *
       * @param maxValue is the maximum value
       * that the progress bar will show
       * @param type is the theme type
       * @param progressType is onr of Progress::ProgressType
       * @param tqparent is the tqparent widget
       */
      Progress(int maxValue, int type,
           ProgressType progressType,
           TQWidget *tqparent, const char *name = 0,
           WFlags fl = 0);
      /**
       * destructs KSim::Chart
       */
      virtual ~Progress();

      /**
       * @return the current value
       */
      int value() const;
      /**
       * @return the minimum value
       */
      int minValue() const;
      /**
       * @return the maximum value
       */
      int maxValue() const;

      /**
       * @return the area that the progress meter will be drawn
       */
      const TQRect &rectOrigin() const;
      /**
       * reimplemented for internal reasons
       */
      virtual void configureObject(bool tqrepaintWidget = true);
      /**
       * reimplemented for internal reasons
       */
      virtual TQSize tqsizeHint() const;

    public slots:
      /**
       * calls KSim::Label::clear() and resets the value(),
       *  maxValue() and minValue() to 0
       */
      virtual void reset();
      /**
       * sets the current value the progress bar will display
       */
      void setValue(int);
      /**
       * sets the minimum value the progress bar will display
       */
      void setMinValue(int);
      /**
       * sets the maximum value the progress bar will display
       */
      void setMaxValue(int);

    protected:
      /**
       * sets the area that the progess bar will be drawn
       */
      void setOrigin(const TQRect &);
      /**
       * sets the progress bar pixmap
       */
      void setMeterPixmap(const TQPixmap &);
      /**
       * @return the pixel position where the meter should be drawn
       */
      int xLocation() const;
      /**
       * reimplemented for internal reasons
       */
      virtual void paintEvent(TQPaintEvent *);
      /**
       * reimplemented for internal reasons
       */
      virtual void resizeEvent(TQResizeEvent *);
      /**
       * paints the meter image onto the widget
       */
      void drawMeter();

    private:
      /**
       * inits the widget
       */
      void init(int, int = 0, ProgressType = Meter);

      class Private;
      Private *d;
  };
}
#endif
