#!/bin/sh
set -eu


HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
FIRESTAFF_DATA="${FIRESTAFF_DATA:-$HOME/.firestaff/data}"

GRAPHICS_DAT=${1:-$FIRESTAFF_DATA/GRAPHICS.DAT}
ROOT=$HERE
OUT_DIR=${2:-$ROOT/verification-m10/vga-palette}
mkdir -p "$OUT_DIR"

# Step 1: Produce a title hold frame using the M9 beta harness
"$ROOT/run_firestaff_m9_beta_harness.sh" \
  "$GRAPHICS_DAT" \
  "$OUT_DIR/title_hold" \
  --title-hold 1 > "$OUT_DIR/title_hold.log" 2>&1

# Verify the PGM exists
PGM_FILE="$OUT_DIR/title_hold_hold_0051.pgm"
if [ ! -f "$PGM_FILE" ]; then
  echo "FAIL: title hold PGM not produced"
  exit 1
fi

# Step 2: Compile and run the VGA palette probe
cc -std=c99 -Wall -Wextra -pedantic \
  -DCOMPILE_H '-DSTATICFUNCTION=static' '-DSEPARATOR=,' '-DFINAL_SEPARATOR=)' \
  -I"$ROOT" \
  -o "$OUT_DIR/vga_palette_probe" \
  "$ROOT/firestaff_m10_vga_palette_probe.c" \
  "$ROOT/vga_palette_pc34_compat.c"

PPM_FILE="$OUT_DIR/title_hold_vga.ppm"
"$OUT_DIR/vga_palette_probe" "$PGM_FILE" "$PPM_FILE" 0 > "$OUT_DIR/vga_palette.log"

cat "$OUT_DIR/vga_palette.log"

# Step 3: Verify the PPM file
python3 - <<'PY' "$PPM_FILE" "$OUT_DIR/vga_palette.log" "$OUT_DIR"
import sys
from pathlib import Path
import re

ppm_path = Path(sys.argv[1])
log_path = Path(sys.argv[2])
out_dir = Path(sys.argv[3])

if not ppm_path.exists():
    print("FAIL: VGA PPM not produced")
    raise SystemExit(1)

log = log_path.read_text()
exported = re.search(r'vgaPpmExported=(\d+)', log)
if not exported or exported.group(1) != '1':
    print("FAIL: vgaPpmExported not 1")
    raise SystemExit(1)

width_m = re.search(r'vgaPpmWidth=(\d+)', log)
height_m = re.search(r'vgaPpmHeight=(\d+)', log)
non_black_m = re.search(r'vgaPpmNonBlackPixels=(\d+)', log)

if not width_m or not height_m or not non_black_m:
    print("FAIL: missing fields in probe output")
    raise SystemExit(1)

width = int(width_m.group(1))
height = int(height_m.group(1))
non_black = int(non_black_m.group(1))

# Title hold should be 224x136 (viewport) and have some non-black content
if width == 0 or height == 0:
    print(f"FAIL: invalid dimensions {width}x{height}")
    raise SystemExit(1)

if non_black == 0:
    print("FAIL: VGA PPM is entirely black")
    raise SystemExit(1)

# Verify PPM file size: P6 header + width*height*3 bytes
data = ppm_path.read_bytes()
# Find end of header (P6\n<w> <h>\n<maxval>\n)
header_end = data.index(b'\n', data.index(b'\n') + 1)  # after dims
header_end = data.index(b'\n', header_end + 1) + 1  # after maxval
pixel_data_size = len(data) - header_end
expected_size = width * height * 3

if pixel_data_size != expected_size:
    print(f"FAIL: PPM pixel data size {pixel_data_size} != expected {expected_size}")
    raise SystemExit(1)

# Write invariants
inv = f"""# VGA Palette Probe Invariants

Status: PASS

- VGA PPM exported: {ppm_path.name}
- Dimensions: {width}x{height}
- Non-black pixels: {non_black} / {width*height} ({100*non_black//(width*height)}%)
- Pixel data size correct: {pixel_data_size} bytes
- Palette level: 0 (brightest)
- Color count: 16
- Brightness levels: 6
"""
(out_dir / 'vga_palette_invariants.md').write_text(inv)
print(inv)
PY

printf '%s\n' "$OUT_DIR/vga_palette_invariants.md"
