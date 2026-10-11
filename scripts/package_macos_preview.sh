#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
APP_NAME="Firestaff"
VERSION="${VERSION:-${VERSION:-dev}}"
SHORT_VERSION="${SHORT_VERSION:-$VERSION}"
PACKAGE_SUFFIX="${PACKAGE_SUFFIX:-}"
ARTIFACT_VERSION="${ARTIFACT_VERSION:-${VERSION}${PACKAGE_SUFFIX}}"
STAGE_DIR="$ROOT/release/macos-stage"
APP_DIR="$STAGE_DIR/$APP_NAME.app"
CONTENTS_DIR="$APP_DIR/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
RESOURCES_DIR="$CONTENTS_DIR/Resources"
FRAMEWORKS_DIR="$CONTENTS_DIR/Frameworks"
DMG_DIR="$ROOT/release"
DMG_PATH="$DMG_DIR/Firestaff-${ARTIFACT_VERSION}.dmg"
ZIP_PATH="$DMG_DIR/Firestaff-${ARTIFACT_VERSION}.zip"
README_SRC="$ROOT/README.md"
RELEASE_NOTES_SRC="${RELEASE_NOTES_SRC:-$ROOT/README.md}"
RELEASE_NOTES_VERSION="${RELEASE_NOTES_VERSION:-}"
BIN_SRC="$BUILD_DIR/firestaff"
ARTPACK_STUDIO_APP_SRC="${ARTPACK_STUDIO_APP_SRC:-$BUILD_DIR/artpack-studio-bundle/dist/Firestaff Artpack Studio.app}"
DUNGEON_STUDIO_APP_SRC="${DUNGEON_STUDIO_APP_SRC:-$BUILD_DIR/dungeon-studio-bundle/dist/Firestaff Dungeon Studio.app}"
SAVEGAME_EDITOR_APP_SRC="${SAVEGAME_EDITOR_APP_SRC:-$BUILD_DIR/savegame-editor-bundle/dist/Firestaff Savegame Editor.app}"
SDL_DYLIB="$(otool -L "$BIN_SRC" | awk '/libSDL3.*dylib/ {print $1; exit}')"
SDL_TTF_DYLIB="$(otool -L "$BIN_SRC" | awk '/libSDL3_ttf.*dylib/ {print $1; exit}')"

if [[ ! -x "$BIN_SRC" ]]; then
  echo "Missing built binary: $BIN_SRC" >&2
  exit 1
fi
if [[ ! -d "$ARTPACK_STUDIO_APP_SRC/Contents/MacOS" ]]; then
  echo "Missing bundled Artpack Studio app: $ARTPACK_STUDIO_APP_SRC" >&2
  exit 1
fi
if [[ ! -d "$DUNGEON_STUDIO_APP_SRC/Contents/MacOS" ]]; then
  echo "Missing bundled Dungeon Studio app: $DUNGEON_STUDIO_APP_SRC" >&2
  exit 1
fi
if [[ ! -d "$SAVEGAME_EDITOR_APP_SRC/Contents/MacOS" ]]; then
  echo "Missing bundled Savegame Editor app: $SAVEGAME_EDITOR_APP_SRC" >&2
  exit 1
fi

if [[ -z "${SDL_DYLIB:-}" || ! -f "$SDL_DYLIB" ]]; then
  echo "Could not locate linked SDL3 dylib" >&2
  exit 1
fi

rm -rf "$STAGE_DIR"
mkdir -p "$MACOS_DIR" "$RESOURCES_DIR" "$FRAMEWORKS_DIR" "$DMG_DIR"
mkdir -p "$RESOURCES_DIR/scripts"

cp "$BIN_SRC" "$MACOS_DIR/Firestaff"
cp -R "$ARTPACK_STUDIO_APP_SRC" "$STAGE_DIR/Firestaff Artpack Studio.app"
cp -R "$DUNGEON_STUDIO_APP_SRC" "$STAGE_DIR/Firestaff Dungeon Studio.app"
cp -R "$SAVEGAME_EDITOR_APP_SRC" "$STAGE_DIR/Firestaff Savegame Editor.app"
RUNTIME_PREFIX="$(brew --prefix 2>/dev/null || true)"
if [[ -n "${SDL_TTF_DYLIB:-}" && ! -f "$SDL_TTF_DYLIB" ]]; then
  echo "Could not locate linked SDL3_ttf dylib: $SDL_TTF_DYLIB" >&2
  exit 1
fi
cp -L "$SDL_DYLIB" "$FRAMEWORKS_DIR/$(basename "$SDL_DYLIB")"
if [[ -n "${SDL_TTF_DYLIB:-}" ]]; then
  cp -L "$SDL_TTF_DYLIB" "$FRAMEWORKS_DIR/$(basename "$SDL_TTF_DYLIB")"
fi
declare -a runtime_queue=("$SDL_DYLIB")
if [[ -n "${SDL_TTF_DYLIB:-}" ]]; then
  runtime_queue+=("$SDL_TTF_DYLIB")
fi
runtime_seen='|'
while ((${#runtime_queue[@]})); do
  runtime_source="${runtime_queue[0]}"
  runtime_queue=("${runtime_queue[@]:1}")
  [[ -n "$RUNTIME_PREFIX" && "$runtime_source" == "$RUNTIME_PREFIX"/* ]] || continue
  runtime_name="$(basename "$runtime_source")"
  case "$runtime_seen" in *"|$runtime_source|"*) continue ;; esac
  runtime_seen+="$runtime_source|"
  if [[ ! -e "$FRAMEWORKS_DIR/$runtime_name" ]]; then
    cp -L "$runtime_source" "$FRAMEWORKS_DIR/$runtime_name"
  fi
  while IFS= read -r runtime_dep; do
    [[ "$runtime_dep" == "$RUNTIME_PREFIX"/* ]] && runtime_queue+=("$runtime_dep")
  done < <(otool -L "$runtime_source" | tail -n +2 | awk '{print $1}')
done
cp "$README_SRC" "$STAGE_DIR/README.md"
if [[ -n "$RELEASE_NOTES_VERSION" ]]; then
  python3 "$ROOT/scripts/extract_release_notes.py" \
    --notes "$RELEASE_NOTES_SRC" \
    --version "$RELEASE_NOTES_VERSION" \
    --output "$STAGE_DIR/RELEASE_NOTES.md"
else
  cp "$RELEASE_NOTES_SRC" "$STAGE_DIR/RELEASE_NOTES.md"
fi
if [[ -f "$ROOT/assets/icons/firestaff.icns" ]]; then
  cp "$ROOT/assets/icons/firestaff.icns" "$RESOURCES_DIR/firestaff.icns"
fi
if [[ -f "$ROOT/assets/branding/firestaff-logo.png" ]]; then
  cp "$ROOT/assets/branding/firestaff-logo.png" "$RESOURCES_DIR/firestaff-logo.png"
fi
cp "$ROOT/assets/branding/firestaff-startup-intro.ppm" "$RESOURCES_DIR/firestaff-startup-intro.ppm"
cp "$ROOT/assets/branding/firestaff-startup-dungeon-v2.ppm" "$RESOURCES_DIR/firestaff-startup-dungeon-v2.ppm"
cp "$ROOT/assets/cards/platforms/platforms.ppm" "$RESOURCES_DIR/firestaff-platform-cards.ppm"

cat > "$CONTENTS_DIR/Info.plist" <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleExecutable</key>
  <string>Firestaff</string>
  <key>CFBundleIdentifier</key>
  <string>com.firestaff.app</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>Firestaff</string>
  <key>CFBundleIconFile</key>
  <string>firestaff.icns</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleShortVersionString</key>
  <string>${SHORT_VERSION}</string>
  <key>CFBundleVersion</key>
  <string>${VERSION}</string>
  <key>LSMinimumSystemVersion</key>
  <string>14.0</string>
  <key>NSHighResolutionCapable</key>
  <true/>
</dict>
</plist>
EOF

for staged_binary in "$MACOS_DIR/Firestaff" "$FRAMEWORKS_DIR"/*.dylib; do
  [[ -f "$staged_binary" ]] || continue
  if [[ "$staged_binary" != "$MACOS_DIR/Firestaff" ]]; then
    install_name_tool -id "@rpath/$(basename "$staged_binary")" "$staged_binary"
  fi
  while IFS= read -r runtime_dep; do
    [[ -n "${RUNTIME_PREFIX:-}" && "$runtime_dep" == "$RUNTIME_PREFIX"/* ]] || continue
    runtime_name="$(basename "$runtime_dep")"
    if [[ -e "$FRAMEWORKS_DIR/$runtime_name" ]]; then
      install_name_tool -change "$runtime_dep" "@rpath/$runtime_name" "$staged_binary"
    fi
  done < <(otool -L "$staged_binary" | tail -n +2 | awk '{print $1}')
done
if ! otool -l "$MACOS_DIR/Firestaff" | grep -q 'path @executable_path/../Frameworks'; then
  install_name_tool -add_rpath '@executable_path/../Frameworks' "$MACOS_DIR/Firestaff"
fi
chmod +x "$MACOS_DIR/Firestaff"
ln -s /Applications "$STAGE_DIR/Applications"

codesign --force --deep --sign - "$APP_DIR" >/dev/null 2>&1 || true
rm -f "$DMG_PATH" "$ZIP_PATH"
# Keep both launchable applications in the ZIP, not just Firestaff.app.
(
  cd "$STAGE_DIR"
  /usr/bin/ditto -c -k --sequesterRsrc . "$ZIP_PATH"
)
# GitHub's arm64 macOS runners occasionally retain a transient DiskImages lock.
# Retry the idempotent create operation rather than failing an otherwise complete release.
for attempt in 1 2 3; do
  if hdiutil create -volname "Firestaff" -srcfolder "$STAGE_DIR" -ov -format UDZO "$DMG_PATH" >/dev/null; then
    break
  fi
  if [[ "$attempt" == 3 ]]; then
    echo "Could not create DMG after ${attempt} attempts: $DMG_PATH" >&2
    exit 1
  fi
  rm -f "$DMG_PATH"
  sleep "$attempt"
done

echo "Created: $DMG_PATH"
echo "Created: $ZIP_PATH"
ls -lh "$DMG_PATH" "$ZIP_PATH"
