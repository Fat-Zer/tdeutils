kde_module_LTLIBRARIES = ksim_net.la
ksim_net_la_SOURCES = ksimnet.cpp netdialog.cpp netconfig.cpp

ksim_net_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
ksim_net_la_LIBADD = ../../library/libksimcore.la $(LIB_KIO)

EXTRA_DIST = Net.desktop

INCLUDES= -I$(top_srcdir)/ksim/library $(all_includes)

METASOURCES = AUTO

mon_DATA = Net.desktop
mondir = $(kde_datadir)/ksim/monitors
