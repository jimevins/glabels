INCLUDES = \
	$(GLABELS_CFLAGS) 				\
	-DG_DISABLE_DEPRECATED				\
	-DGDK_DISABLE_DEPRECATED			\
	-DGTK_DISABLE_DEPRECATED			\
	-DGDK_PIXBUF_DISABLE_DEPRECATED			\
	-DGNOME_DISABLE_DEPRECATED			

noinst_LTLIBRARIES = libmygal.la

libmygal_la_SOURCES = 			\
	widget-color-combo.c		\
	widget-color-combo.h		\
	e-util.h			\
	e-colors.c			\
	e-colors.h			\
	gtk-combo-box.c			\
	gtk-combo-box.h			\
	color-palette.c			\
	color-palette.h			\
	color-group.c			\
	color-group.h			\
	e-marshal.c			\
	e-marshal.h

marshal_sources = \
	e-marshal.c \
	e-marshal.h

$(libmygal_la_OBJECTS) : $(marshal_sources)

e-marshal.h: e-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=e_marshal > $@
e-marshal.c: e-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --body --prefix=e_marshal > $@

EXTRA_DIST = e-marshal.list
CLEANFILES = $(marshal_sources)

DONT_DIST_SOURCE = $(marshal_sources)

dist-hook:
	for file in $(DONT_DIST_SOURCE) ; do \
	    rm -f $(distdir)/$$file ; \
	done
