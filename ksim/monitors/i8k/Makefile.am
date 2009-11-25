kde_module_LTLIBRARIES = ksim_i8k.la
ksim_i8k_la_SOURCES = ksimi8k.cpp

ksim_i8k_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
ksim_i8k_la_LIBADD = ../../library/libksimcore.la

EXTRA_DIST = i8k.desktop

INCLUDES= -I$(top_srcdir)/ksim/library \
                   $(all_includes)

METASOURCES = AUTO

mon_DATA = i8k.desktop
mondir = $(kde_datadir)/ksim/monitors
