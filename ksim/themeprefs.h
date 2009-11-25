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

#ifndef THEMEIMPL_H
#define THEMEIMPL_H

#include <kurl.h>
#include <qwidget.h>

class QListViewItem;
class QGridLayout;
class QLabel;
class KListView;
class KURLLabel;
class KComboBox;
class KSqueezedTextLabel;
class KIntSpinBox;
class KSeparator;

namespace KSim
{
  class Config;

  class ThemeInfo
  {
    public:
      ThemeInfo() : name(0), url(0), alternatives(0) {}
      ThemeInfo(const QString &name, const KURL &url, int alts=0)
         : name(name), url(url), alternatives(alts) {}

      bool operator==(const ThemeInfo &rhs) const
      {
        return (rhs.name == name && rhs.url == url
           && rhs.alternatives == alternatives);
      }

      bool operator!=(const ThemeInfo &rhs) const
      {
        return !(operator==(rhs));
      }

      ThemeInfo &operator=(const ThemeInfo &rhs)
      {
        if (*this == rhs)
          return *this;

        name = rhs.name;
        url = rhs.url;
        alternatives = rhs.alternatives;
        return *this;
      }

      QString name;
      KURL url;
      int alternatives;
  };

  typedef QValueList<ThemeInfo> ThemeInfoList;

  class ThemePrefs : public QWidget
  {
    Q_OBJECT
    public:
      ThemePrefs(QWidget *parent, const char *name=0);
      ~ThemePrefs();

    public slots:
      void saveConfig(KSim::Config *);
      void readConfig(KSim::Config *);

    private slots:
      void setCurrentTheme(const ThemeInfo &);
      void setThemeAlts(int);
      void openURL(const QString &);
      void insertItems(const ThemeInfoList &);
      void clear();
      void completed();
      void selectItem(QListViewItem *item);
      void readThemes(const QString &);
      void showFontDialog(int);

    private:
      KListView *m_listView;
      KURLLabel *m_urlLabel;
      KIntSpinBox *m_altTheme;
      KComboBox *m_fontsCombo;
      QLabel *m_label;
      QLabel *m_authorLabel;
      KSqueezedTextLabel *m_authLabel;
      QLabel *m_alternateLabel;
      QLabel *m_fontLabel;
      KSeparator *m_line;
      ThemeInfo m_currentTheme;
      QFont m_font;
      ThemeInfoList m_themeList;
      QGridLayout *m_themeLayout;
  };
}
#endif
