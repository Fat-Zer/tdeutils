INCLUDES = $(all_includes) 

kde_module_LTLIBRARIES = kcharselect_panelapplet.la

kcharselect_panelapplet_la_SOURCES = charselectapplet.cpp

METASOURCES = AUTO
noinst_HEADERS = charselectapplet.h

lnkdir = $(kde_datadir)/kicker/applets
lnk_DATA = kcharselectapplet.desktop

EXTRA_DIST = $(lnk_DATA)

kcharselect_panelapplet_la_LDFLAGS = $(all_libraries) $(KDE_PLUGIN) -module
kcharselect_panelapplet_la_LIBADD = $(LIB_KDEUI)

messages:
	$(XGETTEXT) *.cpp *.h -o $(podir)/kcharselectapplet.pot
