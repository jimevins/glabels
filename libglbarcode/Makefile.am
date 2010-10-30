configdir = $(datadir)/$(LIBGLBARCODE_BRANCH)

INCLUDES = \
	$(LIBGLBARCODE_CFLAGS)				\
	-DLIBGLBARCODE_CONFIG_DIR=\""$(configdir)"\" \
	$(DISABLE_DEPRECATED_CFLAGS)

libglbarcode_3_0_la_LDFLAGS=\
        -version-info $(LIBGLBARCODE_API_VERSION) \
        $(LIBGLBARCODE_LIBS) \
        -no-undefined 

lib_LTLIBRARIES = libglbarcode-3.0.la

libglbarcode_3_0_la_SOURCES =		\
	lgl-barcode.c			\
	lgl-barcode.h			\
	lgl-barcode-create.c		\
	lgl-barcode-create.h		\
	lgl-barcode-render-to-cairo.c	\
	lgl-barcode-render-to-cairo.h	\
	lgl-barcode-type.h		\
	lgl-barcode-onecode.c		\
	lgl-barcode-onecode.h		\
	lgl-barcode-postnet.c		\
	lgl-barcode-postnet.h		\
	lgl-barcode-code39.c		\
	lgl-barcode-code39.h

libglbarcode_3_0includedir=$(includedir)/$(LIBGLBARCODE_BRANCH)

libglbarcode_3_0include_HEADERS = 	\
	libglbarcode.h

libglbarcode_3_0subincludedir=$(includedir)/$(LIBGLBARCODE_BRANCH)/libglbarcode

libglbarcode_3_0subinclude_HEADERS = 	\
	lgl-barcode.h			\
	lgl-barcode-create.h		\
	lgl-barcode-render-to-cairo.h	\
	lgl-barcode-type.h		\
	lgl-barcode-onecode.h		\
	lgl-barcode-postnet.h		\
	lgl-barcode-code39.h

EXTRA_DIST =			\
	$(LIBGLBARCODE_BRANCH).pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(LIBGLBARCODE_BRANCH).pc

