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

#include "progress.h"
#include "progress.moc"

#include <kdebug.h>
#include <tqpainter.h>
#include <tqpixmap.h>
#include <tqimage.h>

#include <themeloader.h>

class KSim::Progress::Private
{
  public:
    TQPixmap meterPixmap;
    TQRect rectOrigin;
    ProgressType type;
    int krellDepth;
    int value;
    int minValue;
    int maxValue;
};

KSim::Progress::Progress(int maxValue,
   TQWidget *parent, const char *name,
   WFlags fl) : KSim::Label(parent, name, fl)
{
  init(maxValue);
  configureObject();
}

KSim::Progress::Progress(int maxValue,
   int type, const TQString &label, TQWidget *parent,
   const char *name, WFlags fl)
   : KSim::Label(type, label, parent, name, fl)
{
  init(maxValue);
  configureObject();
}

KSim::Progress::Progress(int maxValue,
   int type, const TQString &label, int value,
   TQWidget *parent, const char *name, WFlags fl)
   : KSim::Label(type, label, parent, name, fl)
{
  init(maxValue, value);
  configureObject();
}

KSim::Progress::Progress(int maxValue,
   int type, TQWidget *parent,
   const char *name, WFlags fl)
   : KSim::Label(type, parent, name, fl)
{
  init(maxValue);
  configureObject();
}

KSim::Progress::Progress(int maxValue, int type,
   ProgressType progressType,
   TQWidget *parent, const char *name, WFlags fl)
   : KSim::Label(type, parent, name, fl)
{
  init(maxValue, 0, progressType);
  configureObject();
}

KSim::Progress::~Progress()
{
  delete d;
}

int KSim::Progress::value() const
{
  return d->value;
}

int KSim::Progress::minValue() const
{
  return d->minValue;
}

int KSim::Progress::maxValue() const
{
  return d->maxValue;
}

const TQRect &KSim::Progress::rectOrigin() const
{
  return d->rectOrigin;
}

void KSim::Progress::configureObject(bool repaintWidget)
{
  KSim::Label::configureObject(false);

  if (d->type == Panel) {
    setMeterPixmap(themeLoader().current().splitPixmap(KSim::Theme::KrellPanel));
    setThemePixmap(themeLoader().current().panelPixmap(type()));
  }
  else {
    setMeterPixmap(themeLoader().current().splitPixmap(KSim::Theme::KrellMeter));
  }

  if (repaintWidget)
    update();
}

TQSize KSim::Progress::sizeHint() const
{
  TQSize hint(Label::sizeHint());

  if (d->meterPixmap.height() > hint.height())
    hint.setHeight(d->meterPixmap.height());

  return hint;
}

void KSim::Progress::reset()
{
  KSim::Label::clear();
  setMinValue(0);
  setMaxValue(0);
  setValue(0);
}

void KSim::Progress::setValue(int value)
{
  if (value == d->value)
    return;

  d->value = value;
  if (value < minValue())
    d->value = minValue();

  if (value > maxValue())
    d->value = maxValue();

  update();
}

void KSim::Progress::setMinValue(int minValue)
{
  if (d->minValue == minValue)
    return;

  if (!(minValue > maxValue())) {
    d->minValue = minValue;
    update();
  }
}

void KSim::Progress::setMaxValue(int maxValue)
{
  if (d->maxValue == maxValue)
    return;

  if (!(maxValue < minValue())) {
    d->maxValue = maxValue;
    update();
  }
}

void KSim::Progress::setOrigin(const TQRect &origin)
{
  d->rectOrigin = origin;
}

void KSim::Progress::setMeterPixmap(const TQPixmap &pixmap)
{
  TQSize oldSize = sizeHint();
  d->meterPixmap = pixmap;
  relayoutLabel(oldSize);
}

int KSim::Progress::xLocation() const
{
  int position = d->rectOrigin.width() - d->meterPixmap.width();
  int rangePos = maxValue() - minValue();
  int valuePos = value() - minValue();

  int returnValue = 0;
  if (rangePos)
    returnValue = position * valuePos / rangePos;

  return returnValue;
}

void KSim::Progress::paintEvent(TQPaintEvent *ev)
{
  KSim::Label::paintEvent(ev);
  drawMeter();
}

void KSim::Progress::resizeEvent(TQResizeEvent *ev)
{
  KSim::Label::resizeEvent(ev);
  setOrigin(rect());
}

void KSim::Progress::drawMeter()
{
  bitBlt(this, xLocation(), 2, &d->meterPixmap);
}

void KSim::Progress::init(int maxValue, int value, ProgressType type)
{
  d = new Private;
  d->type = type;
  d->value = d->minValue = 0;
  d->maxValue = maxValue;
  setValue(value);
}
