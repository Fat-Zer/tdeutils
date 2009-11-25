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

#ifndef KSIM__LED_H
#define KSIM__LED_H

#include "progress.h"
#include <kdemacros.h>
#include <qpixmap.h>

namespace KSim
{
  /**
   * A custom led for use with KSim::LedLabel,
   * usually you shouldn't need to use this class, use KSim::LedLabel instead
   * @see KSim::LedLabel
   *
   * @short led using gkrellm themes
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT Led : public QPixmap
  {
    public:
      enum Type { First = 0, Second };
      Led();
      /**
       * Constructs a KSim::Led.
       *
       * @param type is the led type
       * @param ImageName is the image to display
       * the image's height is divided by 4 and each item is used for send in,
       * send out, receive in and receive out
       * @param parent is the parent widget
       * @param name is the object instance name
       */
      Led(Type type, const QString &imageName);
      /**
       * destructor for KSim::Led.
       */
      virtual ~Led();

      /**
       * sets the led on
       */
      void setOn(bool force = false);
      /**
       * sets the led off
       */
      void setOff(bool force = false);
      /**
       * toggles the current state of the led
       */
      void toggle();
      /**
       * sets the image of the led
       */
      void setPixmap(const QString &);
      /**
       * @return the location of the filename
       */
      const QString &fileName() const;
      /**
       * @return true if the led is on
       */
      bool isOn() const;
      /**
       * sets the type of led, send or receive
       */
      void setType(Type type);
      /**
       * @return the type of the led
       */
      Type type() const;
      /**
       * Force the Led to repaint its state
       */
      void update();

    private:
      class Private;
      Private *d;
  };

  /**
   * A custom widget that inheriits KSim::Progress.
   * this wiget provides two KSim::Led's and a label
   *
   * @short Label widget with 2 leds
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT LedLabel : public KSim::Progress
  {
    Q_OBJECT
    public:
      /**
       * Constructs a KSim::LedLabel.
       *
       * @param type is the theme type
       * @param label is the default text to display
       * @param parent is the parent widget
       * @see KSim::ThemeLoader
       */
      LedLabel(int max, int type, const QString &label,
            QWidget *parent, const char *name = 0, WFlags fl = 0);
      /**
       * Constructs a KSim::LedLabel.
       *
       * @param type is the theme type
       * @param label is the default text to display
       * @param parent is the parent widget
       * @see KSim::ThemeLoader
       */
      LedLabel(int max, int type, QWidget *parent,
             const char *name = 0, WFlags fl = 0);
      /**
       * Constructs a KSim::LedLabel.
       *
       * @param label is the default text to display
       * @param parent is the parent widget
       * @see KSim::ThemeLoader
       */
      LedLabel(int max, QWidget *parent,
             const char *name = 0, WFlags fl = 0);
      /**
       * destructor for KSim::LedLabel.
       */
      virtual ~LedLabel();

      /**
       * reimplemented for internal reasons
       */
      virtual void configureObject(bool reapaintWidget = true);
      /**
       * reimplemented for internal reasons
       */
      virtual QSize sizeHint() const;

    public slots:
      /**
       * reimplemented for internal reasons
       */
      virtual void reset();
      /**
       * sets @ref ledType on
       */
      void setOn(Led::Type type);
      /**
       * sets @ref ledType off
       */
      void setOff(Led::Type type);
      /**
       * toggles the state of @ref ledType
       * if the led is on then it will be set off,
       * if the led is off it will be set on.
       */
      void toggle(Led::Type type);

    protected:
      void drawLeds();
      /**
       * reimplemented for internal reasons
       */
      virtual void paintEvent(QPaintEvent *);
      /**
       * reimplemented for internal reasons
       */
      virtual void resizeEvent(QResizeEvent *);

    private:
      void layoutLeds();
      void init();

      class Private;
      Private *d;
  };
}
#endif // KSIMLED_H
