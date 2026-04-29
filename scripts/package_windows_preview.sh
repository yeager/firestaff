#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
VERSION="${VERSION:-0.2.5-preview}"
STAGE_DIR="$ROOT/release/windows-stage/Firestaff-${VERSION}-windows"
ZIP_PATH="$ROOT/release/Firestaff-${VERSION}-windows.zip"
README_SRC="$ROOT/README.md"
RELEASE_NOTES_SRC="${RELEASE_NOTES_SRC:-$ROOT/RELEASE_0_2_5_PREVIEW.md}"
BIN_SRC="$BUILD_DIR/firestaff.exe"

if [[ ! -x "$BIN_SRC" ]]; then
  echo "Missing built binary: $BIN_SRC" >&2
  exit 1
fi

rm -rf "$STAGE_DIR"
mkdir -p "$STAGE_DIR" "$ROOT/release"
cp "$BIN_SRC" "$STAGE_DIR/firestaff.exe"
cp "$README_SRC" "$STAGE_DIR/README.md"
cp "$RELEASE_NOTES_SRC" "$STAGE_DIR/RELEASE_NOTES.md"

SDL_DLL=""
for candidate in \
  "${MINGW_PREFIX:-}/bin/SDL3.dll" \
  "/ucrt64/bin/SDL3.dll" \
  "/mingw64/bin/SDL3.dll"; do
  if [[ -n "$candidate" && -f "$candidate" ]]; then
    SDL_DLL="$candidate"
    break
  fi
done

if [[ -z "$SDL_DLL" ]]; then
  echo "Could not locate SDL3.dll" >&2
  exit 1
fi
cp "$SDL_DLL" "$STAGE_DIR/SDL3.dll"

(
  cd "$ROOT/release/windows-stage"
  rm -f "$ZIP_PATH"
  if command -v 7z >/dev/null 2>&1; then
    7z a -tzip "$ZIP_PATH" "$(basename "$STAGE_DIR")" >/dev/null
  else
    powershell -NoProfile -Command "Compress-Archive -Path '$(basename "$STAGE_DIR")' -DestinationPath '$ZIP_PATH' -Force"
  fi
)

echo "Created: $ZIP_PATH"
ls -lh "$ZIP_PATH"
