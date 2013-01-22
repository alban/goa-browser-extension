noinst_LTLIBRARIES = libgoa_npapi_plugin.la

libgoa_npapi_plugin_la_CPPFLAGS = \
	$(GOABROWSER_NPAPI_PLUGIN_CFLAGS) \
	$(NPAPI_DEBUG_CFLAGS) \
	-I$(top_srcdir)/lib \
	-DG_LOG_DOMAIN=\"goabrowser\" \
	-DXP_UNIX=1

libgoa_npapi_plugin_la_SOURCES = \
	npapi-headers/headers/npapi.h \
	npapi-headers/headers/npfunctions.h \
	npapi-headers/headers/npruntime.h \
	npapi-headers/headers/nptypes.h \
	object.c \
	object.h \
	plugin.c \
	plugin.h

libgoa_npapi_plugin_la_LDFLAGS = \
        -avoid-version \
	-module \
	-shared \
	-export-dynamic \
	-rpath /nowhere

libgoa_npapi_plugin_la_LIBADD = \
	$(top_builddir)/lib/libgoabrowser.la \
	$(GOABROWSER_NPAPI_PLUGIN_LIBS)

noinst_HEADERS = \
	npapi-headers/headers/npapi.h		\
	npapi-headers/headers/npfunctions.h	\
	npapi-headers/headers/npruntime.h	\
	npapi-headers/headers/nptypes.h
