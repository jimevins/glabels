LIB_BARCODE_DIR = ../barcode-0.96

SUBDIRS = pixmaps

INCLUDES = \
	-I$(top_srcdir)/intl \
	$(GNOME_INCLUDEDIR) \
	$(GNOME_XML_CFLAGS) \
	$(GNOME_PRINT_CFLAGS) \
	$(GDK_PIXBUF_CFLAGS) \
	$(GNOME_CANVAS_PIXBUF_CFLAGS) \
	-I$(LIB_BARCODE_DIR)

bin_PROGRAMS = glabels glabels-batch

glabels_SOURCES = \
	glabels.c \
	splash.h \
	splash.c \
	mdi.h \
	mdi.c \
	display.c \
	display.h \
	item.c \
	item.h \
	item_text.c \
	item_text.h \
	item_line.c \
	item_line.h \
	item_box.c \
	item_box.h \
	item_ellipse.c \
	item_ellipse.h \
	item_image.c \
	item_image.h \
	item_barcode.c \
	item_barcode.h \
	highlight.c \
	highlight.h \
	propertybox.c \
	propertybox.h \
	prop_size.c \
	prop_size.h \
	prop_position.c \
	prop_position.h \
	prop_line.c \
	prop_line.h \
	prop_fill.c \
	prop_fill.h \
	prop_text.c \
	prop_text.h \
	prop_text_entry.c \
	prop_text_entry.h \
	prop_vector.c \
	prop_vector.h \
	prop_bc.c \
	prop_bc.h \
	prop_bc_style.c \
	prop_bc_style.h \
	prop_bc_data.c \
	prop_bc_data.h \
	label.h \
	label.c \
        template.h \
        template.c \
	text_node.h \
	text_node.c \
	merge_properties.h \
	merge_properties.c \
	merge.h \
	merge.c \
	merge_text.h \
	merge_text.c \
	merge_ui.h \
	merge_ui.c \
	merge_ui_text.h \
	merge_ui_text.c \
	bc.c \
	bc.h \
	bc_postnet.c \
	bc_postnet.h \
	bc_gnubarcode.c \
	bc_gnubarcode.h \
	tools.h \
	tools.c \
	stock.h \
	stock.c \
	menu.h \
	menu.c \
	file.c \
	file.h \
	media_select.c \
	media_select.h \
	rotate_select.c \
	rotate_select.h \
	print.c \
	print.h \
	print_dialog.c \
	print_dialog.h \
	print_copies.c \
	print_copies.h \
	print_merge.c \
	print_merge.h \
	mini_preview.c \
	mini_preview.h \
	edit.c \
	edit.h \
	prefs.h \
	prefs.c \
	util.c \
	util.h \
	hack.c \
	hack.h \
	gnome-canvas-hacktext.h \
	debug.h 

glabels_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(INTLLIBS) \
	$(GNOME_XML_LIBS) \
	$(GNOME_PRINT_LIBS) \
	$(GDK_PIXBUF_LIBS) \
	$(GNOME_CANVAS_PIXBUF_LIBS) \
	-L$(LIB_BARCODE_DIR) -lbarcode

glabels_batch_SOURCES = \
	glabels_batch.c \
	label.h \
	label.c \
        template.h \
        template.c \
	text_node.h \
	text_node.c \
	merge.h \
	merge.c \
	merge_text.h \
	merge_text.c \
	bc.c \
	bc.h \
	bc_postnet.c \
	bc_postnet.h \
	bc_gnubarcode.c \
	bc_gnubarcode.h \
	print.c \
	print.h \
	util.c \
	util.h \
	hack.c \
	hack.h \
	debug.h 

glabels_batch_LDADD = \
	$(GNOME_LIBDIR) \
	$(GNOMEUI_LIBS) \
	$(INTLLIBS) \
	$(GNOME_XML_LIBS) \
	$(GNOME_PRINT_LIBS) \
	$(GDK_PIXBUF_LIBS) \
	$(GNOME_CANVAS_PIXBUF_LIBS) \
	-L$(LIB_BARCODE_DIR) -lbarcode

$(bin_PROGRAMS): $(LIB_BARCODE_DIR)/libbarcode.a

$(LIB_BARCODE_DIR)/libbarcode.a:
	cd $(LIB_BARCODE_DIR); $(MAKE)

clean: barcode_clean

barcode_clean:
	cd $(LIB_BARCODE_DIR); $(MAKE) clean

distclean maintainer-clean: barcode_distclean

barcode_distclean:
	cd $(LIB_BARCODE_DIR); $(MAKE) distclean
