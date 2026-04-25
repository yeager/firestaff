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

python3 - "$OUT_DIR/01_ingame_start_latest.ppm" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
data = path.read_bytes()
parts = data.split(None, 4)
if len(parts) < 5 or parts[0] != b"P6":
    raise SystemExit(f"invalid PPM header: {path.name}")
w = int(parts[1])
h = int(parts[2])
pixels = parts[4]
if len(pixels) < w * h * 3:
    raise SystemExit(f"truncated PPM payload: {path.name}")

# Guard against the stale pre-decode capture binary that made the viewport
# look like saturated EGA/CGA rainbow static. Correct DM palette captures may
# contain bright reds/yellows in UI, but the dungeon viewport must not be
# dominated by these obsolete DAC colours.
stale_ega_colours = {
    (0, 0, 170),
    (85, 255, 85),
    (255, 85, 85),
    (255, 255, 85),
    (85, 85, 255),
}
bad = 0
total = 0
for y in range(33, min(169, h)):
    for x in range(0, min(224, w)):
        i = (y * w + x) * 3
        rgb = tuple(pixels[i:i + 3])
        total += 1
        if rgb in stale_ega_colours:
            bad += 1

if total == 0:
    raise SystemExit("empty viewport sample")
ratio = bad / total
if ratio > 0.01:
    raise SystemExit(
        f"stale EGA-like capture colours in viewport: {bad}/{total} ({ratio:.2%})"
    )
PY

echo "In-game capture smoke PASS: ${#expected[@]} screenshots"
