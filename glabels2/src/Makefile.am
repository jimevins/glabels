LIB_BARCODE_DIR = ../barcode-0.98

SUBDIRS= pixmaps stock-pixmaps mygal recent-files

bin_PROGRAMS = glabels glabels-batch

INCLUDES = \
	-I$(top_srcdir) 					\
	-I$(top_builddir) 					\
	$(GLABELS_CFLAGS) 					\
	-I$(LIB_BARCODE_DIR)					\
	-DPREFIX=\""$(prefix)"\"                        	\
	-DSYSCONFDIR=\""$(sysconfdir)"\"                	\
	-DDATADIR=\""$(datadir)"\"                      	\
	-DLIBDIR=\""$(libdir)"\"                        	\
	-DGLABELS_LOCALEDIR=\""$(datadir)/locale"\" 		\
	-DGLABELS_GLADE_DIR=\""$(datadir)/glabels/glade/"\"	\
	-DG_LOG_DOMAIN=\""glabels\""				\
	$(DISABLE_DEPRECATED_CFLAGS)

glabels_LDFLAGS = -export-dynamic

glabels_LDADD = 			\
	$(GLABELS_LIBS)			\
	../libglabels/libglabels.la	\
	mygal/libmygal.la		\
	recent-files/librecent.la	\
	$(LIB_BARCODE_DIR)/libbarcode.la

glabels_batch_LDFLAGS = -export-dynamic

glabels_batch_LDADD = 			\
	$(GLABELS_LIBS)			\
	../libglabels/libglabels.la	\
	$(LIB_BARCODE_DIR)/libbarcode.la

BUILT_SOURCES = 			\
	marshal.c			\
	marshal.h			

glabels_SOURCES = 			\
	glabels.c			\
	splash.c 			\
	splash.h 			\
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
	recent.h			\
	recent.c			\
	prefs.c 			\
	prefs.h 			\
	prefs-model.c 			\
	prefs-model.h 			\
	prefs-dialog.c 			\
	prefs-dialog.h 			\
	hig.c				\
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
	view-highlight.c		\
	view-highlight.h		\
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
	print.c				\
	print.h				\
	print-dialog.c			\
	print-dialog.h			\
	template-designer.c		\
	template-designer.h		\
	bc.c				\
	bc.h				\
	bc-gnubarcode.c			\
	bc-gnubarcode.h			\
	bc-postnet.c			\
	bc-postnet.h			\
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
	xml-label-191.c			\
	xml-label-191.h			\
	pixbuf-cache.c			\
	pixbuf-cache.h			\
	base64.c			\
	base64.h			\
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
	wdgt-print-copies.c		\
	wdgt-print-copies.h		\
	wdgt-print-merge.c		\
	wdgt-print-merge.h		\
	wdgt-media-select.c		\
	wdgt-media-select.h		\
	wdgt-mini-preview.c		\
	wdgt-mini-preview.h		\
	wdgt-rotate-label.c		\
	wdgt-rotate-label.h		\
	wdgt-chain-button.c		\
	wdgt-chain-button.h		\
	util.c				\
	util.h				\
	color.c				\
	color.h				\
	canvas-hacktext.c		\
	canvas-hacktext.h		\
	debug.c 			\
	debug.h 			\
	$(BUILT_SOURCES)

glabels_batch_SOURCES = 		\
	glabels-batch.c			\
	print.c				\
	print.h				\
	bc.c				\
	bc.h				\
	bc-gnubarcode.c			\
	bc-gnubarcode.h			\
	bc-postnet.c			\
	bc-postnet.h			\
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
	xml-label-191.c			\
	xml-label-191.h			\
	pixbuf-cache.c			\
	pixbuf-cache.h			\
	base64.c			\
	base64.h			\
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
	util.c				\
	util.h				\
	color.c				\
	color.h				\
	debug.c 			\
	debug.h 			\
	$(BUILT_SOURCES)

marshal.h: marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=gl_marshal > $@

marshal.c: marshal.list $(GLIB_GENMARSHAL)
	echo "#include \"marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=gl_marshal >> $@

EXTRA_DIST = \
	marshal.list			

CLEANFILES = $(BUILT_SOURCES)

$(bin_PROGRAMS): $(LIB_BARCODE_DIR)/libbarcode.la

$(LIB_BARCODE_DIR)/libbarcode.la:
	cd $(LIB_BARCODE_DIR); $(MAKE)

../libglabels/libglabels.la:
	cd ../libglabels; $(MAKE)

