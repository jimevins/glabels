NULL = 

SUBDIRS = \
	cursors \
	$(NULL)

bin_PROGRAMS = glabels-4

glabels_4_SOURCES = \
	TMP_gdk_pixdata.vapi \
	glabels.vala \
	barcode_backends.vala \
	barcode_backends/gnu_barcode.vala \
	barcode_backends/iec16022.vala \
	barcode_backends/libbarcode.vapi \
	barcode_backends/libiec16022.vapi \
	barcode_backends/libqrencode.vapi \
	barcode_backends/libzint.vapi \
	barcode_backends/qrencode.vala \
	barcode_backends/zint.vala \
	barcode_menu.vala \
	barcode_menu_button.vala \
	barcode_menu_item.vala \
	barcode_style.vala \
	color.vala \
	color_button.vala \
	color_history.vala \
	color_menu.vala \
	color_menu_item.vala \
	color_node.vala \
	color_swatch.vala \
	combo_util.vala \
	cursor.vapi \
	enum_util.vala \
	field_button.vala \
	field_menu.vala \
	field_menu_item.vala \
	file.vala \
	file_util.vala \
	font_button.vala \
	font_families.vala \
	font_history.vala \
	font_menu.vala \
	font_menu_item.vala \
	font_sample.vala \
	help.vala \
	handle.vala \
	label.vala \
	label_object.vala \
	label_object_barcode.vala \
	label_object_box.vala \
	label_object_ellipse.vala \
	label_object_image.vala \
	label_object_line.vala \
	label_object_shape.vala \
	label_object_text.vala \
	label_region.vala \
	label_state.vala \
	merge.vala \
	merge_backends.vala \
	merge_backends/merge_none.vala \
	merge_backends/merge_text.vala \
	merge_field.vala \
	merge_info.vala \
	merge_record.vala \
	message_bar.vala \
	model.vala \
	model_clipboard.vala \
	model_print.vala \
	model_undo_redo.vala \
	mini_preview.vala \
	new_label_dialog.vala \
	object_editor.vala \
	outline.vala \
	pixbuf_cache.vala \
	pixmaps.vapi \
	prefs.vala \
	print_op.vala \
	print_op_dialog.vala \
	property_editor.vala \
	svg_cache.vala \
	template_history.vala \
	text_line.vala \
	text_lines.vala \
	text_node.vala \
	ui.vala \
	units_util.vala \
	valign_type.vala \
	view.vala \
	window.vala \
	xml_label.vala \
	$(NULL)


INCLUDES = \
	-include config.h \
	$(GLABELS_CFLAGS) \
	-I../libglabels \
	-I../libglbarcode \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DGLABELS_BRANCH=\""$(GLABELS_BRANCH)"\" \
	$(LIBBARCODE_CFLAGS) \
	$(LIBIEC16022_CFLAGS) \
	$(LIBQRENCODE_CFLAGS) \
	$(LIBZINT_CFLAGS) \
	$(NULL)

VALAFLAGS = \
	--vapidir=$(srcdir)/../vapi \
	--vapidir=$(srcdir)/../libglabels \
	--vapidir=$(srcdir)/../libglbarcode \
	--pkg config \
	--pkg posix \
	--pkg gtk+-3.0 \
	--pkg libxml-2.0 \
	--pkg gee-1.0 \
	--pkg librsvg-2.0 \
	--pkg libglabels-4 \
	--pkg libglbarcode-4 \
	$(LIBBARCODE_VALAFLAGS) \
	$(LIBIEC16022_VALAFLAGS) \
	$(LIBQRENCODE_VALAFLAGS) \
	$(LIBZINT_VALAFLAGS) \
	$(NULL)


glabels_4_LDADD = \
	-L../libglabels -lglabels-4.0 \
	-L../libglbarcode -lglbarcode-4.0 \
	$(GLABELS_LIBS) \
	$(LIBBARCODE_LIBS) \
	$(LIBIEC16022_LIBS) \
	$(LIBQRENCODE_LIBS) \
	$(LIBZINT_LIBS) \
	$(NULL)


EXTRA_DIST = \
	$(NULL)

DISTCLEANFILES = \
	$(NULL)


$(bin_PROGRAMS): libglabels libglbarcode

libglabels:
	cd ../libglabels; $(MAKE)

libglbarcode:
	cd ../libglbarcode; $(MAKE)
