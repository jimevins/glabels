
IMAGES = 	 			\
	stock_arrow_24.png		\
	stock_arrow_16.png		\
	stock_barcode_24.png	 	\
	stock_barcode_16.png	 	\
	stock_box_24.png		\
	stock_box_16.png		\
	stock_ellipse_24.png	 	\
	stock_ellipse_16.png	 	\
	stock_image_24.png	 	\
	stock_image_16.png	 	\
	stock_line_24.png	 	\
	stock_line_16.png	 	\
	stock_merge_24.png 		\
	stock_merge_16.png 		\
	stock_text_24.png		\
	stock_text_16.png		\
	stock_order_top_16.png		\
	stock_order_bottom_16.png	\
	stock_rotate_left_16.png	\
	stock_rotate_right_16.png	\
	stock_flip_horiz_16.png		\
	stock_flip_vert_16.png		\
	stock_align_left_16.png		\
	stock_align_right_16.png	\
	stock_align_hcenter_16.png	\
	stock_align_top_16.png		\
	stock_align_bottom_16.png	\
	stock_align_vcenter_16.png

VARIABLES1 = \
		stock_arrow_24 		$(srcdir)/stock_arrow_24.png	\
		stock_arrow_16 		$(srcdir)/stock_arrow_16.png	\
		stock_barcode_24	$(srcdir)/stock_barcode_24.png	\
		stock_barcode_16	$(srcdir)/stock_barcode_16.png	\
		stock_box_24   		$(srcdir)/stock_box_24.png	\
		stock_box_16   		$(srcdir)/stock_box_16.png	\
		stock_ellipse_24	$(srcdir)/stock_ellipse_24.png	\
		stock_ellipse_16	$(srcdir)/stock_ellipse_16.png	\
		stock_image_24 		$(srcdir)/stock_image_24.png	\
		stock_image_16 		$(srcdir)/stock_image_16.png	\
		stock_line_24  		$(srcdir)/stock_line_24.png	\
		stock_line_16  		$(srcdir)/stock_line_16.png	\
		stock_merge_24 		$(srcdir)/stock_merge_24.png	\
		stock_merge_16 		$(srcdir)/stock_merge_16.png	\
		stock_text_24  		$(srcdir)/stock_text_24.png     \
		stock_text_16  		$(srcdir)/stock_text_16.png	\
		stock_order_top_16	$(srcdir)/stock_order_top_16.png \
		stock_order_bottom_16	$(srcdir)/stock_order_bottom_16.png \
		stock_rotate_left_16	$(srcdir)/stock_rotate_left_16.png \
		stock_rotate_right_16	$(srcdir)/stock_rotate_right_16.png \
		stock_flip_horiz_16	$(srcdir)/stock_flip_horiz_16.png \
		stock_flip_vert_16	$(srcdir)/stock_flip_vert_16.png \
		stock_align_left_16	$(srcdir)/stock_align_left_16.png \
		stock_align_right_16	$(srcdir)/stock_align_right_16.png \
		stock_align_hcenter_16	$(srcdir)/stock_align_hcenter_16.png \
		stock_align_top_16	$(srcdir)/stock_align_top_16.png \
		stock_align_bottom_16	$(srcdir)/stock_align_bottom_16.png \
		stock_align_vcenter_16	$(srcdir)/stock_align_vcenter_16.png

noinst_DATA = stockpixbufs.h
CLEANFILES = $(noinst_DATA)

stockpixbufs.h: $(IMAGES)
	gdk-pixbuf-csource						  \
	   --raw --build-list $(VARIABLES1) > $(srcdir)/stockpixbufs.h || \
           ( rm -f $(srcdir)/stockpixbufs.h && false )

EXTRA_DIST = $(IMAGES) stockpixbufs.h
