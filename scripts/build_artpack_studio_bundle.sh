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
VENV_DIR="$OUT_DIR/venv"

if [[ ! -f "$SCRIPT" ]]; then
  echo "Missing Artpack Studio source: $SCRIPT" >&2
  exit 1
fi

if [[ "$(uname -s)" == "Darwin" ]]; then
  # Tk 8.5 supplied by Xcode paints an empty, CPU-hungry window on current
  # macOS. The release workflow supplies Homebrew's python-tk runtime.
  for candidate in "${FIRESTAFF_ARTPACK_PYTHON:-}" "$(command -v python3 2>/dev/null || true)" /usr/bin/python3 \
    /Applications/Xcode.app/Contents/Developer/usr/bin/python3; do
    if [[ -n "$candidate" && -x "$candidate" ]] && "$candidate" -c '
import tkinter as tk
raise SystemExit(0 if tuple(map(int, str(tk.TkVersion).split(".")[:2])) >= (8, 6) else 1)
' >/dev/null 2>&1; then
      PYTHON="$candidate"
      break
    fi
  done
  if [[ -z "${PYTHON:-}" ]]; then
    echo "A Python 3 installation with Tk 8.6 or newer is required to bundle Artpack Studio" >&2
    exit 1
  fi
elif command -v python3 >/dev/null 2>&1; then
  PYTHON="$(command -v python3)"
elif command -v python >/dev/null 2>&1; then
  PYTHON="$(command -v python)"
else
  echo "Python 3 is required to bundle Artpack Studio" >&2
  exit 1
fi

rm -rf "$OUT_DIR/dist" "$WORK_DIR" "$SPEC_DIR" "$VENV_DIR"
mkdir -p "$OUT_DIR/dist" "$WORK_DIR" "$SPEC_DIR"

# CI images may mark their system Python as externally managed.  A local venv
# keeps Pillow and PyInstaller self-contained without modifying that runtime.
IS_MSYS=false
case "$(uname -s)" in
  MINGW*|MSYS*)
    # MSYS provides matching binary Pillow/PyInstaller packages.  PyPI has no
    # wheel for its rolling Python yet, so compiling Pillow is unreliable.
    IS_MSYS=true
    ;;
esac
if [[ "$IS_MSYS" == "true" ]]; then
  "$PYTHON" -m venv --system-site-packages "$VENV_DIR"
else
  "$PYTHON" -m venv "$VENV_DIR"
fi
PYTHON="$VENV_DIR/bin/python"
# MSYS Python uses the POSIX venv layout even on Windows; native Windows
# Python creates Scripts/python.exe.  Prefer the layout that actually exists.
if [[ ! -x "$PYTHON" && -x "$VENV_DIR/Scripts/python.exe" ]]; then
  PYTHON="$VENV_DIR/Scripts/python.exe"
fi
if [[ "$IS_MSYS" == "true" ]]; then
  "$PYTHON" -c 'import PIL, PyInstaller'
else
  "$PYTHON" -m pip install --upgrade Pillow pyinstaller
fi
"$PYTHON" -c 'import tkinter; from PIL import ImageTk'

case "$(uname -s)" in
  Darwin)
    # A real .app appears in Finder and Launchpad alongside Firestaff.
    if [[ -f "$ROOT/assets/icons/firestaff.icns" ]]; then
      "$PYTHON" -m PyInstaller --noconfirm --clean --windowed --onedir \
        --collect-all tkinter --hidden-import tkinter.ttk --hidden-import PIL.ImageTk \
        --name "Firestaff Artpack Studio" \
        --icon "$ROOT/assets/icons/firestaff.icns" \
        --distpath "$OUT_DIR/dist" --workpath "$WORK_DIR" --specpath "$SPEC_DIR" \
        "$SCRIPT"
    else
      "$PYTHON" -m PyInstaller --noconfirm --clean --windowed --onedir \
        --collect-all tkinter --hidden-import tkinter.ttk --hidden-import PIL.ImageTk \
        --name "Firestaff Artpack Studio" \
        --distpath "$OUT_DIR/dist" --workpath "$WORK_DIR" --specpath "$SPEC_DIR" \
        "$SCRIPT"
    fi
    ;;
  *)
    # One executable carries Python, Pillow, Tk and the studio source.
    "$PYTHON" -m PyInstaller --noconfirm --clean --windowed --onefile \
      --collect-all tkinter --hidden-import tkinter.ttk --hidden-import PIL.ImageTk \
      --name firestaff_artpack_studio \
      --distpath "$OUT_DIR/dist" --workpath "$WORK_DIR" --specpath "$SPEC_DIR" \
      "$SCRIPT"
    ;;
esac

if [[ "$(uname -s)" == "Darwin" ]]; then
  "$OUT_DIR/dist/Firestaff Artpack Studio.app/Contents/MacOS/Firestaff Artpack Studio" --check-tkinter
  "$OUT_DIR/dist/Firestaff Artpack Studio.app/Contents/MacOS/Firestaff Artpack Studio" --smoke-ui
else
  "$OUT_DIR/dist/firestaff_artpack_studio" --check-tkinter
  # Linux release runners are headless. Exercise the non-GUI paths there;
  # macOS above performs the actual widget-tree test with a window server.
  "$OUT_DIR/dist/firestaff_artpack_studio" --self-test
fi

find "$OUT_DIR/dist" -maxdepth 2 -type f -o -type d
