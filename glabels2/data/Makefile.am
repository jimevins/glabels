## Process this file with automake to produce Makefile.in

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES = glabels.desktop.in
DESKTOP_FILES =$(DESKTOP_IN_FILES:.desktop.in=.desktop)

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

@INTLTOOL_KEYS_RULE@

KEYS_IN_FILES=glabels.keys.in
KEYS_FILES = $(KEYS_IN_FILES:.keys.in=.keys)

mimedir = $(datadir)/mime-info
mime_DATA = $(KEYS_FILES) glabels.mime

appdir = $(datadir)/application-registry
app_DATA = glabels.applications

runtime_files = \
	glabels-2.0.dtd \
	paper-sizes.xml \
	avery-us-templates.xml \
	avery-iso-templates.xml \
	zweckform-iso-templates.xml \
	misc-us-templates.xml \
	misc-iso-templates.xml

EXTRA_DIST = \
	$(DESKTOP_IN_FILES)	\
	$(KEYS_IN_FILES)	\
	$(mime_DATA)            \
	$(app_DATA)		\
	$(runtime_files)

configdir=$(datadir)/glabels
install-data-local:
	  $(mkinstalldirs) $(DESTDIR)$(configdir); \
	  for file in $(runtime_files); do \
	    if test -f $$file; then \
	      $(INSTALL_DATA) $$file $(DESTDIR)$(configdir); \
	    fi \
	  done

CLEANFILES = $(KEY_FILES) $(DESKTOP_FILES)

