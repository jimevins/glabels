
gsettings_files = \
	org.gnome.glabels-3.gschema.xml.in

gsettings_SCHEMAS = $(gsettings_files:.xml.in=.xml)

@INTLTOOL_XML_NOMERGE_RULE@
@GSETTINGS_RULES@


EXTRA_DIST = \
	$(gsettings_files)

CLEANFILES = \
	$(gsettings_SCHEMAS)

DISTCLEANFILES = \
	$(gsettings_files) \
	$(gsettings_SCHEMAS)

MAINTAINERCLEANFILES = \
	$(DISTCLEANFILES) \
	$(gsettings_SCHEMAS:.xml=.valid)
