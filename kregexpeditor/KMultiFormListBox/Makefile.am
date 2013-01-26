AM_CPPFLAGS = -DQT_NO_CAST_ASCII
noinst_LTLIBRARIES  = libtdemultiformlistbox.la

INCLUDES= -I$(srcdir)/../KWidgetStreamer $(all_includes)

include_HEADERS = tdemultiformlistboxentry.h tdemultiformlistbox.h \
	tdemultiformlistboxfactory.h 
noinst_HEADERS = ccp.h indexWindow.h tdemultiformlistbox-multivisible.h \
	tdemultiformlistbox-windowed.h widgetwindow.h windowlistboxitem.h
libtdemultiformlistbox_la_SOURCES = ccp.cpp tdemultiformlistboxentry.cpp \
	tdemultiformlistbox.cpp tdemultiformlistbox-multivisible.cpp \
	tdemultiformlistboxfactory.cpp indexWindow.cpp \
	tdemultiformlistbox-windowed.cpp widgetwindow.cpp windowlistboxitem.cpp

METASOURCES = AUTO
