Summary: A modular, graphical, system monitoring program.
Name: procmeter3
Version: 3.3a
Release: 1
URL: http://www.gedanken.org.uk/software/procmeter3/
Copyright: GPL
Group: Applications/System
Source: http://www.gedanken.org.uk/software/procmeter3/download/%{name}-%{version}.tgz
BuildRoot: %{_tmppath}/%{name}-root

%description
The ProcMeter program itself is a framework on which a number of modules
(plugins) are loaded.  More modules can be written as required to perform
more monitoring and informational functions.  Displays in a series of
graph (using X windows) statistics about the system status (amount of
CPU, memory, disk accesses, IP packets) and user level information (date,
time, mailbox size).  Uses the /proc filesystem and other sources of
information.  Compiles with either the Athena or GTK widget set.

%prep
%setup -q

%build
make CFLAGS="$RPM_OPT_FLAGS" \
	INSTDIR=%{_prefix}
        LIB_PATH=%{_libdir}/X11/ProcMeter3 \
        MOD_PATH=%{_libdir}/X11/ProcMeter3/modules \
        RC_PATH=%{_libdir}/X11/ProcMeter3

%install
[ $RPM_BUILD_ROOT != "/" ] && rm -rf $RPM_BUILD_ROOT

make INSTDIR=$RPM_BUILD_ROOT/%{_prefix} \
	LIB_PATH=$RPM_BUILD_ROOT/%{_libdir}/X11/ProcMeter3 \
	MOD_PATH=$RPM_BUILD_ROOT/%{_libdir}/X11/ProcMeter3/modules \
	RC_PATH=$RPM_BUILD_ROOT/%{_libdir}/X11/ProcMeter3 \
	install

mkdir -p $RPM_BUILD_ROOT/%{_mandir}
mv $RPM_BUILD_ROOT/%{_prefix}/man/* $RPM_BUILD_ROOT/%{_mandir}
 
%clean
[ "$RPM_BUILD_ROOT" != "/" ] && rm -rf $RPM_BUILD_ROOT

%files
%defattr(-,root,root)
%doc ANNOUNCE ChangeLog LSM NEWS README
%{_bindir}/*
%{_libdir}/X11/ProcMeter3/example/*
%{_libdir}/X11/ProcMeter3/include/*
%{_libdir}/X11/ProcMeter3/modules/*
%{_libdir}/X11/ProcMeter3/procmeterrc
%{_mandir}/man1/*
%{_mandir}/man5/*

%changelog
* Mon Mar 11 2002 Christopher Barton <cpbarton@uiuc.edu> 3.3a-1
- Initial package
