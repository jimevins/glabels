## Process this file with automake to produce Makefile.in

keys_in_files=glabels.keys.in
keys_files = $(keys_in_files:.keys.in=.keys)
@INTLTOOL_KEYS_RULE@

xml_in_files=glabels.xml.in
xml_files = $(xml_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

mimedir = $(datadir)/mime-info
mime_DATA = $(keys_files) glabels.mime

appdir = $(datadir)/application-registry
app_DATA = glabels.applications

newmimedir = $(datadir)/mime/packages
newmime_DATA = $(xml_files)

EXTRA_DIST = \
	$(keys_in_files)	\
	$(mime_DATA)            \
	$(app_DATA)		\
	$(xml_in_files)		\
	$(newmime_DATA)

CLEANFILES = $(keys_files) $(xml_files)


install-data-hook:
if HAVE_NEW_MIME
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(datadir)/mime"
endif
if HAVE_NEWER_MIME
	$(UPDATE_DESKTOP_DATABASE) 
endif

uninstall-hook:
if HAVE_NEW_MIME
	$(UPDATE_MIME_DATABASE) "$(DESTDIR)$(datadir)/mime"
endif
if HAVE_NEWER_MIME
	$(UPDATE_DESKTOP_DATABASE) 
endif

