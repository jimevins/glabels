## Process this file with automake to produce Makefile.in

desktopdir 	 = $(datadir)/applications
desktop_in_files = glabels.desktop.in
desktop_DATA 	 = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

runtime_files = \
	paper-sizes.xml \
	predefined-templates.xml \
	glabels-2.0.dtd

EXTRA_DIST = \
	$(desktop_DATA) \
	$(runtime_files)

configdir=$(datadir)/glabels
install-data-local:
	  $(mkinstalldirs) $(configdir); \
	  for file in $(runtime_files); do \
	    if test -f $$file; then \
	      $(INSTALL_DATA) $$file $(configdir); \
	    fi \
	  done

