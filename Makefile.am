## Process this file with automake to produce Makefile.in

LIB_BARCODE_DIR = barcode-0.98
LIB_IEC16022_DIR = iec16022-0.2.1
LIB_QRENCODE_DIR = qrencode-3.1.0

#distuninstallcheck_listfiles = find . -type f -print | grep -v scrollkeeper

SUBDIRS = \
	po \
	$(LIB_BARCODE_DIR) \
	$(LIB_IEC16022_DIR) \
	$(LIB_QRENCODE_DIR) \
	libglabels \
	src \
	data \
	templates \
	help \
	docs

EXTRA_DIST = \
	README \
	COPYING \
	COPYING-DOCS \
	COPYING-LIBGLABELS \
	COPYING-TEMPLATES \
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

