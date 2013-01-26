INCLUDES= $(all_includes)

SUBDIRS=. icons konfigurator

bin_PROGRAMS = tdewalletmanager
tdewalletmanager_SOURCES = walletwidget.ui tdewalletmanager.cpp \
			tdewalletmanager.skel main.cpp tdewalletpopup.cpp \
			tdewalleteditor.cpp kwmapeditor.cpp allyourbase.cpp \
			kbetterthankdialogbase.ui
tdewalletmanager_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor
tdewalletmanager_LDADD   = $(LIB_KIO) -ltdewalletclient
METASOURCES = AUTO
noinst_HEADERS = tdewalletmanager.h tdewalletpopup.h walletwidget.h tdewalleteditor.h\
		allyourbase.h kwmapeditor.h

xdg_apps_DATA = tdewalletmanager.desktop tdewalletmanager-tdewalletd.desktop

KDE_ICON = tdewalletmanager

rc_DATA = tdewalletmanager.rc tdewalleteditor.rc
rcdir = $(kde_datadir)/tdewalletmanager

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/tdewalletmanager.pot

kbetterthankdialogbase.lo: kbetterthankdialogbase.ui kbetterthankdialogbase.ui.h
