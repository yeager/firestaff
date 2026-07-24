#!/usr/bin/env bash
set -euo pipefail

# Produce a self-contained Artpack Studio executable.  The application uses
# Pillow and Tk, so shipping the C launcher alone would make every release
# depend on a user-installed Python environment.

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${1:-$ROOT/build/artpack-studio-bundle}"
WORK_DIR="$OUT_DIR/work"
SPEC_DIR="$OUT_DIR/spec"
SCRIPT="$ROOT/scripts/firestaff_artpack_studio.py"

if [[ ! -f "$SCRIPT" ]]; then
  echo "Missing Artpack Studio source: $SCRIPT" >&2
  exit 1
fi

python3 -m pip install --user --upgrade Pillow pyinstaller

rm -rf "$OUT_DIR/dist" "$WORK_DIR" "$SPEC_DIR"
mkdir -p "$OUT_DIR/dist" "$WORK_DIR" "$SPEC_DIR"

case "$(uname -s)" in
  Darwin)
    # A real .app appears in Finder and Launchpad alongside Firestaff.
    if [[ -f "$ROOT/assets/icons/firestaff.icns" ]]; then
      python3 -m PyInstaller --noconfirm --clean --windowed --onedir \
        --name "Firestaff Artpack Studio" \
        --icon "$ROOT/assets/icons/firestaff.icns" \
        --distpath "$OUT_DIR/dist" --workpath "$WORK_DIR" --specpath "$SPEC_DIR" \
        "$SCRIPT"
    else
      python3 -m PyInstaller --noconfirm --clean --windowed --onedir \
        --name "Firestaff Artpack Studio" \
        --distpath "$OUT_DIR/dist" --workpath "$WORK_DIR" --specpath "$SPEC_DIR" \
        "$SCRIPT"
    fi
    ;;
  *)
    # One executable carries Python, Pillow, Tk and the studio source.
    python3 -m PyInstaller --noconfirm --clean --windowed --onefile \
      --name firestaff_artpack_studio \
      --distpath "$OUT_DIR/dist" --workpath "$WORK_DIR" --specpath "$SPEC_DIR" \
      "$SCRIPT"
    ;;
esac

find "$OUT_DIR/dist" -maxdepth 2 -type f -o -type d
