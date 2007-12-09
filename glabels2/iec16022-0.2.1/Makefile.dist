CC=/usr/bin/cc
INSTALL=/usr/bin/install
GZIP=/bin/gzip

prefix=/usr/local
bindir=$(prefix)/bin
mandir=$(prefix)/share/man

CFLAGS=-Wall

.PHONY: test clean

all: iec16022 manpage

iec16022: iec16022ecc200.o image.o iec16022.c
	$(CC) -c iec16022.c
	$(CC) -o iec16022 $(CFLAGS) reedsol.o iec16022.o image.o iec16022ecc200.o -lz -lpopt

manpage: iec16022.1
	$(GZIP) -f --best < iec16022.1 > iec16022.1.gz

install: all
	$(INSTALL) -d -m 755 $(DESTDIR)$(bindir) $(DESTDIR)$(mandir)/man1
	$(INSTALL) -m 755 iec16022 $(DESTDIR)$(bindir)
	$(INSTALL) -m 644 iec16022.1.gz $(DESTDIR)$(mandir)/man1

test: iec16022
	cd test; ./testsuite.sh

iec16022ecc200.o: iec16022ecc200.c iec16022ecc200.h reedsol.o
	$(CC) -c iec16022ecc200.c

image.o: image.c image.h
	$(CC) -c image.c

reedsol.o: reedsol.c reedsol.h
	$(CC)  -DLIB -c reedsol.c

clean:
	rm -f iec16022 iec16022.1.gz *.o
