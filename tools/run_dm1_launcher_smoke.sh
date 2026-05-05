#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
BIN="${FIRESTAFF_BIN:-$BUILD_DIR/firestaff}"
DATA_DIR="${FIRESTAFF_DATA:-${FIRESTAFF_DATA_DIR:-}}"

if [[ ! -x "$BIN" ]]; then
  echo "dm1-launcher-smoke: missing binary: $BIN" >&2
  exit 1
fi

if [[ -z "$DATA_DIR" ]]; then
  for candidate in \
    "$HOME/.openclaw/data/firestaff-original-games/DM/_canonical/dm1" \
    "$HOME/.firestaff/data" \
    "$ROOT/data/dm1"; do
    if [[ -f "$candidate/GRAPHICS.DAT" || -f "$candidate/Graphics.dat" || -f "$candidate/TITLE" ]]; then
      DATA_DIR="$candidate"
      break
    fi
  done
fi

if [[ -z "$DATA_DIR" || ! -d "$DATA_DIR" ]]; then
  echo "dm1-launcher-smoke: DM1 data not found; set FIRESTAFF_DATA=/path/to/dm1" >&2
  exit 2
fi

# Native modern launcher canvas is 1920x1080. The script clicks:
# 1. DM1 card center in the main menu.
# 2. Visible Launch button center in the game-options panel.
# FIRESTAFF_AUTOTEST lets the entrance wait auto-confirm after the launcher
# handoff, so the smoke catches the immediate Launch-click freeze/regression.
SCRIPT="click:590:500,click:960:609"

SDL_VIDEODRIVER="${SDL_VIDEODRIVER:-dummy}" \
FIRESTAFF_AUTOTEST=1 \
FIRESTAFF_FAIL_IF_NO_LAUNCH=1 \
FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
timeout "${FIRESTAFF_LAUNCH_SMOKE_TIMEOUT:-15s}" \
  "$BIN" \
  --duration "${FIRESTAFF_LAUNCH_SMOKE_DURATION_MS:-6500}" \
  --width 1920 \
  --height 1080 \
  --data-dir "$DATA_DIR" \
  --script "$SCRIPT"

echo "dm1-launcher-smoke: PASS ($DATA_DIR)"
