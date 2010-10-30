
SUBDIRS= ../libglabels ../libglbarcode pixmaps

bin_PROGRAMS = glabels-3 glabels-3-batch

INCLUDES = \
	-I$(top_builddir)/libglabels				\
	-I$(top_builddir)/libglbarcode				\
	$(GLABELS_CFLAGS) 					\
	$(LIBEBOOK_CFLAGS)					\
	$(LIBBARCODE_CFLAGS)					\
	$(LIBZINT_CFLAGS)					\
	$(LIBQRENCODE_CFLAGS)					\
	$(LIBIEC16022_CFLAGS)					\
	$(DISABLE_DEPRECATED_CFLAGS)				\
	-DGLABELS_LOCALE_DIR=\""$(datadir)/locale"\" 		\
	-DGLABELS_DATA_DIR=\""$(datadir)/$(GLABELS_BRANCH)"\"	\
	-DGLABELS_ICON_NAME=\""$(GLABELS_BRANCH)\""		\
	-DG_LOG_DOMAIN=\""glabels\""

glabels_3_LDFLAGS = -export-dynamic

glabels_3_LDADD = 				\
	$(GLABELS_LIBS)				\
	../libglabels/$(LIBGLABELS_BRANCH).la	\
	../libglbarcode/$(LIBGLBARCODE_BRANCH).la	\
	$(LIBEBOOK_LIBS)		 	\
	$(LIBBARCODE_LIBS)		 	\
	$(LIBZINT_LIBS)				\
	$(LIBQRENCODE_LIBS)			\
	$(LIBIEC16022_LIBS)			\
	-lm

glabels_3_batch_LDFLAGS = -export-dynamic

glabels_3_batch_LDADD = 			\
	$(GLABELS_LIBS)				\
	../libglabels/$(LIBGLABELS_BRANCH).la	\
	../libglbarcode/$(LIBGLBARCODE_BRANCH).la	\
	$(LIBEBOOK_LIBS)		 	\
	$(LIBBARCODE_LIBS)		 	\
	$(LIBZINT_LIBS)				\
	$(LIBQRENCODE_LIBS)			\
	$(LIBIEC16022_LIBS)			\
	-lm

BUILT_SOURCES = 			\
	marshal.c			\
	marshal.h			

glabels_3_SOURCES = 			\
	glabels.c			\
	warning-handler.c		\
	warning-handler.h		\
	critical-error-handler.c	\
	critical-error-handler.h	\
	window.c			\
	window.h			\
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
	view.c				\
	view.h				\
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
	bc-backends.c			\
	bc-backends.h			\
	bc-builtin.c			\
	bc-builtin.h			\
	bc-gnubarcode.c			\
	bc-gnubarcode.h			\
	bc-zint.c			\
	bc-zint.h 			\
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
	pixbuf-util.c			\
	pixbuf-util.h			\
	xml-label.c			\
	xml-label.h			\
	xml-label-04.c			\
	xml-label-04.h			\
	pixbuf-cache.c			\
	pixbuf-cache.h			\
	svg-cache.c			\
	svg-cache.h			\
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
	media-select.c			\
	media-select.h			\
	message-bar.c			\
	message-bar.h			\
	template-history.c		\
	template-history.h		\
	template-history-model.c	\
	template-history-model.h	\
	mini-preview.c			\
	mini-preview.h			\
	mini-label-preview.c		\
	mini-label-preview.h		\
	wdgt-chain-button.c		\
	wdgt-chain-button.h		\
	field-button.c			\
	field-button.h			\
	field-button-menu.c		\
	field-button-menu.h		\
	color-combo.c			\
	color-combo.h			\
	color-combo-button.c		\
	color-combo-button.h		\
	color-combo-menu.c		\
	color-combo-menu.h		\
	color-combo-color-menu-item.c	\
	color-combo-color-menu-item.h	\
	color-history-model.c		\
	color-history-model.h		\
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
	font-history.c			\
	font-history.h			\
	font-history-model.c		\
	font-history-model.h		\
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
	units-util.c			\
	units-util.h			\
	str-util.c			\
	str-util.h			\
	color.c				\
	color.h				\
	debug.c 			\
	debug.h 			\
	$(BUILT_SOURCES)


glabels_3_batch_SOURCES = 		\
	glabels-batch.c			\
	file-util.h			\
	file-util.c			\
	print.c				\
	print.h				\
	print-op.c			\
	print-op.h			\
	bc-backends.c			\
	bc-backends.h			\
	bc-builtin.c			\
	bc-builtin.h			\
	bc-gnubarcode.c			\
	bc-gnubarcode.h			\
	bc-zint.c			\
	bc-zint.h			\
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
	pixbuf-util.c			\
	pixbuf-util.h			\
	xml-label.c			\
	xml-label.h			\
	xml-label-04.c			\
	xml-label-04.h			\
	pixbuf-cache.c			\
	pixbuf-cache.h			\
	svg-cache.c			\
	svg-cache.h			\
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
	font-history.c			\
	font-history.h			\
	font-history-model.c		\
	font-history-model.h		\
	template-history.c		\
	template-history.h		\
	template-history-model.c	\
	template-history-model.h	\
	str-util.c			\
	str-util.h			\
	color.c				\
	color.h				\
	debug.c 			\
	debug.h 			\
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

$(bin_PROGRAMS): ../libglabels/$(LIBGLABELS_BRANCH).la ../libglbarcode/$(LIBGLBARCODE_BRANCH).la

../libglabels/$(LIBGLABELS_BRANCH).la:
	cd ../libglabels; $(MAKE)

../libglbarcode/$(LIBGLBARCODE_BRANCH).la:
	cd ../libglbarcode; $(MAKE)

