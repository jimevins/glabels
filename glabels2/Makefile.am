## Process this file with automake to produce Makefile.in

LIB_BARCODE_DIR = barcode-0.98

SUBDIRS = po src data pixmaps doc

EXTRA_DIST = \
	README \
	COPYING \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS \
	TODO \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	glabels.spec.in \
	glabels.spec

dist-hook:
	if test -d $(LIB_BARCODE_DIR); then \
	  if test -f $(LIB_BARCODE_DIR)/Makefile; then \
	    (cd $(LIB_BARCODE_DIR); $(MAKE) distclean); \
	  fi; \
	  mkdir $(distdir)/$(LIB_BARCODE_DIR); \
	  cp -rp $(LIB_BARCODE_DIR)/* $(distdir)/$(LIB_BARCODE_DIR); \
	fi
