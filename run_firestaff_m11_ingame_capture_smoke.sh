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
  python3 - "$path" <<'PY'
from pathlib import Path
import sys

path = Path(sys.argv[1])
data = path.read_bytes()
parts = data.split(None, 4)
if len(parts) < 5 or parts[0] != b"P6":
    raise SystemExit(f"invalid PPM header: {path.name}")
w = int(parts[1])
h = int(parts[2])
maxval = int(parts[3])
payload = parts[4]
expected = w * h * 3
if (w, h, maxval) != (320, 200, 255):
    raise SystemExit(f"unexpected PPM geometry in {path.name}: {w}x{h} max={maxval}")
if len(payload) != expected:
    raise SystemExit(f"unexpected PPM payload size in {path.name}: {len(payload)} != {expected}")
PY
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

python3 - "$OUT_DIR/02_ingame_turn_right_latest.ppm" "$OUT_DIR/06_ingame_inventory_panel_latest.ppm" "$OUT_DIR/04_ingame_spell_panel_latest.ppm" <<'PY'
from pathlib import Path
import sys

def read_ppm(path):
    data = Path(path).read_bytes()
    parts = data.split(None, 4)
    if len(parts) < 5 or parts[0] != b"P6":
        raise SystemExit(f"invalid PPM header: {Path(path).name}")
    w = int(parts[1]); h = int(parts[2]); maxval = int(parts[3])
    if (w, h, maxval) != (320, 200, 255):
        raise SystemExit(f"unexpected PPM geometry in {Path(path).name}: {w}x{h} max={maxval}")
    return w, h, parts[4]

def count_rgb(pixels, w, x0, y0, x1, y1, rgb):
    n = 0
    for y in range(y0, y1):
        for x in range(x0, x1):
            i = (y * w + x) * 3
            if tuple(pixels[i:i + 3]) == rgb:
                n += 1
    return n

action_w, _, action_pixels = read_ppm(sys.argv[1])
inv_w, _, inv_pixels = read_ppm(sys.argv[2])
spell_w, _, spell_pixels = read_ppm(sys.argv[3])

# The deterministic capture champion holds a dagger.  In the right-side
# action cell, source dagger icon colour 12 must be remapped by G0498 to
# cyan.  Slot 0 inner icon box is x=235..250, y=95..110.
dm_cyan = (0, 219, 219)
action_cyan = count_rgb(action_pixels, action_w, 235, 95, 251, 111, dm_cyan)
if action_cyan < 180:
    raise SystemExit(
        f"action fixture dagger icon did not show G0498 cyan coverage: {action_cyan}/256"
    )

# In the inventory panel the same source dagger icon must be blitted without
# the action-area palette rewrite.  Right-hand slot icon inset is x=34..49,
# y=53..68; source colour 12 is DM dark gray in the capture palette.
dm_dark_gray = (73, 73, 73)
inv_dark_gray = count_rgb(inv_pixels, inv_w, 34, 53, 50, 69, dm_dark_gray)
if inv_dark_gray < 180:
    raise SystemExit(
        f"inventory fixture dagger icon did not preserve source dark gray: {inv_dark_gray}/256"
    )

# After the scripted first rune input, normal V1 should keep spell feedback
# inside the DM1 right-column spell area, not Firestaff's old modal viewport
# workbench.  The selected source C011 rune-label cell currently appears in
# the top-right spell-panel strip at x=248..261, y=43..55 and keeps the native
# brown/red pattern.
dm_brown = (219, 146, 109)
dm_red = (255, 0, 0)
selected_brown = count_rgb(spell_pixels, spell_w, 248, 43, 262, 56, dm_brown)
selected_red = count_rgb(spell_pixels, spell_w, 248, 43, 262, 56, dm_red)
if selected_brown < 40 or selected_red < 20:
    raise SystemExit(
        "spell fixture did not show right-column native C011 selected-rune cell "
        f"(brown={selected_brown}, red={selected_red})"
    )
PY

echo "In-game capture smoke PASS: ${#expected[@]} screenshots"
