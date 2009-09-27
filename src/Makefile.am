LIB_BARCODE_DIR = ../barcode-0.98
LIB_IEC16022_DIR = ../iec16022-0.2.1
LIB_QRENCODE_DIR = ../qrencode-3.1.0

SUBDIRS= pixmaps stock-pixmaps

bin_PROGRAMS = glabels glabels-batch

INCLUDES = \
	-I$(top_srcdir) 					\
	-I$(top_builddir) 					\
	-I$(LIB_BARCODE_DIR)					\
	-I$(LIB_IEC16022_DIR)					\
	-I$(LIB_QRENCODE_DIR)					\
	$(GLABELS_CFLAGS) 					\
	$(DISABLE_DEPRECATED_CFLAGS)				\
	-DPREFIX=\""$(prefix)"\"                        	\
	-DSYSCONFDIR=\""$(sysconfdir)"\"                	\
	-DDATADIR=\""$(datadir)"\"                      	\
	-DLIBDIR=\""$(libdir)"\"                        	\
	-DGLABELS_LOCALEDIR=\""$(datadir)/locale"\" 		\
	-DGLABELS_BUILDER_DIR=\""$(datadir)/glabels/builder/"\"	\
	-DGLABELS_ICON_DIR=\""$(datadir)/pixmaps/"\"	\
	-DGLABELS_PIXMAP_DIR=\""$(datadir)/pixmaps/glabels/"\"	\
	-DG_LOG_DOMAIN=\""glabels\""

glabels_LDFLAGS = -export-dynamic

glabels_LDADD = 				\
	$(GLABELS_LIBS)				\
	../libglabels/libglabels.la		\
	$(LIB_BARCODE_DIR)/libbarcode.la 	\
	$(LIB_IEC16022_DIR)/libiec16022.la	\
	$(LIB_QRENCODE_DIR)/libqrencode.la

glabels_batch_LDFLAGS = -export-dynamic

glabels_batch_LDADD = 				\
	$(GLABELS_LIBS)				\
	../libglabels/libglabels.la		\
	$(LIB_BARCODE_DIR)/libbarcode.la	\
	$(LIB_IEC16022_DIR)/libiec16022.la	\
	$(LIB_QRENCODE_DIR)/libqrencode.la

BUILT_SOURCES = 			\
	marshal.c			\
	marshal.h			

glabels_SOURCES = 			\
	glabels.c			\
	warning-handler.c		\
	warning-handler.h		\
	critical-error-handler.c	\
	critical-error-handler.h	\
	window.c			\
	window.h			\
	stock.c				\
	stock.h				\
	ui.h				\
	ui.c				\
	ui-util.h			\
	ui-util.c			\
	ui-commands.h			\
	ui-commands.c			\
	ui-property-bar.h		\
	ui-property-bar.c		\
	ui-sidebar.h			\
	ui-sidebar.c			\
	file.h				\
	file.c				\
	file-util.h			\
	file-util.c			\
	new-label-dialog.h		\
	new-label-dialog.c		\
	recent.h			\
	recent.c			\
	prefs.c 			\
	prefs.h 			\
	prefs-model.c 			\
	prefs-model.h 			\
	prefs-dialog.c 			\
	prefs-dialog.h 			\
	hig.h				\
	view.c				\
	view.h				\
	view-object.c			\
	view-object.h			\
	view-box.c			\
	view-box.h			\
	view-ellipse.c			\
	view-ellipse.h			\
	view-line.c			\
	view-line.h			\
	view-image.c			\
	view-image.h			\
	view-text.c			\
	view-text.h			\
	view-barcode.c			\
	view-barcode.h			\
	merge-properties-dialog.c	\
	merge-properties-dialog.h	\
	object-editor.c			\
	object-editor.h			\
	object-editor-private.h		\
	object-editor-position-page.c	\
	object-editor-size-page.c	\
	object-editor-lsize-page.c	\
	object-editor-line-page.c	\
	object-editor-fill-page.c	\
	object-editor-image-page.c	\
	object-editor-text-page.c	\
	object-editor-edit-page.c	\
	object-editor-bc-page.c		\
	object-editor-data-page.c	\
	object-editor-shadow-page.c	\
	print.c				\
	print.h				\
	print-op.c			\
	print-op.h			\
	print-op-dialog.c		\
	print-op-dialog.h		\
	template-designer.c		\
	template-designer.h		\
	bc.c				\
	bc.h				\
	bc-gnubarcode.c			\
	bc-gnubarcode.h			\
	bc-postnet.c			\
	bc-postnet.h			\
	bc-iec16022.c			\
	bc-iec16022.h			\
	bc-iec18004.c			\
	bc-iec18004.h			\
	label.c				\
	label.h				\
	label-object.c			\
	label-object.h			\
	label-text.c			\
	label-text.h			\
	label-box.c			\
	label-box.h			\
	label-line.c			\
	label-line.h			\
	label-ellipse.c			\
	label-ellipse.h			\
	label-image.c			\
	label-image.h			\
	label-barcode.c			\
	label-barcode.h			\
	xml-label.c			\
	xml-label.h			\
	xml-label-04.c			\
	xml-label-04.h			\
	pixbuf-cache.c			\
	pixbuf-cache.h			\
	merge.c				\
	merge.h				\
	merge-init.c			\
	merge-init.h			\
	merge-text.c			\
	merge-text.h			\
	merge-evolution.c		\
	merge-evolution.h		\
	merge-vcard.c			\
	merge-vcard.h			\
	text-node.c			\
	text-node.h			\
	mini-preview-pixbuf.c		\
	mini-preview-pixbuf.h		\
	mini-preview-pixbuf-cache.c	\
	mini-preview-pixbuf-cache.h	\
	wdgt-media-select.c		\
	wdgt-media-select.h		\
	mini-preview.c			\
	mini-preview.h			\
	mini-label-preview.c		\
	mini-label-preview.h		\
	rotate-label-button.c		\
	rotate-label-button.h		\
	wdgt-chain-button.c		\
	wdgt-chain-button.h		\
	wdgt-merge-menu.c		\
	wdgt-merge-menu.h		\
	color-combo.c			\
	color-combo.h			\
	color-combo-button.c		\
	color-combo-button.h		\
	color-combo-menu.c		\
	color-combo-menu.h		\
	color-combo-color-menu-item.c	\
	color-combo-color-menu-item.h	\
	color-combo-history.c		\
	color-combo-history.h		\
	color-swatch.c			\
	color-swatch.h			\
	font-combo.c			\
	font-combo.h			\
	font-combo-menu.c		\
	font-combo-menu.h		\
	font-combo-menu-item.c		\
	font-combo-menu-item.h		\
	font-sample.c			\
	font-sample.h			\
	font-util.c			\
	font-util.h			\
	cairo-label-path.c		\
	cairo-label-path.h		\
	cairo-markup-path.c		\
	cairo-markup-path.h		\
	cairo-ellipse-path.c		\
	cairo-ellipse-path.h		\
	combo-util.c			\
	combo-util.h			\
	builder-util.c			\
	builder-util.h			\
	str-util.c			\
	str-util.h			\
	color.c				\
	color.h				\
	debug.c 			\
	debug.h 			\
	$(BUILT_SOURCES)


glabels_batch_SOURCES = 		\
	glabels-batch.c			\
	file-util.h			\
	file-util.c			\
	print.c				\
	print.h				\
	print-op.c			\
	print-op.h			\
	bc.c				\
	bc.h				\
	bc-gnubarcode.c			\
	bc-gnubarcode.h			\
	bc-postnet.c			\
	bc-postnet.h			\
	bc-iec16022.c			\
	bc-iec16022.h			\
	bc-iec18004.c			\
	bc-iec18004.h			\
	label.c				\
	label.h				\
	label-object.c			\
	label-object.h			\
	label-text.c			\
	label-text.h			\
	label-box.c			\
	label-box.h			\
	label-line.c			\
	label-line.h			\
	label-ellipse.c			\
	label-ellipse.h			\
	label-image.c			\
	label-image.h			\
	label-barcode.c			\
	label-barcode.h			\
	xml-label.c			\
	xml-label.h			\
	xml-label-04.c			\
	xml-label-04.h			\
	pixbuf-cache.c			\
	pixbuf-cache.h			\
	merge.c				\
	merge.h				\
	merge-init.c			\
	merge-init.h			\
	merge-text.c			\
	merge-text.h			\
	merge-evolution.c		\
	merge-evolution.h		\
	merge-vcard.c			\
	merge-vcard.h			\
	text-node.c			\
	text-node.h			\
	prefs.c 			\
	prefs.h 			\
	prefs-model.c 			\
	prefs-model.h 			\
	font-util.c			\
	font-util.h			\
	str-util.c			\
	str-util.h			\
	color.c				\
	color.h				\
	debug.c 			\
	debug.h 			\
	hig.h				\
	cairo-label-path.c		\
	cairo-label-path.h		\
	cairo-ellipse-path.c		\
	cairo-ellipse-path.h		\
	$(BUILT_SOURCES)

marshal.h: marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) $(GLIB_GENMARSHAL) $< --header --prefix=gl_marshal > $@

marshal.c: marshal.list $(GLIB_GENMARSHAL)
	$(AM_V_GEN) echo "#include \"marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=gl_marshal >> $@

EXTRA_DIST = \
	marshal.list			

CLEANFILES = $(BUILT_SOURCES)

$(bin_PROGRAMS): $(LIB_BARCODE_DIR)/libbarcode.la \
		 $(LIB_IEC16022_DIR)/libiec16022.la \
		 ../libglabels/libglabels.la

$(LIB_BARCODE_DIR)/libbarcode.la:
	cd $(LIB_BARCODE_DIR); $(MAKE)

$(LIB_IEC16022_DIR)/libiec16022.la:
	cd $(LIB_IEC16022_DIR); $(MAKE)

$(LIB_QRENCODE_DIR)/libqrencode.la:
	cd $(LIB_QRENCODE_DIR); $(MAKE)

../libglabels/libglabels.la:
	cd ../libglabels; $(MAKE)

