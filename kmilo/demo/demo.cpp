/*
   This file is part of the KDE project

   Copyright (c) 2003 George Staikos <staikos@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.

*/

#include <kgenericfactory.h>

#include "demo.h"

namespace KMilo {

Demo::Demo(QObject *parent, const char *name, const QStringList& args)
: Monitor(parent, name, args) {
}


Demo::~Demo() {
}


bool Demo::init() {
return true;
}


Monitor::DisplayType Demo::poll() {
Monitor::DisplayType rc = None;
return rc;
}


int Demo::progress() const {
	return 10;
}

};

K_EXPORT_COMPONENT_FACTORY(kmilo_demo, KGenericFactory<Demo>)

