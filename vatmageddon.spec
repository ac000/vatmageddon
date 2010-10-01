Name:		vatmageddon	
Version:	001
Release:	2%{?dist}
Summary:	GUI for calculating VAT, Gross & Net values

Group:		Applications/System
License:	GPLv2
URL:		http://github.com/ac000/vatmageddon
Source0:	vatmageddon-%{version}.tar
BuildRoot:	%(mktemp -ud %{_tmppath}/%{name}-%{version}-%{release}-XXXXXX)

BuildRequires:	gtk2-devel glib2-devel libround
Requires:	libround

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
* Sat Oct 02 2010 Andrew Clayton <andrew@digital-domain.net> - 001-2
- Some minor spelling and url updates

* Sun Sep 26 2010 Andrew Clayton <andrew@digital-domain.net> - 001-1
- Initial version
