// -*- Mode: C++; c-basic-offset: 2; indent-tabs-mode: t; tab-width: 2; -*-
/*
   This file is part of the KDE project

   Copyright (c) 2003 Zack Rusin <staikos@kde.org>
	 Pretty much ripped of from :
	 George Staikos <staikos@kde.org> :)

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
#ifndef _PBMONITOR_H_
#define _PBMONITOR_H_

#include "monitor.h"

struct tagitem;

namespace KMilo {

class PowerBookMonitor : public KMilo::Monitor
{
public:
	PowerBookMonitor(TQObject *parent, const char *name, const TQStringList&);
	virtual ~PowerBookMonitor();

	virtual bool init();
	virtual DisplayType poll();
	virtual int progress() const;
	virtual TQString message() const;

private:
	struct tagitem* readMessage();
private:
	int m_progress;
	TQString m_message;
	bool m_sleep;
};

}

#endif
