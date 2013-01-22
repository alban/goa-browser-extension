pkglib_LTLIBRARIES = libgoabrowser.la

libgoabrowser_la_CPPFLAGS = \
	$(GOABROWSER_CFLAGS) \
	-DG_LOG_DOMAIN=\"goabrowser\"

libgoabrowser_la_SOURCES = \
	goabrowser.c \
	goabrowser.h

libgoabrowser_la_LDFLAGS = \
	-release $(VERSION) \
	-shared \
	-export-dynamic

libgoabrowser_la_LIBADD = \
	$(GOABROWSER_LIBS)
