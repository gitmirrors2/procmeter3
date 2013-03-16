%define name 		procmeter3
%define version 	3.4a
%define release 	1

Summary: 		ProcMeter3 - Linux system monitoring program.
Name: 			%{name} 
Version: 		%{version}
Release: 		%{release}
Copyright: 		GPL
Group: 			Applications/System
Source: 		%{name}-%{version}.tgz
URL: 			http://www.gedanken.org.uk/software/procmeter3/
Packager: 		thias <matthias.brill@akamail.com>
BuildRoot: 		%{_tmppath}/%{name}-%{version}-buildroot

%description

Displays in a series of graph (using X windows) statistics about the
system status (amount of CPU, memory, disk accesses, IP packets) and
user level information (date, time, mailbox size). 

%prep
%setup
# separate the READMEs
%{__cp} modules/README modules/README.modules
# adjust the Makefile
%{__mv} Makefile Makefile.default
%{__cat} Makefile.default \
  | %{__sed} -e "s#^INSTDIR=.*#INSTDIR=%{_prefix}#" \
  | %{__sed} -e "s#^MANDIR=.*#MANDIR=%{_mandir}#" \
  | %{__sed} -e "s#^LIB_PATH=.*#LIB_PATH=%{_datadir}/procmeter3#" \
  > Makefile

%build
%{__make}


%install
%{__rm} -rf $RPM_BUILD_ROOT
%{__mkdir} -p $RPM_BUILD_ROOT%{_bindir}
%{__mkdir} -p $RPM_BUILD_ROOT%{_mandir}/{man1,man5}
%{__mkdir} -p $RPM_BUILD_ROOT%{_includedir}
%{__mkdir} -p $RPM_BUILD_ROOT%{_datadir}/procmeter3/modules

# binaries
%{__install} -m 755 procmeter3 $RPM_BUILD_ROOT%{_bindir}
%{__install} -m 755 gprocmeter3 $RPM_BUILD_ROOT%{_bindir}
%{__install} -m 755 procmeter3-log $RPM_BUILD_ROOT%{_bindir}
%{__install} -m 755 procmeter3-lcd $RPM_BUILD_ROOT%{_bindir}

# manual pages
%{__install} -m 644 man/*.1 $RPM_BUILD_ROOT%{_mandir}/man1
%{__install} -m 644 man/*.5 $RPM_BUILD_ROOT%{_mandir}/man5

# rc file
%{__install} -m 644 procmeterrc.install $RPM_BUILD_ROOT%{_datadir}/procmeter3/procmeterrc

# header file
%{__install} -m 644 procmeter.h $RPM_BUILD_ROOT%{_includedir}

# modules
%{__install} -m 755 modules/*.so $RPM_BUILD_ROOT%{_datadir}/procmeter3/modules

%clean
%{__rm} -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%{_bindir}/*
%{_mandir}/man*/*
%{_datadir}/procmeter3
%{_includedir}/procmeter.h
%doc ANNOUNCE LSM NEWS README ChangeLog 
%doc modules/README.modules modules/template.c

