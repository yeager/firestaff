#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
VERSION="${VERSION:-0.2.9-preview}"
ZIP_STAGE="$ROOT/release/windows-stage/Firestaff-${VERSION}-windows"
OUT_DIR="$ROOT/release"
INSTALLER_PATH="$OUT_DIR/Firestaff-${VERSION}-windows-installer.exe"
ISS_PATH="$OUT_DIR/Firestaff-${VERSION}.iss"

if [[ ! -d "$ZIP_STAGE" ]]; then
  echo "Missing Windows stage directory: $ZIP_STAGE" >&2
  echo "Run package_windows_preview.sh first." >&2
  exit 1
fi

ISCC="${ISCC:-}"
if [[ -z "$ISCC" ]]; then
  for candidate in \
    iscc \
    ISCC.exe \
    "/c/Program Files (x86)/Inno Setup 6/ISCC.exe" \
    "/c/Program Files/Inno Setup 6/ISCC.exe"; do
    if command -v "$candidate" >/dev/null 2>&1 || [[ -x "$candidate" ]]; then
      ISCC="$candidate"
      break
    fi
  done
fi
if [[ -z "$ISCC" ]]; then
  echo "Could not locate ISCC.exe. Install Inno Setup first." >&2
  exit 1
fi

mkdir -p "$OUT_DIR"
ZIP_STAGE_WIN="$(cygpath -w "$ZIP_STAGE" 2>/dev/null || echo "$ZIP_STAGE")"
OUT_DIR_WIN="$(cygpath -w "$OUT_DIR" 2>/dev/null || echo "$OUT_DIR")"
cat > "$ISS_PATH" <<ISS
#define FirestaffVersion "$VERSION"
#define FirestaffSource "$ZIP_STAGE_WIN"

[Setup]
AppId={{4DF59FE5-0100-4F99-9771-7914E3F03069}}
AppName=Firestaff Preview
AppVersion={#FirestaffVersion}
AppPublisher=Firestaff
DefaultDirName={localappdata}\\Firestaff Preview
DefaultGroupName=Firestaff Preview
DisableProgramGroupPage=yes
OutputDir=$OUT_DIR_WIN
OutputBaseFilename=Firestaff-{#FirestaffVersion}-windows-installer
Compression=lzma2
SolidCompression=yes
PrivilegesRequired=lowest
SetupLogging=yes
UninstallDisplayIcon={app}\\firestaff.exe

[Files]
Source: "{#FirestaffSource}\\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{autodesktop}\\Firestaff Preview"; Filename: "{app}\\firestaff.exe"
Name: "{group}\\Firestaff Preview"; Filename: "{app}\\firestaff.exe"
Name: "{group}\\Firestaff Artpack Studio"; Filename: "{app}\\firestaff_artpack_studio.exe"
Name: "{group}\\Firestaff Dungeon Studio"; Filename: "{app}\\firestaff_dungeon_studio.exe"
Name: "{group}\\Firestaff Savegame Editor"; Filename: "{app}\\firestaff_savegame_editor.exe"
Name: "{group}\\Uninstall Firestaff Preview"; Filename: "{uninstallexe}"
ISS

"$ISCC" "$ISS_PATH"
ls -lh "$INSTALLER_PATH"
