%define name    barcode
%define ver     0.98
%define rel	1
%define prefix   /usr
%define sysconfdir	/etc

Summary: GNU barcode
Name: %name
Version: %ver
Release: %rel
Copyright: GPL
Group: Applications/Productivity
Source:  ftp://ar.linux.it/pub/barcode/%name-%{ver}.tar.gz
BuildRoot: /var/tmp/%name-%{ver}-root
URL: http://gnu.systemy.it/software/barcode
Prefix: %prefix

%description
This is GNU-barcode.
The package is meant to solve most needs in barcode creation with a
conventional printer. It can create printouts for the conventional
product tagging standards: UPC-A, UPC-E, EAN-13, EAN-8, ISBN, as well
as a few other formats. Ouput is generated as either Postscript or
Encapsulated Postscript (other back-ends may be added if needed).

%package devel
Summary: GNU barcode files for development
Group: Development/Libraries

%description devel
This is GNU-barcode.
The package is meant to solve most needs in barcode creation with a
conventional printer. It can create printouts for the conventional
product tagging standards: UPC-A, UPC-E, EAN-13, EAN-8, ISBN, as well
as a few other formats. Ouput is generated as either Postscript or
Encapsulated Postscript (other back-ends may be added if needed).

This package contain the C header, the static library and man page
for development.

%prep
%setup

%ifarch alpha
  ARCH_FLAGS="--host=alpha-redhat-linux"
%endif

export -n LANG LINGUAS LC_ALL 
if [ ! -f configure ]; then
  CFLAGS="$RPM_OPT_FLAGS" ./autogen.sh $ARCH_FLAGS --prefix=%{prefix} --sysconfdir=%{sysconfdir}
else
  CFLAGS="$RPM_OPT_FLAGS" ./configure $ARCH_FLAGS --prefix=%{prefix} --sysconfdir=%{sysconfdir}
fi

%build
export -n LANG LINGUAS LC_ALL 

if [ "$SMP" != "" ]; then
  (make "MAKE=make -k -j $SMP"; exit 0)
  make
else
  make
fi

%install
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT
make prefix=$RPM_BUILD_ROOT%{prefix} sysconfdir=$RPM_BUILD_ROOT%{sysconfdir} install

%clean
[ -n "$RPM_BUILD_ROOT" -a "$RPM_BUILD_ROOT" != / ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)

%doc COPYING ChangeLog INSTALL README TODO doc/*.html doc/*.pdf doc/*.ps

%attr(0755,root,root) %{prefix}/bin/barcode
%attr(0644,root,root) %{prefix}/info/barcode.info*
%attr(0644,root,root) %{prefix}/man/man1/barcode.1*

%files devel
%attr(0644,root,root) %{prefix}/include/barcode.h
%attr(0644,root,root) %{prefix}/lib/libbarcode.a
%attr(0644,root,root) %{prefix}/man/man3/barcode.3*
