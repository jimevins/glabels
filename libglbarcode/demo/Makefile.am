NULL = 

bin_PROGRAMS = glbarcode

glbarcode_SOURCES = \
	glbarcode.vala \
	$(NULL)

glbarcode_LDADD = \
	-L.. -lglbarcode-4.0 \
	$(LIBGLBARCODE_LIBS) \
	$(NULL)

INCLUDES = \
	$(LIBGLBARCODE_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DLIBGLBARCODE_BRANCH=\""$(LIBGLBARCODE_BRANCH)"\" \
	-I.. \
	$(NULL)

VALAFLAGS = \
	--pkg posix \
	--pkg gobject-2.0 \
	--pkg cairo \
	--pkg pangocairo \
	--vapidir .. \
	../libglbarcode-4.vapi \
	$(NULL)

EXTRA_DIST = \
	$(NULL)

DISTCLEANFILES = \
	$(NULL)


