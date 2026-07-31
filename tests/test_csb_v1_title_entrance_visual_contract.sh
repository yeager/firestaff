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
    --duration 14000 >"$capture_dir/firestaff.log" 2>&1

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

def bmp_pixels(path):
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
    pixels = []
    for row in range(height):
        start = offset + row * stride
        for pixel in range(start, start + width * 3, 3):
            color = data[pixel:pixel + 3]
            colors[color] += 1
            pixels.append(color)
    return width, height, colors, pixels

def non_background_bounds(width, height, pixels):
    background, _ = Counter(pixels).most_common(1)[0]
    xs = []
    ys = []
    for y in range(height):
        for x in range(width):
            if pixels[y * width + x] != background:
                xs.append(x)
                ys.append(y)
    if not xs:
        raise SystemExit("FAIL: CSB source frame has no foreground pixels")
    return min(xs), min(ys), max(xs), max(ys)

presents_w, presents_h, presents, presents_pixels = bmp_pixels(captures[4])
ftl_w, ftl_h, ftl, ftl_pixels = bmp_pixels(captures[5])
title_w, title_h, title, title_pixels = bmp_pixels(captures[6])
entrance_w, entrance_h, entrance, entrance_pixels = bmp_pixels(captures[7])

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

# Palette signatures alone can pass when an IMG2 row is decoded with a wrong
# stride or blitted at the wrong C001 destination.  Require the PC3.4 source
# composition as well.  The default 960x540 host page is a 3x presentation
# of the source coordinates in ReDMCSB TITLE.C F0437 / ENTRANCE.C F0806.
px0, py0, px1, py1 = non_background_bounds(presents_w, presents_h, presents_pixels)
if not (220 <= px1 - px0 + 1 <= 240 and 40 <= py1 - py0 + 1 <= 50 and
        abs((px0 + px1) // 2 - presents_w // 2) <= 24 and
        abs((py0 + py1) // 2 - presents_h // 2) <= 8):
    raise SystemExit("FAIL: CSB PRESENTS has wrong C001 placement or IMG2 stride")

fx0, fy0, fx1, fy1 = non_background_bounds(ftl_w, ftl_h, ftl_pixels)
if not (40 <= fx1 - fx0 + 1 <= 180 and 5 <= fy1 - fy0 + 1 <= 80 and
        abs((fx0 + fx1) // 2 - ftl_w // 2) <= 12):
    raise SystemExit(f"FAIL: CSB CHAOS zoom has wrong C425 placement or scale: {(fx0, fy0, fx1, fy1)}")

tx0, ty0, tx1, ty1 = non_background_bounds(title_w, title_h, title_pixels)
if not (700 <= tx1 - tx0 + 1 <= 920 and 120 <= ty1 - ty0 + 1 <= 190 and
        abs((tx0 + tx1) // 2 - title_w // 2) <= 48 and ty0 >= title_h // 2):
    raise SystemExit(f"FAIL: CSB STRIKES BACK has wrong C426 placement or scale: {(tx0, ty0, tx1, ty1)}")

# Entrance owns the full source page rather than a C001 strip.  This catches
# a title palette/page accidentally surviving into C002-C005.
ex0, ey0, ex1, ey1 = non_background_bounds(entrance_w, entrance_h, entrance_pixels)
if not (ex0 == 0 and ey0 == 0 and ex1 == entrance_w - 1 and
        ey1 == entrance_h - 1):
    raise SystemExit("FAIL: CSB Entrance did not consume the complete C002-C005 page")

print("PASS: CSB PC3.4 title/PRESENTS/FTL/Entrance source palettes remain visible")
PY
