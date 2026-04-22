#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
APP_NAME="Firestaff"
VERSION="0.1.0-mac-preview"
STAGE_DIR="$ROOT/release/macos-stage"
APP_DIR="$STAGE_DIR/$APP_NAME.app"
CONTENTS_DIR="$APP_DIR/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
RESOURCES_DIR="$CONTENTS_DIR/Resources"
FRAMEWORKS_DIR="$CONTENTS_DIR/Frameworks"
DMG_DIR="$ROOT/release"
DMG_PATH="$DMG_DIR/Firestaff-${VERSION}.dmg"
ZIP_PATH="$DMG_DIR/Firestaff-${VERSION}.zip"
README_SRC="$ROOT/README.md"
RELEASE_NOTES_SRC="$ROOT/RELEASE_0_1_0_MAC_PREVIEW.md"
BIN_SRC="$BUILD_DIR/firestaff"
SDL_DYLIB="$(otool -L "$BIN_SRC" | awk '/libSDL3.*dylib/ {print $1; exit}')"

if [[ ! -x "$BIN_SRC" ]]; then
  echo "Missing built binary: $BIN_SRC" >&2
  exit 1
fi

if [[ -z "${SDL_DYLIB:-}" || ! -f "$SDL_DYLIB" ]]; then
  echo "Could not locate linked SDL3 dylib" >&2
  exit 1
fi

rm -rf "$STAGE_DIR"
mkdir -p "$MACOS_DIR" "$RESOURCES_DIR" "$FRAMEWORKS_DIR" "$DMG_DIR"

cp "$BIN_SRC" "$MACOS_DIR/Firestaff"
cp "$SDL_DYLIB" "$FRAMEWORKS_DIR/$(basename "$SDL_DYLIB")"
cp "$README_SRC" "$STAGE_DIR/README.md"
cp "$RELEASE_NOTES_SRC" "$STAGE_DIR/RELEASE_NOTES.md"
cp "$ROOT/assets/branding/firestaff-logo.png" "$RESOURCES_DIR/firestaff-logo.png"

cat > "$CONTENTS_DIR/Info.plist" <<'EOF'
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>CFBundleDevelopmentRegion</key>
  <string>en</string>
  <key>CFBundleExecutable</key>
  <string>Firestaff</string>
  <key>CFBundleIdentifier</key>
  <string>com.firestaff.preview</string>
  <key>CFBundleInfoDictionaryVersion</key>
  <string>6.0</string>
  <key>CFBundleName</key>
  <string>Firestaff</string>
  <key>CFBundlePackageType</key>
  <string>APPL</string>
  <key>CFBundleShortVersionString</key>
  <string>0.1.0</string>
  <key>CFBundleVersion</key>
  <string>0.1.0-mac-preview</string>
  <key>LSMinimumSystemVersion</key>
  <string>14.0</string>
  <key>NSHighResolutionCapable</key>
  <true/>
</dict>
</plist>
EOF

install_name_tool -change "$SDL_DYLIB" "@executable_path/../Frameworks/$(basename "$SDL_DYLIB")" "$MACOS_DIR/Firestaff"
chmod +x "$MACOS_DIR/Firestaff"
ln -s /Applications "$STAGE_DIR/Applications"

codesign --force --deep --sign - "$APP_DIR" >/dev/null 2>&1 || true
rm -f "$DMG_PATH" "$ZIP_PATH"
/usr/bin/ditto -c -k --sequesterRsrc --keepParent "$APP_DIR" "$ZIP_PATH"
hdiutil create -volname "Firestaff Preview" -srcfolder "$STAGE_DIR" -ov -format UDZO "$DMG_PATH" >/dev/null

echo "Created: $DMG_PATH"
echo "Created: $ZIP_PATH"
ls -lh "$DMG_PATH" "$ZIP_PATH"
