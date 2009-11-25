kde_module_LTLIBRARIES = ksim_snmp.la
ksim_snmp_la_SOURCES = plugin.cpp view.cpp configpage.cpp configwidget.ui snmp.cpp hostdialogbase.ui hostdialog.cpp value.cpp monitor.cpp \
                       monitordialogbase.ui monitordialog.cpp labelmonitor.cpp chartmonitor.cpp \
		       identifier.cpp monitorconfig.cpp hostconfig.cpp session.cpp probedialog.cpp snmplib.cpp \
		       pdu.cpp proberesultdialogbase.ui proberesultdialog.cpp browsedialogbase.ui browsedialog.cpp \
		       walker.cpp

ksim_snmp_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
ksim_snmp_la_LIBADD = ../../library/libksimcore.la $(LIB_KIO) $(LIBSNMP)

INCLUDES= -I$(top_srcdir)/ksim/library $(all_includes)

METASOURCES = AUTO

mon_DATA = Snmp.desktop
mondir = $(kde_datadir)/ksim/monitors
