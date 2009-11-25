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

#ifndef _DISPLAYSKIN_H_
#define _DISPLAYSKIN_H_

#include <kdemacros.h>

#include <qstring.h>
#include <qpixmap.h>

namespace KMilo {

class KDE_EXPORT DisplaySkin {
	public:
		DisplaySkin();
		virtual ~DisplaySkin();

		virtual void clear() = 0;

		virtual void displayText(const QString& text,const QPixmap& customPixmap=QPixmap()) = 0;
		virtual void displayProgress(const QString& text,
						int percent, const QPixmap& customPixmap=QPixmap()) = 0;
};

}

#endif
