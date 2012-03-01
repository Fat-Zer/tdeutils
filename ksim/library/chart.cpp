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

#include "chart.h"
#include "chart.moc"

#include <tqrect.h>
#include <tqpainter.h>
#include <tqimage.h>
#include <tqcolor.h>
#include <tqtimer.h>
#include <tqvaluelist.h>
#include <tqbitmap.h>

#include <kdebug.h>
#include <klocale.h>
#include <kglobalsettings.h>
#include <kconfig.h>

#include <themeloader.h>
#include <led.h>
#include <ksimconfig.h>
#include "themetypes.h"

class KSim::Chart::Timer
{
  public:
    static void syncChart(KSim::Chart *chart)
    {
      if (!m_timer) {
        m_timer = new TQTimer;
        m_timer->start(1000);
        tqAddPostRoutine(cleanup);
      }

      TQObject::connect(m_timer, TQT_SIGNAL(timeout()),
         chart, TQT_SLOT(updateDisplay()));
    }

    static void disconnect(KSim::Chart *chart)
    {
      if (!m_timer)
        return;

      TQObject::disconnect(m_timer, TQT_SIGNAL(timeout()),
         chart, TQT_SLOT(updateDisplay()));
    }

  private:
    static void cleanup()
    {
      if (!m_timer)
        return;

      delete m_timer;
      m_timer = 0;
    }
    
    static TQTimer *m_timer;
};

TQTimer *KSim::Chart::Timer::m_timer = 0;

class KSim::Chart::Private
{
  public:
    typedef TQPair<int, int> ValuePair;

    TQSize size;
    int type;
    bool showShadow;
    bool showKrell;
    TQString inText;
    TQString outText;
    TQString title;
    TQPixmap chartPixmap;
    TQPixmap gridPixmap;
    TQPixmap graphData;
    TQColor mColour;
    TQColor sColour;
    TQColor dataInColour;
    TQColor dataOutColour;
    Chart::LabelType labelType;
    KSim::Label *krell;
    // first will be data In and second will be data out
    TQValueList<ValuePair> values;
    TQValueList<int> maxValues;
    int minValue;
    int maxValue;
    bool variableGraphs;
};

KSim::Chart::Chart(bool showKrell, int maxValue,
   const TQString &title, TQWidget *parent, const char *name,
   WFlags fl) : TQWidget(parent, name, fl)
{
  init(showKrell, maxValue, title);
}

KSim::Chart::Chart(bool showKrell, int maxValue,
   TQWidget *parent, const char *name, WFlags fl)
   : TQWidget(parent, name, fl)
{
  init(showKrell, maxValue, i18n("None"));
}

KSim::Chart::~Chart()
{
  delete d->krell;
  delete d;
}

const TQString &KSim::Chart::text(DataType type) const
{
  if (type == DataIn)
    return d->inText;

  return d->outText;
}

const TQString &KSim::Chart::title() const
{
  return d->title;
}

bool KSim::Chart::displayMeter() const
{
  return d->showKrell;
}

int KSim::Chart::minValue() const
{
  return d->minValue;
}

int KSim::Chart::maxValue() const
{
  return d->maxValue;
}

int KSim::Chart::value(DataType dataType) const
{
  switch (dataType) {
    case DataIn:
      return d->values.first().first;
      break;
    case DataOut:
      return d->values.first().second;
      break;
  }

  return 0;
}

void KSim::Chart::setLabelType(LabelType labelType)
{
  d->labelType = labelType;

  // return if d->krell is not NULL and if the label type is the
  // same or if showKrell is set to false
  if ((d->krell && d->labelType == labelType) || !d->showKrell)
    return;

  delete d->krell;

  switch (labelType) {
    case Label:
      d->krell = new KSim::Label(this);
      break;
    case Progress:
      d->krell = new KSim::Progress(maxValue(), KSim::Types::None,
         KSim::Progress::Panel, this);
      break;
    case Led:
      d->krell = new KSim::LedLabel(maxValue(), KSim::Types::None, this);
      break;
  }

  int height = d->krell->height() - 2;
  d->krell->setText(title());
  d->krell->setFixedSize(width(), height);
  d->krell->move(0, d->size.height() - height);
  d->krell->show();
}

KSim::Chart::LabelType KSim::Chart::labelType() const
{
  return d->labelType;
}

void KSim::Chart::buildPixmaps()
{
  TQImage image(themeLoader().current().chartPixmap());
  KSim::ThemeLoader::self().reColourImage(image);
  d->chartPixmap.convertFromImage(image.smoothScale(chartSize()));

  // We need to reset the image data just
  // incase the krellDataIn image doesn't exist
  image.reset();
  image.load(themeLoader().current().gridPixmap());
  KSim::ThemeLoader::self().reColourImage(image);
  d->gridPixmap.convertFromImage(image.smoothScale(chartSize().width(), 1));

  // load these values here so we dont need to keep accessing them while
  // painting to keep the cpu usage down
  d->dataInColour = themeLoader().current().chartInColour();
  d->dataOutColour = themeLoader().current().chartOutColour();

  image.reset();
  image.load(themeLoader().current().dataInPixmap());
  KSim::ThemeLoader::self().reColourImage(image);
  if (!image.isNull()) {
    d->dataInColour = image.smoothScale(chartSize()).pixel(2, 2);
    kdDebug(2003) << className() << ": Using krellDataIn() = "
       << themeLoader().current().dataInPixmap() << endl;
  }
  else {
    kdDebug(2003) << className() << ": Using chartInColor() = "
       << TQString(d->dataInColour.name()) << endl;
  }

  image.reset();
  image.load(themeLoader().current().dataOutPixmap());
  KSim::ThemeLoader::self().reColourImage(image);
  if (!image.isNull()) {
    d->dataOutColour = image.smoothScale(chartSize()).pixel(2, 2);
    kdDebug(2003) << className() << ": Using krellDataOut() = "
       << themeLoader().current().dataOutPixmap() << endl;
  }
  else {
    kdDebug(2003) << className() << ": Using chartOutColor() = "
       << TQString(d->dataOutColour.name()) << endl;
  }
}

void KSim::Chart::configureObject(bool repaintWidget)
{
  TQSize oldSize = sizeHint();
  KSim::Config::config()->setGroup("Misc");
  d->size = KSim::Config::config()->readSizeEntry("GraphSize");

  if (d->krell && d->showKrell) {
    int krellHeight = d->krell->fontMetrics().height() - 2;
    d->size.setHeight(d->size.height() + krellHeight);
    d->krell->setFixedHeight(krellHeight);
    d->krell->move(0, d->size.height() - krellHeight);
    d->krell->show();
  }

  // Update our geometry if we need to let any
  // layout know about our sizeHint() change
  if (oldSize != sizeHint()) {
    // Using resize() here seems to be needed
    resize(sizeHint());
    updateGeometry();
  }

  buildPixmaps();

  setConfigValues();

  if (repaintWidget)
    update();
}

TQSize KSim::Chart::sizeHint() const
{
  return d->size;
}

TQSize KSim::Chart::minimumSizeHint() const
{
  return sizeHint();
}

void KSim::Chart::resizeEvent(TQResizeEvent *re)
{
  if (d->chartPixmap.size() != chartSize())
    buildPixmaps();
  TQWidget::resizeEvent(re);
}

void KSim::Chart::disableAutomaticUpdates()
{
    KSim::Chart::Timer::disconnect(this);
}

void KSim::Chart::clear()
{
  d->values.clear();
  d->maxValues.clear();
  updateDisplay();
}

void KSim::Chart::setTitle(const TQString &name)
{
  if (d->krell) {
    d->title = name;
    d->krell->setText(name);
  }
}

void KSim::Chart::setDisplayMeter(bool value)
{
  if (d->showKrell == value)
    return;

  d->showKrell = value;
  setLabelType(d->labelType);

  // delete the meter if value is false
  if (!value) {
    delete d->krell;
    d->krell = 0;
  }
}

void KSim::Chart::setText(const TQString &in, const TQString &out)
{
  bool repaint = false;

  if (d->inText != in) {
    repaint = true;
    d->inText = in;
  }

  if (d->outText != out) {
    repaint = true;
    d->outText = out;
  }

  if (repaint)
    update();
}

void KSim::Chart::setMinValue(int minValue)
{
  if (d->minValue == minValue)
    return;

  d->minValue = minValue;

  if (d->krell && labelType() != Label)
    static_cast<KSim::Progress *>(d->krell)->setMinValue(minValue);
}

void KSim::Chart::setMaxValue(int maxValue)
{
  if (d->maxValue == maxValue)
    return;

  d->maxValue = maxValue;

  if (d->krell && labelType() != Label)
    static_cast<KSim::Progress *>(d->krell)->setMaxValue(maxValue);
}

void KSim::Chart::setValue(int valueIn, int valueOut)
{
  d->values.prepend(tqMakePair(range(valueIn), range(valueOut)));

  if (d->variableGraphs) {
    d->maxValues.prepend(valueIn > valueOut ? valueIn : valueOut);
  }
  else {
    if (valueIn > maxValue())
      setMaxValue(valueIn);

    if (valueOut > maxValue())
      setMaxValue(valueOut);
  }

  if (d->krell && labelType() != Label)
    static_cast<KSim::Progress *>(d->krell)->setValue(valueIn ? valueIn
       : valueOut);

  // Remove the first entry from our lists to
  // make sure we only have a list the size of
  // our widgets width
  if (d->values.count() == (uint)(width() + 1)) {
    d->values.remove(--(d->values.end()));
    d->maxValues.remove(--(d->maxValues.end()));
  }

  if (d->krell && labelType() == Led) {
    static_cast<KSim::LedLabel *>(d->krell)->toggle(KSim::Led::First);
    static_cast<KSim::LedLabel *>(d->krell)->toggle(KSim::Led::Second);
  }
}

void KSim::Chart::setConfigValues()
{
  TQFont newFont = font();
  bool repaint = themeLoader().current().fontColours(this,
     newFont, d->mColour, d->sColour, d->showShadow);

  if (font() != newFont)
    setFont(newFont);

  if (repaint)
    update();
}

void KSim::Chart::extraTypeCall()
{
  setConfigValues();
}

TQColor KSim::Chart::chartColour(const DataType &dataType, int, int) const
{
  switch (dataType) {
    case DataIn:
      return d->dataInColour;
      break;
    case DataOut:
      return d->dataOutColour;
      break;
  }

  return TQColor(); // avoid a g++ warning
}

void KSim::Chart::paintEvent(TQPaintEvent *)
{
  if (d->krell && labelType() == Led) {
    static_cast<KSim::LedLabel *>(d->krell)->setOff(KSim::Led::First);
    static_cast<KSim::LedLabel *>(d->krell)->setOff(KSim::Led::Second);
  }

  const TQSize &size = chartSize();
  TQPixmap pixmap(size);
  TQPainter painter;
  painter.begin(&pixmap, this);

  int location = size.height() / 5;
  painter.drawPixmap(0, 0, d->chartPixmap);
  // draw the plotted data onto the graph
  painter.drawPixmap(0, 0, d->graphData);
  painter.drawPixmap(0, location, d->gridPixmap);
  painter.drawPixmap(0, location * 2, d->gridPixmap);
  painter.drawPixmap(0, location * 3, d->gridPixmap);
  painter.drawPixmap(0, location * 4, d->gridPixmap);
  painter.drawPixmap(0, location * 5, d->gridPixmap);

  if (d->showShadow) {
    painter.setPen(d->sColour);
    location = (fontMetrics().height() / 2) + 5;
    painter.drawText(3, location, d->inText);
    if (!d->outText.isNull())
      painter.drawText(3, location * 2, d->outText);
  }

  painter.setPen(d->mColour);
  location = (fontMetrics().height() / 2) + 3;
  painter.drawText(1, location, d->inText);
  if (!d->outText.isNull())
    painter.drawText(1, location * 2, d->outText);

  painter.end();
  bitBlt(this, 0, 0, &pixmap);
}

void KSim::Chart::fontChange(const TQFont &)
{
  if (d->krell)
    d->krell->setFont(font());
}

void KSim::Chart::updateDisplay()
{
  drawChart();
  update();
}

int KSim::Chart::yLocation(int value) const
{
  int krellHeight = (d->krell ? d->krell->height() : 0);

  int rangePos = maxValue() - minValue();
  int valuePos = value - minValue();

  int returnValue = 0;
  if (rangePos)
    returnValue = (height() - krellHeight) * valuePos / rangePos;

  // Make sure we dont return a negative value
  return returnValue >= 0 ? returnValue : 0;
}

void KSim::Chart::drawChart()
{
  if (chartSize() != d->graphData.size())
    d->graphData.resize(chartSize());

  if (d->variableGraphs) {
    int maxValue = 0;
    TQValueList<int>::ConstIterator max;
    for (max = d->maxValues.begin(); max != d->maxValues.end(); ++max) {
      if ((*max) > maxValue)
        maxValue = (*max);
    }

    setMaxValue(maxValue);
  }

  TQPainter painter;
  d->graphData.setMask(drawMask(&painter));
  painter.begin(&d->graphData, this);

  int position = width() - 1;
  TQValueList<Private::ValuePair>::ConstIterator it;
  for (it = d->values.begin(); it != d->values.end(); ++it) {
    // Draw the data In lines first if its higher than the data out lines
    if ((*it).first >= (*it).second) {
      drawIn(&painter, (*it).first, position);
      drawOut(&painter, (*it).second, position);
    }
    else {
      drawOut(&painter, (*it).second, position);
      drawIn(&painter, (*it).first, position);
    }
    --position;
  }

  painter.end();
}

TQSize KSim::Chart::chartSize() const
{
  TQSize sz(size());
  if (d->krell && d->showKrell)
    sz.setHeight(sz.height() - d->krell->height());

  return sz;
}

TQBitmap KSim::Chart::drawMask(TQPainter *painter)
{
  TQBitmap bitmap(chartSize(), true);
  painter->begin(&bitmap, this);
  painter->setPen(color1);

  int position = width() - 1;
  TQValueList<Private::ValuePair>::ConstIterator it;
  for (it = d->values.begin(); it != d->values.end(); ++it) {
    drawIn(painter, (*it).first, position, true);
    drawOut(painter, (*it).second, position, true);
    --position;
  }

  painter->end();

  return bitmap;
}

void KSim::Chart::drawIn(TQPainter *painter, int value, int pos, bool dontSet)
{
  if (!dontSet) {
    painter->setPen(chartColour(DataIn));
  }

  int location = yLocation(value);
  painter->moveTo(0, 0);
  painter->drawLine(pos, d->graphData.size().height(),
     pos, d->graphData.size().height() - location);
}

void KSim::Chart::drawOut(TQPainter *painter, int value, int pos, bool dontSet)
{
  if (!dontSet) {
    painter->setPen(chartColour(DataOut));
  }

  int location = yLocation(value);
  painter->moveTo(0, 0);
  painter->drawLine(pos, d->graphData.size().height(),
     pos, d->graphData.size().height() - location);
}

int KSim::Chart::range(int value) const
{
  if (value > maxValue())
    return maxValue();

  if (value < minValue())
    return minValue();

  return value;
}

void KSim::Chart::init(bool krell, int maxValue, const TQString &title)
{
  setConfigString("StyleChart");
  setThemeConfigOnly(false);
  setSizePolicy(TQSizePolicy(TQSizePolicy::MinimumExpanding, TQSizePolicy::Fixed));

  d = new Private;
  KSim::Config::config()->setGroup("Misc");
  d->variableGraphs = KSim::Config::config()->readBoolEntry("UseVariableGraphs", true);
  d->showKrell = krell;
  d->krell = 0L;
  d->title = title;
  d->minValue = 0;
  d->maxValue = 0;

  KSim::Chart::Timer::syncChart(this);
  setBackgroundMode(NoBackground);
  setLabelType(Label);
  setMaxValue(maxValue);
  configureObject(false);
}
