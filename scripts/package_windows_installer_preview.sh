#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION="${VERSION:-0.2.9-preview}"
ZIP_STAGE="$ROOT/release/windows-stage/Firestaff-${VERSION}-windows"
OUT_DIR="$ROOT/release"
INSTALLER_PATH="$OUT_DIR/Firestaff-${VERSION}-windows-installer.exe"
NSI_PATH="$OUT_DIR/Firestaff-${VERSION}.nsi"

if [[ ! -d "$ZIP_STAGE" ]]; then
  echo "Missing Windows stage directory: $ZIP_STAGE" >&2
  echo "Run package_windows_preview.sh first." >&2
  exit 1
fi

MAKENSIS="${MAKENSIS:-}"
if [[ -z "$MAKENSIS" ]]; then
  for candidate in \
    makensis \
    "/c/Program Files (x86)/NSIS/makensis.exe" \
    "/c/Program Files/NSIS/makensis.exe"; do
    if command -v "$candidate" >/dev/null 2>&1 || [[ -x "$candidate" ]]; then
      MAKENSIS="$candidate"
      break
    fi
  done
fi
if [[ -z "$MAKENSIS" ]]; then
  echo "Could not locate makensis. Install NSIS first." >&2
  exit 1
fi

mkdir -p "$OUT_DIR"
cat > "$NSI_PATH" <<NSI
Unicode true
Name "Firestaff Preview"
OutFile "$(cygpath -w "$INSTALLER_PATH" 2>/dev/null || echo "$INSTALLER_PATH")"
InstallDir "\$LOCALAPPDATA\\Firestaff Preview"
RequestExecutionLevel user

Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "Firestaff" SEC01
  SetOutPath "\$INSTDIR"
  File /r "$(cygpath -w "$ZIP_STAGE" 2>/dev/null || echo "$ZIP_STAGE")\\*"
  CreateShortcut "\$DESKTOP\\Firestaff Preview.lnk" "\$INSTDIR\\firestaff.exe"
  WriteUninstaller "\$INSTDIR\\Uninstall.exe"
SectionEnd

Section "Uninstall"
  Delete "\$DESKTOP\\Firestaff Preview.lnk"
  RMDir /r "\$INSTDIR"
SectionEnd
NSI

"$MAKENSIS" "$NSI_PATH"
ls -lh "$INSTALLER_PATH"
