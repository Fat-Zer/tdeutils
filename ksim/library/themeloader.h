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

#include <qstring.h>
#include <qrect.h>
#include <qcolor.h>
#include <qfont.h>
#include <qvaluelist.h>

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
      const QString &path() const;
      /**
       * @return the name of the theme
       */
      QString name() const;
      /**
       * @return the author of the theme
       */
      QString author() const;
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
      QRect frameTopBorder(const QRect &defValue = QRect()) const;
      /**
       * @return a rect of the bottom frame border
       */
      QRect frameBottomBorder(const QRect &defValue = QRect()) const;
      /**
       * @return a rect of the left frame border
       */
      QRect frameLeftBorder(const QRect &defValue = QRect()) const;
      /**
       * @return a rect of the right frame border
       */
      QRect frameRightBorder(const QRect &defValue = QRect()) const;
      /**
       * @return the color of the chart in
       */
      QColor chartInColour(const QColor &defValue = QColor()) const;
      /**
       * @return the color of the chart in grid
       */
      QColor chartInColourGrid(const QColor &defValue = QColor()) const;
      /**
       * @return the color of the chart out
       */
      QColor chartOutColour(const QColor &defValue = QColor()) const;
      /**
       * @return the color of the chart out grid
       */
      QColor chartOutColourGrid(const QColor &defValue = QColor()) const;
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
      QRect sliderPanel(const QRect &defValue = QRect()) const;
      /**
       * @return the area for the slider meter
       */
      QRect sliderMeter(const QRect &defValue = QRect()) const;
      /**
       * @return the border for the timer label/button
       */
      QRect timerBorder(const QRect &defValue = QRect()) const;
      /**
       * @return the border for the panel button
      */
      QRect buttonPanelBorder(const QRect &defValue = QRect()) const;
      /**
       * @return the border for the meter button
       */
      QRect buttonMeterBorder(const QRect &defValue = QRect()) const;
      /**
       * @return the large font that the theme specifies
       */
      QFont largeFont() const;
      /**
       * @return the normal font that the theme specifies
       */
      QFont normalFont() const;
      /**
       * @return the small font that the theme specifies
       */
      QFont smallFont() const;
      /**
       * @return the current font to be used
       * according to fontItem()
       */
      QFont currentFont() const;
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
      QString meterPixmap(int type, bool useDefault = true) const;
      /**
       * @return the meter image (bg_panel.[png|jpg|gif])
       * of the current theme, it will first look in the dir 'type'
       * if no image is found it will drop back a dir
       * @param type is one of: net, inet
       */
      QString panelPixmap(int type, bool useDefault = true) const;
      /**
       * @return the decal net leds image
       * (usually decal_net_leds.[png|jpg|gif]) of the
       * current theme, it will first look in the dir 'type'
       * if no image is found it will drop back a dir
       * @param type is one of: net, inet
       */
      QString ledPixmap(int type, bool useDefault = true) const;
      /**
       * @return the frame image of the current theme,
       * @param type is one of: top, bottom, left, right
       */
      QString framePixmap(int type, bool useDefault = true) const;
      /**
       * @return the krell chart image (bg_chart.[png|jpg|gif])
       * of the current theme
       */
      QString chartPixmap(bool useDefault = true) const;
      /**
       * @return the krell grid image (bg_grid.[png|jpg|gif])
       * of the current theme
       */
      QString gridPixmap(bool useDefault = true) const;
      /**
       * @return the krell panel image (krell_panel.[png|jpg|gif])
       * of the current theme
       */
      QString krellPanelPixmap(bool useDefault = true) const;
      /**
       * @return the krell meter image (krell_meter.[png|jpg|gif])
       * of the current theme
       */
      QString krellMeterPixmap(bool useDefault = true) const;
      /**
       * @return the krell slider image (krell_slider.[png|jpg|gif])
       * of the current theme
       */
      QString krellSliderPixmap(bool useDefault = true) const;
      /**
       * @return the data in image (data_in.[png|jpg|gif])
       * of the current theme
       */
      QString dataInPixmap(bool useDefault = true) const;
      /**
       * @return the data out image (data_out.[png|jpg|gif])
       * of the current theme
       */
      QString dataOutPixmap(bool useDefault = true) const;
      /**
       * @return the mail image (mail/decal_mail.[png|jpg|gif])
       * of the current theme
       */
      QString mailPixmap(bool useDefault = true) const;
      /**
       * @return a segmant of an image, using @p itemNo to
       * get the segmant and from the ImageType @p type
       */
      QPixmap splitPixmap(PixmapType type, uint itemNo = 0,
         bool useDefault = true) const;
      /**
       * Same as the above function but returns an array of pixmaps
       * with the maximum size of limitAmount, or all the pixmaps if @p
       * limitAmount is -1 (default)
       * @return an array of pixmaps
       */
      QValueList<QPixmap> pixmapToList(PixmapType type,
         int limitAmount = -1, bool useDefault = true) const;
      /**
       * returns the transparency level from the specified keys
       */
      int transparency(const QString &, const QString &) const;
      /**
       * @return true if shadow text is enabled
       */
      bool textShadow(const QString &, const QString &) const;
      /**
       * @return the shadow color (if any) for the specified keys
       */
      QColor shadowColour(const QString &, const QString &) const;
      /**
       * @return the text color for the specified keys
       */
      QColor textColour(const QString &, const QString &) const;
      /**
       * reads an entry and returns it as an int
       */
      int readIntEntry(const QString &, const QString &) const;
      /**
       * @return a rect from the specified keys
       */
      QRect readRectEntry(const QString &, const QString &) const;
      /**
       * reads an entry from the specified keys
       */
      QString readEntry(const QString &, const QString &) const;
      /**
       * reads a color entry from the specified keys
       */
      QString readColourEntry(const QString &, const QString &, int) const;
      /**
       * reads a font entry from the specified keys
       */
      QFont readFontEntry(const QString &, const QString &) const;
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
       *   QFont font;
       *   QColor textColour, shadowColour;
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
      bool fontColours(int type, const QString &string, QFont &font,
         QColor &textColour, QColor &shadowColour, bool &showShadow) const;
      /**
       * convenience function.
       *
       * collects the theme type and config key from the base pointer
       */
      bool fontColours(const KSim::Base *const base, QFont &font,
         QColor &textColour, QColor &shadowColour, bool &showShadow) const;

      Theme(const KSim::Theme &);

    private:
      Theme();
      Theme(const QString &url, const QString &fileName, int alt,
         const QValueVector<QString> &vector, const QStringList &list,
         KConfig *globalReader);
      void create(const QValueVector<QString> &, const QStringList &, KConfig *);
      void init(const QString &url, const QString &fileName, int alt);
      void reparse(const QString &url, const QString &fileName, int alt);
      QString parseConfig(const QString &, const QString &);
      QString loader(int, bool useDefault = true) const;
      QString createType(int, const QString &) const;
      void setRecolour(bool);

      int internalNumEntry(const QString &, int) const;
      QRect internalRectEntry(const QString &, const QRect &) const;
      QColor internalColourEntry(const QString &, const QColor &) const;
      QString internalStringEntry(const QString &, const QString &) const;

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
      KSim::Theme theme(const QString &url,
         const QString &rcFile = "gkrellmrc", int alt = 0) const;
      /**
       * re-colours an image to the current KDE
       * color scheme
       */
      void reColourImage(QImage &image);
      /**
       * changes some of the entries in the config file so kconfig
       * can handle the file better.
       * @return the location of the config file
       */
      QString parseConfig(const QString &url, const QString &file);
      /**
       * change the dir structure of a theme directory
       * so KSim can understand them better
       */
      void parseDir(const QString &url, int alt);
      /**
       * checks if the themeUrl entry is valid,
       * if not it then reverts the current theme
       * to the default url
       */
      void validate();
      /**
       * sets the pallete of the current theme
       */
      void themeColours(QWidget *);
      /**
       * @return current theme name
       */
      static QString currentName();
      /**
       * @return current theme url, if there is no current theme
       * then it will return the default theme
       */
      static QString currentUrl();
      /**
       * @return the default theme url
       */
      static QString defaultUrl();
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
      static QString alternativeAsString(int alt = -1);
      /**
       * @return the font of the theme if the theme font is set to custom
       */
      static QFont currentFont();
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
