LIB_BARCODE_DIR = ../barcode-0.98

SUBDIRS= pixmaps stock-pixmaps

bin_PROGRAMS = glabels glabels-batch

uidir = $(datadir)/glabels/ui/


INCLUDES = \
	-I$(top_srcdir) 				\
	-I$(top_builddir) 				\
	$(GLABELS_CFLAGS) 				\
	-I$(LIB_BARCODE_DIR)				\
	-DPREFIX=\""$(prefix)"\"                        \
	-DSYSCONFDIR=\""$(sysconfdir)"\"                \
	-DDATADIR=\""$(datadir)"\"                      \
	-DLIBDIR=\""$(libdir)"\"                        \
	-DGLABELS_LOCALEDIR=\""$(datadir)/locale"\" 	\
	-DGLABELS_UI_DIR=\""$(datadir)/glabels/ui/"\" 	\
	-DG_DISABLE_DEPRECATED				\
	-DGDK_DISABLE_DEPRECATED			\
	-DGTK_DISABLE_DEPRECATED			\
	-DGDK_PIXBUF_DISABLE_DEPRECATED			\
	-DGNOME_DISABLE_DEPRECATED			

glabels_LDFLAGS = -export-dynamic

glabels_LDADD = 			\
	$(GLABELS_LIBS)			\
	-L$(LIB_BARCODE_DIR) -lbarcode	

glabels_batch_LDFLAGS = -export-dynamic

glabels_batch_LDADD = 			\
	$(GLABELS_LIBS)			\
	-L$(LIB_BARCODE_DIR) -lbarcode	

BUILT_SOURCES = 			\
	marshal.c			\
	marshal.h			\
	gnome-recent-marshal.c		\
	gnome-recent-marshal.h

glabels_SOURCES = 			\
	glabels.c			\
	splash.c 			\
	splash.h 			\
	window.c			\
	window.h			\
	stock.c				\
	stock.h				\
	ui.h				\
	ui.c				\
	commands.h			\
	commands.c			\
	file.h				\
	file.c				\
	recent.h			\
	recent.c			\
	tools.h				\
	tools.c				\
	prefs.c 			\
	prefs.h 			\
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
	print.c				\
	print.h				\
	print-dialog.c			\
	print-dialog.h			\
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
	template.c 			\
	template.h 			\
	xml-label.c			\
	xml-label.h			\
	xml-label-04.c			\
	xml-label-04.h			\
	merge.c				\
	merge.h				\
	merge-init.c			\
	merge-init.h			\
	merge-text.c			\
	merge-text.h			\
	text-node.c			\
	text-node.h			\
	wdgt-text-props.c		\
	wdgt-text-props.h		\
	wdgt-text-entry.c		\
	wdgt-text-entry.h		\
	wdgt-position.c			\
	wdgt-position.h			\
	wdgt-size.c			\
	wdgt-size.h			\
	wdgt-line.c			\
	wdgt-line.h			\
	wdgt-fill.c			\
	wdgt-fill.h			\
	wdgt-vector.c			\
	wdgt-vector.h			\
	wdgt-bc-props.c			\
	wdgt-bc-props.h			\
	wdgt-bc-style.c			\
	wdgt-bc-style.h			\
	wdgt-bc-data.c			\
	wdgt-bc-data.h			\
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
	util.c				\
	util.h				\
	color.c				\
	color.h				\
	canvas-hacktext.c		\
	canvas-hacktext.h		\
	gnome-recent-model.c		\
	gnome-recent-model.h		\
	gnome-recent-util.c		\
	gnome-recent-util.h		\
	gnome-recent-view.c		\
	gnome-recent-view.h		\
	gnome-recent-view-bonobo.c	\
	gnome-recent-view-bonobo.h	\
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
	template.c 			\
	template.h 			\
	xml-label.c			\
	xml-label.h			\
	xml-label-04.c			\
	xml-label-04.h			\
	merge.c				\
	merge.h				\
	merge-init.c			\
	merge-init.h			\
	merge-text.c			\
	merge-text.h			\
	text-node.c			\
	text-node.h			\
	prefs.c 			\
	prefs.h 			\
	util.c				\
	util.h				\
	debug.c 			\
	debug.h 			\
	$(BUILT_SOURCES)

marshal.h: marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=gl_marshal > $@

marshal.c: marshal.list $(GLIB_GENMARSHAL)
	echo "#include \"marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=gl_marshal >> $@

gnome-recent-marshal.h: gnome-recent-marshal.list $(GLIB_GENMARSHAL)
	$(GLIB_GENMARSHAL) $< --header --prefix=gnome_recent > $@

gnome-recent-marshal.c: gnome-recent-marshal.list $(GLIB_GENMARSHAL)
	echo "#include \"gnome-recent-marshal.h\"" > $@ && \
	$(GLIB_GENMARSHAL) $< --body --prefix=gnome_recent >> $@


ui_DATA = glabels-ui.xml

EXTRA_DIST = \
	$(ui_DATA)			\
	marshal.list			\
	gnome-recent-marshal.list

CLEANFILES = $(BUILT_SOURCES)

$(bin_PROGRAMS): $(LIB_BARCODE_DIR)/libbarcode.a

$(LIB_BARCODE_DIR)/libbarcode.a:
	cd $(LIB_BARCODE_DIR); $(MAKE)

clean: barcode_clean

barcode_clean:
	cd $(LIB_BARCODE_DIR); $(MAKE) clean

distclean maintainer-clean: barcode_distclean

barcode_distclean:
	cd $(LIB_BARCODE_DIR); $(MAKE) distclean

