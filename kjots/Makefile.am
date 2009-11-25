INCLUDES = $(all_includes)
AM_LDFLAGS   = $(all_libraries) $(KDE_RPATH)
LDADD =  $(LIB_KFILE) $(LIB_KDEPRINT)

bin_PROGRAMS = kjots

kjots_SOURCES = main.cpp\
	KJotsMain.cpp\
	kjotsentry.cpp\
	kjotsedit.cpp\
	kjotsbookmarks.cpp\
	confpagefont.ui\
	confpagemisc.ui \
	KJotsSettings.kcfgc

kjots_METASOURCES = AUTO

SUBDIRS = icons 

xdg_apps_DATA = Kjots.desktop
kde_kcfg_DATA = kjots.kcfg

rcdir = $(kde_datadir)/kjots
rc_DATA = kjotsui.rc

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kjots.pot

