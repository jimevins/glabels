templatesdir = $(datadir)/glabels/templates

INCLUDES = \
	$(LIBGLABELS_CFLAGS)				\
	-DDATADIR=\""$(datadir)"\"			\
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
	enums.h			\
	paper.h			\
	paper.c			\
	template.h		\
	template.c		\
	xml-paper.h		\
	xml-paper.c		\
	xml-template.h		\
	xml-template.c		\
	xml.h			\
	xml.c			

libglabelsinclude_HEADERS = 	\
	libglabels.h		\
	enums.h			\
	paper.h			\
	template.h		\
	xml-paper.h		\
	xml-template.h		\
	xml.h			

EXTRA_DIST =			\
	libglabels.pc.in

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libglabels.pc

