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

#ifndef KSIMFRAME_H
#define KSIMFRAME_H

#include <tqwidget.h>
#include <tqimage.h>
#include <common.h>

namespace KSim
{
  class PanelExtension;

  /**
   * @internal
   */
  class Frame : public TQWidget, public KSim::Base
  {
    Q_OBJECT
  TQ_OBJECT
    public:
      Frame(int type, TQWidget *parent, const char *name = 0);
      ~Frame();

      const TQPixmap *const background() const { return &m_background; }
      virtual void configureObject(bool repaintWidget=true);

    protected:
      virtual void paintEvent(TQPaintEvent *);
      virtual void resizeEvent(TQResizeEvent *);

    private:
      // helper functions to make
      // the source more readable
      void setFrameHeight(int height);
      void setFrameWidth(int width);

      int m_origWidth;
      int m_origHeight;
      TQPoint m_globalBottomRight;
      TQPoint m_globalTopLeft;
      TQPoint m_origPos;
      TQImage m_image;
      TQPixmap m_background;
  };
}
#endif
