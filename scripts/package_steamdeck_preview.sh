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
PKG_FULL_VERSION="${PKG_VERSION}-${PKG_REL}"
PKG_ROOT="$OUT_DIR/steamdeck-stage/${PKG_NAME}-${PKG_FULL_VERSION}-${PKG_ARCH}"
PKG_PATH="$OUT_DIR/Firestaff-${VERSION}-steamdeck-${PKG_ARCH}.pkg.tar.zst"
PKG_PATH_CANONICAL="$OUT_DIR/${PKG_NAME}-${PKG_FULL_VERSION}-${PKG_ARCH}.pkg.tar.zst"
SUMMARY="Firestaff preview Dungeon Master engine"
DESCRIPTION="Firestaff preview build for ongoing source-backed Dungeon Master compatibility work. This preview does not bundle original game data."

if [[ ! -x "$BIN_SRC" ]]; then
  echo "Missing built binary: $BIN_SRC" >&2
  exit 1
fi

if ! command -v zstd >/dev/null 2>&1; then
  echo "Missing zstd; Steam Deck package output requires zstd compression" >&2
  exit 1
fi
if ! command -v bsdtar >/dev/null 2>&1; then
  echo "Missing bsdtar/libarchive; Steam Deck package output requires ALPM mtree metadata" >&2
  exit 1
fi

script_sha256() {
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum "$1" | awk '{print $1}'
  else
    shasum -a 256 "$1" | awk '{print $1}'
  fi
}

SDL3_LIB="${SDL3_LIB:-}"
if [[ -z "$SDL3_LIB" ]] && command -v ldd >/dev/null 2>&1; then
  SDL3_LIB="$(ldd "$BIN_SRC" 2>/dev/null | awk '/libSDL3[.]so/ {print $3; exit}')"
fi
if [[ -z "$SDL3_LIB" || ! -f "$SDL3_LIB" ]]; then
  echo "Missing SDL3 runtime library for Steam Deck package." >&2
  echo "Set SDL3_LIB=/path/to/libSDL3.so.0 or build on Linux with ldd support." >&2
  exit 1
fi

mkdir -p "$OUT_DIR"
rm -rf "$PKG_ROOT"
mkdir -p \
  "$PKG_ROOT/usr/bin" \
  "$PKG_ROOT/usr/lib/$PKG_NAME" \
  "$PKG_ROOT/usr/share/doc/$PKG_NAME" \
  "$PKG_ROOT/usr/share/pixmaps" \
  "$PKG_ROOT/usr/share/applications"

cp "$BIN_SRC" "$PKG_ROOT/usr/lib/$PKG_NAME/firestaff-bin"
chmod 0755 "$PKG_ROOT/usr/lib/$PKG_NAME/firestaff-bin"
cp -L "$SDL3_LIB" "$PKG_ROOT/usr/lib/$PKG_NAME/libSDL3.so.0"
chmod 0755 "$PKG_ROOT/usr/lib/$PKG_NAME/libSDL3.so.0"
cat > "$PKG_ROOT/usr/bin/firestaff" <<'WRAPPER'
#!/usr/bin/env sh
set -eu
export LD_LIBRARY_PATH="/usr/lib/firestaff${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec /usr/lib/firestaff/firestaff-bin "$@"
WRAPPER
chmod 0755 "$PKG_ROOT/usr/bin/firestaff"
cp "$README_SRC" "$PKG_ROOT/usr/share/doc/$PKG_NAME/README.md"
cp "$RELEASE_NOTES_SRC" "$PKG_ROOT/usr/share/doc/$PKG_NAME/RELEASE_NOTES.md"
cp "$ROOT/assets/branding/firestaff-logo.png" "$PKG_ROOT/usr/share/pixmaps/firestaff.png"
cat > "$PKG_ROOT/usr/share/applications/firestaff.desktop" <<DESKTOP
[Desktop Entry]
Type=Application
Name=Firestaff
Comment=$SUMMARY
Exec=firestaff
Icon=firestaff
Categories=Game;RolePlaying;
Terminal=false
DESKTOP

installed_kib="$(du -sk "$PKG_ROOT/usr" | awk '{print $1}')"
installed_size="$((installed_kib * 1024))"
builddate="$(date +%s)"

cat > "$PKG_ROOT/.PKGINFO" <<PKGINFO
pkgname = $PKG_NAME
pkgbase = $PKG_NAME
xdata = pkgtype=pkg
pkgver = $PKG_FULL_VERSION
pkgdesc = $SUMMARY
url = https://github.com/yeager/firestaff
builddate = $builddate
packager = Firestaff Preview <noreply@github.com>
size = $installed_size
arch = $PKG_ARCH
license = MIT
PKGINFO

cat > "$PKG_ROOT/.BUILDINFO" <<BUILDINFO
format = 2
pkgname = $PKG_NAME
pkgbase = $PKG_NAME
pkgver = $PKG_FULL_VERSION
pkgarch = $PKG_ARCH
pkgbuild_sha256sum = $(script_sha256 "$ROOT/scripts/package_steamdeck_preview.sh")
packager = Firestaff Preview <noreply@github.com>
builddate = $builddate
builddir = $BUILD_DIR
startdir = $ROOT
buildtool = firestaff-package-script
buildtoolver = $PKG_FULL_VERSION
buildenv = !distcc
buildenv = !color
buildenv = !ccache
buildenv = check
buildenv = !sign
options = !strip
options = docs
options = !libtool
options = !staticlibs
options = emptydirs
BUILDINFO

(
  cd "$PKG_ROOT"
  bsdtar --format=mtree \
    --options='!all,use-set,type,uid,gid,mode,time,size,sha256' \
    -cf - \
    ./.BUILDINFO ./.PKGINFO ./usr \
    | gzip -n > .MTREE
)

rm -f "$PKG_PATH" "$PKG_PATH_CANONICAL"
(
  cd "$PKG_ROOT"
  bsdtar --format=ustar -cf - .BUILDINFO .MTREE .PKGINFO usr \
    | zstd -19 -T0 -q -o "$PKG_PATH"
)
cp "$PKG_PATH" "$PKG_PATH_CANONICAL"

ls -lh "$PKG_PATH" "$PKG_PATH_CANONICAL"
