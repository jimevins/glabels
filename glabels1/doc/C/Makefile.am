glabels_helpdir = $(datadir)/gnome/help/glabels/C

glabels_help_DATA = 	\
	index.html	\
	topic.dat

SGML_FILES = 		\
	glabels.sgml

EXTRA_DIST = 		\
	topic.dat	\
	$(SGML_FILES)

all: index.html

index.html: glabels/index.html
	-cp glabels/index.html .

glabels/index.html: $(SGML_FILES)
	-db2html glabels.sgml
	-cp -rp images glabels/images

dist-hook: index.html
	-mkdir $(distdir)/glabels
	-mkdir $(distdir)/glabels/stylesheet-images
	-mkdir $(distdir)/glabels/images
	-cp glabels/*.html $(distdir)/glabels
	-cp glabels/*.css $(distdir)/glabels
	-cp glabels/stylesheet-images/*.gif $(distdir)/glabels/stylesheet-images
	-cp glabels/images/*.jpg $(distdir)/glabels/images
	-mkdir $(distdir)/images
	-cp images/*.jpg $(distdir)/images

install-data-local: index.html
	-$(mkinstalldirs) $(DESTDIR)$(glabels_helpdir)/images
	-$(mkinstalldirs) $(DESTDIR)$(glabels_helpdir)/stylesheet-images
	-for file in $(srcdir)/glabels/*.html $(srcdir)/glabels/*.css; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glabels_helpdir)/$$basefile; \
	done
	-for file in $(srcdir)/glabels/images/*.jpg; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glabels_helpdir)/images/$$basefile; \
	done
	-for file in $(srcdir)/glabels/stylesheet-images/*.gif; do \
	  basefile=`echo $$file | sed -e 's,^.*/,,'`; \
	  $(INSTALL_DATA) $$file $(DESTDIR)$(glabels_helpdir)/stylesheet-images/$$basefile; \
	done

glabels.ps: glabels.sgml
	-db2ps $<

glabels.rtf: glabels.sgml
	-db2rtf $<

