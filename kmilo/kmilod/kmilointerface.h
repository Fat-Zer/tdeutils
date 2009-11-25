// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
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
#ifndef _KMILOI_H_
#define _KMILOI_H_

#include <qobject.h>
#include <qstring.h>
#include "kmilod.h"

namespace KMilo {

// FIXME: make put the methods into .cpp file when the header is installed.
class KMiloInterface : public QObject {
	Q_OBJECT
	public:
		KMiloInterface(KMiloD *p) : QObject() { _p = p; }
		virtual ~KMiloInterface() {}

	public slots:
		int pollMilliSeconds() const { return _p->pollMilliSeconds(); }
		bool setPollMilliSeconds(int ms) { return _p->setPollMilliSeconds(ms); }

		void displayText(const QString& text) { _p->displayText(text); }
		void displayText(const QString& text, const QPixmap& customPixmap) { _p->displayText(text, customPixmap); }

		void displayProgress(const QString& text, int progress, const QPixmap& customPixmap) { _p->displayProgress(text, progress, customPixmap); }
		void displayProgress(const QString& text, int progress) { _p->displayProgress(text, progress); }

		void reconfigure() { _p->reconfigure(); }

	private:
		KMiloD *_p;
};

}

#endif
