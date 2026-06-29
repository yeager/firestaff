#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
VERSION="${VERSION:-0.2.9-preview}"
RELEASE_NOTES_SRC="${RELEASE_NOTES_SRC:-$ROOT/README.md}"
README_SRC="$ROOT/README.md"
BIN_SRC="$BUILD_DIR/firestaff"
OUT_DIR="$ROOT/release"
PKG_NAME="firestaff"
PKG_ARCH="x86_64"
PKG_REL="${PKG_REL:-1}"
PKG_VERSION="${VERSION//-/_}"
PKG_ROOT="$OUT_DIR/steamdeck-stage/${PKG_NAME}-${PKG_VERSION}-${PKG_REL}-${PKG_ARCH}"
PKG_PATH="$OUT_DIR/Firestaff-${VERSION}-steamdeck-${PKG_ARCH}.pkg.tar.zst"

if [[ ! -x "$BIN_SRC" ]]; then
  echo "Missing built binary: $BIN_SRC" >&2
  exit 1
fi

if ! command -v zstd >/dev/null 2>&1; then
  echo "Missing zstd; Steam Deck package output requires zstd compression" >&2
  exit 1
fi

mkdir -p "$OUT_DIR"
rm -rf "$PKG_ROOT"
mkdir -p \
  "$PKG_ROOT/usr/bin" \
  "$PKG_ROOT/usr/share/doc/$PKG_NAME" \
  "$PKG_ROOT/usr/share/pixmaps" \
  "$PKG_ROOT/usr/share/applications"

cp "$BIN_SRC" "$PKG_ROOT/usr/bin/firestaff"
chmod 0755 "$PKG_ROOT/usr/bin/firestaff"
cp "$README_SRC" "$PKG_ROOT/usr/share/doc/$PKG_NAME/README.md"
cp "$RELEASE_NOTES_SRC" "$PKG_ROOT/usr/share/doc/$PKG_NAME/RELEASE_NOTES.md"
cp "$ROOT/assets/branding/firestaff-logo.png" "$PKG_ROOT/usr/share/pixmaps/firestaff.png"
cat > "$PKG_ROOT/usr/share/applications/firestaff.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Firestaff
Comment=Firestaff preview Dungeon Master engine
Exec=firestaff
Icon=firestaff
Categories=Game;RolePlaying;
Terminal=false
DESKTOP

installed_kib="$(du -sk "$PKG_ROOT/usr" | awk '{print $1}')"
installed_size="$((installed_kib * 1024))"

cat > "$PKG_ROOT/.PKGINFO" <<PKGINFO
pkgname = $PKG_NAME
pkgbase = $PKG_NAME
pkgver = ${PKG_VERSION}-${PKG_REL}
pkgdesc = Firestaff preview Dungeon Master engine
url = https://github.com/yeager/firestaff
builddate = $(date +%s)
packager = Firestaff Preview <noreply@github.com>
size = $installed_size
arch = $PKG_ARCH
license = MIT
depend = sdl3
PKGINFO

rm -f "$PKG_PATH"
(
  cd "$PKG_ROOT"
  tar -cf - . | zstd -19 -T0 -q -o "$PKG_PATH"
)

ls -lh "$PKG_PATH"
