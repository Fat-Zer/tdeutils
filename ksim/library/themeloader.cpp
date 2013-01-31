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

#include "themeloader.h"
#include <ksimconfig.h>
#include "themetypes.h"
#include "common.h"

#include <tqfile.h>
#include <tqstringlist.h>
#include <tqvaluevector.h>
#include <tqregexp.h>
#include <tqapplication.h>
#include <tqfileinfo.h>
#include <tqdir.h>
#include <tqimage.h>
#include <tqbitmap.h>

#include <kdebug.h>
#include <ksimpleconfig.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kglobal.h>

class KSim::Theme::Private
{
  public:
    Private(const TQValueVector<TQString> &names,
       const TQStringList &list) : fileNames(names),
       imageTypes(list) {}

    TQStringList file;
    TQStringList dFile;
    TDEConfig *globalReader;
    TQString altTheme;
    TQString location;
    const TQValueVector<TQString> &fileNames;
    const TQStringList &imageTypes;
    int alternative;
    int font;
    bool recolour;

    TQString readOption(const TQString &entry,
       bool useGlobal = true,
       const TQString &defValue = TQString())
    {
      TQString text;
      TQStringList::ConstIterator it;
      for (it = file.begin(); it != file.end(); ++it) {
        if ((*it).find(entry) != -1) {
          text = TQStringList::split("=", (*it))[1].stripWhiteSpace();
        }
      }

      if (!text.isEmpty() || dFile.isEmpty())
        return text;

      TQStringList::ConstIterator it2;
      for (it2 = dFile.begin(); it2 != dFile.end(); ++it) {
        if ((*it2).find(entry) != -1) {
          text = TQStringList::split("=", (*it2))[1].stripWhiteSpace();
        }
      }

      if (!text.isEmpty())
        return text;

      if (!globalReader || !useGlobal)
        return defValue;

      text = globalReader->readEntry(entry, defValue);
      return text;
    }
};

class KSim::ThemeLoader::Private
{
  public:
    TQValueVector<TQString> fileNames;
    TQStringList imageTypes;
    TDEConfig *globalReader;
    bool recolour;
    TQColor pixelColour;
};

bool KSim::Theme::operator==(const KSim::Theme &rhs) const
{
  return d == rhs.d;
}

bool KSim::Theme::operator!=(const KSim::Theme &rhs) const
{
  return !(operator==(rhs));
}

KSim::Theme &KSim::Theme::operator=(const KSim::Theme &rhs)
{
  if (*this == rhs)
    return *this;

  delete d;
  d = rhs.d;
  return *this;
}

KSim::Theme::~Theme()
{
  delete d;
}

const TQString &KSim::Theme::path() const
{
  return d->location;
}

TQString KSim::Theme::name() const
{
  TQString name = d->location;
  if (name.endsWith("/"))
    name.remove(name.length() - 1, 1);

  return name.remove(0, name.findRev("/") + 1);
}

TQString KSim::Theme::author() const
{
  TQString author(d->readOption("author", false));
  return author.replace(TQRegExp("\""), TQString());
}

int KSim::Theme::fontItem() const
{
  return d->font;
}

int KSim::Theme::alternative() const
{
  return d->alternative;
}

int KSim::Theme::alternatives() const
{
  return d->readOption("theme_alternatives").toInt();
}

int KSim::Theme::chartWidthRef(int defValue) const
{
  return internalNumEntry("chart_width_ref", defValue);
}

int KSim::Theme::chartWidthMin(int defValue) const
{
  return internalNumEntry("chart_width_min", defValue);
}

bool KSim::Theme::scaling(bool defValue) const
{
  return internalNumEntry("allow_scaling", defValue);
}

int KSim::Theme::frameTopHeight(int defValue) const
{
  return KMIN(2, internalNumEntry("frame_top_height", defValue));
}

int KSim::Theme::frameBottomHeight(int defValue) const
{
  return KMIN(2, internalNumEntry("frame_bottom_height", defValue));
}

int KSim::Theme::frameLeftWidth(int defValue) const
{
  return KMIN(2, internalNumEntry("frame_left_width", defValue));
}

int KSim::Theme::frameRightWidth(int defValue) const
{
  return KMIN(2, internalNumEntry("frame_right_width", defValue));
}

TQRect KSim::Theme::frameTopBorder(const TQRect &defValue) const
{
  return internalRectEntry("frame_top_border", defValue);
}

TQRect KSim::Theme::frameBottomBorder(const TQRect &defValue) const
{
  return internalRectEntry("frame_bottom_border", defValue);
}

TQRect KSim::Theme::frameLeftBorder(const TQRect &defValue) const
{
  return internalRectEntry("frame_left_border", defValue);
}

TQRect KSim::Theme::frameRightBorder(const TQRect &defValue) const
{
  return internalRectEntry("frame_right_border", defValue);
}

TQColor KSim::Theme::chartInColour(const TQColor &defValue) const
{
  if (d->recolour)
    return TQApplication::palette().active().background();

  return internalColourEntry("chart_in_color", defValue);
}

TQColor KSim::Theme::chartInColourGrid(const TQColor &defValue) const
{
  return internalColourEntry("chart_in_color_grid", defValue);
}

TQColor KSim::Theme::chartOutColour(const TQColor &defValue) const
{
  if (d->recolour)
    return TQApplication::palette().active().background();

  return internalColourEntry("chart_out_color", defValue);
}

TQColor KSim::Theme::chartOutColourGrid(const TQColor &defValue) const
{
  return internalColourEntry("chart_out_color_grid", defValue);
}

bool KSim::Theme::bgGridMode(bool defValue) const
{
  return internalNumEntry("bg_grid_mode", defValue);
}

int KSim::Theme::rxLedX(int defValue) const
{
  return internalNumEntry("rx_led_x", defValue);
}

int KSim::Theme::rxLedY(int defValue) const
{
  return internalNumEntry("rx_led_y", defValue);
}

int KSim::Theme::txLedX(int defValue) const
{
  return internalNumEntry("tx_led_x", defValue);
}

int KSim::Theme::txLedY(int defValue) const
{
  return internalNumEntry("tx_led_y", defValue);
}

int KSim::Theme::mailFrames(int defValue) const
{
  return internalNumEntry("decal_mail_frames", defValue);
}

int KSim::Theme::mailDelay(int defValue) const
{
  return internalNumEntry("decal_mail_delay", defValue);
}

int KSim::Theme::krellSliderDepth(int defValue) const
{
  return internalNumEntry("krell_slider_depth", defValue);
}

int KSim::Theme::krellSliderXHot(int defValue) const
{
  return internalNumEntry("krell_slider_x_hot", defValue);
}

TQRect KSim::Theme::sliderPanel(const TQRect &defValue) const
{
  return internalRectEntry("bg_slider_panel_border", defValue);
}

TQRect KSim::Theme::sliderMeter(const TQRect &defValue) const
{
  return internalRectEntry("bg_slider_meter_border", defValue);
}

TQRect KSim::Theme::timerBorder(const TQRect &defValue) const
{
  return internalRectEntry("bg_timer_border", defValue);
}

TQRect KSim::Theme::buttonPanelBorder(const TQRect &defValue) const
{
  return internalRectEntry("button_panel_border", defValue);
}

TQRect KSim::Theme::buttonMeterBorder(const TQRect &defValue) const
{
  return internalRectEntry("button_meter_border", defValue);
}

TQFont KSim::Theme::largeFont() const
{
  TQString font(internalStringEntry("large_font", TQString()));

  if (font.isEmpty())
    return TQApplication::font();

  TQFont themeFont;
  themeFont.setRawName(font.replace(TQRegExp("\""), TQString()));
  return themeFont;
}

TQFont KSim::Theme::normalFont() const
{
  TQString font(internalStringEntry("normal_font", TQString()));

  if (font.isEmpty())
    return TQApplication::font();

  TQFont themeFont;
  themeFont.setRawName(font.replace(TQRegExp("\""), TQString()));
  return themeFont;
}

TQFont KSim::Theme::smallFont() const
{
  TQString font(internalStringEntry("small_font", TQString()));

  if (font.isEmpty())
    return TQApplication::font();

  TQFont themeFont;
  themeFont.setRawName(font.replace(TQRegExp("\""), TQString()));
  return themeFont;
}

TQFont KSim::Theme::currentFont() const
{
  switch (fontItem()) {
    case 0:
      return smallFont();
      break;
    case 1:
      return normalFont();
      break;
    case 2:
      return largeFont();
      break;
    case 3:
      return KSim::ThemeLoader::currentFont();
      break;
    case 4:
      return TDEGlobalSettings::generalFont();
      break;
  }

  return TQFont();
}

TQString KSim::Theme::meterPixmap(int type, bool useDefault) const
{
  TQString imageFile = createType(type, d->location);
  TQString text;
  TQString file = d->fileNames[7];

  TQStringList::ConstIterator it;
  for (it = d->imageTypes.begin(); it != d->imageTypes.end(); ++it) {
    if (TQFile::exists(imageFile + file + d->altTheme + "." + *it)) {
      text = imageFile + file + d->altTheme + "." + *it;
      break;
    }
    else
    if (TQFile::exists(d->location + file + d->altTheme + "." + *it)) {
      text = d->location + file + d->altTheme + "." + *it;
      break;
    }
  }

  if (text.isNull() && useDefault)
    return KSim::ThemeLoader::defaultUrl() + d->fileNames[7] + ".png";

  return text;
}

TQString KSim::Theme::panelPixmap(int type, bool useDefault) const
{
  TQString imageFile = createType(type, d->location);
  TQString text;
  TQString file = d->fileNames[6];

  TQStringList::ConstIterator it;
  for (it = d->imageTypes.begin(); it != d->imageTypes.end(); ++it) {
    if (TQFile::exists(imageFile + file + d->altTheme + "." + *it)) {
      text = imageFile + file + d->altTheme + "." + *it;
      break;
    }
    else
    if (TQFile::exists(d->location + file + d->altTheme + "." + *it)) {
      text = d->location + file + d->altTheme + "." + *it;
      break;
    }
  }

  if (text.isNull() && useDefault)
    return KSim::ThemeLoader::defaultUrl() + d->fileNames[6] + ".png";

  return text;
}

TQString KSim::Theme::ledPixmap(int type, bool useDefault) const
{
  TQString imageFile = createType(type, d->location);
  TQString text;
  TQString file = d->fileNames[30];

  TQStringList::ConstIterator it;
  for (it = d->imageTypes.begin(); it != d->imageTypes.end(); ++it) {
    if (TQFile::exists(imageFile + file + d->altTheme + "." + *it)) {
      text = imageFile + file + d->altTheme + "." + *it;
      break;
    }
    else
    if (TQFile::exists(d->location + file + d->altTheme + "." + *it)) {
      text = d->location + file + d->altTheme + "." + *it;
      break;
    }
  }

  if (text.isNull() && useDefault)
    return KSim::ThemeLoader::defaultUrl() + d->fileNames[30] + ".png";

  return text;
}

TQString KSim::Theme::framePixmap(int type, bool useDefault) const
{
  TQString text;
  TQString file;

  switch (type) {
    case Types::TopFrame:
      file = d->fileNames[0];
      break;
    case Types::BottomFrame:
      file = d->fileNames[1];
      break;
    case Types::LeftFrame:
      file = d->fileNames[2];
      break;
    case Types::RightFrame:
      file = d->fileNames[3];
      break;
  }

  TQStringList::ConstIterator it;
  for (it = d->imageTypes.begin(); it != d->imageTypes.end(); ++it) {
    if (TQFile::exists(d->location + file + d->altTheme + "." + *it)) {
      text = d->location + file + d->altTheme + "." + *it;
      break;
    }
  }

  if (text.isNull() && useDefault) {
    switch (type) {
      case Types::TopFrame:
        return KSim::ThemeLoader::defaultUrl() + d->fileNames[0] + ".png";
        break;
      case Types::BottomFrame:
        return KSim::ThemeLoader::defaultUrl() + d->fileNames[1] + ".png";
        break;
      case Types::LeftFrame:
        return KSim::ThemeLoader::defaultUrl() + d->fileNames[2] + ".png";
        break;
      case Types::RightFrame:
        return KSim::ThemeLoader::defaultUrl() + d->fileNames[3] + ".png";
        break;
    }
  }

  return text;
}

TQString KSim::Theme::chartPixmap(bool useDefault) const
{
  return loader(4, useDefault);
}

TQString KSim::Theme::gridPixmap(bool useDefault) const
{
  return loader(5, useDefault);
}

TQString KSim::Theme::krellPanelPixmap(bool useDefault) const
{
  return loader(14, useDefault);
}

TQString KSim::Theme::krellMeterPixmap(bool useDefault) const
{
  return loader(15, useDefault);
}

TQString KSim::Theme::krellSliderPixmap(bool useDefault) const
{
  return loader(16, useDefault);
}

TQString KSim::Theme::dataInPixmap(bool useDefault) const
{
  return loader(18, useDefault);
}

TQString KSim::Theme::dataOutPixmap(bool useDefault) const
{
  return loader(20, useDefault);
}

TQString KSim::Theme::mailPixmap(bool useDefault) const
{
  TQString imageFile = createType(KSim::Types::Mail, d->location);
  TQString text;
  TQString file = d->fileNames[25];

  TQStringList::ConstIterator it;
  for (it = d->imageTypes.begin(); it != d->imageTypes.end(); ++it) {
    if (TQFile::exists(imageFile + file + d->altTheme + "." + *it)) {
      text = imageFile + file + d->altTheme + "." + *it;
      break;
    }
    else
    if (TQFile::exists(d->location + file + d->altTheme + "." + *it)) {
      text = d->location + file + d->altTheme + "." + *it;
      break;
    }
  }

  if (text.isNull() && useDefault)
    return KSim::ThemeLoader::defaultUrl()
       + createType(KSim::Types::Mail, TQString()) + file + ".png";

  return text;
}

TQPixmap KSim::Theme::splitPixmap(PixmapType type, uint itemNo,
   bool useDefault) const
{
  return pixmapToList(type, itemNo, useDefault)[itemNo];
}

TQValueList<TQPixmap> KSim::Theme::pixmapToList(PixmapType type,
   int limitAmount, bool useDefault) const
{
  TQImage image;
  int xOffset = 0;
  int yOffset = 0;
  int depth = 0;

  switch (type) {
    case KrellPanel:
      depth = readIntEntry("StylePanel", "*.krell_depth");
      xOffset = readIntEntry("StylePanel", "*.krell_x_hot");
      yOffset = readIntEntry("StylePanel", "*.krell_yoff");
      image.load(krellPanelPixmap(useDefault));
      kdDebug(2003) << "KSim::Theme: type = KrellPanel" << endl;
      break;
    case KrellMeter:
      depth = readIntEntry("StyleMeter", "*.krell_depth");
      xOffset = readIntEntry("StyleMeter", "*.krell_x_hot");
      yOffset = readIntEntry("StyleMeter", "*.krell_yoff");
      image.load(krellMeterPixmap(useDefault));
      kdDebug(2003) << "KSim::Theme: type = KrellMeter" << endl;
      break;
    case KrellSlider:
      depth = krellSliderDepth();
      image.load(krellSliderPixmap(useDefault));
      kdDebug(2003) << "KSim::Theme: type = KrellSlider" << endl;
      break;
    default:
      return TQValueList<TQPixmap>();
      break;
  }

  if (image.isNull())
    return TQValueList<TQPixmap>();

  TQValueList<TQPixmap> list;
  int size = image.height();
  if (depth)
    size = image.height() / depth;

  KSim::ThemeLoader::self().reColourImage(image);
  TQPixmap pixmap = image;
  TQPixmap newPixmap(image.width() - xOffset, size);

  for (int i = 0; i < (depth + 1); ++i) {
    newPixmap.fill();

    if (pixmap.mask()) {
      TQBitmap mask(newPixmap.size());
      bitBlt(&mask, 0, 0, pixmap.mask(), xOffset, yOffset,
         image.width() - xOffset, size);
      newPixmap.setMask(mask);
    }

    bitBlt(&newPixmap, 0, 0, &pixmap, xOffset, yOffset,
       image.width() - xOffset, size);

    list.append(newPixmap);

    if (limitAmount == i)
      break;
  }

  kdDebug(2003) << "KSim::Theme: size = " << size << endl;
  kdDebug(2003) << "KSim::Theme: depth = " << depth << endl;
  kdDebug(2003) << "KSim::Theme: xOffset = " << xOffset << endl;
  kdDebug(2003) << "KSim::Theme: yOffset = " <<  xOffset << endl;

  return list;
}

int KSim::Theme::transparency(const TQString &itemType,
   const TQString &entry) const
{
  return readIntEntry(itemType, entry);
}

bool KSim::Theme::textShadow(const TQString &itemType,
   const TQString &entry) const
{
  if (d->recolour)
    return false;

  TQString shadow = readEntry(itemType, entry);
  if (shadow.isEmpty() || shadow.findRev("none") != -1)
    return false;

  return true;
}

TQColor KSim::Theme::shadowColour(const TQString &itemType,
   const TQString &entry) const
{
  return readColourEntry(itemType, entry, 1);
}

TQColor KSim::Theme::textColour(const TQString &itemType,
   const TQString &entry) const
{
  if (d->recolour)
    return TDEGlobalSettings::textColor();

  return readColourEntry(itemType, entry, 0);
}

int KSim::Theme::readIntEntry(const TQString &itemType,
    const TQString &entry) const
{
  TQString entryName = itemType + " " + entry;
  return internalNumEntry(entryName, 0);
}

TQRect KSim::Theme::readRectEntry(const TQString &itemType,
   const TQString &entry) const
{
  TQString entryName = itemType + " " + entry;
  return internalRectEntry(entryName, TQRect());
}

TQString KSim::Theme::readEntry(const TQString &itemType,
   const TQString &entry) const
{
  TQString entryName = itemType + " " + entry;
  return internalStringEntry(entryName, TQString());
}

TQString KSim::Theme::readColourEntry(const TQString &itemType,
   const TQString &entry, int row) const
{
  TQString color = readEntry(itemType, entry);
  if (color.isEmpty())
    color = TQString::fromLatin1("#ffffff #ffffff");

  return TQStringList::split(' ', color)[row];
}

TQFont KSim::Theme::readFontEntry(const TQString &itemType,
   const TQString &entry) const
{
  const TQString &font = readEntry(itemType, entry);

  // If only there was a better way of doing this
  if (font == "small_font")
    return smallFont();
  else
  if (font == "normal_font")
    return normalFont();
  else
  if (font == "large_font")
    return largeFont();

  return TQFont();
}

bool KSim::Theme::fontColours(int type, const TQString &string, TQFont &font,
   TQColor &text, TQColor &shadow, bool &showShadow) const
{
  TQString key = KSim::Types::typeToString(type, false);
  bool repaint = false;

  // set colours from the string 'key'
  if (!readEntry(string, key + ".textcolor").isEmpty()) {
    text= textColour(string, key + ".textcolor");
    shadow = shadowColour(string, key + ".textcolor");
    showShadow = textShadow(string, key + ".textcolor");
    repaint = true;
  }
  else {
    text= textColour(string, "*.textcolor");
    shadow = shadowColour(string, "*.textcolor");
    showShadow = textShadow(string, "*.textcolor");
  }

  // set fonts from the string 'key'
  if (!readEntry(string, key + ".font").isEmpty()) {
    if (KSim::ThemeLoader::currentFontItem() != 3) {
      font = readFontEntry(string, key + ".font");
      repaint = true;
    }
  }
  else {
    font = currentFont();
  }

  return repaint;
}

bool KSim::Theme::fontColours(const KSim::Base *const base, TQFont &font,
   TQColor &text, TQColor &shadow, bool &showShadow) const
{
  if (!base)
    return false;

  return fontColours(base->type(), base->configString(), font,
     text, shadow, showShadow);
}

KSim::Theme::Theme()
{
  d = 0;
}

KSim::Theme::Theme(const TQString &url, const TQString &fileName, int alt,
   const TQValueVector<TQString> &vector, const TQStringList &list,
   TDEConfig *globalReader)
{
  create(vector, list, globalReader);
  init(url, fileName, alt);

  KSim::Config::config()->setGroup("Misc");
  d->recolour = KSim::Config::config()->readBoolEntry("ReColourTheme", false);
}

void KSim::Theme::create(const TQValueVector<TQString> &vector,
   const TQStringList &list, TDEConfig *globalReader)
{
  d = new Private(vector, list);
  d->globalReader = globalReader;
}

void KSim::Theme::init(const TQString &url, const TQString &fileName, int alt)
{
  d->altTheme = KSim::ThemeLoader::alternativeAsString(alt);
  d->location = url;
  d->alternative = alt;
  d->font = KSim::ThemeLoader::currentFontItem();

  d->file = TQStringList::split("\n", parseConfig(url, fileName));

  if (alt != 0)
    d->dFile = TQStringList::split("\n", parseConfig(url, "gkrellmrc"));
}

void KSim::Theme::reparse(const TQString &url, const TQString &fileName, int alt)
{
  init(url, fileName, alt);
}

TQString KSim::Theme::parseConfig(const TQString &url,
     const TQString &fileName)
{
  return KSim::ThemeLoader::self().parseConfig(url, fileName);
}

TQString KSim::Theme::loader(int value, bool useDefault) const
{
  TQString text;
  TQString file = d->fileNames[value];

  TQStringList::ConstIterator it;
  for (it = d->imageTypes.begin(); it != d->imageTypes.end(); ++it) {
    if (TQFile::exists(d->location + file + d->altTheme + "." + *it)) {
      text = d->location + file + d->altTheme + "." + *it;
      break;
    }
  }

  if (text.isNull() && useDefault)
    return KSim::ThemeLoader::defaultUrl() + d->fileNames[value] + ".png";

  return text;
}

TQString KSim::Theme::createType(int type, const TQString &text) const
{
  if (type == Types::None)
    return text;

  return text + KSim::Types::typeToString(type);
}

void KSim::Theme::setRecolour(bool value)
{
  if (!d)
    return;

  d->recolour = value;
}

// Keep the ugliness here to make the rest
// of the class readable
int KSim::Theme::internalNumEntry(const TQString &entry, int defValue) const
{
  return d->readOption(entry, true, TQString::number(defValue)).toInt();
}

TQRect KSim::Theme::internalRectEntry(const TQString &entry,
   const TQRect &defValue) const
{
  TQString rect;
  rect += TQString::number(defValue.left());
  rect += ",";
  rect += TQString::number(defValue.top());
  rect += ",";
  rect += TQString::number(defValue.width());
  rect += ",";
  rect += TQString::number(defValue.height());

  TQStringList list = TQStringList::split(",", d->readOption(entry, true, rect));
  TQRect rect2(list[0].toInt(), list[1].toInt(), list[2].toInt(), list[3].toInt());

  return rect2;
}

TQColor KSim::Theme::internalColourEntry(const TQString &entry,
   const TQColor &defValue) const
{
  return d->readOption(entry, true, defValue.name());
}

TQString KSim::Theme::internalStringEntry(const TQString &entry,
   const TQString &defValue) const
{
  return d->readOption(entry, true, defValue);
}

// check if the dir is writable, if not generate the new file in the home dir
#define KSIM_THEME_URL(url) \
{ \
}

KSim::ThemeLoader *KSim::ThemeLoader::m_self = 0; // initialize pointer
KSim::ThemeLoader &KSim::ThemeLoader::self()
{
  if (!m_self) { // is it the first call?
    m_self = new KSim::ThemeLoader; // create sole instance
    tqAddPostRoutine(cleanup);
  }

  return *m_self; // address of sole instance
}

bool KSim::ThemeLoader::isDifferent() const
{
  // if the theme path, theme alternative or the
  // specified font has changed then we should
  // return true, else false

  KSim::Config::config()->setGroup("Misc");
  bool recolour = KSim::Config::config()->readBoolEntry("ReColourTheme", false);

  return (current().path() != currentUrl()
     || current().alternative() != currentAlternative()
     || current().fontItem() != currentFontItem()
     || d->recolour != recolour);
}

void KSim::ThemeLoader::reload()
{
  reColourItems();
  grabColour();

  if (!isDifferent())
    return;

  if (currentUrl() != defaultUrl()) {
    if (!d->globalReader)
      d->globalReader = new TDEConfig(defaultUrl() + "gkrellmrc_ksim");
  }
  else {
    delete d->globalReader;
    d->globalReader = 0;
  }

  if (m_theme.d)
    m_theme.d->globalReader = d->globalReader;

  TQString fileName = TQString::fromLatin1("gkrellmrc") + alternativeAsString();
  m_theme.reparse(currentUrl(), fileName, currentAlternative());
}

const KSim::Theme &KSim::ThemeLoader::current() const
{
  return m_theme;
}

KSim::Theme KSim::ThemeLoader::theme(const TQString &url,
   const TQString &rcFile, int alt) const
{
  return KSim::Theme(url, rcFile, alt, d->fileNames,
     d->imageTypes, d->globalReader);
}

void KSim::ThemeLoader::reColourImage(TQImage &image)
{
  if (!d->recolour || image.isNull())
    return;

  TQColor color = TQApplication::palette().active().background();
  TQImage output(image.width(), image.height(), 32);
  output.setAlphaBuffer(image.hasAlphaBuffer());

  TQ_UINT32 r = color.red();
  TQ_UINT32 g = color.green();
  TQ_UINT32 b = color.blue();
  TQ_UINT32 *write = reinterpret_cast<TQ_UINT32 *>(output.bits());
  TQ_UINT32 *read  = reinterpret_cast<TQ_UINT32 *>(image.bits());
  int size = image.width() * image.height();

  for (int pos = 0; pos < size; pos++)
  {
    TQRgb basePix = static_cast<TQRgb>(*read);

    // Here, we assume that source is really gray, so R=G=B=I
    // Use blue since it's very easy to extract.
    TQ_UINT32 i = tqBlue(basePix);

    TQ_UINT32 cr = (r * i + 128) >> 8; // Fixed point..
    TQ_UINT32 cg = (g * i + 128) >> 8;
    TQ_UINT32 cb = (b * i + 128) >> 8;

    TQ_UINT32 alpha = tqAlpha(basePix);
    *write = tqRgba(cr, cg, cb, alpha);
    write++;
    read++;
  }

  image = output;
}

TQString KSim::ThemeLoader::parseConfig(const TQString &url,
   const TQString &fileName)
{
  TQFile origFile(url + fileName);

  if (!origFile.open(IO_ReadOnly))
    return TQString();

  TQTextStream origStream(&origFile);
  TQString text;
  TQRegExp reg("\\*"); // regexp for '*' chars
  TQRegExp number("[0-9]+");  // regexp for all numbers
  TQRegExp numbers("[0-9]+,[0-9]+,[0-9]+,[0-9]+"); // regexp for int,int,int,int
  TQRegExp minus("[a-zA-Z]+ \\- [a-zA-Z]+"); // regexp for 'someText - someText'
  while (!origStream.atEnd()) {
    TQString line(origStream.readLine().simplifyWhiteSpace());

    if (line.find(reg) == 0) // find the location of the * comments
      // replace all * comments with # comments so TDEConfig doesn't complain
      line.replace(reg, "#");

    if (line.find("#") == -1) { // find the location of the string 'gkrellmms'
      if (line.findRev("=") == -1) { // if found we check for the string '='
        int numLoc = line.findRev(numbers);
        if (numLoc != -1)
          // if '=' doesn't exist we add one so TDEConfig doesn't complain
          line.insert(numLoc, " = ");

        numLoc = line.findRev(number);
        if (numLoc != -1)
          // if '=' doesn't exist we add one so TDEConfig doesn't complain
          line.insert(numLoc, " = ");

        numLoc = line.findRev(minus);
        if (numLoc != -1)
          // replace the '-' with an '=' so TDEConfig doesn't get confused
          line.replace(TQRegExp("-"), "=");
      }
    }

    text.append(line).append('\n');
  }

  return text;
}

// GKrellM themes seem to be rather inconsistant
// so the following code changes the dir structure
// of a theme to be more consistant, but the dir structure
// is still compliant with GKrellM.
void KSim::ThemeLoader::parseDir(const TQString &url, int alt)
{
  if ( !TQFileInfo( url ).isWritable() && currentName() != "ksim" )
  {
    TQString homePath = TQDir::current().path();
    homePath = locateLocal( "data", "ksim" )
       + TQString::fromLatin1( "/themes" )
       + homePath.right( homePath.length()
       - homePath.findRev( TQRegExp( "\\/" ),
       homePath.length() ) );

    if ( !TQFile::exists( homePath ) )
      TDEStandardDirs::makeDir( homePath );

    kdWarning() << "Cant write to current dir, setting dir to "
       << homePath << endl;

    TQDir::setCurrent( homePath );
  }

  int alternatives = ++alt;

  TQStringList formats;
  TQStringList panels;
  TQStringList meters;

  formats << "png" << "jpg" << "jpeg" << "gif" << "xpm";
  panels << "inet" << "net" << "proc" << "cpu" << "disk";
  meters << "mem" << "fs" << "mail" << "apm" << "uptime"
     << "clock" << "cal" << "timer" << "host" << "swap";

  TQDir directory;
  for (int i = 0; i < alternatives; ++i) {
    TQString altString = KSim::ThemeLoader::alternativeAsString(i);
    if (alternatives == 1 || i == 0)
      altString = TQString();

    TQStringList::ConstIterator format;
    for (format = formats.begin(); format != formats.end(); ++format) {
      // go through the meters array and move the files to the correct dir/filename
      TQStringList::Iterator meter;
      for (meter = meters.begin(); meter != meters.end(); ++meter) {
        TQString bgMeter = TQString::fromLatin1("bg_meter_");
        if (TQFile::exists(bgMeter + (*meter) + altString + "." +  (*format))) {
          if (TDEStandardDirs::makeDir(url + (*meter)))
            directory.rename(bgMeter + (*meter) + altString + "." + (*format),
                 (*meter) + "/bg_meter" + altString + "." + (*format));
        }
      }

      // go through the panels array and move the files to the correct dir/filename
      TQStringList::ConstIterator panel;
      for (panel = panels.begin(); panel != panels.end(); ++panel) {
        TQString bgPanel = TQString::fromLatin1("bg_panel_");
        if (TQFile::exists(bgPanel + (*panel) + altString + "." + (*format))) {
          if (TDEStandardDirs::makeDir(url + (*panel)))
            directory.rename(bgPanel + (*panel) + altString + "." + (*format),
                (*panel) + "/bg_panel" + altString + "." + (*format));
        }
      }

      // fix stupid themes that have a bg_panel image in the host dir
      TQString tempFile = TQString::fromLatin1("host/bg_panel");
      if (TQFile::exists(tempFile + altString + "." + (*format)))
        directory.rename(tempFile + altString + "." + (*format), "host/bg_meter"
           + altString + "." + (*format));

      // move decal_net_leds* to the net folder to be more consistant
      tempFile = TQString::fromLatin1("decal_net_leds");
      if (TQFile::exists(tempFile + altString + "." + (*format))) {
        if (TDEStandardDirs::makeDir(url + "net"))
          directory.rename(tempFile + altString + "." + (*format),
             "net/decal_net_leds" + altString + "." + (*format));
      }
    }
  }
}

void KSim::ThemeLoader::validate()
{
  if (!TQFile::exists(currentUrl())) {
    KSim::Config::config()->setGroup("Theme");
    KSim::Config::config()->writeEntry("Name", "ksim");
    KSim::Config::config()->writeEntry("Alternative", 0);
    KSim::Config::config()->sync();
  }
}

void KSim::ThemeLoader::themeColours(TQWidget *widget)
{
  widget->setEraseColor(d->pixelColour);
}

TQString KSim::ThemeLoader::currentName()
{
  KSim::Config::config()->setGroup("Theme");
  return KSim::Config::config()->readEntry("Name", "ksim");
}

TQString KSim::ThemeLoader::currentUrl()
{
  KSim::Config::config()->setGroup("Theme");
  TQString folder(KSim::Config::config()->readEntry("Name"));
  folder.prepend("ksim/themes/").append("/");
  TQString dirName(TDEGlobal::dirs()->findResourceDir("data", folder));
  dirName += folder;

  return dirName;
}

TQString KSim::ThemeLoader::defaultUrl()
{
  return TDEGlobal::dirs()->findDirs("data", "ksim/themes/ksim").first();
}

int KSim::ThemeLoader::currentAlternative()
{
  KSim::Config::config()->setGroup("Theme");
  int alternative = KSim::Config::config()->readNumEntry("Alternative");

  if ( alternative > self().current().alternatives() )
    alternative = self().current().alternatives();

  return alternative;
}

TQString KSim::ThemeLoader::alternativeAsString(int alt)
{
  int alternative = (alt == -1 ? currentAlternative() : alt);
  return (alternative == 0 ? TQString() : TQString::fromLatin1("_")
     + TQString::number(alternative));
}

TQFont KSim::ThemeLoader::currentFont()
{
  if (currentFontItem() != 3)
    return self().current().currentFont();

  KSim::Config::config()->setGroup("Theme");
  return KSim::Config::config()->readFontEntry("Font");
}

int KSim::ThemeLoader::currentFontItem()
{
  KSim::Config::config()->setGroup("Theme");
  return KSim::Config::config()->readNumEntry("FontItem", 0);
}

KSim::ThemeLoader::ThemeLoader()
{
  m_self = this;

  d = new Private;
  d->imageTypes << "png" << "jpg" << "jpeg" << "xpm" << "gif";

  if (currentUrl() != defaultUrl())
    d->globalReader = new TDEConfig(defaultUrl() + "gkrellmrc_ksim");
  else
    d->globalReader = 0;

  d->fileNames.resize(31);
  d->fileNames[0] = "frame_top";
  d->fileNames[1] = "frame_bottom";
  d->fileNames[2] = "frame_left";
  d->fileNames[3] = "frame_right";
  d->fileNames[4] = "bg_chart";
  d->fileNames[5] = "bg_grid";
  d->fileNames[6] = "bg_panel";
  d->fileNames[7] = "bg_meter";
  d->fileNames[8] = "bg_slider_panel";
  d->fileNames[9] = "bg_slider_meter";
  d->fileNames[10] = "button_panel_in";
  d->fileNames[11] = "button_panel_out";
  d->fileNames[12] = "button_meter_in";
  d->fileNames[13] = "button_meter_out";
  d->fileNames[14] = "krell_panel";
  d->fileNames[15] = "krell_meter";
  d->fileNames[16] = "krell_slider";
  d->fileNames[17] = "decal_misc";
  d->fileNames[18] = "data_in";
  d->fileNames[19] = "data_in_grid";
  d->fileNames[20] = "data_out";
  d->fileNames[21] = "data_out_grid";
  d->fileNames[22] = "krell";
  d->fileNames[23] = "spacer_top";
  d->fileNames[24] = "spacer_bottom";
  d->fileNames[25] = "decal_mail";
  d->fileNames[26] = "krell_penguin";
  d->fileNames[27] = "bg_volt";
  d->fileNames[28] = "decal_timer_button";
  d->fileNames[29] = "bg_timer";
  d->fileNames[30] = "decal_net_leds";

  m_theme.create(d->fileNames, d->imageTypes, d->globalReader);

  TQString fileName = TQString::fromLatin1("gkrellmrc") + alternativeAsString();
  m_theme.init(currentUrl(), fileName, currentAlternative());

  reColourItems();
  grabColour();
}

KSim::ThemeLoader::~ThemeLoader()
{
  delete d->globalReader;
  delete d;
}

void KSim::ThemeLoader::cleanup()
{
  if (!m_self)
    return;

  delete m_self;
  m_self = 0;
}

void KSim::ThemeLoader::reColourItems()
{
  KSim::Config::config()->setGroup("Misc");
  d->recolour = KSim::Config::config()->readBoolEntry("ReColourTheme", false);
  m_theme.setRecolour(d->recolour);
}

void KSim::ThemeLoader::grabColour()
{
  KSim::Config::config()->setGroup("Theme");
  TQPoint pos(2, 2);
  pos = KSim::Config::config()->readPointEntry("PixelLocation", &pos);

  TQImage image(current().meterPixmap(Types::None));
  reColourImage(image);
  d->pixelColour = image.pixel(pos.x(), pos.y());
}
