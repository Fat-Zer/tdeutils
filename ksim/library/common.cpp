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

#include "common.h"
#include <kdebug.h>

#include <themeloader.h>
#include "themetypes.h"
#include "../baselist.h"

class KSim::Base::Private
{
  public:
    int type;
    bool theme;
    QString string;
};

KSim::Base::Base()
{
  KSim::BaseList::add(this);

  d = new Base::Private;
  d->type = KSim::Types::None;
  d->theme = true;
}

KSim::Base::Base(int type)
{
  KSim::BaseList::add(this);

  d = new Base::Private;
  d->type = type;
  d->theme = true;
}

KSim::Base::~Base()
{
  if (!KSim::BaseList::remove(this))
    kdError() << "While trying to remove " << this << " from the list" << endl;
  delete d;
}

bool KSim::Base::isThemeConfigOnly() const
{
  return d->theme;
}

void KSim::Base::setType(int type)
{
  if (d->type == type)
    return;

  d->type = type;
  extraTypeCall();
}

int KSim::Base::type() const
{
  return d->type;
}

void KSim::Base::setConfigString(const QString &string)
{
  d->string = string;
}

const QString &KSim::Base::configString() const
{
  return d->string;
}

void KSim::Base::extraTypeCall()
{
}

KSim::ThemeLoader &KSim::Base::themeLoader() const
{
  // Provided for convenience
  return KSim::ThemeLoader::self();
}

void KSim::Base::setThemeConfigOnly(bool value)
{
  d->theme = value;
}
