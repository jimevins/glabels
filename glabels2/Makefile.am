## Process this file with automake to produce Makefile.in

LIB_BARCODE_DIR = barcode-0.98
LIB_IEC16022_DIR = iec16022-0.2.1

#distuninstallcheck_listfiles = find . -type f -print | grep -v scrollkeeper

SUBDIRS = \
	po \
	$(LIB_BARCODE_DIR) \
	$(LIB_IEC16022_DIR) \
	libglabels \
	src \
	data \
	help \
	docs

EXTRA_DIST = \
	README \
	COPYING \
	COPYING-DOCS \
	COPYING.LIBGLABELS \
	AUTHORS \
	ChangeLog \
	INSTALL \
	NEWS \
	TODO \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	gnome-doc-utils.make \
	glabels.spec.in \
	glabels.spec \
	MESSAGE-OF-DOOM

DISTCLEANFILES = gnome-doc-utils.make

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper

