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

#include "ksimview.h"
#include "ksimview.moc"

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "ksimsysinfo.h"
#include "ksim.h"
#include "ksimframe.h"
#include <label.h>
#include <common.h>
#include <themeloader.h>
#include <pluginmodule.h>
#include <pluginloader.h>
#include <ksimconfig.h>
#include "ksimpref.h"
#include "baselist.h"
#include <themetypes.h>

#include <kapplication.h>
#include <klocale.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kmessagebox.h>
#include <kdesktopfile.h>
#include <kwin.h>
#include <krun.h>

#include <qfile.h>
#include <qbitmap.h>
#include <qtimer.h>
#include <qlayout.h>
#include <qobjectlist.h>
#include <qpainter.h>
#include <qcursor.h>
#include <qpopupmenu.h>
#include <qvbox.h>

#include <unistd.h>
#include <sys/param.h>

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN 64
#endif

KSim::MainView::MainView(KConfig *config,
   bool loadPlugins, KSim::PanelExtension *topLevel,
   const char *name) : DCOPObject("KSim"),
   QWidget(topLevel, name)
{
  // create the local "themes" and "monitors" dirs
  makeDirs();

  setBackgroundMode(PaletteBackground);

  m_topLevel = topLevel;
  m_oldLocation = 1;
  m_prefDialog = 0L;

  m_config = new KSim::Config(config);

  // Make sure the current theme is valid
  // if not then revert to the default theme
  KSim::ThemeLoader::self().validate();

  // parse the current theme in case it doesn't have the correct dir structure
  if (KSim::ThemeLoader::currentName() != "ksim")
    KSim::ThemeLoader::self().parseDir(KSim::ThemeLoader::currentUrl(),
       KSim::ThemeLoader::currentAlternative());

  kdDebug(2003) << "loadPlugins: " << loadPlugins << endl;

  m_subLayout = new QVBoxLayout(this);

  m_topFrame = new KSim::Frame(KSim::Types::TopFrame, this);
  m_subLayout->addWidget(m_topFrame);

  m_sizeLayout = new QHBoxLayout;
  m_subLayout->addLayout(m_sizeLayout);

  m_leftFrame = new KSim::Frame(KSim::Types::LeftFrame, this);
  m_sizeLayout->addWidget(m_leftFrame);

  m_pluginLayout = new QBoxLayout(QBoxLayout::TopToBottom);
  m_sizeLayout->addLayout(m_pluginLayout);

  QVBoxLayout *vb = new QVBoxLayout;
  QSpacerItem *item = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
  vb->addItem(item);
  
  m_hostLabel = new KSim::Label(KSim::Types::Host, this);
  m_hostLabel->installEventFilter(this);
  vb->addWidget(m_hostLabel);
  
  m_pluginLayout->addLayout(vb);

  char hostName[MAXHOSTNAMELEN];
  if (gethostname(hostName, sizeof(hostName)))
    m_hostLabel->setText(i18n("Unknown"));
  else {
    QCString host(hostName);
    int dotLocation = host.find(".");
    if (!m_config->displayFqdn() && dotLocation != -1)
      host.truncate(dotLocation);

    m_hostLabel->setText(host);
  }

  vb = new QVBoxLayout;
  item = new QSpacerItem(0, 0, QSizePolicy::Expanding, QSizePolicy::Expanding);
  vb->addItem(item);
  m_sysinfo = new KSim::Sysinfo(m_config, this);
  vb->addWidget(m_sysinfo);
  m_pluginLayout->addLayout(vb);

  m_bottomFrame = new KSim::Frame(KSim::Types::BottomFrame, this);
  m_subLayout->addWidget(m_bottomFrame);

  m_rightFrame = new KSim::Frame(KSim::Types::RightFrame, this);
  m_sizeLayout->addWidget(m_rightFrame);

  connect(&KSim::PluginLoader::self(),
     SIGNAL(pluginLoaded(const KSim::Plugin &)),
     this, SLOT(addMonitor(const KSim::Plugin &)));

  KSim::ThemeLoader::self().themeColours(this);

  // load the plugins and create the plugin menu
  if (loadPlugins) {
    addPlugins();
  }
  
  connect(&m_maskTimer, SIGNAL(timeout()), SLOT(slotMaskMainView()));
}

KSim::MainView::~MainView()
{
  delete m_config;
}

void KSim::MainView::show()
{
  maskMainView();
  QWidget::show();
}

void KSim::MainView::cleanup()
{
  // delete our configuration dialog and remove
  // the plugin config pages from the configuration
  // dialog before we unload our plugins
  delete m_prefDialog;

  // Clean up the singleton instances
  KSim::PluginLoader::cleanup();
}

KSim::Config *KSim::MainView::config() const
{
  return m_config;
}

void KSim::MainView::makeDirs()
{
  QString homeDir = locateLocal("data", "ksim");
  QString themeDir = homeDir + QString::fromLatin1("/themes");
  QString monitorDir = homeDir + QString::fromLatin1("/monitors");

  // return true if the dirs already exist
  if (QFile::exists(themeDir) && QFile::exists(monitorDir))
    return;

  bool themeCreated = KStandardDirs::makeDir(themeDir);
  bool monitorCreated = KStandardDirs::makeDir(monitorDir);

  if (!themeCreated || !monitorCreated)
  {
    KMessageBox::sorry(0, i18n("There was an error while trying to create "
       "the local folders. This could be caused by permission problems."));
  }
}

const QString &KSim::MainView::hostname() const
{
  return m_hostLabel->text();
}

void KSim::MainView::slotMaskMainView()
{
  maskMainView();
}

void KSim::MainView::maskMainView()
{
  if (!m_topFrame->background()->mask() ||
      !m_leftFrame->background()->mask() ||
      !m_rightFrame->background()->mask() ||
      !m_bottomFrame->background()->mask())
  {
    topLevelWidget()->clearMask();
    return;
  }
  
  QBitmap topPixmap(*m_topFrame->background()->mask());
  QBitmap leftPixmap(*m_leftFrame->background()->mask());
  QBitmap rightPixmap(*m_rightFrame->background()->mask());
  QBitmap bottomPixmap(*m_bottomFrame->background()->mask());

  QSize insideSize(m_pluginLayout->geometry().size());

  // make a cleared bigrect where we can put our pixmap masks on
  QBitmap bigBitmap(topLevelWidget()->size(), true);

  // better return if our bitmap is null so we can avoid crashes
  if (bigBitmap.isNull())
    return;

  QPoint ofs = mapTo(topLevelWidget(), QPoint(0,0));
  int ofsX = ofs.x();
  int ofsY = ofs.y();

  QPainter painter;
  painter.begin(&bigBitmap);
  painter.setBrush(color1);
  painter.setPen(color1);
  QRect rect = m_pluginLayout->geometry();
  rect.moveBy(ofsX, ofsY);
  painter.drawRect(rect);
  painter.drawPixmap(ofsX, ofsY, topPixmap);
  painter.drawPixmap(ofsX, topPixmap.height()+ofsY, leftPixmap);
  painter.drawPixmap(ofsX+insideSize.width() + leftPixmap.width(),
     ofsY+ topPixmap.height(), rightPixmap);
  painter.drawPixmap(ofsX, ofsY + height() - bottomPixmap.height(),
     bottomPixmap);

  painter.end();
  topLevelWidget()->setMask(bigBitmap);
}

void KSim::MainView::reparseConfig(bool emitReload,
   const KSim::ChangedPluginList &list)
{
  kdDebug(2003) << "creating a new view" << endl;

  // Update the theme information for the theme classes
  bool themeChanged = KSim::ThemeLoader::self().isDifferent();
  if (themeChanged)
    KSim::ThemeLoader::self().reload();

  const KSim::PluginList &pluginList = KSim::PluginLoader::self().pluginList();
  KSim::PluginList::ConstIterator plugin;
  for (plugin = pluginList.begin(); plugin != pluginList.end(); ++plugin) {
    if ((*plugin).configPage()) {
      (*plugin).configPage()->saveConfig();
      KConfig *conf = (*plugin).configPage()->config();
      if (conf) conf->sync();
    }

    if (!(*plugin).isDifferent() && (*plugin).isEnabled()) {
      kdDebug(2003) << "Recreating " << (*plugin).name() << "'s view" << endl;
      kapp->processEvents();
      // Make sure the plugins background color is
      // set to the correct theme color
      if (themeChanged && (*plugin).view())
        KSim::ThemeLoader::self().themeColours((*plugin).view());

      // Reload the config file and and recreate the plugins view
      if ((*plugin).configPage() && (*plugin).configPage()->config())
        (*plugin).configPage()->config()->reparseConfiguration();

      if ((*plugin).view()) {
        (*plugin).view()->config()->reparseConfiguration();
        (*plugin).view()->reparseConfig();
      }
    }
  }

  ChangedPluginList::ConstIterator it;
  for (it = list.begin(); it != list.end(); ++it) {
    if ((*it).isDifferent()) {
      if ((*it).isEnabled()) { // Go through the added/removed plugins and load/unload them
        const KDesktopFile kdf((*it).filename());
        addPlugin(kdf);
        m_prefDialog->createPage((*it).libName());
      }
      else {
        m_prefDialog->removePage((*it).libName());
        const KDesktopFile kdf((*it).filename());
        removePlugin(kdf);
      }
    }
  }

  // reparse various non-plugin specific code
  // recreate the plugin menu and the dock icon
  KSim::BaseList::configureObjects(themeChanged);
  if (themeChanged)
    KSim::ThemeLoader::self().themeColours(this);

  m_sysinfo->createView();
  maskMainView();

  m_topLevel->reparse();

  if (emitReload)
    emit reload();

  kdDebug(2003) << "new view created" << endl;
}

void KSim::MainView::addPlugins()
{
  QStringList locatedFiles = KGlobal::dirs()->findAllResources("data", "ksim/monitors/*.desktop");
  QStringList::ConstIterator it;
  for (it = locatedFiles.begin(); it != locatedFiles.end(); ++it)
  {
    const KDesktopFile kdf(*it, true);
    addPlugin(kdf);
  }
}

void KSim::MainView::addPlugin(const KDesktopFile &file, bool force)
{
  if (force || m_config->enabledMonitor(file.readEntry("X-KSIM-LIBRARY")))
    KSim::PluginLoader::self().loadPlugin(file);
}

// provided for convenience
void KSim::MainView::removePlugin(const KDesktopFile &file)
{
  KSim::PluginLoader::self().unloadPlugin(file.readEntry("X-KSIM-LIBRARY").local8Bit());
}

void KSim::MainView::addMonitor(const KSim::Plugin &plugin)
{
  if (!plugin.view())
    return;

  plugin.view()->reparent(this, 0, QPoint(0, 0), true);
  KSim::ThemeLoader::self().themeColours(plugin.view());

//  int location = m_config->monitorLocation(plugin.libName());
//  if (location == -1)
//    location = m_oldLocation + 1;
//  else {
//    location += 1;
//    if (location > m_oldLocation)
//      location = m_oldLocation + 1;
//  }

//  kdDebug(2003) << "m_oldLocation: " << m_oldLocation << endl;
//  kdDebug(2003) << "location: " << location << endl;
  m_pluginLayout->addWidget(plugin.view());
  connect(plugin.view(), SIGNAL(runCommand(const QCString &)),
     SLOT(runCommand(const QCString &)));

//  if (location > m_oldLocation)
//    m_oldLocation = location;
}

void KSim::MainView::runCommand(const QCString &name)
{
  if (name.isNull())
    return;

  kdDebug(2003) << "runCommand(" << name.mid(5) << ")" << endl;
  QString exec = m_config->monitorCommand(name.mid(5));
  kdDebug(2003) << "exec is " << exec << endl;
  KRun::runCommand(exec);
}

void KSim::MainView::preferences()
{
  if (m_prefDialog == 0L) {
    m_prefDialog = new KSim::ConfigDialog(m_config, this, "m_prefDialog");
    connect(m_prefDialog, SIGNAL(reparse(bool, const KSim::ChangedPluginList &)),
       this, SLOT(reparseConfig(bool, const KSim::ChangedPluginList &)));
  }

  m_prefDialog->exec();
  destroyPref();
}

void KSim::MainView::resizeEvent(QResizeEvent *re)
{
  QWidget::resizeEvent(re);
  m_maskTimer.start(0, true);
}

void KSim::MainView::paletteChange(const QPalette &)
{
  // Call true here to fool the KSim::Base pointers to
  // think our theme changed, we can afford todo this
  // since the palette doesn't change often.
  KSim::BaseList::configureObjects(true);

  // Probably find a better way to do this
  KSim::PluginList::ConstIterator it;
  const KSim::PluginList &pluginList = KSim::PluginLoader::self().pluginList();
  for (it = pluginList.begin(); it != pluginList.end(); ++it)
    KSim::ThemeLoader::self().themeColours((*it).view());
}

void KSim::MainView::destroyPref()
{
  if (m_prefDialog != 0L) {
    kdDebug(2003) << "deleting KSimPref" << endl;
    delete m_prefDialog;
    m_prefDialog = 0L;
  }
}

QSize KSim::MainView::sizeHint(KPanelExtension::Position p, QSize) const
{
  int width = 0;
  int height = 0;
  
  QLayoutItem *child;
  for( QLayoutIterator it = m_pluginLayout->iterator();
       (child = it.current()) != 0; ++it)
  {
     QSize sz = child->minimumSize();
     if ((p == KPanelExtension::Right) || (p == KPanelExtension::Left))
     {
        width = QMAX(width, sz.width());
        height += sz.height();
     }
     else
     {
        width += sz.width();
        height = QMAX(height, sz.height());
     }
  }

  width += m_leftFrame->minimumWidth() + m_rightFrame->minimumWidth();
  height += m_topFrame->minimumHeight() + m_bottomFrame->minimumHeight();

  return QSize(width, height);
}

void KSim::MainView::positionChange(KPanelExtension::Orientation o)
{
  if (o == KPanelExtension::Vertical)
     m_pluginLayout->setDirection(QBoxLayout::TopToBottom);
  else
     m_pluginLayout->setDirection(QBoxLayout::LeftToRight);
}
