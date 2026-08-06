#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
VERSION="${VERSION:-0.2.9-preview}"
ARCH_DEB="${ARCH_DEB:-$(dpkg --print-architecture 2>/dev/null || echo amd64)}"
ARCH_RPM="${ARCH_RPM:-$(uname -m)}"
RELEASE_NOTES_SRC="${RELEASE_NOTES_SRC:-$ROOT/README.md}"
README_SRC="$ROOT/README.md"
BIN_SRC="$BUILD_DIR/firestaff"
ARTPACK_STUDIO_BIN_SRC="${ARTPACK_STUDIO_BIN_SRC:-$BUILD_DIR/artpack-studio-bundle/dist/firestaff_artpack_studio}"
DUNGEON_STUDIO_BIN_SRC="${DUNGEON_STUDIO_BIN_SRC:-$BUILD_DIR/dungeon-studio-bundle/dist/firestaff_dungeon_studio}"
SAVEGAME_EDITOR_BIN_SRC="${SAVEGAME_EDITOR_BIN_SRC:-$BUILD_DIR/savegame-editor-bundle/dist/firestaff_savegame_editor}"
OUT_DIR="$ROOT/release"
PKG_NAME="firestaff"
SUMMARY="Firestaff preview Dungeon Master engine"
DESCRIPTION="Firestaff preview build for ongoing source-backed Dungeon Master compatibility work. This preview does not bundle original game data."

if [[ ! -x "$BIN_SRC" ]]; then
  echo "Missing built binary: $BIN_SRC" >&2
  exit 1
fi
if [[ ! -x "$ARTPACK_STUDIO_BIN_SRC" ]]; then
  echo "Missing built Artpack Studio launcher: $ARTPACK_STUDIO_BIN_SRC" >&2
  exit 1
fi
if [[ ! -x "$DUNGEON_STUDIO_BIN_SRC" ]]; then
  echo "Missing built Dungeon Studio launcher: $DUNGEON_STUDIO_BIN_SRC" >&2
  exit 1
fi
if [[ ! -x "$SAVEGAME_EDITOR_BIN_SRC" ]]; then
  echo "Missing built Savegame Editor launcher: $SAVEGAME_EDITOR_BIN_SRC" >&2
  exit 1
fi

mkdir -p "$OUT_DIR"

# Debian package -------------------------------------------------------------
DEB_ROOT="$OUT_DIR/deb-stage/${PKG_NAME}_${VERSION}_${ARCH_DEB}"
rm -rf "$DEB_ROOT"
mkdir -p "$DEB_ROOT/DEBIAN" "$DEB_ROOT/usr/bin" "$DEB_ROOT/usr/share/doc/$PKG_NAME" "$DEB_ROOT/usr/share/pixmaps" "$DEB_ROOT/usr/share/applications" "$DEB_ROOT/usr/share/firestaff/po" "$DEB_ROOT/usr/share/firestaff/scripts"
cp "$BIN_SRC" "$DEB_ROOT/usr/bin/firestaff"
cp "$ARTPACK_STUDIO_BIN_SRC" "$DEB_ROOT/usr/bin/firestaff_artpack_studio"
cp "$DUNGEON_STUDIO_BIN_SRC" "$DEB_ROOT/usr/bin/firestaff_dungeon_studio"
cp "$SAVEGAME_EDITOR_BIN_SRC" "$DEB_ROOT/usr/bin/firestaff_savegame_editor"
cp "$ROOT/assets/branding/firestaff-startup-intro.ppm" "$DEB_ROOT/usr/share/firestaff/firestaff-startup-intro.ppm"
cp "$ROOT"/po/startup-menu.*.po "$DEB_ROOT/usr/share/firestaff/po/"
chmod 0755 "$DEB_ROOT/usr/bin/firestaff"
chmod 0755 "$DEB_ROOT/usr/bin/firestaff_artpack_studio"
chmod 0755 "$DEB_ROOT/usr/bin/firestaff_dungeon_studio"
chmod 0755 "$DEB_ROOT/usr/bin/firestaff_savegame_editor"
cp "$README_SRC" "$DEB_ROOT/usr/share/doc/$PKG_NAME/README.md"
cp "$RELEASE_NOTES_SRC" "$DEB_ROOT/usr/share/doc/$PKG_NAME/RELEASE_NOTES.md"
if [[ -f "$ROOT/assets/branding/firestaff-logo.png" ]]; then
  cp "$ROOT/assets/branding/firestaff-logo.png" "$DEB_ROOT/usr/share/pixmaps/firestaff.png"
fi
cat > "$DEB_ROOT/usr/share/applications/firestaff.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Firestaff
Name[sv]=Firestaff
Comment=Source-faithful Dungeon Master engine
Comment[sv]=Källtrogen spelmotor för Dungeon Master
Exec=firestaff
Icon=firestaff
Categories=Game;RolePlaying;
Terminal=false
DESKTOP
cat > "$DEB_ROOT/usr/share/applications/firestaff-artpack-studio.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Firestaff Artpack Studio
Comment=Create and edit Firestaff V2.2 artpacks
Exec=firestaff_artpack_studio
Icon=firestaff
Categories=Graphics;Game;RolePlaying;
Terminal=false
DESKTOP
cat > "$DEB_ROOT/usr/share/applications/firestaff-dungeon-studio.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Firestaff Dungeon Studio
Comment=Create and edit Firestaff dungeon data
Exec=firestaff_dungeon_studio
Icon=firestaff
Categories=Graphics;Game;RolePlaying;
Terminal=false
DESKTOP
cat > "$DEB_ROOT/usr/share/applications/firestaff-savegame-editor.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Firestaff Savegame Editor
Comment=Edit Firestaff savegame files
Exec=firestaff_savegame_editor
Icon=firestaff
Categories=Utility;Game;RolePlaying;
Terminal=false
DESKTOP
cat > "$DEB_ROOT/DEBIAN/control" <<CONTROL
Package: $PKG_NAME
Version: $VERSION
Section: games
Priority: optional
Architecture: $ARCH_DEB
Maintainer: Firestaff Preview <noreply@github.com>
Depends: libsdl3-0 | libsdl3-0-0
Description: $SUMMARY
 $DESCRIPTION
CONTROL
DEB_PATH="$OUT_DIR/${PKG_NAME}_${VERSION}_${ARCH_DEB}.deb"
rm -f "$DEB_PATH"
dpkg-deb --build --root-owner-group "$DEB_ROOT" "$DEB_PATH"

# RPM package ----------------------------------------------------------------
RPM_TOP="$OUT_DIR/rpmbuild"
RPM_ROOT="$RPM_TOP/BUILDROOT/${PKG_NAME}-${VERSION}-1.${ARCH_RPM}"
rm -rf "$RPM_TOP"
mkdir -p "$RPM_TOP/BUILD" "$RPM_TOP/RPMS" "$RPM_TOP/SOURCES" "$RPM_TOP/SPECS" "$RPM_TOP/SRPMS"
mkdir -p "$RPM_ROOT/usr/bin" "$RPM_ROOT/usr/share/doc/$PKG_NAME" "$RPM_ROOT/usr/share/pixmaps" "$RPM_ROOT/usr/share/applications" "$RPM_ROOT/usr/share/firestaff/po" "$RPM_ROOT/usr/share/firestaff/scripts"
cp "$BIN_SRC" "$RPM_ROOT/usr/bin/firestaff"
cp "$ARTPACK_STUDIO_BIN_SRC" "$RPM_ROOT/usr/bin/firestaff_artpack_studio"
cp "$DUNGEON_STUDIO_BIN_SRC" "$RPM_ROOT/usr/bin/firestaff_dungeon_studio"
cp "$SAVEGAME_EDITOR_BIN_SRC" "$RPM_ROOT/usr/bin/firestaff_savegame_editor"
cp "$ROOT/assets/branding/firestaff-startup-intro.ppm" "$RPM_ROOT/usr/share/firestaff/firestaff-startup-intro.ppm"
cp "$ROOT"/po/startup-menu.*.po "$RPM_ROOT/usr/share/firestaff/po/"
chmod 0755 "$RPM_ROOT/usr/bin/firestaff"
chmod 0755 "$RPM_ROOT/usr/bin/firestaff_artpack_studio"
chmod 0755 "$RPM_ROOT/usr/bin/firestaff_dungeon_studio"
chmod 0755 "$RPM_ROOT/usr/bin/firestaff_savegame_editor"
cp "$README_SRC" "$RPM_ROOT/usr/share/doc/$PKG_NAME/README.md"
cp "$RELEASE_NOTES_SRC" "$RPM_ROOT/usr/share/doc/$PKG_NAME/RELEASE_NOTES.md"
RPM_ICON_ENTRY=""
if [[ -f "$ROOT/assets/branding/firestaff-logo.png" ]]; then
  cp "$ROOT/assets/branding/firestaff-logo.png" "$RPM_ROOT/usr/share/pixmaps/firestaff.png"
  RPM_ICON_ENTRY="/usr/share/pixmaps/firestaff.png"
fi
cp "$DEB_ROOT/usr/share/applications/firestaff.desktop" "$RPM_ROOT/usr/share/applications/firestaff.desktop"
cp "$DEB_ROOT/usr/share/applications/firestaff-artpack-studio.desktop" "$RPM_ROOT/usr/share/applications/firestaff-artpack-studio.desktop"
cp "$DEB_ROOT/usr/share/applications/firestaff-dungeon-studio.desktop" "$RPM_ROOT/usr/share/applications/firestaff-dungeon-studio.desktop"
cp "$DEB_ROOT/usr/share/applications/firestaff-savegame-editor.desktop" "$RPM_ROOT/usr/share/applications/firestaff-savegame-editor.desktop"
cat > "$RPM_TOP/SPECS/firestaff.spec" <<SPEC
Name:           $PKG_NAME
Version:        ${VERSION//-/_}
Release:        1%{?dist}
Summary:        $SUMMARY
License:        MIT
URL:            https://github.com/yeager/firestaff
Requires:       SDL3

%description
$DESCRIPTION

%files
/usr/bin/firestaff
/usr/bin/firestaff_artpack_studio
/usr/bin/firestaff_dungeon_studio
/usr/bin/firestaff_savegame_editor
/usr/share/doc/$PKG_NAME/README.md
/usr/share/doc/$PKG_NAME/RELEASE_NOTES.md
/usr/share/applications/firestaff.desktop
/usr/share/applications/firestaff-artpack-studio.desktop
/usr/share/applications/firestaff-dungeon-studio.desktop
/usr/share/applications/firestaff-savegame-editor.desktop
/usr/share/firestaff/firestaff-startup-intro.ppm
/usr/share/firestaff/po/*.po
$RPM_ICON_ENTRY
SPEC
rpmbuild --define "_topdir $RPM_TOP" --define "buildroot $RPM_ROOT" --target "$ARCH_RPM" -bb "$RPM_TOP/SPECS/firestaff.spec"
RPM_PATH="$(find "$RPM_TOP/RPMS" -type f -name '*.rpm' | head -1)"
cp "$RPM_PATH" "$OUT_DIR/${PKG_NAME}-${VERSION}.${ARCH_RPM}.rpm"

ls -lh "$DEB_PATH" "$OUT_DIR/${PKG_NAME}-${VERSION}.${ARCH_RPM}.rpm"
