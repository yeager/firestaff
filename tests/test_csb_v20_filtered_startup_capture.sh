#!/bin/sh
set -eu

firestaff_bin="${1:?firestaff executable path is required}"
data_dir="${FIRESTAFF_CSB_PC_DATA:-$HOME/.firestaff/data/csb}"
script_dir="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"

# shellcheck source=verify_csb_presented_capture_surface.sh
. "$script_dir/verify_csb_presented_capture_surface.sh"

if [ ! -x "$firestaff_bin" ]; then
    echo "SKIP: firestaff executable is unavailable: $firestaff_bin"
    exit 0
fi
if [ ! -f "$data_dir/GRAPHICS.DAT" ] || [ ! -f "$data_dir/DUNGEON.DAT" ]; then
    echo "SKIP: verified PC CSB data is unavailable: $data_dir"
    exit 0
fi

test_home="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v20-home-XXXXXX")"
capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v20-presented-XXXXXX")"
runtime_v20_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v20-runtime-XXXXXX")"
runtime_v1_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v1-runtime-XXXXXX")"
trap 'rm -rf "$test_home" "$capture_dir" "$runtime_v20_dir" "$runtime_v1_dir"' EXIT HUP INT TERM

# Keep this isolated from a developer's preferences while exercising the
# indexed V2.0 filter chain that must not invalidate source-page receipts.
config_dir="$test_home/Library/Application Support/Firestaff"
mkdir -p "$config_dir"
printf '%s\n' \
    'csb_v2_crt_scanlines_enabled = 1' \
    'csb_v2_palette_correction_enabled = 1' \
    'csb_v2_dither_cleanup_enabled = 1' \
    > "$config_dir/startup-menu.toml"

HOME="$test_home" \
FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR="$capture_dir" \
SDL_VIDEODRIVER=dummy \
"$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v20 --duration 7000 \
    >"$capture_dir/firestaff.log" 2>&1

capture_count="$(find "$capture_dir" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
if [ "$capture_count" -ne 4 ] ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=4' "$capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=5' "$capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=6' "$capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=7' "$capture_dir/firestaff.log"; then
    cat "$capture_dir/firestaff.log" >&2
    echo "FAIL: CSB V2.0 filters invalidated source startup captures" >&2
    exit 1
fi

for capture in "$capture_dir"/*.bmp; do
    if ! verify_csb_presented_capture_surface "$capture"; then
        exit 1
    fi
done

# The indexed V2.0 filters must leave source-owned bytes intact, then alter
# the actual presented runtime surface. This proves the controls are live
# after the original Prison handoff rather than merely accepted at startup.
HOME="$test_home" \
FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$runtime_v20_dir" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$runtime_v20_dir/presented" \
SDL_VIDEODRIVER=dummy \
"$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v20 \
    --boot-probe --width 960 --height 600 --scale-mode 4 \
    --script 'wait120,key:enter,wait200' >"$runtime_v20_dir/firestaff.log" 2>&1

HOME="$test_home" \
FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$runtime_v1_dir" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$runtime_v1_dir/presented" \
SDL_VIDEODRIVER=dummy \
"$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v1 \
    --boot-probe --width 960 --height 600 --scale-mode 4 \
    --script 'wait120,key:enter,wait200' >"$runtime_v1_dir/firestaff.log" 2>&1

if ! grep -Fq 'presentationMode=1 presentation=640x400' "$runtime_v20_dir/firestaff.log" ||
   ! grep -Fq 'phase=inactive startupActive=0' "$runtime_v20_dir/firestaff.log" ||
   ! grep -Fq 'runtimeTick=148' "$runtime_v20_dir/firestaff.log" ||
   ! grep -Fq 'presentationMode=0 presentation=320x200' "$runtime_v1_dir/firestaff.log"; then
    cat "$runtime_v20_dir/firestaff.log" >&2
    cat "$runtime_v1_dir/firestaff.log" >&2
    echo "FAIL: CSB V2.0/V1 runtime filter baseline did not reach Prison runtime" >&2
    exit 1
fi

python3 - "$runtime_v1_dir" "$runtime_v20_dir" <<'PY'
from pathlib import Path
import struct
import sys

def read_bmp(path):
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"invalid BMP: {path}")
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    bpp = struct.unpack_from("<H", data, 28)[0]
    if width <= 0 or abs(height) <= 0 or bpp != 24:
        raise ValueError(f"unexpected BMP geometry: {path}: {width}x{height}x{bpp}")
    stride = ((width * 3 + 3) // 4) * 4
    if offset + stride * abs(height) > len(data):
        raise ValueError(f"truncated BMP: {path}")
    rows = []
    for y in range(abs(height)):
        source_y = y if height < 0 else abs(height) - 1 - y
        begin = offset + source_y * stride
        rows.append(data[begin:begin + width * 3])
    return width, abs(height), b"".join(rows)

def one(root, pattern):
    paths = list(Path(root).glob(pattern))
    if len(paths) != 1:
        raise SystemExit(f"FAIL: expected one {pattern} in {root}, found {len(paths)}")
    return read_bmp(paths[0])

v1_raw = one(sys.argv[1], "*.bmp")
v20_raw = one(sys.argv[2], "*.bmp")
v1_presented = one(sys.argv[1], "presented/*.bmp")
v20_presented = one(sys.argv[2], "presented/*.bmp")
if v1_raw[:2] != (320, 200) or v20_raw[:2] != (320, 200) or v1_raw != v20_raw:
    raise SystemExit("FAIL: CSB V2.0 changed the source-owned 320x200 runtime page")
if v1_presented[:2] != v20_presented[:2] or v1_presented[:2] != (960, 540):
    raise SystemExit("FAIL: CSB V2.0/V1 presented runtime geometry mismatch")
if v1_presented[2] == v20_presented[2]:
    raise SystemExit("FAIL: CSB V2.0 enabled filters did not affect the presented runtime page")
PY

echo "PASS: CSB V2.0 preserves source startup/runtime bytes and filters the presented Prison page"
