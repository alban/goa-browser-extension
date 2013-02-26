CLEANFILES = $(TMPDIR)/* res/*
EXTRA_DIST =

TMPDIR = extension
RSAFILE = $(abs_top_builddir)/gnome-online-accounts.pem

extensiondir = $(libdir)/gnome-online-accounts-chromium
extension_DATA=res/gnome-online-accounts.crx

clean-extension-package:
	rm -f $(CLEANFILES)

SCRIPT_FILES = \
	$(wildcard *.html) \
	$(wildcard *.js)

noinst_DATA = $(SCRIPT_FILES)

NPAPI_PLUGIN=$(top_builddir)/npapi-plugin/.libs/libgoa_npapi_plugin.so

$(TMPDIR): $(SCRIPT_FILES) manifest.json gnome-online-accounts.json $(NPAPI_PLUGIN)
	-rm -R $@
	mkdir $@
	cp -r $^ $@

$(RSAFILE):
	openssl genrsa 1024 > $@

res/gnome-online-accounts.crx: $(TMPDIR) $(RSAFILE)
	echo $(abs_builddir)
	$(srcdir)/crxmake.sh $(TMPDIR) $(RSAFILE)
	mv res/extension.crx res/gnome-online-accounts.crx

if WITH_CHROMIUM
chromiummanifestdir = $(datadir)/chromium/extensions/
chromiummanifest_DATA = $(wildcard $(top_builddir)/chromium-extension/extension/*.json)
endif

if WITH_GOOGLE_CHROME
googlechromemanifestdir = $(datadir)/google-chrome/extensions/
googlechromemanifest_DATA = $(wildcard $(top_builddir)/chromium-extension/extension/*.json)
endif

%.json: %.json.in
	sed -e "s|\@EXTDIR\@|$(extensiondir)/gnome-online-accounts.crx|" \
	    -e "s|\@PACKAGE_URL\@|$(PACKAGE_URL)|"                       \
	    -e "s|\@VERSION\@|$(VERSION)|" $< > $@

CLEANFILES += \
	manifest.json \
	gnome-online-accounts.json \
	libgoa_npapi_plugin.so \
	extension.crx

EXTRA_DIST += \
	manifest.json.in \
	gnome-online-accounts.json.in \
	crxmake.sh \
	$(SCRIPT_FILES)
