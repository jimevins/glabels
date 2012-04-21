NULL = 

SUBDIRS = \
	cursors \
	$(NULL)

bin_PROGRAMS = glabels-4

glabels_4_SOURCES = \
	glabels.vala \
	TMP_gdk_key.vala \
	color.vala \
	color_button.vala \
	color_history.vala \
	color_menu.vala \
	color_menu_item.vala \
	color_node.vala \
	color_swatch.vala \
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
	label_object_box.vala \
	label_region.vala \
	label_state.vala \
	merge.vala \
	merge_factory.vala \
	merge_field.vala \
	merge_none.vala \
	merge_record.vala \
	merge_text.vala \
	message_bar.vala \
	model.vala \
	model_clipboard.vala \
	model_print.vala \
	model_undo_redo.vala \
	mini_preview.vala \
	new_label_dialog.vala \
	object_editor.vala \
	prefs.vala \
	print_op.vala \
	print_op_dialog.vala \
	property_editor.vala \
	template_history.vala \
	ui.vala \
	units_util.vala \
	view.vala \
	window.vala \
	xml_label.vala \
	$(NULL)


INCLUDES = \
	-include config.h \
	$(GLABELS_CFLAGS) \
	-I../libglabels \
	-DLOCALEDIR=\""$(localedir)"\" \
	-DDATADIR=\""$(datadir)"\" \
	-DGLABELS_BRANCH=\""$(GLABELS_BRANCH)"\" \
	$(NULL)

VALAFLAGS = \
	--vapidir=$(srcdir)/../vapi \
	--vapidir=$(srcdir)/../libglabels \
	--pkg config \
	--pkg posix \
	--pkg gtk+-3.0 \
	--pkg libxml-2.0 \
	--pkg gee-1.0 \
	--pkg libglabels-4 \
	$(NULL)


glabels_4_LDADD = \
	-L../libglabels -lglabels-4.0 \
	$(GLABELS_LIBS) \
	$(NULL)


EXTRA_DIST = \
	$(NULL)

DISTCLEANFILES = \
	$(NULL)


$(bin_PROGRAMS): libglabels

libglabels:
	cd ../libglabels; $(MAKE)
