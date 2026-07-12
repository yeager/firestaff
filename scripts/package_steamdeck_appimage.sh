#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
VERSION="${VERSION:-0.2.9-preview}"
RELEASE_NOTES_SRC="${RELEASE_NOTES_SRC:-$ROOT/README.md}"
APPIMAGETOOL="${APPIMAGETOOL:-appimagetool}"
BIN_SRC="$BUILD_DIR/firestaff"
OUT_DIR="$ROOT/release"
APPDIR="$OUT_DIR/steamdeck-appimage-stage/AppDir"
APPIMAGE_PATH="$OUT_DIR/Firestaff-${VERSION}-steamdeck-x86_64.AppImage"

if [[ ! -x "$BIN_SRC" ]]; then
  echo "Missing built binary: $BIN_SRC" >&2
  exit 1
fi
if ! command -v "$APPIMAGETOOL" >/dev/null 2>&1; then
  echo "Missing appimagetool; install it or set APPIMAGETOOL." >&2
  exit 1
fi

SDL3_LIB="${SDL3_LIB:-}"
if [[ -z "$SDL3_LIB" ]] && command -v ldd >/dev/null 2>&1; then
  SDL3_LIB="$(ldd "$BIN_SRC" 2>/dev/null | awk '/libSDL3[.]so/ {print $3; exit}')"
fi
if [[ -z "$SDL3_LIB" || ! -f "$SDL3_LIB" ]]; then
  echo "Missing SDL3 runtime library for Steam Deck AppImage." >&2
  echo "Set SDL3_LIB=/path/to/libSDL3.so.0 or build on Linux with ldd support." >&2
  exit 1
fi

rm -rf "$APPDIR"
mkdir -p \
  "$APPDIR/usr/bin" \
  "$APPDIR/usr/lib/firestaff" \
  "$APPDIR/usr/share/applications" \
  "$APPDIR/usr/share/icons/hicolor/256x256/apps" \
  "$APPDIR/usr/share/doc/firestaff"

cp "$BIN_SRC" "$APPDIR/usr/bin/firestaff"
chmod 0755 "$APPDIR/usr/bin/firestaff"
cp -L "$SDL3_LIB" "$APPDIR/usr/lib/firestaff/libSDL3.so.0"
chmod 0755 "$APPDIR/usr/lib/firestaff/libSDL3.so.0"
cp "$ROOT/assets/branding/firestaff-logo.png" \
  "$APPDIR/usr/share/icons/hicolor/256x256/apps/firestaff.png"
cp "$ROOT/README.md" "$APPDIR/usr/share/doc/firestaff/README.md"
cp "$RELEASE_NOTES_SRC" "$APPDIR/usr/share/doc/firestaff/RELEASE_NOTES.md"

cat > "$APPDIR/firestaff.desktop" <<'DESKTOP'
[Desktop Entry]
Type=Application
Name=Firestaff
Comment=Firestaff source-faithful Dungeon Master engine
Exec=firestaff
Icon=firestaff
Categories=Game;RolePlaying;
Terminal=false
DESKTOP
cp "$APPDIR/firestaff.desktop" "$APPDIR/usr/share/applications/firestaff.desktop"
ln -sf usr/share/icons/hicolor/256x256/apps/firestaff.png "$APPDIR/firestaff.png"

cat > "$APPDIR/AppRun" <<'APPRUN'
#!/usr/bin/env sh
set -eu
HERE="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
export LD_LIBRARY_PATH="$HERE/usr/lib/firestaff${LD_LIBRARY_PATH:+:$LD_LIBRARY_PATH}"
exec "$HERE/usr/bin/firestaff" "$@"
APPRUN
chmod 0755 "$APPDIR/AppRun"

mkdir -p "$OUT_DIR"
rm -f "$APPIMAGE_PATH"
APPIMAGE_EXTRACT_AND_RUN=1 ARCH=x86_64 "$APPIMAGETOOL" "$APPDIR" "$APPIMAGE_PATH"
chmod 0755 "$APPIMAGE_PATH"
ls -lh "$APPIMAGE_PATH"
