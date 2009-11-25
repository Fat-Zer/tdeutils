lib_LTLIBRARIES = libksimcore.la

libksimcore_la_SOURCES = common.cpp themeloader.cpp \
                                        chart.cpp label.cpp \
                                        led.cpp progress.cpp \
                                        pluginglobal.cpp pluginloader.cpp \
                                        pluginmodule.cpp ksimconfig.cpp

libksimcore_la_LDFLAGS = $(all_libraries) -version-info 1:0 -no-undefined
libksimcore_la_LIBADD = $(LIB_KDEUI)

ksiminclude_HEADERS = common.h themeloader.h \
                                        chart.h label.h \
                                        led.h progress.h \
                                        pluginglobal.h pluginloader.h \
                                        pluginmodule.h themetypes.h \
                                        ksimconfig.h


ksimincludedir = $(includedir)/ksim

INCLUDES= $(all_includes)

METASOURCES = AUTO

