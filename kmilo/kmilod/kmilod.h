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
#ifndef _KMILOD_H_
#define _KMILOD_H_

#include <kdedmodule.h>
#include <qstring.h>
#include <qtimer.h>
#include <qptrlist.h>
#include "monitor.h"
#include "displayskin.h"

namespace KMilo {

class KMiloInterface;

class KMiloD : public KDEDModule {
	Q_OBJECT
	K_DCOP
	public:
		KMiloD(const QCString &name);
		virtual ~KMiloD();

	k_dcop:
		virtual void setEnabled(bool enabled);
		virtual void enable();
		virtual void disable();
		virtual bool enabled() const;
		virtual int pollMilliSeconds() const;
		virtual bool setPollMilliSeconds(int ms);

		virtual void displayText(const QString& text);
		virtual void displayText(const QString& text, const QPixmap& customPixmap);
		virtual void displayProgress(const QString& text, int progress, const QPixmap& customPixmap);
		virtual void displayProgress(const QString& text, int progress);

		virtual void reconfigure();

	private slots:
		virtual void doTimer();

	private:
		QTimer _timer;
		int _interval;
		QPtrList<Monitor> _monitors;
		DisplaySkin *_display;
		KMiloInterface *_miface;
};

}

#endif
