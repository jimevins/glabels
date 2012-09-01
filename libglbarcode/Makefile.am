NULL = 

lib_LTLIBRARIES = libglbarcode-4.0.la

libglbarcode_4_0_la_SOURCES = \
	factory.vala \
	barcode.vala \
	barcode_code39.vala \
	barcode_code39_extended.vala \
	barcode_onecode.vala \
	barcode_postnet.vala \
	shape.vala \
	renderer.vala \
	cairo_renderer.vala \
	constants.vala \
	$(NULL)

INCLUDES = \
	-include config.h \
	$(LIBGLBARCODE_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DLIBGLBARCODE_BRANCH=\""$(LIBGLBARCODE_BRANCH)"\" \
	$(NULL)

VALAFLAGS = \
	--vapidir=$(srcdir)/../vapi \
	--pkg config \
	--pkg posix \
	--pkg gobject-2.0 \
	--pkg cairo \
	--pkg pangocairo \
	--header=libglbarcode-4.h \
	--vapi=libglbarcode-4.vapi \
	$(NULL)

EXTRA_DIST = \
	libglbarcode-4.h \
	libglbarcode-4.vapi \
	$(NULL)

DISTCLEANFILES = \
	$(NULL)

