## Makefile.am for tdessh
AM_CPPFLAGS = -DQT_NO_CAST_ASCII

INCLUDES= $(all_includes)

## tdessh
bin_PROGRAMS = 	tdessh
tdessh_SOURCES = tdessh.cpp sshdlg.cpp
tdessh_LDFLAGS = $(all_libraries) $(KDE_RPATH) $(LIB_QT) -lDCOP $(LIB_TDECORE) $(LIB_TDEUI) -ltdefx $(LIB_TDEIO) -ltdetexteditor
tdessh_LDADD   = $(LIB_TDEUI) -ltdesu
tdessh_METASOURCES =  AUTO
noinst_HEADERS = sshdlg.h

## Messages
messages:
	$(XGETTEXT) $(tdessh_SOURCES) -o $(podir)/tdessh.pot

