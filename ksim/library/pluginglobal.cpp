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

#include "pluginglobal.h"
#include "pluginmodule.h"

#include <kdebug.h>
#include <kdesktopfile.h>
#include <klocale.h>
#include <kiconloader.h>

class KSim::Plugin::Private
{
  public:
    Private() : plugin(0),
       view(0), page(0)
    {
      oldState = true;
      enabled = true;
      // Set the ref-count to 1
      count = 1;
    }

    ~Private()
    {
      kdDebug(2003) << "Deleting " << (plugin ?
         plugin->name() : QString("Null"))
         << " objects." << endl;

      // Remember to delete the objects we own
      delete plugin;
      delete view;
      delete page;
      
      plugin = 0;
      view = 0;
      page = 0;
    }

    void ref() { ++count; }
    bool deref() { return !--count; }

    uint count;
    QString name;
    QPixmap icon;
    QCString libName;
    QString filename;
    KSim::PluginObject *plugin;
    KSim::PluginView *view;
    KSim::PluginPage *page;
    bool oldState;
    bool enabled;
};

KSim::Plugin KSim::Plugin::null;

// Null Plugin
KSim::Plugin::Plugin() : d(0)
{
}

KSim::Plugin::Plugin(KSim::PluginObject *plugin, const KDesktopFile &file)
{
  init(plugin, file);

  if (d) {
    d->view = d->plugin ? d->plugin->createView(d->libName) : 0;
    d->page = d->plugin ? d->plugin->createConfigPage(d->libName) : 0;
  }
}

KSim::Plugin::Plugin(const KSim::Plugin &rhs)
{
  d = rhs.d;
  if (d)
    d->ref();
}

KSim::Plugin::~Plugin()
{
  if (d && d->deref())
    delete d;
}

KSim::Plugin &KSim::Plugin::operator=(const KSim::Plugin &rhs)
{
  if (*this == rhs)
    return *this;

  if (rhs.d) {
    rhs.d->ref();
    if (d && d->deref())
      delete d;

    d = rhs.d;
    return *this;
  }

  // rhs is a null plugin so we just go ahead
  // and make this a null plugin too.
  if (d && d->deref())
    delete d;

  d = 0;
  return *this;
}

bool KSim::Plugin::operator==(const KSim::Plugin &rhs) const
{
  return d == rhs.d;
}

bool KSim::Plugin::operator!=(const KSim::Plugin &rhs) const
{
  return !(operator==(rhs));
}

void KSim::Plugin::setEnabled(bool enabled)
{
  if (!d)
    return;

  d->oldState = d->enabled;
  d->enabled = enabled;
}

bool KSim::Plugin::isEnabled() const
{
  return (d ? d->enabled : false);
}

bool KSim::Plugin::isDifferent() const
{
  return d ? d->enabled != d->oldState : false;
}

bool KSim::Plugin::isNull() const
{
  return !d;
}

const QString &KSim::Plugin::name() const
{
  return d ? d->name : QString::null;
}

QPixmap KSim::Plugin::icon() const
{
  return d ? d->icon : QPixmap();
}

QCString KSim::Plugin::libName() const
{
  return d ? d->libName : QCString();
}

const QString &KSim::Plugin::fileName() const
{
  return d ? d->filename : QString::null;
}

KSim::PluginObject *KSim::Plugin::plugin() const
{
  return d ? d->plugin : 0;
}

KSim::PluginView *KSim::Plugin::view() const
{
  return d ? d->view : 0;
}

KSim::PluginPage *KSim::Plugin::configPage() const
{
  return d ? d->page : 0;
}

void KSim::Plugin::init(KSim::PluginObject *plugin, const KDesktopFile &file)
{
  if (!plugin || file.fileName().isEmpty()) {
    d = 0;
    return;
  }

  d = new Private;

  d->libName = "ksim_" + file.readEntry("X-KSIM-LIBRARY").local8Bit();
  d->name = file.readName();
  d->icon = SmallIcon(file.readIcon());
  d->filename = file.fileName();
  d->plugin = plugin;
}
