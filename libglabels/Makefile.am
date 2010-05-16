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

libglabels_3_0_la_SOURCES =		\
	libglabels-private.h	\
	db.h			\
	db.c			\
	units.h			\
	units.c			\
	paper.h			\
	paper.c			\
	category.h		\
	category.c		\
	vendor.h		\
	vendor.c		\
	template.h		\
	template.c		\
	xml-paper.h		\
	xml-paper.c		\
	xml-category.h		\
	xml-category.c		\
	xml-vendor.h		\
	xml-vendor.c		\
	xml-template.h		\
	xml-template.c		\
	xml.h			\
	xml.c			\
	str.h			\
	str.c

libglabels_3_0includedir=$(includedir)/$(LIBGLABELS_BRANCH)

libglabels_3_0include_HEADERS = 	\
	libglabels.h

libglabels_3_0subincludedir=$(includedir)/$(LIBGLABELS_BRANCH)/libglabels

libglabels_3_0subinclude_HEADERS = 	\
	db.h                    \
	units.h			\
	paper.h			\
	category.h		\
	template.h		\
	xml-paper.h		\
	xml-category.h		\
	xml-template.h		\
	xml.h			\
	str.h			

EXTRA_DIST =			\
	$(LIBGLABELS_BRANCH).pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = $(LIBGLABELS_BRANCH).pc

