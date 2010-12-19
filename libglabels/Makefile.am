configdir = $(datadir)/$(LIBGLABELS_BRANCH)

INCLUDES = \
	$(LIBGLABELS_CFLAGS)				\
	-DLIBGLABELS_CONFIG_DIR=\""$(configdir)"\" \
	$(DISABLE_DEPRECATED_CFLAGS)

libglabels_3_0_la_LDFLAGS=\
        -version-info $(LIBGLABELS_API_VERSION) \
        $(LIBGLABELS_LIBS) \
        -no-undefined 

lib_LTLIBRARIES = libglabels-3.0.la

libglabels_3_0_la_SOURCES =	\
	libglabels-private.h	\
	lgl-db.h		\
	lgl-db.c		\
	lgl-units.h		\
	lgl-units.c		\
	lgl-paper.h		\
	lgl-paper.c		\
	lgl-category.h		\
	lgl-category.c		\
	lgl-vendor.h		\
	lgl-vendor.c		\
	lgl-template.h		\
	lgl-template.c		\
	lgl-xml-paper.h		\
	lgl-xml-paper.c		\
	lgl-xml-category.h	\
	lgl-xml-category.c	\
	lgl-xml-vendor.h	\
	lgl-xml-vendor.c	\
	lgl-xml-template.h	\
	lgl-xml-template.c	\
	lgl-xml.h		\
	lgl-xml.c		\
	lgl-str.h		\
	lgl-str.c

libglabels_3_0includedir=$(includedir)/$(LIBGLABELS_BRANCH)

libglabels_3_0include_HEADERS = 	\
	libglabels.h

libglabels_3_0subincludedir=$(includedir)/$(LIBGLABELS_BRANCH)/libglabels

libglabels_3_0subinclude_HEADERS =  \
	lgl-db.h                \
	lgl-units.h		\
	lgl-paper.h		\
	lgl-category.h		\
	lgl-template.h		\
	lgl-xml-paper.h		\
	lgl-xml-category.h	\
	lgl-xml-template.h	\
	lgl-xml.h		\
	lgl-str.h			

EXTRA_DIST =			\
	$(LIBGLABELS_BRANCH).pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(LIBGLABELS_BRANCH).pc

