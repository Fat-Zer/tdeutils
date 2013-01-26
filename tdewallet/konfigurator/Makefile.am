INCLUDES = $(all_includes)
METASOURCES = AUTO

kde_module_LTLIBRARIES = kcm_tdewallet.la
kcm_tdewallet_la_SOURCES = walletconfigwidget.ui konfigurator.cpp
kcm_tdewallet_la_LDFLAGS = $(all_libraries) -module -avoid-version -no-undefined
kcm_tdewallet_la_LIBADD = $(LIB_TDEUI) -ltdewalletclient

messages: rc.cpp
	$(XGETTEXT) *.cpp -o $(podir)/kcmtdewallet.pot

xdg_apps_DATA = tdewalletconfig.desktop

kde_services_DATA = tdewallet_config.desktop tdewalletmanager_show.desktop

