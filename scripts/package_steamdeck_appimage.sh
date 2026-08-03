#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
VERSION="${VERSION:-0.2.9-preview}"
RELEASE_NOTES_SRC="${RELEASE_NOTES_SRC:-$ROOT/README.md}"
APPIMAGETOOL="${APPIMAGETOOL:-appimagetool}"
BIN_SRC="$BUILD_DIR/firestaff"
ARTPACK_STUDIO_BIN_SRC="${ARTPACK_STUDIO_BIN_SRC:-$BUILD_DIR/artpack-studio-bundle/dist/firestaff_artpack_studio}"
DUNGEON_STUDIO_BIN_SRC="${DUNGEON_STUDIO_BIN_SRC:-$BUILD_DIR/dungeon-studio-bundle/dist/firestaff_dungeon_studio}"
SAVEGAME_EDITOR_BIN_SRC="${SAVEGAME_EDITOR_BIN_SRC:-$BUILD_DIR/savegame-editor-bundle/dist/firestaff_savegame_editor}"
OUT_DIR="$ROOT/release"
APPDIR="$OUT_DIR/steamdeck-appimage-stage/AppDir"
APPIMAGE_PATH="$OUT_DIR/Firestaff-${VERSION}-steamdeck-x86_64.AppImage"

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
  "$APPDIR/usr/share/firestaff/scripts" \
  "$APPDIR/usr/share/applications" \
  "$APPDIR/usr/share/icons/hicolor/256x256/apps" \
  "$APPDIR/usr/share/doc/firestaff"

cp "$BIN_SRC" "$APPDIR/usr/bin/firestaff"
cp "$ARTPACK_STUDIO_BIN_SRC" "$APPDIR/usr/bin/firestaff_artpack_studio"
cp "$DUNGEON_STUDIO_BIN_SRC" "$APPDIR/usr/bin/firestaff_dungeon_studio"
cp "$SAVEGAME_EDITOR_BIN_SRC" "$APPDIR/usr/bin/firestaff_savegame_editor"
cp "$ROOT/assets/branding/firestaff-startup-intro.ppm" "$APPDIR/usr/share/firestaff/firestaff-startup-intro.ppm"
chmod 0755 "$APPDIR/usr/bin/firestaff"
chmod 0755 "$APPDIR/usr/bin/firestaff_artpack_studio"
chmod 0755 "$APPDIR/usr/bin/firestaff_dungeon_studio"
chmod 0755 "$APPDIR/usr/bin/firestaff_savegame_editor"
cp -L "$SDL3_LIB" "$APPDIR/usr/lib/firestaff/libSDL3.so.0"
chmod 0755 "$APPDIR/usr/lib/firestaff/libSDL3.so.0"
if [[ -f "$ROOT/assets/branding/firestaff-logo.png" ]]; then
  cp "$ROOT/assets/branding/firestaff-logo.png" \
    "$APPDIR/usr/share/icons/hicolor/256x256/apps/firestaff.png"
else
  # AppImage requires a real root icon even when a branded source icon is
  # unavailable in a source checkout.  Emit a tiny opaque PNG fallback.
  python3 - "$APPDIR/usr/share/icons/hicolor/256x256/apps/firestaff.png" <<'PY'
import struct
import sys
import zlib

pixel = b'\x00\x22\xD7\xC8\xFF'
chunk = lambda kind, data: (struct.pack('>I', len(data)) + kind + data +
                            struct.pack('>I', zlib.crc32(kind + data) & 0xffffffff))
png = b'\x89PNG\r\n\x1a\n' + chunk(b'IHDR', struct.pack('>IIBBBBB', 1, 1, 8, 6, 0, 0, 0))
png += chunk(b'IDAT', zlib.compress(pixel)) + chunk(b'IEND', b'')
open(sys.argv[1], 'wb').write(png)
PY
fi
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
cat > "$APPDIR/usr/share/applications/firestaff-artpack-studio.desktop" <<'DESKTOP'
[Desktop Entry]
Type=Application
Name=Firestaff Artpack Studio
Comment=Create and edit Firestaff V2.2 artpacks
Exec=firestaff_artpack_studio
Icon=firestaff
Categories=Graphics;Game;RolePlaying;
Terminal=false
DESKTOP
cat > "$APPDIR/usr/share/applications/firestaff-savegame-editor.desktop" <<'DESKTOP'
[Desktop Entry]
Type=Application
Name=Firestaff Savegame Editor
Comment=Edit Firestaff savegame files
Exec=firestaff_savegame_editor
Icon=firestaff
Categories=Utility;Game;RolePlaying;
Terminal=false
DESKTOP
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
