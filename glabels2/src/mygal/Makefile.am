INCLUDES = \
	$(MYGAL_CFLAGS)


libmygal_la_LDFLAGS=\
        $(MYGAL_LIBS)

noinst_LTLIBRARIES = libmygal.la

libmygal_la_SOURCES = 			\
	widget-color-combo.c		\
	widget-color-combo.h		\
	e-util.h			\
	e-colors.c			\
	e-colors.h			\
	mygal-combo-box.c		\
	mygal-combo-box.h		\
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
