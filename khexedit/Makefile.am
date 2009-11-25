INCLUDES = $(all_includes)

SUBDIRS = pics lib parts

bin_PROGRAMS = khexedit

khexedit_SOURCES = main.cc toplevel.cc hexmanagerwidget.cc \
hexeditorwidget.cc hextoolwidget.cc hexviewwidget.cc hexbuffer.cc \
conversion.cc dialog.cc hexvalidator.cc hexclipboard.cc \
optiondialog.cc printdialogpage.cc statusbarprogress.cc \
hexprinter.cc hexerror.cc draglabel.cc stringdialog.cc \
bitswapwidget.cc chartabledialog.cc hexdrag.cc exportdialog.cc \
fileinfodialog.cc converterdialog.cc listview.cc searchbar.cc

noinst_HEADERS = toplevel.h hexmanagerwidget.h hexeditorwidget.h \
hextoolwidget.h hexviewwidget.h hexbuffer.h conversion.h dialog.h \
hexvalidator.h hexclipboard.h optiondialog.h \
printdialogpage.h statusbarprogress.h hexprinter.h  hexerror.h \
draglabel.h progress.h hexeditstate.h chartabledialog.h hexdrag.h \
exportdialog.h fileinfodialog.h converterdialog.h listview.h searchbar.h

khexedit_METASOURCES = AUTO
khexedit_LDFLAGS = $(all_libraries) $(KDE_RPATH)
khexedit_LDADD = $(LIB_KIO) $(LIB_KDEPRINT)

xdg_apps_DATA = khexedit.desktop

KDE_ICON = khexedit

rcdir = $(kde_datadir)/khexedit
rc_DATA = khexeditui.rc

messages: rc.cpp
	$(EXTRACTRC) parts/kpart/*rc >> rc.cpp
	$(XGETTEXT) rc.cpp `find . -name \*.cc -o -name \*.cpp` -o $(podir)/khexedit.pot
