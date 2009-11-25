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

#include "monitor.h"

KMilo::Monitor::Monitor(QObject *parent, const char *name, const QStringList&)
: QObject(parent, name) {
	_poll = true;
}

KMilo::Monitor::~Monitor() {
}

void KMilo::Monitor::reconfigure(KConfig*)
{
}

KMilo::Monitor::DisplayType KMilo::Monitor::poll() { return None; }

int KMilo::Monitor::progress() const { return 42; }

QString KMilo::Monitor::message() const { return QString::null; }
QPixmap KMilo::Monitor::customPixmap() const { return QPixmap(); }
bool KMilo::Monitor::init() { return true; }

