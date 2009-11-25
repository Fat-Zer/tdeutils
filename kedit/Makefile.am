#
#                           KEdit Makefile.am
#
#  Copyright 1997 Bernd Johannes Wuebben, wuebben@math.cornell.edu
#
#

INCLUDES = $(all_includes)

bin_PROGRAMS =
lib_LTLIBRARIES =
kdeinit_LTLIBRARIES = kedit.la

kedit_la_SOURCES = kedit.cpp ktextfiledlg.cpp misc.ui color.ui prefs.kcfgc
kedit_la_LIBADD = -lkdeprint $(LIB_KFILE)
kedit_la_LDFLAGS = -module -avoid-version $(KDE_PLUGIN) $(all_libraries)

noinst_HEADERS = kedit.h ktextfiledlg.h\
		version.h misc.ui.h

METASOURCES = AUTO

SUBDIRS = pixmaps

xdg_apps_DATA = KEdit.desktop
kde_kcfg_DATA = kedit.kcfg

EXTRA_DIST = $(xdg_apps_DATA)

rcdir = $(kde_datadir)/kedit
rc_DATA = keditui.rc

messages: rc.cpp
	$(XGETTEXT) rc.cpp *.cpp -o $(podir)/kedit.pot

