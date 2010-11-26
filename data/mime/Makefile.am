## Process this file with automake to produce Makefile.in

xml_in_files=$(GLABELS_BRANCH).xml.in
xml_files = $(xml_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

mimedir = $(datadir)/mime/packages
mime_DATA = $(xml_files)

EXTRA_DIST = \
	$(xml_in_files)

CLEANFILES = \
	$(xml_files)


