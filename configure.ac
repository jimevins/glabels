dnl Process this file with autoconf to produce a configure script.


dnl ---------------------------------------------------------------------------
dnl - GLABELS version
dnl ---------------------------------------------------------------------------
m4_define([glabels_major_version], [3])
m4_define([glabels_minor_version], [99])
m4_define([glabels_micro_version], [0])

m4_define([glabels_version],
          [glabels_major_version.glabels_minor_version.glabels_micro_version])

dnl ---------------------------------------------------------------------------


AC_INIT([glabels], [glabels_version],
	[http://bugzilla.gnome.org/enter_bug.cgi?product=glabels],
        [glabels])

AC_CONFIG_SRCDIR([Makefile.am])
AC_CONFIG_HEADERS(config.h)
AM_INIT_AUTOMAKE([dist-bzip2])
AM_MAINTAINER_MODE
m4_ifdef([AM_SILENT_RULES],[AM_SILENT_RULES([yes])])

AC_PROG_CC
AM_PROG_CC_C_O
AC_DISABLE_STATIC
AC_PROG_LIBTOOL

GNOME_DOC_INIT

GLIB_GSETTINGS

AC_PATH_PROG(GDK_PIXBUF_CSOURCE,      gdk-pixbuf-csource)
AC_PATH_PROG(GTK_UPDATE_ICON_CACHE,   gtk-update-icon-cache)

dnl ---------------------------------------------------------------------------
dnl - GLABELS branch
dnl ---------------------------------------------------------------------------
GLABELS_BRANCH=glabels-4.0
AC_SUBST(GLABELS_BRANCH)

dnl ---------------------------------------------------------------------------
dnl - LIBGLABELS branch
dnl ---------------------------------------------------------------------------
LIBGLABELS_BRANCH=libglabels-4.0
AC_SUBST(LIBGLABELS_BRANCH)

dnl ---------------------------------------------------------------------------
dnl - LIBGLBARCODE branch
dnl ---------------------------------------------------------------------------
LIBGLBARCODE_BRANCH=libglbarcode-4.0
AC_SUBST(LIBGLBARCODE_BRANCH)


AC_PATH_PROG(VALAC, valac, valac)
AC_SUBST(VALAC)

AH_TEMPLATE([GETTEXT_PACKAGE], [Package name for gettext])
GETTEXT_PACKAGE=glabels-4.0
AC_DEFINE_UNQUOTED(GETTEXT_PACKAGE, "$GETTEXT_PACKAGE")
AC_SUBST(GETTEXT_PACKAGE)
AM_GLIB_GNU_GETTEXT
IT_PROG_INTLTOOL([0.35.0])

AC_SUBST(CFLAGS)
AC_SUBST(CPPFLAGS)
AC_SUBST(LDFLAGS)

dnl Required dependencies
GLIB_REQUIRED=2.28.0
GTK_REQUIRED=3.0.9
LIBXML_REQUIRED=2.7.8
GEE_REQUIRED=0.6.2.1
CAIRO_REQUIRED=1.10.2
GDK_PIXBUF_REQUIRED=2.24.0
LIBRSVG_REQUIRED=2.36.0
PANGOCAIRO_REQUIRED=1.30.0

dnl Optional dependencies
LIBEBOOK_REQUIRED=2.30.3
LIBBARCODE_REQUIRED=0.98
LIBQRENCODE_REQUIRED=3.1.0
LIBIEC16022_REQUIRED=0.2.4
LIBZINT_REQUIRED=2.4.0

dnl Make above strings available for packaging files (e.g. rpm spec files)
AC_SUBST(GLIB_REQUIRED)
AC_SUBST(GTK_REQUIRED)
AC_SUBST(LIBXML_REQUIRED)
AC_SUBST(GEE_REQUIRED)
AC_SUBST(LIBRSVG_REQUIRED)
AC_SUBST(CAIRO_REQUIRED)
AC_SUBST(PANGO_REQUIRED)
AC_SUBST(GDK_PIXBUF_REQUIRED)
AC_SUBST(LIBRSVG_REQUIRED)
AC_SUBST(PANGOCAIRO_REQUIRED)
AC_SUBST(LIBEBOOK_REQUIRED)
AC_SUBST(LIBBARCODE_REQUIRED)
AC_SUBST(LIBQRENCODE_REQUIRED)
AC_SUBST(LIBIEC16022_REQUIRED)
AC_SUBST(LIBZINT_REQUIRED)


PKG_CHECK_MODULES(GLABELS, [\
			   glib-2.0 >= $GLIB_REQUIRED \
			   gobject-2.0 >= $GLIB_REQUIRED \
			   gtk+-3.0 >= $GTK_REQUIRED \
			   libxml-2.0 >= $LIBXML_REQUIRED \
			   gee-1.0 >= $GEE_REQUIRED \
                           librsvg-2.0 > $LIBRSVG_REQUIRED \
])

AC_SUBST(GLABELS_CFLAGS)
AC_SUBST(GLABELS_LIBS)

PKG_CHECK_MODULES(LIBGLABELS, [\
			      glib-2.0 >= $GLIB_REQUIRED \
			      gobject-2.0 >= $GLIB_REQUIRED \
			      libxml-2.0 >= $LIBXML_REQUIRED \
			      gee-1.0 >= $GEE_REQUIRED \
                              cairo >= $CAIRO_REQUIRED \
                              gdk-pixbuf-2.0 >= $GDK_PIXBUF_REQUIRED \
])

AC_SUBST(LIBGLABELS_CFLAGS)
AC_SUBST(LIBGLABELS_LIBS)

PKG_CHECK_MODULES(LIBGLBARCODE, [\
                                glib-2.0 >= $GLIB_REQUIRED \
                                gobject-2.0 >= $GLIB_REQUIRED \
                                gee-1.0 >= $GEE_REQUIRED \
                                cairo >= $CAIRO_REQUIRED \
                                pangocairo >= $PANGOCAIRO_REQUIRED \
])

AC_SUBST(LIBGLBARCODE_CFLAGS)
AC_SUBST(LIBGLBARCODE_LIBS)


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
	LIBBARCODE_VALAFLAGS="-D HAVE_LIBBARCODE"
	LIBBARCODE_CFLAGS=""
	LIBBARCODE_LIBS="-lbarcode"
	AC_SUBST(LIBBARCODE_VALAFLAGS)
	AC_SUBST(LIBBARCODE_CFLAGS)
	AC_SUBST(LIBBARCODE_LIBS)
else
	help_libbarcode="(See http://www.gnu.org/software/barcode/barcode.html)"
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
	LIBQRENCODE_VALAFLAGS="-D HAVE_LIBQRENCODE"
	AC_SUBST(LIBQRENCODE_VALAFLAGS)
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
	LIBIEC16022_VALAFLAGS="-D HAVE_LIBIEC16022"
	AC_SUBST(LIBIEC16022_VALAFLAGS)
	AC_SUBST(LIBIEC16022_CFLAGS)
	AC_SUBST(LIBIEC16022_LIBS)
else
	help_libiec16022="(See http://datenfreihafen.org/projects/iec16022.html)"
fi


AC_CONFIG_FILES([Makefile
	vapi/Makefile
	libglabels/Makefile
	libglbarcode/Makefile
	libglbarcode/demo/Makefile
	glabels/Makefile
	glabels/cursors/Makefile
	po/Makefile.in
        templates/Makefile
        data/Makefile
        data/icons/Makefile
        data/icons/16x16/Makefile
        data/icons/22x22/Makefile
        data/icons/24x24/Makefile
        data/icons/32x32/Makefile
        data/icons/48x48/Makefile
        data/pixmaps/Makefile
        data/schemas/Makefile
        data/schemas/org.gnome.glabels-4.gschema.xml.in
        data/ui/Makefile
        help/Makefile])

AC_OUTPUT


dnl ---------------------------------------------------------------------------
dnl - Print configuration information
dnl ---------------------------------------------------------------------------
echo "

Configuration:

        Package ................. ${PACKAGE}-${VERSION}
        Installation prefix ..... ${prefix}
        Source code location .... ${srcdir}
        C Compiler .............. ${CC} 
        VALA Compiler ........... ${VALAC} 


Optional barcode backends:

        GNU Barcode ............. ${have_libbarcode} ${help_libbarcode}
        QR Code ................. ${have_libqrencode} ${help_libqrencode}
        IEC 16022 ............... ${have_libiec16022} ${help_libiec16022}

"
