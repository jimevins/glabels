%define name	glabels
%define ver	0.4.3
%define RELEASE	1
%define rel	%{?CUSTOM_RELEASE} %{!?CUSTOM_RELEASE:%RELEASE}
%define prefix /usr
%define sysconfdir /etc

Summary: glabels is a GNOME program to create labels and business cards
Name:      %name
Version:   %ver
Release:   %rel
Copyright: GPL
Group: Applications/Graphics
Source: glabels-%{ver}.tar.gz
URL: http://snaught.com/glabels/
BuildRoot: /var/tmp/glabels-%{PACKAGE_VERSION}-root

Requires: gtk+ >= 1.2
Requires: gnome-libs >= 1.2.8
Requires: gnome-print >= 0.25
Requires: gdk-pixbuf >= 0.11.0

%description
gLabels is a lightweight program for creating labels and
business cards for the GNOME desktop environment.
It is designed to work with various laser/ink-jet peel-off
label and business card sheets that you'll find at most office
supply stores.


%prep
%setup

%build
CFLAGS="$RPM_OPT_FLAGS" LDFLAGS="-s" ./configure --prefix=%{prefix} 
make

%install
rm -rf $RPM_BUILD_ROOT

make prefix=$RPM_BUILD_ROOT%{prefix} install

%clean
rm -rf $RPM_BUILD_ROOT

%files
%defattr(-, root, root)
%doc README COPYING ChangeLog NEWS AUTHORS INSTALL
%{prefix}/bin/glabels
%{prefix}/share/*


%changelog
* Sat May 19 2001 Jim Evins <evins@snaught.com>
- Created

