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

#ifndef THEMELOADER_H
#define THEMELOADER_H

#include <tqstring.h>
#include <tqrect.h>
#include <tqcolor.h>
#include <tqfont.h>
#include <tqvaluelist.h>

#include <kdemacros.h>

class QStringList;
template<class> class QValueVector;
class KConfig;
class QWidget;

namespace KSim
{
  class Base;

  /**
   * a class containing various information 
   * about a theme, use KSim::ThemeLoader to
   * get an instance
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   * @short Provides a loader for the themes
   */
  class KDE_EXPORT Theme
  {
    friend class ThemeLoader;
    public:
      enum PixmapType { KrellPanel = 0, KrellMeter, KrellSlider };
      ~Theme();
      bool operator==(const KSim::Theme &rhs) const;
      bool operator!=(const KSim::Theme &rhs) const;
      KSim::Theme &operator=(const KSim::Theme &rhs);
      /**
       * @return the theme path
       */
      const TQString &path() const;
      /**
       * @return the name of the theme
       */
      TQString name() const;
      /**
       * @return the author of the theme
       */
      TQString author() const;
      /**
       * @return the set font item for the theme
       */
      int fontItem() const;
      /**
       * @return the amount of alternatives the theme has
       */
      int alternative() const;
      /**
       * @return the amount of alternatives the theme has
       */
      int alternatives() const;
      /**
       * @return the width leds should be scaled to (if scaling() returns true)
       */
      int chartWidthRef(int defValue = 0) const;
      int chartWidthMin(int defValue = 0) const;
      /**
       * @return true if sacling should be enabled
       */
      bool scaling(bool defValue = false) const;
      /**
       * @return the height of the top frame
       */
      int frameTopHeight(int defValue = 0) const;
      /**
       * @return the height of the bottom frame
       */
      int frameBottomHeight(int defValue = 0) const;
      /**
       * @return the width of the left frame
       */
      int frameLeftWidth(int defValue = 0) const;
      /**
       * @return the width of the right frame
       */
      int frameRightWidth(int defValue = 0) const;
      /**
       * @return a rect of the top frame border
       */
      TQRect frameTopBorder(const TQRect &defValue = TQRect()) const;
      /**
       * @return a rect of the bottom frame border
       */
      TQRect frameBottomBorder(const TQRect &defValue = TQRect()) const;
      /**
       * @return a rect of the left frame border
       */
      TQRect frameLeftBorder(const TQRect &defValue = TQRect()) const;
      /**
       * @return a rect of the right frame border
       */
      TQRect frameRightBorder(const TQRect &defValue = TQRect()) const;
      /**
       * @return the color of the chart in
       */
      TQColor chartInColour(const TQColor &defValue = TQColor()) const;
      /**
       * @return the color of the chart in grid
       */
      TQColor chartInColourGrid(const TQColor &defValue = TQColor()) const;
      /**
       * @return the color of the chart out
       */
      TQColor chartOutColour(const TQColor &defValue = TQColor()) const;
      /**
       * @return the color of the chart out grid
       */
      TQColor chartOutColourGrid(const TQColor &defValue = TQColor()) const;
      /**
       * if false then the grid lines should be drawn at the
       * top and bottom of the graphs
       */
      bool bgGridMode(bool defValue = false) const;
      /**
       * @return the X location of the receive led
       */
      int rxLedX(int defValue = 0) const;
      /**
       * @return the Y location of the receive led
       */
      int rxLedY(int defValue = 0) const;
      /**
       * @return the X location of the send led
       */
      int txLedX(int defValue = 0) const;
      /**
       * @return the Y location of the send led
       */
      int txLedY(int defValue = 0) const;
      /**
       * @return the amount of mail frames
       */
      int mailFrames(int defValue = 0) const;
      /**
       * @return the mail check delay
       */
      int mailDelay(int defValue = 0) const;
      /**
       * @return the slider depth
       */
      int krellSliderDepth(int defValue = 7) const;
      /**
       * @return the vertical location of the start of the image
       */
      int krellSliderXHot(int defValue = 0) const;
      /**
       * @return the area for the slider panel
       */
      TQRect sliderPanel(const TQRect &defValue = TQRect()) const;
      /**
       * @return the area for the slider meter
       */
      TQRect sliderMeter(const TQRect &defValue = TQRect()) const;
      /**
       * @return the border for the timer label/button
       */
      TQRect timerBorder(const TQRect &defValue = TQRect()) const;
      /**
       * @return the border for the panel button
      */
      TQRect buttonPanelBorder(const TQRect &defValue = TQRect()) const;
      /**
       * @return the border for the meter button
       */
      TQRect buttonMeterBorder(const TQRect &defValue = TQRect()) const;
      /**
       * @return the large font that the theme specifies
       */
      TQFont largeFont() const;
      /**
       * @return the normal font that the theme specifies
       */
      TQFont normalFont() const;
      /**
       * @return the small font that the theme specifies
       */
      TQFont smallFont() const;
      /**
       * @return the current font to be used
       * according to fontItem()
       */
      TQFont currentFont() const;
      /**
       * @return the meter image (bg_meter.[png|jpg|gif])
       * of the current theme, if type is specified then it will
       * look in the folder type.
       *
       * it will first look in the dir 'type' if no image is found
       * it will drop back a dir
       * @param type is one of: none, apm, cal, clock, fs,
       * host, mail, mem, swap, timer, uptime
       */
      TQString meterPixmap(int type, bool useDefault = true) const;
      /**
       * @return the meter image (bg_panel.[png|jpg|gif])
       * of the current theme, it will first look in the dir 'type'
       * if no image is found it will drop back a dir
       * @param type is one of: net, inet
       */
      TQString panelPixmap(int type, bool useDefault = true) const;
      /**
       * @return the decal net leds image
       * (usually decal_net_leds.[png|jpg|gif]) of the
       * current theme, it will first look in the dir 'type'
       * if no image is found it will drop back a dir
       * @param type is one of: net, inet
       */
      TQString ledPixmap(int type, bool useDefault = true) const;
      /**
       * @return the frame image of the current theme,
       * @param type is one of: top, bottom, left, right
       */
      TQString framePixmap(int type, bool useDefault = true) const;
      /**
       * @return the krell chart image (bg_chart.[png|jpg|gif])
       * of the current theme
       */
      TQString chartPixmap(bool useDefault = true) const;
      /**
       * @return the krell grid image (bg_grid.[png|jpg|gif])
       * of the current theme
       */
      TQString gridPixmap(bool useDefault = true) const;
      /**
       * @return the krell panel image (krell_panel.[png|jpg|gif])
       * of the current theme
       */
      TQString krellPanelPixmap(bool useDefault = true) const;
      /**
       * @return the krell meter image (krell_meter.[png|jpg|gif])
       * of the current theme
       */
      TQString krellMeterPixmap(bool useDefault = true) const;
      /**
       * @return the krell slider image (krell_slider.[png|jpg|gif])
       * of the current theme
       */
      TQString krellSliderPixmap(bool useDefault = true) const;
      /**
       * @return the data in image (data_in.[png|jpg|gif])
       * of the current theme
       */
      TQString dataInPixmap(bool useDefault = true) const;
      /**
       * @return the data out image (data_out.[png|jpg|gif])
       * of the current theme
       */
      TQString dataOutPixmap(bool useDefault = true) const;
      /**
       * @return the mail image (mail/decal_mail.[png|jpg|gif])
       * of the current theme
       */
      TQString mailPixmap(bool useDefault = true) const;
      /**
       * @return a segmant of an image, using @p itemNo to
       * get the segmant and from the ImageType @p type
       */
      TQPixmap splitPixmap(PixmapType type, uint itemNo = 0,
         bool useDefault = true) const;
      /**
       * Same as the above function but returns an array of pixmaps
       * with the maximum size of limitAmount, or all the pixmaps if @p
       * limitAmount is -1 (default)
       * @return an array of pixmaps
       */
      TQValueList<TQPixmap> pixmapToList(PixmapType type,
         int limitAmount = -1, bool useDefault = true) const;
      /**
       * returns the transparency level from the specified keys
       */
      int transparency(const TQString &, const TQString &) const;
      /**
       * @return true if shadow text is enabled
       */
      bool textShadow(const TQString &, const TQString &) const;
      /**
       * @return the shadow color (if any) for the specified keys
       */
      TQColor shadowColour(const TQString &, const TQString &) const;
      /**
       * @return the text color for the specified keys
       */
      TQColor textColour(const TQString &, const TQString &) const;
      /**
       * reads an entry and returns it as an int
       */
      int readIntEntry(const TQString &, const TQString &) const;
      /**
       * @return a rect from the specified keys
       */
      TQRect readRectEntry(const TQString &, const TQString &) const;
      /**
       * reads an entry from the specified keys
       */
      TQString readEntry(const TQString &, const TQString &) const;
      /**
       * reads a color entry from the specified keys
       */
      TQString readColourEntry(const TQString &, const TQString &, int) const;
      /**
       * reads a font entry from the specified keys
       */
      TQFont readFontEntry(const TQString &, const TQString &) const;
      /**
       * sets the font, textColour, shadowColour and showShadow to
       * the fonts and colours KSim should use depending on type and
       * string.
       *
       * returns true if one of the parameters were changed using
       * type and string, else the parameters will be set to the current
       * colours and font that KSim should use and returns false.
       *
       * example usage:
       * <pre>
       *   TQFont font;
       *   TQColor textColour, shadowColour;
       *   bool showShadow;
       *   if (KSim::ThemeLoader::self().current().fontColours(type(), configString(), font,
       *      textColour, shadowColour, showShadow)
       *   {
       *     setFont(font);
       *     // Do something with textColour, shadowColour and showShadow
       *   }
       * </pre>
       * @param type is the theme type you want, generally if you're calling this
       * from a KSim::Base (or derived from) object then use type()
       * @param string is the config string entry you want the function to read
       * from, generally if you're calling this from a KSim::Base (or derived from)
       * object then use configString()
       */
      bool fontColours(int type, const TQString &string, TQFont &font,
         TQColor &textColour, TQColor &shadowColour, bool &showShadow) const;
      /**
       * convenience function.
       *
       * collects the theme type and config key from the base pointer
       */
      bool fontColours(const KSim::Base *const base, TQFont &font,
         TQColor &textColour, TQColor &shadowColour, bool &showShadow) const;

      Theme(const KSim::Theme &);

    private:
      Theme();
      Theme(const TQString &url, const TQString &fileName, int alt,
         const TQValueVector<TQString> &vector, const TQStringList &list,
         KConfig *globalReader);
      void create(const TQValueVector<TQString> &, const TQStringList &, KConfig *);
      void init(const TQString &url, const TQString &fileName, int alt);
      void reparse(const TQString &url, const TQString &fileName, int alt);
      TQString parseConfig(const TQString &, const TQString &);
      TQString loader(int, bool useDefault = true) const;
      TQString createType(int, const TQString &) const;
      void setRecolour(bool);

      int internalNumEntry(const TQString &, int) const;
      TQRect internalRectEntry(const TQString &, const TQRect &) const;
      TQColor internalColourEntry(const TQString &, const TQColor &) const;
      TQString internalStringEntry(const TQString &, const TQString &) const;

      class Private;
      Private *d;
  };

  /**
   * returns a class Theme containing
   * information of the current theme or
   * of the theme path specified
   * @author Robbie Ward <linuxphreak@gmx.co.uk>
   * @short Provides a loader for the themes
   */
  class KDE_EXPORT ThemeLoader
  {
    public:
      /**
       * @return a reference to the instance
       */
      static ThemeLoader &self();
      /**
       * @return true if the theme has changed
       */
      bool isDifferent() const;
      /**
       * Updates the info to the current theme
       */
      void reload();
      /**
       * @return the current theme that is set
       */
      const KSim::Theme &current() const;
      /**
       * @return a theme from the path specified
       * @param url is the path of the theme dir
       * @param rcFile is the filename of the config file (optional)
       * @param alt is the theme alternative number (optional)
       */
      KSim::Theme theme(const TQString &url,
         const TQString &rcFile = "gkrellmrc", int alt = 0) const;
      /**
       * re-colours an image to the current KDE
       * color scheme
       */
      void reColourImage(TQImage &image);
      /**
       * changes some of the entries in the config file so kconfig
       * can handle the file better.
       * @return the location of the config file
       */
      TQString parseConfig(const TQString &url, const TQString &file);
      /**
       * change the dir structure of a theme directory
       * so KSim can understand them better
       */
      void parseDir(const TQString &url, int alt);
      /**
       * checks if the themeUrl entry is valid,
       * if not it then reverts the current theme
       * to the default url
       */
      void validate();
      /**
       * sets the pallete of the current theme
       */
      void themeColours(TQWidget *);
      /**
       * @return current theme name
       */
      static TQString currentName();
      /**
       * @return current theme url, if there is no current theme
       * then it will return the default theme
       */
      static TQString currentUrl();
      /**
       * @return the default theme url
       */
      static TQString defaultUrl();
      /**
       * @return the current theme alternative
       */
      static int currentAlternative();
      /**
       * @return the current theme alternative as a string, eg if the
       * current theme alt is 1 then this will return '_1'.
       * if there is no theme alt this will return a null string.
       *
       * if this is set to -1 (the default) then the current alternative
       * will be used
       */
      static TQString alternativeAsString(int alt = -1);
      /**
       * @return the font of the theme if the theme font is set to custom
       */
      static TQFont currentFont();
      /**
       * @return the font item of the theme, eg: 0 would be the small font
       */
      static int currentFontItem();

    protected:
      /**
       * Default constructor, use self() to get an instance
       */
      ThemeLoader();
      ~ThemeLoader();

    private:
      ThemeLoader(const ThemeLoader &);
      ThemeLoader &operator=(const ThemeLoader &);
      /**
       * Deletes the instance and cleans up after itself
       */
      static void cleanup();
      void reColourItems();
      void grabColour();

      class Private;
      Private *d;
      KSim::Theme m_theme;
      static ThemeLoader *m_self;
  };
}
#endif
