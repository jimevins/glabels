
CURSOR_IMAGES =	 			\
	cursor_barcode.png		\
	cursor_box.png			\
	cursor_ellipse.png		\
	cursor_image.png		\
	cursor_line.png			\
	cursor_text.png

VARIABLES1 = \
	cursor_barcode_pixdata	$(srcdir)/cursor_barcode.png	\
	cursor_box_pixdata	$(srcdir)/cursor_box.png	\
	cursor_ellipse_pixdata	$(srcdir)/cursor_ellipse.png	\
	cursor_image_pixdata	$(srcdir)/cursor_image.png	\
	cursor_line_pixdata	$(srcdir)/cursor_line.png	\
	cursor_text_pixdata	$(srcdir)/cursor_text.png

noinst_DATA = cursor_pixdata.h
CLEANFILES = $(noinst_DATA)

cursor_pixdata.h: $(CURSOR_IMAGES)
	$(AM_V_GEN) $(GDK_PIXBUF_CSOURCE) \
	   --struct --build-list $(VARIABLES1) > $(srcdir)/cursor_pixdata.h || \
           ( rm -f $(srcdir)/cursor_pixdata.h && false )

EXTRA_DIST = $(CURSOR_IMAGES) cursor_pixdata.h
