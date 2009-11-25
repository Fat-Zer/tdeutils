kde_module_LTLIBRARIES = ksim_filesystem.la
ksim_filesystem_la_SOURCES = ksimfsystem.cpp fsystemconfig.cpp \
                            fsystemiface.skel filesystemwidget.cpp filesystemstats.cpp

ksim_filesystem_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
ksim_filesystem_la_LIBADD = ../../library/libksimcore.la

EXTRA_DIST = DFree.desktop

INCLUDES= -I$(top_srcdir)/ksim/library \
                   $(all_includes)

METASOURCES = AUTO

mon_DATA = DFree.desktop
mondir = $(kde_datadir)/ksim/monitors
