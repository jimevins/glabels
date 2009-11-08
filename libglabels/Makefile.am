templatesdir = $(datadir)/glabels/templates

INCLUDES = \
	$(LIBGLABELS_CFLAGS)				\
	-DLIBGLABELS_TEMPLATE_DIR=\""$(templatesdir)"\" \
	$(DISABLE_DEPRECATED_CFLAGS)

libglabels_la_LDFLAGS=\
        -version-info $(LIBGLABELS_VERSION_INFO) \
        $(LIBGLABELS_LIBS) \
        -no-undefined 

lib_LTLIBRARIES = libglabels.la

libglabelsincludedir=$(includedir)/libglabels

libglabels_la_SOURCES =		\
	libglabels-private.h	\
	db.h			\
	db.c			\
	units.h			\
	units.c			\
	paper.h			\
	paper.c			\
	category.h		\
	category.c		\
	template.h		\
	template.c		\
	xml-paper.h		\
	xml-paper.c		\
	xml-category.h		\
	xml-category.c		\
	xml-template.h		\
	xml-template.c		\
	xml.h			\
	xml.c			\
	str.h			\
	str.c

libglabelsinclude_HEADERS = 	\
	libglabels.h		\
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
	libglabels.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libglabels.pc

