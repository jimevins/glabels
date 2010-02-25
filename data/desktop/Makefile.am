## Process this file with automake to produce Makefile.in

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES = $(GLABELS_BRANCH).desktop.in
DESKTOP_FILES =$(DESKTOP_IN_FILES:.desktop.in=.desktop)

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

EXTRA_DIST = $(DESKTOP_IN_FILES)

CLEANFILES = $(DESKTOP_FILES)

install-data-hook:
if ENABLE_UPDATE_DESKTOPDB
	$(UPDATE_DESKTOP_DATABASE)
endif

uninstall-hook:
if ENABLE_UPDATE_DESKTOPDB
	$(UPDATE_DESKTOP_DATABASE)
endif
