kde_module_LTLIBRARIES = kcm_laptop.la kded_klaptopdaemon.la 
bin_PROGRAMS = klaptop_acpi_helper klaptop_check
lib_LTLIBRARIES = libkcmlaptop.la
SUBDIRS = pics applnk

libkcmlaptop_la_SOURCES = portable.cpp smapidev.c daemon_state.cpp wake_laptop.cpp krichtextlabel.cpp
libkcmlaptop_la_LDFLAGS = $(all_libraries) 
libkcmlaptop_la_LIBADD  = $(LIB_KDEUI) 

MESSAGE_SOURCES = laptop_daemon.cpp kpcmcia.cpp xautolock.cc kpcmciainfo.cpp daemondock.cpp xautolock_diy.c xautolock_engine.c
kded_klaptopdaemon_la_SOURCES = $(MESSAGE_SOURCES) laptop_daemon.skel  
kded_klaptopdaemon_la_LDFLAGS = $(all_libraries) -module -avoid-version  -lXtst
kded_klaptopdaemon_la_LIBADD   = $(LIB_KDEUI) $(LIB_XSS) libkcmlaptop.la

klaptop_acpi_helper_SOURCES = acpi_helper.cpp

klaptop_check_SOURCES = laptop_check.cpp 
klaptop_check_LDFLAGS = $(all_libraries) 
klaptop_check_LDADD = $(LIB_KFILE) $(LIB_KDEUI) libkcmlaptop.la

kcm_laptop_la_SOURCES = battery.cpp main.cpp pcmcia.cpp power.cpp warning.cpp \
	acpi.cpp sony.cpp profile.cpp buttons.cpp apm.cpp  
kcm_laptop_la_LDFLAGS  = $(all_libraries) $(KDE_PLUGIN) -module
kcm_laptop_la_LIBADD = $(LIB_KFILE) $(LIB_KDEUI) libkcmlaptop.la
kcm_laptop_la_COMPILE_FIRST = crcresult.h

AM_CPPFLAGS = $(all_includes)

AM_LDFLAGS = $(all_libraries) $(KDE_RPATH)

KDE_OPTIONS = nofinal

kcm_laptop_la_METASOURCES = AUTO

messages:
	$(XGETTEXT) $(libkcmlaptop_la_SOURCES) $(klaptop_acpi_helper_SOURCES) $(klaptop_check_SOURCES) $(MESSAGE_SOURCES) -o $(podir)/klaptopdaemon.pot
	$(XGETTEXT) $(kcm_laptop_la_SOURCES) -o $(podir)/kcmlaptop.pot

dummy.cpp:
	echo >dummy.cpp

noinst_PROGRAMS = makecrc
makecrc_SOURCES = makecrc.cpp
makecrc_LDADD = $(LIBZ)

crcresult.h:	makecrc klaptop_acpi_helper
	./makecrc >crcresult.h

CLEANFILES = crcresult.h makecrc
