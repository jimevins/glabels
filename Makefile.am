## Process this file with automake to produce Makefile.in

LIB_BARCODE_DIR = barcode-0.98
LIB_IEC16022_DIR = iec16022-0.2.1

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
	COPYING.README_FIRST \
	COPYING \
	COPYING-DOCS \
	COPYING-LIBGLABELS \
	AUTHORS \
	ChangeLog \
	ChangeLog.pre-2.2 \
	INSTALL \
	NEWS \
	TODO \
	intltool-extract.in \
	intltool-merge.in \
	intltool-update.in \
	glabels.spec.in \
	glabels.spec
	gnome-doc-utils.make

DISTCLEANFILES = gnome-doc-utils.make

DISTCHECK_CONFIGURE_FLAGS = --disable-scrollkeeper

dist-hook:
	@if test -d "$(srcdir)/.git"; \
	then \
		echo Creating ChangeLog && \
		(GIT_DIR=$(top_srcdir)/.git \
		  ./missing --run git log -M -C --name-status --date=short --no-color) | \
		  fmt --split-only > ChangeLog.tmp \
		&& mv -f ChangeLog.tmp $(top_distdir)/ChangeLog \
		|| ( rm -f ChangeLog.tmp ; \
		  echo Failed to generate ChangeLog >&2 ); \
	else \
		echo A git clone is required to generate a ChangeLog >&2; \
	fi
