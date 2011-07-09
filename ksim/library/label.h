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

#include <tqwidget.h>
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
  class KDE_EXPORT Label : public TQWidget, public KSim::Base
  {
    Q_OBJECT
  TQ_OBJECT
    public:
      /**
       * Constructs a KSim::Label.
       *
       * @param tqparent is the tqparent widget
       *
       * Example usage:
       * <pre>
       *  KSim::Label *label = new KSim::Label(this);
       * </pre>
       * To create a KSim::Label with the normal theme look
       * @see KSim::ThemeLoader
       */
      Label(TQWidget *tqparent, const char *name = 0, WFlags fl = 0);
      /**
       * Constructs a KSimLabel.
       *
       * @param type is the theme type
       * @param tqparent is the tqparent widget
       *
       * Example usage:
       * <pre>
       *  KSim::Label *label = new KSim::Label(KSim::Types::None, this);
       * </pre>
       * To create a KSim::Label with the normal theme look,
       * NOTE: this is the same as the KSim::Label(TQWidget *, const char *, WFlags) ctor
       * @see KSim::ThemeLoader
       */
      Label(int type, TQWidget *tqparent, const char *name = 0, WFlags fl = 0);
      /**
       * Constructs a KSim::Label.
       *
       * @param type is the theme type
       * @param text is the default text to display
       * @param tqparent is the tqparent widget
       *
       * Example usage:
       * <pre>
       *  KSim::Label *label = new KSim:Label(KSim::Types::Host, "test label", this);
       * </pre>
       * To create a KSim::Label with the theme look of host
       * @see KSim::ThemeLoader
       */
      Label(int type, const TQString &text,
           TQWidget *tqparent, const char *name = 0, WFlags fl = 0);
      /**
       * destructs KSim::Label.
       */
      virtual ~Label();

      /**
       * returns the current text of the label
       * @see #setText
       */
      const TQString &text() const;
      /**
       * recreates the labels look & feel
       */
      virtual void configureObject(bool repaintWidget = true);
      /**
       * sets a pixmap for the label
       */
      void setPixmap(const TQPixmap &pixmap);
      /**
       * @return the side pixmap
       */
      const TQPixmap &pixmap() const;
      /**
       * reimplemented for internal reasons
       */
      virtual TQSize tqsizeHint() const;
      /**
       * reimplemented for internal reasons
       */
      virtual TQSize tqminimumSizeHint() const;

    public slots:
      /**
       * clears the current text in the label
       */
      void clear();
      /**
       * sets the current label of the widget to @ref text
       * @see text()
       */
      void setText(const TQString &text);

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
      void setTextColour(const TQColor &color);
      /**
       * @return the text color
       */
      const TQColor &textColour() const;
      /**
       * Set the shadow color
       */
      void setShadowColour(const TQColor &color);
      /**
       * @return the shadow color
       */
      const TQColor &shadowColour() const;
      /**
       * Set the config values depending on the chart type
       */
      void setConfigValues();
      /**
       * reimplemented for internal reasons
       */
      virtual void paintEvent(TQPaintEvent *);
      /**
       * reimplemented for internal reasons
       */
      virtual void resizeEvent(TQResizeEvent *);
      /**
       * draw the text onto the label
       */
      void drawText(TQPainter *painter, const TQRect &rect,
          const TQColor &color, const TQString &text);
      /**
       * draw the pixmap onto the label
       */
      void drawPixmap(TQPainter *painter,
          const TQRect &rect, const TQPixmap &pixmap);
      /**
       * sets the location of the text
       */
      void setTextLocation(const TQRect &rect);
      /**
       * @return the location of the text
       */
      const TQRect &textLocation() const;
      /**
       * sets the location of the shadow text
       */
      void setShadowLocation(const TQRect &rect);
      /**
       * @return the location of the shadow text
       */
      const TQRect &shadowLocation() const;
      /**
       * sets the background image to be painted
       */
      void setThemePixmap(const TQString &image);
      void relayoutLabel(const TQSize &old, bool tqrepaint = true);

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
