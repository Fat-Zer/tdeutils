kde_module_LTLIBRARIES = kcm_kvaio.la

kcm_kvaio_la_SOURCES = kcmkvaio_general.ui main.cpp

kcm_kvaio_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kcm_kvaio_la_LIBADD = ../kvaiodriverinterface.lo $(LIB_KDEUI)

INCLUDES= $(all_includes)
 
kcm_kvaio_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmkvaio.pot

xdg_apps_DATA = kvaio.desktop
