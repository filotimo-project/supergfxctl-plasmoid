Name:           supergfxctl-plasmoid
Version:        2.1.1
Release:        3%{?dist}
Epoch:          1
Summary:        KDE Plasma plasmoid for supergfxctl

License:        MPL2
URL:            https://github.com/filotimo-project/supergfxctl-plasmoid
Source:         %{name}-%{version}.tar.gz

BuildRequires:  cmake gcc-c++ extra-cmake-modules kf6-kconfig-devel kf6-ki18n-devel kf6-kirigami-devel kf6-ksvg-devel kf6-solid-devel libplasma-devel qt6-qtbase-devel qt6-qtdeclarative-devel
Requires:       hicolor-icon-theme kf6-kconfig kf6-ki18n kf6-kirigami kf6-ksvg kf6-solid libplasma qt6-qtbase qt6-qtdeclarative supergfxctl

%description
KDE Plasma plasmoid for supergfxctl

%global debug_package %{nil}

%prep
%setup -q

%build
mkdir -p build
cd build
cmake -DCMAKE_INSTALL_PREFIX=/usr -DBUILD_WITH_QT6=ON ..
make


%install
cd build
%make_install


%files
%license LICENSE
/usr/lib64/qt6/plugins/plasma/applets/dev.jhyub.supergfxctl.so
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-dgpu-active.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-dgpu-off.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-dgpu-suspended.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-dedicated-active.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-dedicated.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-compute-active.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-compute.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-egpu-active.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-egpu.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-hybrid-active.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-hybrid.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-integrated-active.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-integrated.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-vfio-active.svg
/usr/share/icons/hicolor/scalable/status/supergfxctl-plasmoid-gpu-vfio.svg
/usr/share/metainfo/dev.jhyub.supergfxctl.appdata.xml
/usr/share/plasma/plasmoids/dev.jhyub.supergfxctl/contents/ui/main.qml
/usr/share/plasma/plasmoids/dev.jhyub.supergfxctl/contents/ui/configGeneral.qml
/usr/share/plasma/plasmoids/dev.jhyub.supergfxctl/contents/config/main.xml
/usr/share/plasma/plasmoids/dev.jhyub.supergfxctl/contents/config/config.qml
/usr/share/plasma/plasmoids/dev.jhyub.supergfxctl/metadata.json
/usr/share/locale/ko/LC_MESSAGES/plasma_applet_dev.jhyub.supergfxctl.mo
/usr/share/locale/pl/LC_MESSAGES/plasma_applet_dev.jhyub.supergfxctl.mo


%changelog
* Sun Mar 09 2025 Thomas Duckworth <tduck@filotimoproject.org>
- add an epoch (tduck@filotimoproject.org)

* Sun Mar 09 2025 Thomas Duckworth <tduck@filotimoproject.org> 2.1.1-2
- new package built with tito

