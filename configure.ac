AC_PREREQ([2.63])

AC_INIT([goa-browser-extension],
        [0.1],
        [http://cgit.collabora.com/git/user/em/goa-browser-extension/],
        [goa-browser-extension],
        [http://cgit.collabora.com/git/user/em/goa-browser-extension/])

AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE([1.11 foreign -Wall -Wno-portability no-define no-dist-gzip dist-xz tar-ustar])

AM_SILENT_RULES([yes])

AC_CONFIG_MACRO_DIR([m4])

AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_CC_C99

AC_PROG_MKDIR_P
AC_PROG_SED

AC_DISABLE_STATIC
LT_INIT

dnl ***************************************************************************
dnl Internationalization
dnl ***************************************************************************
IT_PROG_INTLTOOL([0.50.0])

GETTEXT_PACKAGE=goa_browser_extension
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE,"$GETTEXT_PACKAGE", [GETTEXT package name])


PKG_CHECK_MODULES(GOABROWSER,
    glib-2.0
    )
PKG_CHECK_MODULES(GOABROWSER_NPAPI_PLUGIN,
    glib-2.0
    )
GLIB_GSETTINGS


dnl ***************************************************************************
dnl Chromium extension
dnl ***************************************************************************

AC_ARG_ENABLE(chromium, 
	AS_HELP_STRING([--enable-chromium],
		[Build Chromium extension. [default=yes]]),
		[enable_chromium=$enableval], [enable_chromium="yes"])
if test "x$enable_chromium" = "xyes"; then
    AC_DEFINE([WITH_CHROMIUM], 1, [Build Chromium extension.])
fi
AM_CONDITIONAL([WITH_CHROMIUM], [test "x$enable_chromium" = "xyes"])

AC_ARG_ENABLE(google-chrome,
	AS_HELP_STRING([--enable-google-chrome],
		[Build Google Chrome extension. [default=yes]]),
		[enable_google_chrome=$enableval], [enable_google_chrome="yes"])
if test "x$enable_google_chrome" = "xyes"; then
    AC_DEFINE([WITH_GOOGLE_CHROME], 1, [Build Google Chrome extension.])
fi
AM_CONDITIONAL([WITH_GOOGLE_CHROME], [test "x$enable_google_chrome" = "xyes"])

AC_CONFIG_FILES([
Makefile
lib/Makefile
npapi-plugin/Makefile
chromium-extension/Makefile
po/Makefile.in
])
AC_OUTPUT

AC_MSG_NOTICE([

  goa-browser-extension $VERSION
  ---------------------

  Chromium extension      : ${enable_chromium}
  Google Chrome extension : ${enable_google_chrome}
])
