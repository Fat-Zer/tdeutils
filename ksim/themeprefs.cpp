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

#include <qdir.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qlabel.h>
#include <qregexp.h>

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

class ThemeViewItem : public KListViewItem
{
  public:
    ThemeViewItem(QListView *parent, const QString &text,
       const KURL &url) : KListViewItem(parent, text)
    {
      m_url = url;
    }

    const KURL &url() const { return m_url; }

  private:
    KURL m_url;
};

KSim::ThemePrefs::ThemePrefs(QWidget *parent, const char *name)
   : QWidget(parent, name)
{
  m_themeLayout = new QGridLayout(this);
  m_themeLayout->setSpacing(6);

  m_label = new QLabel(this);
  m_label->setText(i18n("GKrellm theme support. To use"
     " gkrellm themes just untar the themes into the folder below"));
  m_label->setAlignment(QLabel::WordBreak |
     QLabel::AlignVCenter | QLabel::AlignLeft);
  m_themeLayout->addMultiCellWidget(m_label, 0, 0, 0, 4);

  QString themeDir(locateLocal("data", "ksim"));
  themeDir += QString::fromLatin1("/themes");

  m_urlLabel = new KURLLabel(this);
  m_urlLabel->setText(i18n("Open Konqueror in KSim's theme folder"));
  m_urlLabel->setURL(QString::fromLatin1("file://") + themeDir);
  connect(m_urlLabel, SIGNAL(leftClickedURL(const QString &)),
     this, SLOT(openURL(const QString &)));
  m_themeLayout->addMultiCellWidget(m_urlLabel, 1, 1, 0, 4);

  m_line = new KSeparator(Horizontal, this);
  m_themeLayout->addMultiCellWidget(m_line, 2, 2, 0, 4);

  m_authorLabel = new QLabel(this);
  m_authorLabel->setSizePolicy(QSizePolicy(QSizePolicy::Fixed,
     QSizePolicy::Minimum));
  m_authorLabel->setText(i18n("Author:"));
  m_themeLayout->addMultiCellWidget(m_authorLabel, 3, 3, 0, 0);

  m_authLabel = new KSqueezedTextLabel(this);
  m_authLabel->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
     QSizePolicy::Minimum));
  m_authLabel->setText(i18n("None"));
  m_themeLayout->addMultiCellWidget(m_authLabel, 3, 3, 1, 4);

  m_listView = new KListView(this);
  m_listView->addColumn(i18n("Theme"));
  m_listView->setFullWidth(true);
  connect(m_listView, SIGNAL(currentChanged(QListViewItem *)),
     this, SLOT(selectItem(QListViewItem *)));
  m_themeLayout->addMultiCellWidget(m_listView, 4, 4, 0, 4);

  m_alternateLabel = new QLabel(this);
  m_alternateLabel->setText(i18n("Alternate themes:"));
  m_alternateLabel->setAlignment(AlignVCenter | AlignRight);
  m_themeLayout->addMultiCellWidget(m_alternateLabel, 5, 5, 0, 1);

  m_altTheme = new KIntSpinBox(this);
  m_themeLayout->addMultiCellWidget(m_altTheme, 5, 5, 2, 2);

  m_fontLabel = new QLabel(this);
  m_fontLabel->setText(i18n("Font:"));
  m_fontLabel->setAlignment(AlignVCenter | AlignRight);
  m_fontLabel->setSizePolicy(QSizePolicy(QSizePolicy::Minimum,
     QSizePolicy::Fixed));
  m_themeLayout->addMultiCellWidget(m_fontLabel, 5, 5, 3, 3);

  m_fontsCombo = new KComboBox(this);
  m_fontsCombo->insertItem(i18n("Small"));
  m_fontsCombo->insertItem(i18n("Normal"));
  m_fontsCombo->insertItem(i18n("Large"));
  m_fontsCombo->insertItem(i18n("Custom"));
  m_fontsCombo->insertItem(i18n("Default"));
  m_fontsCombo->setSizePolicy(QSizePolicy(QSizePolicy::Expanding,
     QSizePolicy::Fixed));
  connect(m_fontsCombo, SIGNAL(activated(int)),
     this, SLOT(showFontDialog(int)));
  m_themeLayout->addMultiCellWidget(m_fontsCombo, 5, 5, 4, 4);

  QStringList locatedFiles = KGlobal::dirs()->findDirs("data", "ksim/themes");
  for (QStringList::ConstIterator it = locatedFiles.begin(); it != locatedFiles.end(); ++it)
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

void KSim::ThemePrefs::openURL(const QString &url)
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

  for (QListViewItemIterator it(m_listView); it.current(); it++)
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

void KSim::ThemePrefs::selectItem(QListViewItem *item)
{
  if (!item)
    return;

  ThemeViewItem *themeItem = static_cast<ThemeViewItem *>(item);
  ThemeInfoList::ConstIterator it = qFind(m_themeList.begin(),
     m_themeList.end(), ThemeInfo(themeItem->text(0), themeItem->url()));

  if (it == m_themeList.end())
    return;

  m_currentTheme = (*it);

  const KSim::Theme &theme(KSim::ThemeLoader::self().theme(m_currentTheme.url.path()));
  if (theme.name() != "ksim")
    KSim::ThemeLoader::self().parseDir(theme.path(), theme.alternatives());

  QToolTip::remove(m_authLabel);
  if (theme.author().isEmpty()) {
    m_authLabel->setText(i18n("None Specified"));
    QToolTip::add(m_authLabel, i18n("None specified"));
  }
  else {
    m_authLabel->setText(theme.author());
    QToolTip::add(m_authLabel, theme.author());
  }

  kdDebug( 2003 ) << "theme.alternatives() = " << theme.alternatives() << endl;
  setThemeAlts(theme.alternatives());
}

void KSim::ThemePrefs::readThemes(const QString &location)
{
  kdDebug(2003) << "readThemes(" << location << ")" << endl;
  ThemeInfoList themeList;
  QStringList items(QDir(location).entryList(QDir::Dirs, QDir::IgnoreCase));
  QStringList::ConstIterator it;
  for (it = items.begin(); it != items.end(); ++it) {
    if ((*it) != "." && (*it) != "..")
      themeList.append(ThemeInfo((*it), KURL(location + (*it) + "/")));
  }

  insertItems(themeList);
}

void KSim::ThemePrefs::showFontDialog(int currentItem)
{
  if (currentItem == 3) {
    QFont customFont = m_font;
    int result = KFontDialog::getFont(customFont);
    if (result == KFontDialog::Accepted)
      m_font = customFont;
  }
}
