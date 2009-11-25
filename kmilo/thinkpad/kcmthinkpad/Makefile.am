kde_module_LTLIBRARIES = kcm_thinkpad.la

kcm_thinkpad_la_SOURCES = kcmthinkpad_general.ui main.cpp

kcm_thinkpad_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
kcm_thinkpad_la_LIBADD = $(LIB_KIO)

INCLUDES= $(all_includes) -DKDE_BINDIR="\"$(bindir)\""
 
kcm_thinkpad_la_METASOURCES = AUTO

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmthinkpad.pot

xdg_apps_DATA = thinkpad.desktop
