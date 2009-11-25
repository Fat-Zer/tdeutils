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

#include "led.h"
#include "led.moc"

#include <qimage.h>
#include <qpainter.h>
#include <qfile.h>
#include <qbitmap.h>

#include <kdebug.h>
#include <kpixmapsplitter.h>

#include <themeloader.h>
#include "themetypes.h"

class KSim::Led::Private
{
  public:
    KPixmapSplitter splitter;
    QPixmap pixmap;
    QString imageName;
    KSim::Led::Type type;
    bool toggled;
};

KSim::Led::Led() : QPixmap()
{
  d = new Private;

  d->type = First;
  d->toggled = false;

  setPixmap(KSim::ThemeLoader::self().current().ledPixmap(KSim::Types::Net));
  setOff();
}

KSim::Led::Led(Type type, const QString &imageName) : QPixmap()
{
  d = new Private;

  d->type = type;
  d->toggled = false;

  setPixmap(imageName);
  setOff();
}

KSim::Led::~Led()
{
  delete d;
}

void KSim::Led::setOn(bool force)
{
  if (isOn() && !force)
    return;

  // Return if our pixmap is null
  QRect rect = d->splitter.coordinates(d->type == First ? 1 : 3);
  if (d->pixmap.isNull() || rect.isEmpty()) {
    resize(12, 8);
    fill(Qt::white);
    return;
  }

  if (d->pixmap.mask() && !d->pixmap.mask()->isNull()) {
    QBitmap mask(rect.size());
    bitBlt(&mask, QPoint(0, 0), d->pixmap.mask(), rect, CopyROP);
    setMask(mask);
  }

  bitBlt(this, QPoint(0, 0), &d->pixmap, rect, CopyROP);
  d->toggled = true;
}

void KSim::Led::setOff(bool force)
{
  if (!isOn() && !force)
    return;

  // Return if our pixmap is null
  QRect rect = d->splitter.coordinates(d->type == First ? 0 : 2);
  if (d->pixmap.isNull() || rect.isEmpty()) {
    resize(12, 8);
    fill(Qt::white);
    return;
  }

  if (d->pixmap.mask() && !d->pixmap.mask()->isNull()) {
    QBitmap mask(rect.size());
    bitBlt(&mask, QPoint(0, 0), d->pixmap.mask(), rect, CopyROP);
    setMask(mask);
  }

  bitBlt(this, QPoint(0, 0), &d->pixmap, rect, CopyROP);
  d->toggled = false;
}

void KSim::Led::toggle()
{
  if (d->toggled)
    setOff();
  else
    setOn();
}

void KSim::Led::setPixmap(const QString &imageName)
{
  if (imageName == d->imageName)
    return;

  QImage image(imageName);

  if (image.width() >= 19)
    image = image.smoothScale(19, image.height());

  KSim::ThemeLoader::self().reColourImage(image);
  d->pixmap.convertFromImage(image);
  QSize size(image.width(), image.height() / 4);

  d->splitter.setPixmap(d->pixmap);
  d->splitter.setItemSize(size);

  resize(size);
  setMask(QBitmap());
}

const QString &KSim::Led::fileName() const
{
  return d->imageName;
}

bool KSim::Led::isOn() const
{
  return d->toggled;
}

void KSim::Led::setType(Type type)
{
  if (type == d->type)
    return;

  d->type = type;
}

KSim::Led::Type KSim::Led::type() const
{
  return d->type;
}

void KSim::Led::update()
{
  if (isOn())
    setOn(true);
  else
    setOff(true);
}

class KSim::LedLabel::Private
{
  public:
    KSim::Led receiveLed;
    KSim::Led sendLed;
    QPoint sendPoint;
    QPoint receivePoint;
};

KSim::LedLabel::LedLabel(int max, int type, const QString &label,
   QWidget *parent, const char *name, WFlags fl)
   : KSim::Progress(max, type, Panel, parent, name, fl)
{
  init();
  setText(label);
}

KSim::LedLabel::LedLabel(int max, int type,
   QWidget *parent, const char *name, WFlags fl)
   : KSim::Progress(max, type, Panel, parent, name, fl)
{
  init();
}

KSim::LedLabel::LedLabel(int max, 
   QWidget *parent, const char *name, WFlags fl)
   : KSim::Progress(max, KSim::Types::None,
   Panel, parent, name, fl)
{
  init();
}

KSim::LedLabel::~LedLabel()
{
  delete d;
}

void KSim::LedLabel::configureObject(bool reapaintWidget)
{
  KSim::Progress::configureObject(false);

  QPixmap pixmap = themeLoader().current().splitPixmap(KSim::Theme::KrellPanel, 0, false);
  if (pixmap.isNull())
    pixmap = themeLoader().current().splitPixmap(KSim::Theme::KrellSlider);

  setMeterPixmap(pixmap);
  d->receiveLed.setPixmap(themeLoader().current().ledPixmap(KSim::Types::Net));
  d->sendLed.setPixmap(themeLoader().current().ledPixmap(KSim::Types::Net));

  // Update the leds to make sure they get painted correctly
  d->receiveLed.update();
  d->sendLed.update();

  setConfigValues();
  layoutLeds();

  if (reapaintWidget)
    update();
}

QSize KSim::LedLabel::sizeHint() const
{
  QSize hint(Progress::sizeHint());

  if (d->sendLed.height() > hint.height())
    hint.setHeight(d->sendLed.height());

  return hint;
}

void KSim::LedLabel::reset()
{
  KSim::Progress::reset();
  setOff(Led::First);
  setOff(Led::Second);
}

void KSim::LedLabel::setOn(Led::Type type)
{
  if (type == Led::First) {
    if (d->receiveLed.isOn())
      return;

    d->receiveLed.setOn();
  }
  else {
    if (d->sendLed.isOn())
      return;

    d->sendLed.setOn();
  }

  update();
}

void KSim::LedLabel::setOff(Led::Type type)
{
  if (type == Led::First) {
    if (!d->receiveLed.isOn())
      return;

    d->receiveLed.setOff();
  }
  else {
    if (!d->sendLed.isOn())
      return;

    d->sendLed.setOff();
  }

  update();
}

void KSim::LedLabel::toggle(Led::Type type)
{
  if (type == Led::First)
    d->receiveLed.toggle();
  else
    d->sendLed.toggle();

  update();
}

void KSim::LedLabel::drawLeds()
{
  bitBlt(this, d->sendPoint, &d->sendLed);
  bitBlt(this, d->receivePoint, &d->receiveLed);
}

void KSim::LedLabel::paintEvent(QPaintEvent *ev)
{
  KSim::Label::paintEvent(ev);
  drawLeds();
  KSim::Progress::drawMeter();
}

void KSim::LedLabel::resizeEvent(QResizeEvent *ev)
{
  KSim::Progress::resizeEvent(ev);
  layoutLeds();
}

void KSim::LedLabel::layoutLeds()
{
  int ledHeight = height() / 2;

  // get the location of the sendLed
  d->sendPoint.setX((width() - d->sendLed.width()) - 2);
  d->sendPoint.setY(ledHeight - (d->sendLed.height() / 2));

  // get the location of the receiveLed
  d->receivePoint.setX((d->sendPoint.x() - d->receiveLed.width()) - 3);
  d->receivePoint.setY(ledHeight - (d->receiveLed.height() / 2));

  QRect location = textLocation();
  location.setWidth(d->receivePoint.x());
  setTextLocation(location);

  QRect shadow = shadowLocation();
  shadow.setWidth(d->receivePoint.x() + 3);
  setShadowLocation(shadow);

  setOrigin(rect());
}

void KSim::LedLabel::init()
{
  d = new Private;
  d->receiveLed.setType(KSim::Led::First);
  d->sendLed.setType(KSim::Led::Second);
  setConfigString("StylePanel");
  configureObject();
}
