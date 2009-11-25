kde_module_LTLIBRARIES = ksim_disk.la
ksim_disk_la_SOURCES = ksimdisk.cpp

ksim_disk_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
ksim_disk_la_LIBADD = $(LIB_DEVSTAT) ../../library/libksimcore.la

EXTRA_DIST = disk.desktop

INCLUDES= -I$(top_srcdir)/ksim/library \
                   $(all_includes)

METASOURCES = AUTO

mon_DATA = disk.desktop
mondir = $(kde_datadir)/ksim/monitors
