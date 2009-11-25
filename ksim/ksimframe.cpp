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

#include "ksimframe.h"
#include "ksimframe.moc"
#include <themeloader.h>
#include "ksim.h"
#include "themetypes.h"

KSim::Frame::Frame(int type, QWidget *parent, const char *name)
   : QWidget (parent, name)
{
  setType(type);

  setBackgroundMode(NoBackground);
  configureObject();
}

KSim::Frame::~Frame()
{
}

void KSim::Frame::configureObject(bool repaintWidget)
{
  m_image.load(themeLoader().current().framePixmap(type()));

  switch (type()) {
    case KSim::Types::TopFrame:
      setFrameHeight(themeLoader().current().frameTopHeight());
      break;
    case KSim::Types::BottomFrame:
      setFrameHeight(themeLoader().current().frameBottomHeight());
      break;
    case KSim::Types::LeftFrame:
      setFrameWidth(themeLoader().current().frameLeftWidth());
      break;
    case KSim::Types::RightFrame:
      setFrameWidth(themeLoader().current().frameRightWidth());
      break;
  }

  themeLoader().reColourImage(m_image);
  m_background.convertFromImage(m_image.smoothScale(size()));

  if (repaintWidget)
    update();
}

void KSim::Frame::paintEvent(QPaintEvent *)
{
  bitBlt(this, 0, 0, &m_background);
}

void KSim::Frame::resizeEvent(QResizeEvent *ev)
{
  m_background.convertFromImage(m_image.smoothScale(ev->size()));
  QWidget::resizeEvent(ev);
}

void KSim::Frame::setFrameHeight(int height)
{
  if (!height) {
    setFixedHeight(m_image.height());
    return;
  }

  setFixedHeight(height);
}

void KSim::Frame::setFrameWidth(int width)
{
  if (!width) {
    setFixedWidth(m_image.width());
    return;
  }

  setFixedWidth(width);
}
