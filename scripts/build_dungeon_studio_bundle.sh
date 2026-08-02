#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT_DIR="${1:-$ROOT/build/dungeon-studio-bundle}"
WORK_DIR="$OUT_DIR/work"
SPEC_DIR="$OUT_DIR/spec"
SCRIPT="$ROOT/scripts/firestaff_dungeon_studio.py"
VENV_DIR="$OUT_DIR/venv"
LOCALE_DIR="$ROOT/po/locale"
EXAMPLE_FILE="$ROOT/assets/examples/example_dungeon.fsdung"

if [[ ! -f "$SCRIPT" ]]; then
  echo "Missing Dungeon Studio source: $SCRIPT" >&2
  exit 1
fi

if [[ "$(uname -s)" == "Darwin" ]]; then
  for candidate in "${FIRESTAFF_DUNGEON_PYTHON:-}" "$(command -v python3 2>/dev/null || true)" /usr/bin/python3; do
    if [[ -n "$candidate" && -x "$candidate" ]] && "$candidate" -c '
import tkinter as tk
raise SystemExit(0 if tuple(map(int, str(tk.TkVersion).split(".")[:2])) >= (8, 6) else 1)
' >/dev/null 2>&1; then
      PYTHON="$candidate"
      break
    fi
  done
  if [[ -z "${PYTHON:-}" ]]; then
    echo "A Python 3 installation with Tk 8.6 or newer is required to bundle Dungeon Studio" >&2
    exit 1
  fi
elif command -v python3 >/dev/null 2>&1; then
  PYTHON="$(command -v python3)"
elif command -v python >/dev/null 2>&1; then
  PYTHON="$(command -v python)"
else
  echo "Python 3 is required to bundle Dungeon Studio" >&2
  exit 1
fi

rm -rf "$OUT_DIR/dist" "$WORK_DIR" "$SPEC_DIR" "$VENV_DIR"
mkdir -p "$OUT_DIR/dist" "$WORK_DIR" "$SPEC_DIR"

IS_MSYS=false
case "$(uname -s)" in
  MINGW*|MSYS*)
    IS_MSYS=true
    ;;
esac
if [[ "$IS_MSYS" == "true" ]]; then
  "$PYTHON" -m venv --system-site-packages "$VENV_DIR"
else
  "$PYTHON" -m venv "$VENV_DIR"
fi
PYTHON="$VENV_DIR/bin/python"
if [[ ! -x "$PYTHON" && -x "$VENV_DIR/Scripts/python.exe" ]]; then
  PYTHON="$VENV_DIR/Scripts/python.exe"
fi
if [[ "$IS_MSYS" == "true" ]]; then
  "$PYTHON" -c 'import PIL, PyInstaller'
else
  "$PYTHON" -m pip install --upgrade Pillow pyinstaller
fi
"$PYTHON" -c 'import tkinter; from PIL import ImageTk'

# Build --add-data args for locale files and example dungeon
ADD_DATA_ARGS=()
case "$(uname -s)" in
  MINGW*|MSYS*)
    _SEP=";"
    _cvt() { cygpath -w "$1"; }
    ;;
  *)
    _SEP=":"
    _cvt() { echo "$1"; }
    ;;
esac
if [[ -d "$LOCALE_DIR" ]]; then
  ADD_DATA_ARGS+=(--add-data "$(_cvt "$LOCALE_DIR")${_SEP}po/locale")
fi
if [[ -f "$EXAMPLE_FILE" ]]; then
  ADD_DATA_ARGS+=(--add-data "$(_cvt "$EXAMPLE_FILE")${_SEP}assets/examples")
fi

case "$(uname -s)" in
  Darwin)
    ICON_ARG=()
    if [[ -f "$ROOT/assets/icons/firestaff_dungeon_studio.icns" ]]; then
      ICON_ARG=(--icon "$ROOT/assets/icons/firestaff_dungeon_studio.icns")
    elif [[ -f "$ROOT/assets/icons/firestaff.icns" ]]; then
      ICON_ARG=(--icon "$ROOT/assets/icons/firestaff.icns")
    fi
    "$PYTHON" -m PyInstaller --noconfirm --clean --windowed --onedir \
      --collect-all tkinter --hidden-import tkinter.ttk --hidden-import PIL.ImageTk \
      --name "Firestaff Dungeon Studio" \
      "${ICON_ARG[@]}" \
      "${ADD_DATA_ARGS[@]}" \
      --distpath "$OUT_DIR/dist" --workpath "$WORK_DIR" --specpath "$SPEC_DIR" \
      "$SCRIPT"
    ;;
  *)
    "$PYTHON" -m PyInstaller --noconfirm --clean --windowed --onefile \
      --collect-all tkinter --hidden-import tkinter.ttk --hidden-import PIL.ImageTk \
      --name firestaff_dungeon_studio \
      "${ADD_DATA_ARGS[@]}" \
      --distpath "$OUT_DIR/dist" --workpath "$WORK_DIR" --specpath "$SPEC_DIR" \
      "$SCRIPT"
    ;;
esac

if [[ "$(uname -s)" == "Darwin" ]]; then
  "$OUT_DIR/dist/Firestaff Dungeon Studio.app/Contents/MacOS/Firestaff Dungeon Studio" --self-test
else
  "$OUT_DIR/dist/firestaff_dungeon_studio" --self-test
fi

find "$OUT_DIR/dist" -maxdepth 2 -type f -o -type d
