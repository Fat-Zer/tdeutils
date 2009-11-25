noinst_LTLIBRARIES = libsysteminfo.la

libsysteminfo_la_SOURCES = systeminfo.cpp
libsysteminfo_la_LDFLAGS = -module -avoid-version

kde_module_LTLIBRARIES = ksim_panelextension.la
ksim_panelextension_la_SOURCES = ksim.cpp ksimview.cpp ksimview.skel \
                             ksimsysinfo.cpp ksimsysinfo.skel ksimframe.cpp \
                             ksimpref.cpp monitorprefs.cpp generalprefs.cpp \
                             themeprefs.cpp

ksim_panelextension_la_LDFLAGS = -module $(KDE_PLUGIN) $(all_libraries)
ksim_panelextension_la_LIBADD = ./library/libksimcore.la libsysteminfo.la $(LIB_KIO) $(LIB_KDEUI)

SUBDIRS = themes library monitors pics
EXTRA_DIST = ksim.desktop
METASOURCES = AUTO
CLEANFILES = dummy.cpp

extension_DATA = ksim.desktop
extensiondir = $(kde_datadir)/kicker/extensions

dummy.cpp:
	echo > dummy.cpp

INCLUDES= -I$(top_srcdir)/ksim/library \
	$(all_includes)

conf_DATA = ksim_panelextensionrc
confdir = $(kde_confdir)

messages: rc.cpp
	$(EXTRACTRC) */*.rc */*.ui *.ui *.rc >> rc.cpp
	$(XGETTEXT) `find . -name "*.cpp"` -o $(podir)/ksim.pot
