noinst_LTLIBRARIES = libgoabrowser.la

libgoabrowser_la_CPPFLAGS = \
	$(GOABROWSER_CFLAGS) \
	-DG_LOG_DOMAIN=\"goabrowser\"

libgoabrowser_la_SOURCES = \
	json-gvariant.c \
	json-gvariant.h \
	goabrowser.c \
	goabrowser.h

libgoabrowser_la_LDFLAGS = \
	-static

libgoabrowser_la_LIBADD = \
	$(GOABROWSER_LIBS)
