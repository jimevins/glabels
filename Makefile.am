NULL = 

#Build in these directories:

SUBDIRS = \
	libglabels \
	libglbarcode \
	glabels \
	po \
	templates \
	data \
	help \
	$(NULL)

glabelsdocdir = ${prefix}/doc/glabels
glabelsdoc_DATA = \
	$(NULL)

EXTRA_DIST = \
	$(glabelsdoc_DATA) \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in\
	$(NULL)

DISTCLEANFILES = \
	intltool-extract \
	intltool-merge \
	intltool-update \
	po/.intltool-merge-cache \
	$(NULL)

