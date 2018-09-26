dnl Process this file with autoconf to produce a configure script.


dnl ---------------------------------------------------------------------------
dnl - GLABELS version
dnl ---------------------------------------------------------------------------
m4_define([glabels_major_version], [3])
m4_define([glabels_minor_version], [4])
m4_define([glabels_micro_version], [0])

m4_define([glabels_version],
          [glabels_major_version.glabels_minor_version.glabels_micro_version])

dnl ---------------------------------------------------------------------------


AC_PREREQ(2.64)
AC_INIT([glabels],[glabels_version],
	[http://bugzilla.gnome.org/enter_bug.cgi?product=glabels],
	[glabels])

AC_CONFIG_SRCDIR(src/glabels.c)

AM_INIT_AUTOMAKE(AC_PACKAGE_NAME, AC_PACKAGE_VERSION)

AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_CONFIG_HEADERS(config.h)

IT_PROG_INTLTOOL([0.21])

PKG_PROG_PKG_CONFIG

LT_INIT

YELP_HELP_INIT
GTK_DOC_CHECK(1.0)

GLIB_GSETTINGS

AC_SEARCH_LIBS([strerror],[cposix])
AC_PROG_CC
AC_PROG_INSTALL

AX_COMPILER_FLAGS

AC_PATH_PROG(GLIB_GENMARSHAL,         glib-genmarshal)
AC_PATH_PROG(GDK_PIXBUF_CSOURCE,      gdk-pixbuf-csource)
AC_PATH_PROG(GTK_UPDATE_ICON_CACHE,   gtk-update-icon-cache)


dnl ---------------------------------------------------------------------------
dnl - GLABELS branch
dnl ---------------------------------------------------------------------------
GLABELS_BRANCH=glabels-3.0
AC_SUBST(GLABELS_BRANCH)

dnl ---------------------------------------------------------------------------
dnl - LIBGLABELS branch
dnl ---------------------------------------------------------------------------
LIBGLABELS_BRANCH=libglabels-3.0
AC_SUBST(LIBGLABELS_BRANCH)

dnl ---------------------------------------------------------------------------
dnl - LIBGLBARCODE branch
dnl ---------------------------------------------------------------------------
LIBGLBARCODE_BRANCH=libglbarcode-3.0
AC_SUBST(LIBGLBARCODE_BRANCH)

dnl ---------------------------------------------------------------------------
dnl - LIBGLABELS API versioning
dnl ---------------------------------------------------------------------------
dnl From the libtool manual:
dnl 1. Start with version information of `0:0:0' for each libtool library.
dnl 2. Update the version information only immediately before a public release.
dnl    More frequent updates are unnecessary, and only guarantee that the current
dnl    interface number gets larger faster.
dnl 3. If the library source code has changed at all since the last update, then increment
dnl    revision (`c:r:a' becomes `c:r+1:a').
dnl 4. If any interfaces have been added, removed, or changed since the last update,
dnl    increment current, and set revision to 0.
dnl 5. If any interfaces have been added since the last public release, then increment age.
dnl 6. If any interfaces have been removed since the last public release, then set age
dnl    to 0.
LIBGLABELS_C=8
LIBGLABELS_R=0
LIBGLABELS_A=0

LIBGLABELS_API_VERSION=${LIBGLABELS_C}:${LIBGLABELS_R}:${LIBGLABELS_A}
AC_SUBST(LIBGLABELS_API_VERSION)

dnl ---------------------------------------------------------------------------
dnl - LIBGLBARCODE API versioning
dnl ---------------------------------------------------------------------------
dnl From the libtool manual:
dnl 1. Start with version information of `0:0:0' for each libtool library.
dnl 2. Update the version information only immediately before a public release.
dnl    More frequent updates are unnecessary, and only guarantee that the current
dnl    interface number gets larger faster.
dnl 3. If the library source code has changed at all since the last update, then increment
dnl    revision (`c:r:a' becomes `c:r+1:a').
dnl 4. If any interfaces have been added, removed, or changed since the last update,
dnl    increment current, and set revision to 0.
dnl 5. If any interfaces have been added since the last public release, then increment age.
dnl 6. If any interfaces have been removed since the last public release, then set age
dnl    to 0.
LIBGLBARCODE_C=0
LIBGLBARCODE_R=0
LIBGLBARCODE_A=0

LIBGLBARCODE_API_VERSION=${LIBGLBARCODE_C}:${LIBGLBARCODE_R}:${LIBGLBARCODE_A}
AC_SUBST(LIBGLBARCODE_API_VERSION)

dnl ---------------------------------------------------------------------------
dnl - Library dependencies
dnl ---------------------------------------------------------------------------
dnl Required dependencies
GLIB_REQUIRED=2.42.0
GTK_REQUIRED=3.14.0
LIBXML_REQUIRED=2.9.0
LIBRSVG_REQUIRED=2.39.0
CAIRO_REQUIRED=1.14.0
PANGO_REQUIRED=1.36.0

dnl Optional dependencies
LIBEBOOK_REQUIRED=3.12.0
LIBBARCODE_REQUIRED=0.98
LIBQRENCODE_REQUIRED=3.1.0
LIBIEC16022_REQUIRED=0.2.4
LIBZINT_REQUIRED=2.4.0

dnl Make above strings available for packaging files (e.g. rpm spec files)
AC_SUBST(GLIB_REQUIRED)
AC_SUBST(GTK_REQUIRED)
AC_SUBST(LIBXML_REQUIRED)
AC_SUBST(LIBRSVG_REQUIRED)
AC_SUBST(CAIRO_REQUIRED)
AC_SUBST(PANGO_REQUIRED)
AC_SUBST(LIBEBOOK_REQUIRED)
AC_SUBST(LIBBARCODE_REQUIRED)
AC_SUBST(LIBQRENCODE_REQUIRED)
AC_SUBST(LIBIEC16022_REQUIRED)
AC_SUBST(LIBZINT_REQUIRED)


dnl ---------------------------------------------------------------------------
dnl - GLABELS prerequisites
dnl ---------------------------------------------------------------------------
PKG_CHECK_MODULES(GLABELS, [\
	glib-2.0 >= $GLIB_REQUIRED \
	gtk+-3.0 >= $GTK_REQUIRED \
	libxml-2.0 >= $LIBXML_REQUIRED \
	librsvg-2.0 >= $LIBRSVG_REQUIRED \
])

AC_SUBST(GLABELS_CFLAGS)
AC_SUBST(GLABELS_LIBS)


dnl ---------------------------------------------------------------------------
dnl - LIBGLABELS more modest prerequisites
dnl ---------------------------------------------------------------------------
PKG_CHECK_MODULES(LIBGLABELS, [\
	glib-2.0 >= $GLIB_REQUIRED \
	gobject-2.0 >= $GLIB_REQUIRED \
	libxml-2.0 >= $LIBXML_REQUIRED \
])

AC_SUBST(LIBGLABELS_CFLAGS)
AC_SUBST(LIBGLABELS_LIBS)


dnl ---------------------------------------------------------------------------
dnl - LIBGLBARCODE more modest prerequisites
dnl ---------------------------------------------------------------------------
PKG_CHECK_MODULES(LIBGLBARCODE, [\
	glib-2.0 >= $GLIB_REQUIRED \
	cairo >= $CAIRO_REQUIRED \
	pango >= $PANGO_REQUIRED \
	pangocairo >= $PANGO_REQUIRED \
])

AC_SUBST(LIBGLBARCODE_CFLAGS)
AC_SUBST(LIBGLBARCODE_LIBS)


dnl ---------------------------------------------------------------------------
dnl - Check for optional evolution data server
dnl ---------------------------------------------------------------------------
AC_ARG_WITH(libebook,
	    [AS_HELP_STRING([--without-libebook],[build without Evolution Data Server support])])
have_libebook=no
if test "x$with_libebook" != xno; then
	PKG_CHECK_MODULES(LIBEBOOK, libebook-1.2 >= $LIBEBOOK_REQUIRED,
			  [have_libebook=yes], [have_libebook=no])
fi

if test "x$have_libebook" = "xyes"; then
	AC_DEFINE(HAVE_LIBEBOOK,1,[Define to 1 for EDS support])
	AC_SUBST(LIBEBOOK_CFLAGS)
	AC_SUBST(LIBEBOOK_LIBS)
fi


dnl ---------------------------------------------------------------------------
dnl - Check for optional GNU Barcode backend
dnl ---------------------------------------------------------------------------
AC_ARG_WITH(libbarcode,
	    [AS_HELP_STRING([--without-libbarcode],[build without GNU Barcode support])])
have_libbarcode=no
if test "x$with_libbarcode" != xno; then
   AC_CHECK_LIB(barcode, Barcode_Create,
		[have_libbarcode=yes], [have_libbarcode=no])
fi

if test "x$have_libbarcode" = "xyes"; then
	AC_DEFINE(HAVE_LIBBARCODE,1,[Define to 1 for GNU Barcode support])
	LIBBARCODE_CFLAGS=""
	LIBBARCODE_LIBS="-lbarcode"
	AC_SUBST(LIBBARCODE_CFLAGS)
	AC_SUBST(LIBBARCODE_LIBS)
else
	help_libbarcode="(See http://www.gnu.org/software/barcode/barcode.html)"
fi

dnl ---------------------------------------------------------------------------
dnl - Check for optional Zint backend
dnl ---------------------------------------------------------------------------
AC_ARG_WITH(libzint,
	    [AS_HELP_STRING([--without-libzint],[build without Zint Barcode support])])
have_libzint=no
if test "x$with_libzint" != xno; then
   AC_CHECK_LIB(zint, ZBarcode_Render,
		[have_libzint=yes], [have_libzint=no])
fi

if test "x$have_libzint" = "xyes"; then
	AC_DEFINE(HAVE_LIBZINT,1,[Define to 1 for Zint Barcode support])
	LIBZINT_CFLAGS=""
	LIBZINT_LIBS="-lzint"
	AC_SUBST(LIBZINT_CFLAGS)
	AC_SUBST(LIBZINT_LIBS)
else
	help_libzint="(See http://www.zint.org.uk)"
fi


dnl ---------------------------------------------------------------------------
dnl - Check for optional QRencode Barcode backend
dnl ---------------------------------------------------------------------------
AC_ARG_WITH(libqrencode,
	    [AS_HELP_STRING([--without-libqrencode],[build without QR code support])])
have_libqrencode=no
if test "x$with_libqrencode" != xno; then
	PKG_CHECK_MODULES(LIBQRENCODE, libqrencode >= $LIBQRENCODE_REQUIRED,
			  [have_libqrencode=yes], [have_libqrencode=no])
fi

if test "x$have_libqrencode" = "xyes"; then
	AC_DEFINE(HAVE_LIBQRENCODE,1,[Define to 1 for QR code support])
	AC_SUBST(LIBQRENCODE_CFLAGS)
	AC_SUBST(LIBQRENCODE_LIBS)
else
	help_libqrencode="(See http://megaui.net/fukuchi/works/qrencode/index.en.html)"
fi


dnl ---------------------------------------------------------------------------
dnl - Check for IEC16022 Barcode backend
dnl ---------------------------------------------------------------------------
AC_ARG_WITH(libiec16022,
	    [AS_HELP_STRING([--without-libiec16022],[build without IEC 16022 support])])
have_libiec16022=no
if test "x$with_libiec16022" != xno; then
	PKG_CHECK_MODULES(LIBIEC16022, libiec16022 >= $LIBIEC16022_REQUIRED,
			  [have_libiec16022=yes], [have_libiec16022=no])
fi

if test "x$have_libiec16022" = "xyes"; then
	AC_DEFINE(HAVE_LIBIEC16022,1,[Define to 1 for IEC 16022 support])
	AC_SUBST(LIBIEC16022_CFLAGS)
	AC_SUBST(LIBIEC16022_LIBS)
else
	help_libiec16022="(See http://datenfreihafen.org/projects/iec16022.html)"
fi


dnl ---------------------------------------------------------------------------
dnl - Enable deprecation testing
dnl ---------------------------------------------------------------------------
AC_ARG_ENABLE(deprecations,
              [AS_HELP_STRING([--enable-deprecations],[warn about deprecated usages [default=no]])],,
              [enable_deprecations=no])

if test "x$enable_deprecations" = "xyes"; then
   DISABLE_DEPRECATED_CFLAGS="\
-DG_DISABLE_DEPRECATED \
-DGDK_DISABLE_DEPRECATED \
-DGTK_DISABLE_DEPRECATED \
-DGDK_PIXBUF_DISABLE_DEPRECATED \
"
   AC_SUBST(DISABLE_DEPRECATED_CFLAGS)
fi


dnl ---------------------------------------------------------------------------
dnl - i18n support
dnl ---------------------------------------------------------------------------
GETTEXT_PACKAGE=${GLABELS_BRANCH}
AC_SUBST(GETTEXT_PACKAGE)
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE", [Gettext package])

AM_GLIB_GNU_GETTEXT


dnl ---------------------------------------------------------------------------
dnl - Makefiles, etc.
dnl ---------------------------------------------------------------------------
AC_CONFIG_FILES([
Makefile
libglabels/Makefile
libglabels/${LIBGLABELS_BRANCH}.pc
libglbarcode/Makefile
libglbarcode/${LIBGLBARCODE_BRANCH}.pc
src/Makefile
src/cursors/Makefile
src/pixmaps/Makefile
data/Makefile
data/appdata/Makefile
data/desktop/Makefile
data/icons/Makefile
data/icons/16x16/Makefile
data/icons/22x22/Makefile
data/icons/24x24/Makefile
data/icons/32x32/Makefile
data/icons/48x48/Makefile
data/icons/scalable/Makefile
data/man/Makefile
data/mime/Makefile
data/pixmaps/Makefile
data/schemas/Makefile
data/schemas/org.gnome.glabels-3.gschema.xml.in
data/ui/Makefile
templates/Makefile
po/Makefile.in
help/Makefile
docs/Makefile
docs/libglabels/Makefile
docs/libglbarcode/Makefile
glabels.spec
])
AC_OUTPUT


dnl ---------------------------------------------------------------------------
dnl - Print configuration information
dnl ---------------------------------------------------------------------------
echo "

Configuration:

        Package ................. ${PACKAGE}-${VERSION}
        Installation prefix ..... ${prefix}
        Source code location .... ${srcdir}
        Compiler ................ ${CC} 


Optional data merge backends:

        Evolution Data Server ... ${have_libebook}


Optional barcode backends:

        GNU Barcode ............. ${have_libbarcode} ${help_libbarcode}
        QR Code ................. ${have_libqrencode} ${help_libqrencode}
        IEC 16022 ............... ${have_libiec16022} ${help_libiec16022}
        Zint .................... ${have_libzint} ${help_libzint}


"
