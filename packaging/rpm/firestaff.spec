Name:           firestaff
Version:        3.0.321
Release:        1%{?dist}
Summary:        Native engine for the FTL dungeon-crawler games
License:        MIT
URL:            https://github.com/yeager/firestaff
Source0:        %{name}-%{version}.tar.gz
BuildRequires:  cmake >= 3.20
BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  ninja-build
BuildRequires:  pkgconfig(SDL3)
BuildRequires:  pkgconfig(zlib)
BuildRequires:  python3
Requires:       python3

%description
Firestaff is a C11 and SDL engine for Dungeon Master, Chaos Strikes Back,
Dungeon Master II: Skullkeep, Dungeon Master Nexus, and Theron's Quest.
Original game data is not included.

%prep
%autosetup -p1

%build
%cmake -G Ninja -DBUILD_TESTING=OFF
%cmake_build --target firestaff firestaff_artpack_studio

%install
%cmake_install

%files
%license LICENSE
%doc README.md
%{_bindir}/firestaff
%{_bindir}/firestaff_artpack_studio
%{_datadir}/firestaff/scripts/firestaff_artpack_studio.py
%{_mandir}/man1/firestaff.1*

%changelog
* Mon Aug 31 2026 Daniel Nylander <daniel@danielnylander.se> - 3.0.321-1
- Initial RPM package.
