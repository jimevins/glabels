## Process this file with automake to produce Makefile.in

LIB_BARCODE_DIR = barcode-0.98

SUBDIRS = m4  po macros src doc

EXTRA_DIST = config.rpath  \
	glabels.desktop glabels.spec.in glabels.spec autogen.sh

Applicationsdir = $(datadir)/gnome/apps/Applications
Applications_DATA = glabels.desktop

pixmapdir=$(datadir)/pixmaps/glabels
configdir=$(datadir)/glabels

install-data-local:
	@$(NORMAL_INSTALL)
	if test -d $(srcdir)/pixmaps; then \
	  $(mkinstalldirs) $(pixmapdir); \
	  for pixmap in $(srcdir)/pixmaps/*; do \
	    if test -f $$pixmap; then \
	      $(INSTALL_DATA) $$pixmap $(pixmapdir); \
	    fi \
	  done \
	fi
	if test -d $(srcdir)/data; then \
	  $(mkinstalldirs) $(configdir); \
	  for file in $(srcdir)/data/*.template; do \
	    if test -f $$file; then \
	      $(INSTALL_DATA) $$file $(configdir); \
	    fi \
	  done \
	fi 

dist-hook:
	if test -d pixmaps; then \
	  mkdir $(distdir)/pixmaps; \
	  for pixmap in pixmaps/*; do \
	    if test -f $$pixmap; then \
	      cp -p $$pixmap $(distdir)/pixmaps; \
	    fi \
	  done \
	fi
	if test -d data; then \
	  mkdir $(distdir)/data; \
	  for file in data/*.template; do \
	    if test -f $$file; then \
	      cp -p $$file $(distdir)/data; \
	    fi \
	  done \
	fi
	if test -d $(LIB_BARCODE_DIR); then \
	  if test -f $(LIB_BARCODE_DIR)/Makefile; then \
	    (cd $(LIB_BARCODE_DIR); $(MAKE) distclean); \
	  fi; \
	  mkdir $(distdir)/$(LIB_BARCODE_DIR); \
	  cp -rp $(LIB_BARCODE_DIR)/* $(distdir)/$(LIB_BARCODE_DIR); \
	fi

ACLOCAL_AMFLAGS = -I m4
