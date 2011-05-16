## Process this file with automake to produce Makefile.in

SUBDIRS = \
	po \
	libglabels \
	libglbarcode \
	src \
	data \
	templates \
	help \
	docs

EXTRA_DIST = \
	README \
	COPYING.README_FIRST \
	COPYING \
	COPYING-DOCS \
	COPYING-LIBS \
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
	glabels.spec

DISTCLEANFILES = gnome-doc-utils.make

DISTCHECK_CONFIGURE_FLAGS = \
	--disable-scrollkeeper	\
	--enable-gtk-doc

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
