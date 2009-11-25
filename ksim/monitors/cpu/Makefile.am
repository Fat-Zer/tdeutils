kde_module_LTLIBRARIES = ksim_cpu.la
ksim_cpu_la_SOURCES = ksimcpu.cpp

ksim_cpu_la_LIBADD  = ../../library/libksimcore.la
ksim_cpu_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)

EXTRA_DIST = cpu.desktop

INCLUDES= -I$(top_srcdir)/ksim/library \
                   $(all_includes)

METASOURCES = AUTO
KDE_ICON = AUTO

mon_DATA = cpu.desktop
mondir = $(kde_datadir)/ksim/monitors
