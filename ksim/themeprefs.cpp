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

#include "themeprefs.h"
#include "themeprefs.moc"

#include <tqdir.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqlabel.h>
#include <tqregexp.h>

#include <klocale.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <kapplication.h>
#include <kfontdialog.h>
#include <kcombobox.h>
#include <klistview.h>
#include <ksqueezedtextlabel.h>
#include <knuminput.h>
#include <kseparator.h>

#include <ksimconfig.h>
#include <common.h>
#include <themeloader.h>

class ThemeViewItem : public TDEListViewItem
{
  public:
    ThemeViewItem(TQListView *parent, const TQString &text,
       const KURL &url) : TDEListViewItem(parent, text)
    {
      m_url = url;
    }

    const KURL &url() const { return m_url; }

  private:
    KURL m_url;
};

KSim::ThemePrefs::ThemePrefs(TQWidget *parent, const char *name)
   : TQWidget(parent, name)
{
  m_themeLayout = new TQGridLayout(this);
  m_themeLayout->setSpacing(6);

  m_label = new TQLabel(this);
  m_label->setText(i18n("GKrellm theme support. To use"
     " gkrellm themes just untar the themes into the folder below"));
  m_label->setAlignment(TQLabel::WordBreak |
     TQLabel::AlignVCenter | TQLabel::AlignLeft);
  m_themeLayout->addMultiCellWidget(m_label, 0, 0, 0, 4);

  TQString themeDir(locateLocal("data", "ksim"));
  themeDir += TQString::fromLatin1("/themes");

  m_urlLabel = new KURLLabel(this);
  m_urlLabel->setText(i18n("Open Konqueror in KSim's theme folder"));
  m_urlLabel->setURL(TQString::fromLatin1("file://") + themeDir);
  connect(m_urlLabel, TQT_SIGNAL(leftClickedURL(const TQString &)),
     this, TQT_SLOT(openURL(const TQString &)));
  m_themeLayout->addMultiCellWidget(m_urlLabel, 1, 1, 0, 4);

  m_line = new KSeparator(Qt::Horizontal, this);
  m_themeLayout->addMultiCellWidget(m_line, 2, 2, 0, 4);

  m_authorLabel = new TQLabel(this);
  m_authorLabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed,
     TQSizePolicy::Minimum));
  m_authorLabel->setText(i18n("Author:"));
  m_themeLayout->addMultiCellWidget(m_authorLabel, 3, 3, 0, 0);

  m_authLabel = new KSqueezedTextLabel(this);
  m_authLabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Expanding,
     TQSizePolicy::Minimum));
  m_authLabel->setText(i18n("None"));
  m_themeLayout->addMultiCellWidget(m_authLabel, 3, 3, 1, 4);

  m_listView = new TDEListView(this);
  m_listView->addColumn(i18n("Theme"));
  m_listView->setFullWidth(true);
  connect(m_listView, TQT_SIGNAL(currentChanged(TQListViewItem *)),
     this, TQT_SLOT(selectItem(TQListViewItem *)));
  m_themeLayout->addMultiCellWidget(m_listView, 4, 4, 0, 4);

  m_alternateLabel = new TQLabel(this);
  m_alternateLabel->setText(i18n("Alternate themes:"));
  m_alternateLabel->setAlignment(AlignVCenter | AlignRight);
  m_themeLayout->addMultiCellWidget(m_alternateLabel, 5, 5, 0, 1);

  m_altTheme = new KIntSpinBox(this);
  m_themeLayout->addMultiCellWidget(m_altTheme, 5, 5, 2, 2);

  m_fontLabel = new TQLabel(this);
  m_fontLabel->setText(i18n("Font:"));
  m_fontLabel->setAlignment(AlignVCenter | AlignRight);
  m_fontLabel->setSizePolicy(TQSizePolicy(TQSizePolicy::Minimum,
     TQSizePolicy::Fixed));
  m_themeLayout->addMultiCellWidget(m_fontLabel, 5, 5, 3, 3);

  m_fontsCombo = new KComboBox(this);
  m_fontsCombo->insertItem(i18n("Small"));
  m_fontsCombo->insertItem(i18n("Normal"));
  m_fontsCombo->insertItem(i18n("Large"));
  m_fontsCombo->insertItem(i18n("Custom"));
  m_fontsCombo->insertItem(i18n("Default"));
  m_fontsCombo->setSizePolicy(TQSizePolicy(TQSizePolicy::Expanding,
     TQSizePolicy::Fixed));
  connect(m_fontsCombo, TQT_SIGNAL(activated(int)),
     this, TQT_SLOT(showFontDialog(int)));
  m_themeLayout->addMultiCellWidget(m_fontsCombo, 5, 5, 4, 4);

  TQStringList locatedFiles = TDEGlobal::dirs()->findDirs("data", "ksim/themes");
  for (TQStringList::ConstIterator it = locatedFiles.begin(); it != locatedFiles.end(); ++it)
    readThemes(*it);
}

KSim::ThemePrefs::~ThemePrefs()
{
}

void KSim::ThemePrefs::saveConfig(KSim::Config *config)
{
  config->setThemeName(m_currentTheme.name);
  config->setThemeAlt(m_altTheme->value());
  config->setThemeFontItem(m_fontsCombo->currentItem());
  config->setThemeFont(m_font);
}

void KSim::ThemePrefs::readConfig(KSim::Config *config)
{
  setCurrentTheme(ThemeInfo(KSim::ThemeLoader::currentName(),
     KURL( KSim::ThemeLoader::currentUrl() ), KSim::ThemeLoader::self().current().alternatives()));
  m_altTheme->setValue(config->themeAlt());
  m_fontsCombo->setCurrentItem(config->themeFontItem());
  m_font = config->themeFont();
}

void KSim::ThemePrefs::setCurrentTheme(const ThemeInfo &theme)
{
  if (theme == m_currentTheme)
    return;

  m_currentTheme = theme;
  completed();
}

void KSim::ThemePrefs::setThemeAlts(int alternatives)
{
  m_currentTheme.alternatives = alternatives;
  m_altTheme->setMaxValue(alternatives);
  if (m_altTheme->value() > m_altTheme->maxValue())
    m_altTheme->setValue(m_altTheme->maxValue());

  m_altTheme->setEnabled(alternatives == 0 ? false : true);
  m_alternateLabel->setEnabled(alternatives == 0 ? false : true);
}

void KSim::ThemePrefs::openURL(const TQString &url)
{
  kapp->invokeBrowser(url);
}

void KSim::ThemePrefs::insertItems(const ThemeInfoList &itemList)
{
  ThemeInfoList::ConstIterator it;
  for (it = itemList.begin(); it != itemList.end(); ++it) {
    (void) new ThemeViewItem(m_listView, (*it).name, (*it).url);
    m_themeList.append((*it));
  }

  completed();
}

void KSim::ThemePrefs::clear()
{
  m_themeList.clear();
}

void KSim::ThemePrefs::completed()
{
  kdDebug(2003) << "Finished listing" << endl;

  for (TQListViewItemIterator it(m_listView); it.current(); it++)
  {
    if (it.current()->text(0) == m_currentTheme.name)
    {
      m_listView->setSelected(it.current(), true);
      m_listView->setCurrentItem(it.current());
      m_listView->ensureItemVisible(it.current());

      selectItem( it.current() );
      break;
    }
  }
}

void KSim::ThemePrefs::selectItem(TQListViewItem *item)
{
  if (!item)
    return;

  ThemeViewItem *themeItem = static_cast<ThemeViewItem *>(item);
  ThemeInfoList::ConstIterator it = tqFind(m_themeList.begin(),
     m_themeList.end(), ThemeInfo(themeItem->text(0), themeItem->url()));

  if (it == m_themeList.end())
    return;

  m_currentTheme = (*it);

  const KSim::Theme &theme(KSim::ThemeLoader::self().theme(m_currentTheme.url.path()));
  if (theme.name() != "ksim")
    KSim::ThemeLoader::self().parseDir(theme.path(), theme.alternatives());

  TQToolTip::remove(m_authLabel);
  if (theme.author().isEmpty()) {
    m_authLabel->setText(i18n("None Specified"));
    TQToolTip::add(m_authLabel, i18n("None specified"));
  }
  else {
    m_authLabel->setText(theme.author());
    TQToolTip::add(m_authLabel, theme.author());
  }

  kdDebug( 2003 ) << "theme.alternatives() = " << theme.alternatives() << endl;
  setThemeAlts(theme.alternatives());
}

void KSim::ThemePrefs::readThemes(const TQString &location)
{
  kdDebug(2003) << "readThemes(" << location << ")" << endl;
  ThemeInfoList themeList;
  TQStringList items(TQDir(location).entryList(TQDir::Dirs, TQDir::IgnoreCase));
  TQStringList::ConstIterator it;
  for (it = items.begin(); it != items.end(); ++it) {
    if ((*it) != "." && (*it) != "..")
      themeList.append(ThemeInfo((*it), KURL(location + (*it) + "/")));
  }

  insertItems(themeList);
}

void KSim::ThemePrefs::showFontDialog(int currentItem)
{
  if (currentItem == 3) {
    TQFont customFont = m_font;
    int result = TDEFontDialog::getFont(customFont);
    if (result == TDEFontDialog::Accepted)
      m_font = customFont;
  }
}
