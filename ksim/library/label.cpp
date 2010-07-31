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

#include "label.h"
#include "label.moc"
#include <ksimconfig.h>
#include "themetypes.h"

#include <tqpainter.h>
#include <tqstyle.h>
#include <tqstylesheet.h>
#include <tqsimplerichtext.h>
#include <tqcursor.h>
#include <tqpixmap.h>
#include <tqimage.h>

#include <themeloader.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kconfig.h>

class KSim::Label::Private
{
  public:
    TQColor mColour;
    TQColor sColour;
    TQRect loc;
    TQRect shad;
    TQString text;
    TQImage meterImage;
    TQPixmap background;
    TQPixmap sidePixmap;
    bool showShadow;
};

KSim::Label::Label(TQWidget *parent, const char *name, WFlags fl)
   : TQWidget(parent, name, fl)
{
  initWidget(KSim::Types::None);
}

KSim::Label::Label(int type, TQWidget *parent, const char *name, 
   WFlags fl) : TQWidget(parent, name, fl)
{
  initWidget(type);
}

KSim::Label::Label(int type, const TQString &text, TQWidget *parent,
   const char *name, WFlags fl) : TQWidget(parent, name, fl)
{
  initWidget(type);
  setText(text);
}

KSim::Label::~Label()
{
  delete d;
}

const TQString &KSim::Label::text() const
{
  return d->text;
}

void KSim::Label::configureObject(bool repaintWidget)
{
  TQString image = themeLoader().current().meterPixmap(type(), false);
  if (image.isEmpty())
    image = themeLoader().current().panelPixmap(type());

  d->meterImage.load(image);
  KSim::ThemeLoader::self().reColourImage(d->meterImage);
  d->background = d->meterImage.smoothScale(size());
  TQSize oldSize = sizeHint();

  setConfigValues();
  relayoutLabel(oldSize, repaintWidget);
}

void KSim::Label::setPixmap(const TQPixmap &pixmap)
{
  if (pixmap.serialNumber() == d->sidePixmap.serialNumber())
    return;

  TQSize oldSize = sizeHint();
  d->sidePixmap = pixmap;

  relayoutLabel(oldSize);
}

const TQPixmap &KSim::Label::pixmap() const
{
  return d->sidePixmap;
}

TQSize KSim::Label::sizeHint() const
{
  int width = fontMetrics().size(SingleLine, text()).width();
  if (!pixmap().isNull())
    width += pixmap().width() + 5;

  int height = fontMetrics().height() + 4;
  if (!pixmap().isNull() && pixmap().height() > height)
    height = pixmap().height();

  return TQSize(width, height);
}

TQSize KSim::Label::minimumSizeHint() const
{
  return sizeHint();
}

void KSim::Label::clear()
{
  setText(TQString::null);
}

void KSim::Label::setText(const TQString &text)
{
  if (text == d->text)
    return; // If the text is the same, no need to repaint etc

  TQSize oldSize = sizeHint();
  // set the text of our widget and repaint
  d->text = text;
  relayoutLabel(oldSize);
}

void KSim::Label::extraTypeCall()
{
  d->meterImage.load(themeLoader().current().meterPixmap(type()));
  setConfigValues();
}

void KSim::Label::setShowShadow(bool show)
{
  d->showShadow = show;
}

bool KSim::Label::showShadow() const
{
  return d->showShadow;
}

void KSim::Label::setTextColour(const TQColor &color)
{
  d->mColour = color;
}

const TQColor &KSim::Label::textColour() const
{
  return d->mColour;
}

void KSim::Label::setShadowColour(const TQColor &color)
{
  d->sColour = color;
}

const TQColor &KSim::Label::shadowColour() const
{
  return d->sColour;
}

void KSim::Label::setConfigValues()
{
  TQFont newFont = font();
  bool repaint = themeLoader().current().fontColours(this,
     newFont, d->mColour, d->sColour, d->showShadow);

  if (font() != newFont)
    setFont(newFont);

  if (repaint)
    update();
}

void KSim::Label::paintEvent(TQPaintEvent *)
{
  TQPainter painter;
  painter.begin(this);

  // paint our background pixmap onto the widget
  painter.drawPixmap(0, 0, d->background);

  drawPixmap(&painter, d->loc, pixmap());
  if (d->showShadow) { // draw the shadow text on the image
    drawText(&painter, d->shad, d->sColour, d->text);
  }

  // draw the label text onto the widget
  painter.setPen(d->mColour);
  drawText(&painter, d->loc, d->mColour, d->text);
  painter.end();
}

void KSim::Label::resizeEvent(TQResizeEvent *ev)
{
  // set the location of where the shadow'ed text will be drawn
  d->shad.setWidth(ev->size().width() + 3);
  d->shad.setHeight(ev->size().height() + 3);

  // set the location of where the text will be drawn
  d->loc.setWidth(ev->size().width());
  d->loc.setHeight(ev->size().height());

  d->background = d->meterImage.smoothScale(ev->size());
}

void KSim::Label::drawText(TQPainter *painter, const TQRect &rect,
   const TQColor &color, const TQString &text)
{
  TQRect location(rect);
  if (!pixmap().isNull())
    location.setX(pixmap().width() + 5);
   
  style().drawItem(painter, location, AlignCenter, colorGroup(), true,
      0, text, -1, &color);
}

void KSim::Label::drawPixmap(TQPainter *painter, const TQRect &rect,
   const TQPixmap &pixmap)
{
  TQRect location(rect);
  location.setWidth(pixmap.width());

  style().drawItem(painter, location, AlignCenter, colorGroup(), true,
    pixmap.isNull() ? 0 : &pixmap, TQString::null);
}

void KSim::Label::setTextLocation(const TQRect &rect)
{
  d->loc = rect;
}

const TQRect &KSim::Label::textLocation() const
{
  return d->loc;
}

void KSim::Label::setShadowLocation(const TQRect &rect)
{
  d->shad = rect;
}

const TQRect &KSim::Label::shadowLocation() const
{
  return d->shad;
}

void KSim::Label::setThemePixmap(const TQString &image)
{
  TQSize oldSize = sizeHint();
  d->meterImage.reset();
  d->meterImage.load(image);
  KSim::ThemeLoader::self().reColourImage(d->meterImage);
  d->background = d->meterImage.smoothScale(size());
  relayoutLabel(oldSize);
}

void KSim::Label::relayoutLabel(const TQSize &old, bool repaint)
{
  if (sizeHint() != old) {
    updateGeometry();
  }

  if (repaint)
    update();
}

void KSim::Label::initWidget(int type)
{
  d = new Private;
  setType(type);
  setConfigString("StyleMeter");

  // try to reduce flicker as much as possible
  setBackgroundMode(NoBackground);
  setSizePolicy(TQSizePolicy(TQSizePolicy::MinimumExpanding,
     TQSizePolicy::Fixed));

  configureObject();
}
