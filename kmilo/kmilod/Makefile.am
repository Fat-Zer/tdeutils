#    This file is part of the KDE project
#    Copyright (C) 2003 George Staikos <staikos@kde.org>

#    This library is free software; you can redistribute it and/or
#    modify it under the terms of the GNU Library General Public
#    License as published by the Free Software Foundation; either
#    version 2 of the License, or (at your option) any later version.

#    This library is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#    Library General Public License for more details.

#    You should have received a copy of the GNU Library General Public License
#    along with this library; see the file COPYING.LIB.  If not, write to
#    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
#    Boston, MA 02110-1301, USA.

INCLUDES=$(all_includes)

kde_module_LTLIBRARIES = kded_kmilod.la
kded_kmilod_la_LDFLAGS = -module -avoid-version $(all_libraries)
kded_kmilod_la_LIBADD = $(LIB_KIO) libkmilo.la
kded_kmilod_la_SOURCES = kmilod.cpp kmilod.skel defaultskin.cpp \
			defaultwidget.ui kmilointerface.cpp

lib_LTLIBRARIES = libkmilo.la
libkmilo_la_SOURCES = monitor.cpp displayskin.cpp
libkmilo_la_LDFLAGS = -version-info 1:0:0 -no-undefined $(all_libraries)
libkmilo_la_LIBADD = $(LIB_QT)

METASOURCES = AUTO

noinst_HEADERS = kmilod.h monitor.h displayskin.h defaultskin.h

services_DATA = kmilod.desktop
servicesdir = $(kde_servicesdir)/kded

servicetypes_DATA = kmilopluginsvc.desktop
servicetypesdir = $(kde_servicetypesdir)/kmilo

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kmilod.pot
