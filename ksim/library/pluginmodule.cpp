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
#include <qregexp.h>
#include <qpopupmenu.h>
#include <kconfig.h>


class KSim::PluginObject::Private
{
  public:
    QCString name;
    QString configName;
};

KSim::PluginObject::PluginObject(const QCString &name)
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

const QCString &KSim::PluginObject::name() const
{
  return d->name;
}

void KSim::PluginObject::setConfigFileName(const QString &name)
{
  if (d->configName == name)
    return;

  d->configName = name;
  if (name.find("ksim") == -1)
    d->configName.prepend("ksim_");
}

const QString &KSim::PluginObject::configFileName() const
{
  return d->configName;
}

class KSim::PluginPage::Private
{
  public:
    KConfig *config;
    KSim::PluginObject *parent;
};

KSim::PluginPage::PluginPage(KSim::PluginObject *parent, const char *name)
   : QWidget(0, name)
{
  d = new PluginPage::Private;
  d->parent = parent;
  if (parent && !parent->configFileName().isEmpty())
    d->config = new KConfig(parent->configFileName() + "rc");
  else {
    kdWarning() << className() << ": Can not create the config() "
       "pointer due to the parent being null" << endl;
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
    PluginObject *parent;
    QPopupMenu *popupMenu;
    KConfig *config;
};

KSim::PluginView::PluginView(KSim::PluginObject *parent, const char *name)
   : QWidget(0, name)
{
  d = new PluginView::Private;
  d->parent = parent;
  d->popupMenu = new QPopupMenu(this);
  d->popupMenu->insertItem(i18n("About"), this,
     SLOT(showAbout()), 0, -1, 0);

  if (parent && !parent->configFileName().isEmpty())
    d->config = new KConfig(parent->configFileName() + "rc");
  else {
    kdWarning() << className() << ": Can not create the config() "
       "pointer due to the parent being null" << endl;
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

QPopupMenu *KSim::PluginView::menu() const
{
  return d->popupMenu;
}

void KSim::PluginView::doCommand()
{
  emit runCommand(name());
}

void KSim::PluginView::mousePressEvent(QMouseEvent *ev)
{
  if (ev->button() == QMouseEvent::LeftButton)
    doCommand();
}

KSim::PluginObject *KSim::PluginView::parentPlugin() const
{
  return d->parent;
}

void KSim::PluginView::showAbout()
{
  parentPlugin()->showAbout();
}
