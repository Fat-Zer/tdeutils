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

#include "pluginmodule.h"
#include "pluginmodule.moc"

#include <klocale.h>
#include <kapplication.h>
#include <kaboutdata.h>
#include <kdebug.h>
#include <tqregexp.h>
#include <tqpopupmenu.h>
#include <kconfig.h>


class KSim::PluginObject::Private
{
  public:
    TQCString name;
    TQString configName;
};

KSim::PluginObject::PluginObject(const TQCString &name)
{
  d = new PluginObject::Private;
  d->name = name;
  d->configName = "ksim_" + name;
}

KSim::PluginObject::~PluginObject()
{
  delete d;
}

const char *KSim::PluginObject::instanceName() const
{
  return kapp->aboutData()->appName();
}

const TQCString &KSim::PluginObject::name() const
{
  return d->name;
}

void KSim::PluginObject::setConfigFileName(const TQString &name)
{
  if (d->configName == name)
    return;

  d->configName = name;
  if (name.tqfind("ksim") == -1)
    d->configName.prepend("ksim_");
}

const TQString &KSim::PluginObject::configFileName() const
{
  return d->configName;
}

class KSim::PluginPage::Private
{
  public:
    KConfig *config;
    KSim::PluginObject *tqparent;
};

KSim::PluginPage::PluginPage(KSim::PluginObject *tqparent, const char *name)
   : TQWidget(0, name)
{
  d = new PluginPage::Private;
  d->tqparent = tqparent;
  if (tqparent && !tqparent->configFileName().isEmpty())
    d->config = new KConfig(tqparent->configFileName() + "rc");
  else {
    kdWarning() << className() << ": Can not create the config() "
       "pointer due to the tqparent being null" << endl;
    d->config = 0;
  }
}

KSim::PluginPage::~PluginPage()
{
  delete d->config;
  delete d;
}

KConfig *KSim::PluginPage::config() const
{
  if (d)
    return d->config;
  else
    return 0;
}

class KSim::PluginView::Private
{
  public:
    PluginObject *tqparent;
    TQPopupMenu *popupMenu;
    KConfig *config;
};

KSim::PluginView::PluginView(KSim::PluginObject *tqparent, const char *name)
   : TQWidget(0, name)
{
  d = new PluginView::Private;
  d->tqparent = tqparent;
  d->popupMenu = new TQPopupMenu(this);
  d->popupMenu->insertItem(i18n("About"), this,
     TQT_SLOT(showAbout()), 0, -1, 0);

  if (tqparent && !tqparent->configFileName().isEmpty())
    d->config = new KConfig(tqparent->configFileName() + "rc");
  else {
    kdWarning() << className() << ": Can not create the config() "
       "pointer due to the tqparent being null" << endl;
    d->config = 0;
  }
}

KSim::PluginView::~PluginView()
{
  delete d->popupMenu;
  delete d->config;
  delete d;
  d = 0;
}

KConfig *KSim::PluginView::config() const
{
  return d->config;
}

TQPopupMenu *KSim::PluginView::menu() const
{
  return d->popupMenu;
}

void KSim::PluginView::doCommand()
{
  emit runCommand(name());
}

void KSim::PluginView::mousePressEvent(TQMouseEvent *ev)
{
  if (ev->button() == Qt::LeftButton)
    doCommand();
}

KSim::PluginObject *KSim::PluginView::parentPlugin() const
{
  return d->tqparent;
}

void KSim::PluginView::showAbout()
{
  parentPlugin()->showAbout();
}
