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

#ifndef KSIM__LABEL_H
#define KSIM__LABEL_H

#include <qwidget.h>
#include "common.h"

#include <kdemacros.h>

namespace KSim
{
  /**
   * A widget that provides a text or image display
   * which supports themes from gkrellm
   *
   * @short Label widget
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   */
  class KDE_EXPORT Label : public QWidget, public KSim::Base
  {
    Q_OBJECT
    public:
      /**
       * Constructs a KSim::Label.
       *
       * @param parent is the parent widget
       *
       * Example usage:
       * <pre>
       *  KSim::Label *label = new KSim::Label(this);
       * </pre>
       * To create a KSim::Label with the normal theme look
       * @see KSim::ThemeLoader
       */
      Label(QWidget *parent, const char *name = 0, WFlags fl = 0);
      /**
       * Constructs a KSimLabel.
       *
       * @param type is the theme type
       * @param parent is the parent widget
       *
       * Example usage:
       * <pre>
       *  KSim::Label *label = new KSim::Label(KSim::Types::None, this);
       * </pre>
       * To create a KSim::Label with the normal theme look,
       * NOTE: this is the same as the KSim::Label(QWidget *, const char *, WFlags) ctor
       * @see KSim::ThemeLoader
       */
      Label(int type, QWidget *parent, const char *name = 0, WFlags fl = 0);
      /**
       * Constructs a KSim::Label.
       *
       * @param type is the theme type
       * @param text is the default text to display
       * @param parent is the parent widget
       *
       * Example usage:
       * <pre>
       *  KSim::Label *label = new KSim:Label(KSim::Types::Host, "test label", this);
       * </pre>
       * To create a KSim::Label with the theme look of host
       * @see KSim::ThemeLoader
       */
      Label(int type, const QString &text,
           QWidget *parent, const char *name = 0, WFlags fl = 0);
      /**
       * destructs KSim::Label.
       */
      virtual ~Label();

      /**
       * returns the current text of the label
       * @see #setText
       */
      const QString &text() const;
      /**
       * recreates the labels look & feel
       */
      virtual void configureObject(bool repaintWidget = true);
      /**
       * sets a pixmap for the label
       */
      void setPixmap(const QPixmap &pixmap);
      /**
       * @return the side pixmap
       */
      const QPixmap &pixmap() const;
      /**
       * reimplemented for internal reasons
       */
      virtual QSize sizeHint() const;
      /**
       * reimplemented for internal reasons
       */
      virtual QSize minimumSizeHint() const;

    public slots:
      /**
       * clears the current text in the label
       */
      void clear();
      /**
       * sets the current label of the widget to @ref text
       * @see text()
       */
      void setText(const QString &text);

    protected:
      /**
       * reimplemented for internal reasons
       */
      virtual void extraTypeCall();
      /**
       * set wether the shadow should be shown or not
       */
      void setShowShadow(bool show);
      /**
       * @return true if the shadow is to be shown, else false
       */
      bool showShadow() const;
      /**
       * Set the text color
       */
      void setTextColour(const QColor &color);
      /**
       * @return the text color
       */
      const QColor &textColour() const;
      /**
       * Set the shadow color
       */
      void setShadowColour(const QColor &color);
      /**
       * @return the shadow color
       */
      const QColor &shadowColour() const;
      /**
       * Set the config values depending on the chart type
       */
      void setConfigValues();
      /**
       * reimplemented for internal reasons
       */
      virtual void paintEvent(QPaintEvent *);
      /**
       * reimplemented for internal reasons
       */
      virtual void resizeEvent(QResizeEvent *);
      /**
       * draw the text onto the label
       */
      void drawText(QPainter *painter, const QRect &rect,
          const QColor &color, const QString &text);
      /**
       * draw the pixmap onto the label
       */
      void drawPixmap(QPainter *painter,
          const QRect &rect, const QPixmap &pixmap);
      /**
       * sets the location of the text
       */
      void setTextLocation(const QRect &rect);
      /**
       * @return the location of the text
       */
      const QRect &textLocation() const;
      /**
       * sets the location of the shadow text
       */
      void setShadowLocation(const QRect &rect);
      /**
       * @return the location of the shadow text
       */
      const QRect &shadowLocation() const;
      /**
       * sets the background image to be painted
       */
      void setThemePixmap(const QString &image);
      void relayoutLabel(const QSize &old, bool repaint = true);

    private:
      /**
       * initiates the widget
       */
      void initWidget(int type);

      class Private;
      Private *d;
  };
}
#endif
