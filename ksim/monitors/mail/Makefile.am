kde_module_LTLIBRARIES = ksim_mail.la
ksim_mail_la_SOURCES = ksimmail.cpp

ksim_mail_la_LDFLAGS = $(all_libraries) -module $(KDE_PLUGIN)
ksim_mail_la_LIBADD = $(LIB_KIO) ../../library/libksimcore.la

EXTRA_DIST = mail.desktop

INCLUDES= -I$(top_srcdir)/ksim/library \
                   $(all_includes)

METASOURCES = AUTO

mon_DATA = mail.desktop
mondir = $(kde_datadir)/ksim/monitors
