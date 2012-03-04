NULL = 

lib_LTLIBRARIES = libglabels-4.0.la

libglabels_4_0_la_SOURCES = \
	category.vala \
	db.vala \
	mini_preview_pixbuf.vala \
	paper.vala \
	str_util.vala \
	template_coord.vala \
	template_frame_cd.vala \
	template_frame_ellipse.vala \
	template_frame_rect.vala \
	template_frame_round.vala \
	template_frame.vala \
	template_layout.vala \
	template_markup_circle.vala \
	template_markup_ellipse.vala \
	template_markup_line.vala \
	template_markup_margin.vala \
	template_markup_rect.vala \
	template_markup.vala \
	template.vala \
	units.vala \
	vendor.vala \
	xml_category.vala \
	xml_paper.vala \
	xml_template.vala \
	xml_util.vala \
	xml_vendor.vala \
	$(NULL)

INCLUDES = \
	-include config.h \
	$(LIBGLABELS_CFLAGS) \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DLIBGLABELS_BRANCH=\""$(LIBGLABELS_BRANCH)"\" \
	$(NULL)

VALAFLAGS = \
	--vapidir=$(srcdir)/../vapi \
	--pkg config \
	--pkg posix \
	--pkg gobject-2.0 \
	--pkg libxml-2.0 \
	--pkg cairo \
	--pkg gdk-pixbuf-2.0 \
	--pkg gee-1.0 \
	--header=libglabels-4.h \
	--vapi=libglabels-4.vapi \
	$(NULL)

EXTRA_DIST = \
	libglabels-4.h \
	libglabels-4.vapi \
	$(NULL)

DISTCLEANFILES = \
	$(NULL)

