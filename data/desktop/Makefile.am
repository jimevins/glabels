## Process this file with automake to produce Makefile.in

@INTLTOOL_DESKTOP_RULE@

DESKTOP_IN_FILES = glabels.desktop.in
DESKTOP_FILES =$(DESKTOP_IN_FILES:.desktop.in=.desktop)

desktopdir = $(datadir)/applications
desktop_DATA = $(DESKTOP_FILES)

EXTRA_DIST = $(DESKTOP_IN_FILES)

CLEANFILES = $(DESKTOP_FILES)
