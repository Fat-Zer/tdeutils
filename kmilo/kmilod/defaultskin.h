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

#ifndef _DEFAULTSKIN_H_
#define _DEFAULTSKIN_H_

#include <tqstring.h>
#include <tqobject.h>
#include <tqtimer.h>
#include "displayskin.h"

class DefaultWidget;

class DefaultSkin : public TQObject, public KMilo::DisplaySkin {
	Q_OBJECT
	public:
		DefaultSkin();
		virtual ~DefaultSkin();

		virtual void clear();
		virtual void reconfigure( KConfig *config );

		virtual void displayText(const TQString& text, const TQPixmap& customPixmap=TQPixmap());
		virtual void displayProgress(const TQString& text,
						int percent, const TQPixmap& customPixmap=TQPixmap());

	private slots:
		void timeout();

	private:
		void show();

		DefaultWidget *_widget;
		TQTimer _timer;
};


#endif
