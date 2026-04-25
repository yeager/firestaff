#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${BUILD_DIR:-$ROOT/build}"
DATA_DIR="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"
CAPTURE_BIN="$BUILD_DIR/capture_ingame_series"
OUT_DIR="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-ingame-capture.XXXXXX")"

cleanup() {
  rm -rf "$OUT_DIR"
}
trap cleanup EXIT

if [[ ! -x "$CAPTURE_BIN" ]]; then
  echo "capture binary not executable: $CAPTURE_BIN" >&2
  exit 1
fi

if [[ ! -d "$DATA_DIR" ]]; then
  echo "data dir not found: $DATA_DIR" >&2
  exit 1
fi

"$CAPTURE_BIN" "$OUT_DIR" "$DATA_DIR" >/tmp/firestaff-ingame-capture-smoke.log 2>&1 || {
  cat /tmp/firestaff-ingame-capture-smoke.log >&2
  exit 1
}

expected=(
  01_ingame_start_latest.ppm
  02_ingame_turn_right_latest.ppm
  03_ingame_move_forward_latest.ppm
  04_ingame_spell_panel_latest.ppm
  05_ingame_after_cast_latest.ppm
  06_ingame_inventory_panel_latest.ppm
)

for name in "${expected[@]}"; do
  path="$OUT_DIR/$name"
  if [[ ! -s "$path" ]]; then
    echo "missing capture: $name" >&2
    exit 1
  fi
  if [[ "$(head -c 2 "$path")" != "P6" ]]; then
    echo "capture is not binary PPM/P6: $name" >&2
    exit 1
  fi
  size=$(wc -c < "$path")
  if (( size < 1000 )); then
    echo "capture too small: $name ($size bytes)" >&2
    exit 1
  fi
done

echo "In-game capture smoke PASS: ${#expected[@]} screenshots"
