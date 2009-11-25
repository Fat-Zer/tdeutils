kde_module_LTLIBRARIES = ksim_sensors.la

if include_nv
NVCTRL_LIB= NVCtrl.la
NVCtrl_la_SOURCES = NVCtrl.c
NVCtrl_la_LDFLAGS = -module $(all_libraries) $(KDE_PLUGIN)
endif

noinst_LTLIBRARIES = $(NVCTRL_LIB)

ksim_sensors_la_SOURCES = ksimsensors.cpp sensorsconfig.cpp \
                          sensorbase.cpp ksimsensorsiface.skel

ksim_sensors_la_LDFLAGS = $(all_libraries) -module -avoid-version 
ksim_sensors_la_LIBADD = ../../library/libksimcore.la $(NVCTRL_LIB)

EXTRA_DIST = Lm_sensors.desktop

INCLUDES = -I$(top_srcdir)/ksim/library \
                   -I$(top_srcdir)/ksim \
                   $(all_includes)

METASOURCES = AUTO

mon_DATA = Lm_sensors.desktop
mondir = $(kde_datadir)/ksim/monitors
