Name:		vatmageddon	
Version:	007
Release:	1%{?dist}
Summary:	GUI for calculating VAT, Gross & Net values

Group:		Applications/System
License:	GPLv2
URL:		http://github.com/ac000/vatmageddon
Source0:	vatmageddon-%{version}.tar
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:	gtk3-devel, glib2-devel, libround >= 1.0.0
Requires:	gtk3, glib2, libround >= 1.0.0

%description
vatmageddon is a GUI for calculating VAT, Gross & Net values.

%prep
%setup -q


%build
make %{?_smp_mflags}


%install
rm -rf $RPM_BUILD_ROOT
install -Dp -m755 vatmageddon $RPM_BUILD_ROOT/%{_bindir}/vatmageddon
install -Dp -m644 vatmageddon.1.gz $RPM_BUILD_ROOT/%{_mandir}/man1/vatmageddon.1.gz
install -Dp -m0644 vatmageddon.desktop $RPM_BUILD_ROOT/%{_datarootdir}/applications/vatmageddon.desktop


%clean
rm -rf $RPM_BUILD_ROOT


%files
%defattr(-,root,root,-)
%{_bindir}/vatmageddon
%{_mandir}/man1/vatmageddon.1.gz
%{_datarootdir}/applications/vatmageddon.desktop


%changelog

* Wed Feb 27 2013 Andrew Clayton <andrew@digital-domain.net> - 007-1
- Update to new version.

  This now requires GTK+3

* Sat Dec 15 2012 Andrew Clayton <andrew@digital-domain.net> - 006-1
- Update to new version.

  Minor label changes

* Sat May 19 2012 Andrew Clayton <andrew@digital-domain.net> - 005-1
- Update for new version of libround

* Thu Mar 31 2011 Andrew Clayton <andrew@digital-domain.net> - 004-1
- Update to latest version

  Fix's a segfault

* Fri Jan 28 2011 Andrew Clayton <andrew@digital-domain.net> - 003-1
- Update to latest version

* Fri Jan 28 2011 Andrew Clayton <andrew@digital-domain.net> - 002-2
- .desktop file fix

* Thu Jan 27 2011 Andrew Clayton <andrew@digital-domain.net> - 002-1
- Update to latest version

* Sat Oct 02 2010 Andrew Clayton <andrew@digital-domain.net> - 001-2
- Some minor spelling and url updates

* Sun Sep 26 2010 Andrew Clayton <andrew@digital-domain.net> - 001-1
- Initial version
