## Process this file with automake to produce Makefile.in

@INTLTOOL_XML_RULE@

APPDATA_IN_FILES = glabels-3.appdata.xml.in
APPDATA_FILES =$(APPDATA_IN_FILES:.appdata.xml.in=.appdata.xml)

appdatadir = $(datadir)/appdata
appdata_DATA = $(APPDATA_FILES)

EXTRA_DIST = $(APPDATA_IN_FILES)

CLEANFILES = $(APPDATA_FILES)

