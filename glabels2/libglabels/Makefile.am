INCLUDES = \
	$(LIBGLABELS_CFLAGS)		\
	-DDATADIR=\""$(datadir)"\"	

libglabels_la_LDFLAGS=\
        -version-info 0:0:0 \
        $(LIBGLABELS_LIBS) \
        -no-undefined 

lib_LTLIBRARIES = libglabels.la

libglabelsincludedir=$(includedir)/libglabels

libglabels_la_SOURCES =		\
	libglabels-private.h	\
	paper.h			\
	paper.c			\
	template.h		\
	template.c		\
	xml-paper.h		\
	xml-paper.c		\
	xml-template.h		\
	xml-template.c		\
	xml.h			\
	xml.c			

libglabelsinclude_HEADERS = 	\
	libglabels.h		\
	paper.h			\
	template.h		\
	xml-paper.h		\
	xml-template.h		\
	xml.h			
