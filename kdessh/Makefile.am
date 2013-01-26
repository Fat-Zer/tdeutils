## Makefile.am for kdessh
AM_CPPFLAGS = -DQT_NO_CAST_ASCII

INCLUDES= $(all_includes)

## kdessh
bin_PROGRAMS = 	kdessh
kdessh_SOURCES = kdessh.cpp sshdlg.cpp
kdessh_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_KIO) -ltdetexteditor
kdessh_LDADD   = $(LIB_TDEUI) -ltdesu
kdessh_METASOURCES =  AUTO
noinst_HEADERS = sshdlg.h

## Messages
messages:
	$(XGETTEXT) $(kdessh_SOURCES) -o $(podir)/kdessh.pot

