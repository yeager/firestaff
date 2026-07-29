#!/bin/sh
set -eu

firestaff_bin="${1:?firestaff executable path is required}"
data_dir="${FIRESTAFF_CSB_PC_DATA:-$HOME/.firestaff/data/csb}"

if [ ! -x "$firestaff_bin" ]; then
    echo "SKIP: firestaff executable is unavailable: $firestaff_bin"
    exit 0
fi
if [ ! -f "$data_dir/GRAPHICS.DAT" ] || [ ! -f "$data_dir/DUNGEON.DAT" ]; then
    echo "SKIP: verified PC CSB data is unavailable: $data_dir"
    exit 0
fi

test_home="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-title-home-XXXXXX")"
capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-title-capture-XXXXXX")"
trap 'rm -rf "$test_home" "$capture_dir"' EXIT HUP INT TERM

HOME="$test_home" \
FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR="$capture_dir" \
SDL_VIDEODRIVER=dummy \
"$firestaff_bin" --game csb --data-dir "$data_dir" --presentation-mode v1 \
    --duration 7000 >"$capture_dir/firestaff.log" 2>&1

python3 - "$capture_dir/firestaff.log" <<'PY'
from collections import Counter
from pathlib import Path
import re
import struct
import sys

log = Path(sys.argv[1]).read_text(encoding="utf-8", errors="replace")
captures = {}
for palette, path in re.findall(r"CSB PRESENTED SOURCE CAPTURE: palette=(\d+) (.+)", log):
    captures[int(palette)] = Path(path)

if sorted(captures) != [4, 5, 6, 7] or not all(path.is_file() for path in captures.values()):
    raise SystemExit("FAIL: CSB title/Entrance capture lacks its four source phases")

def bmp_colors(path):
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise SystemExit(f"FAIL: invalid CSB capture BMP: {path}")
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    if width < 320 or abs(height) < 200:
        raise SystemExit(f"FAIL: collapsed CSB capture geometry: {path}")
    if height == 0 or offset >= len(data):
        raise SystemExit(f"FAIL: malformed CSB capture BMP: {path}")
    height = abs(height)
    stride = ((width * 3 + 3) // 4) * 4
    if offset + stride * height > len(data):
        raise SystemExit(f"FAIL: truncated CSB capture BMP: {path}")
    colors = Counter()
    for row in range(height):
        start = offset + row * stride
        for pixel in range(start, start + width * 3, 3):
            colors[data[pixel:pixel + 3]] += 1
    return width, height, colors

_, _, presents = bmp_colors(captures[4])
_, _, ftl = bmp_colors(captures[5])
_, _, title = bmp_colors(captures[6])
_, _, entrance = bmp_colors(captures[7])

# These are rendered PC3.4 source-palette signatures, not substitute art:
# C001 PRESENTS is white on the palette-4 blue page; C001's FTL prelude is a
# small multi-colour logo; CHAOS STRIKES BACK has a large red source raster;
# C002 prison/Entrance has the rich original brown palette and UI geometry.
if presents[b"\xff\xff\xff"] < 100:
    raise SystemExit("FAIL: CSB PRESENTS source raster is absent or has the wrong palette")
if len(ftl) < 4 or sum(count for color, count in ftl.items()
                        if color != b"m\x00\x00") < 20:
    raise SystemExit("FAIL: CSB FTL prelude source raster is absent or has the wrong palette")
if title[b"\x00\x00\xff"] < 1000:
    raise SystemExit("FAIL: CSB CHAOS STRIKES BACK source raster is absent or has the wrong palette")
if len(entrance) < 8 or max(entrance.values()) == sum(entrance.values()):
    raise SystemExit("FAIL: CSB Entrance source raster collapsed to a flat frame")

print("PASS: CSB PC3.4 title/PRESENTS/FTL/Entrance source palettes remain visible")
PY
