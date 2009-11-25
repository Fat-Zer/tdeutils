/*
 *  main.h
 *
 *  Copyright (C) 2004 Jonathan Riddell <jr@jriddell.org>
 *
 *  Based on kcm_kvaio
 *  Copyright (C) 2003 Mirko Boehm (mirko@kde.org)
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
 *
 */

#ifndef KCMTHINKPAD_MAIN_H
#define KCMTHINKPAD_MAIN_H

#include <kcmodule.h>

class KCMThinkPadGeneral;

class KCMThinkpadModule : public KCModule {
	Q_OBJECT

 public:
	KCMThinkpadModule(QWidget* parent, const char* name, const QStringList&);

	void save();
	void load();
	void load(bool useDefaults);
	void defaults();
 private:
	KCMThinkpadGeneral* m_KCMThinkpadGeneral;
	QString m_nvramFile;
	bool m_nvramReadable;
	bool m_nvramWriteable;
};

#endif
