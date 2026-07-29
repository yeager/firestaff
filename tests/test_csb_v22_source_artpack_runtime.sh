#!/bin/sh
set -eu

firestaff_bin="${1:?firestaff executable path is required}"
data_dir="${FIRESTAFF_CSB_V22_PC_DATA:-${FIRESTAFF_CSB_PC_DATA:-$HOME/.firestaff/data/csb}}"
manifest="$data_dir/../../assets/csb/modern/modern_asset_manifest.json"

if [ ! -x "$firestaff_bin" ]; then
    echo "SKIP: firestaff executable is unavailable: $firestaff_bin"
    exit 77
fi
if [ ! -f "$data_dir/GRAPHICS.DAT" ] || [ ! -f "$data_dir/DUNGEON.DAT" ]; then
    echo "SKIP: verified PC CSB data is unavailable: $data_dir"
    exit 77
fi
if [ ! -f "$manifest" ] ||
   ! grep -Fq '"packId": "firestaff-csb-v22-pc34-source"' "$manifest"; then
    echo "SKIP: complete source-derived CSB V2.2 artpack is unavailable"
    exit 77
fi

# BSD mktemp requires the trailing X sequence to be at the end of its
# template. Keep the temporary output extension-free so this probe works on
# macOS as well as GNU/Linux.
output="$(mktemp "${TMPDIR:-/tmp}/firestaff-csb-v22-source-runtime-XXXXXX")"
test_home="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v22-home-XXXXXX")"
capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v22-source-startup-XXXXXX")"
v1_capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v1-source-startup-XXXXXX")"
runtime_capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v22-source-runtime-capture-XXXXXX")"
v1_runtime_capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v1-runtime-capture-XXXXXX")"
trap 'rm -f "$output"; rm -rf "$test_home" "$capture_dir" "$v1_capture_dir" "$runtime_capture_dir" "$v1_runtime_capture_dir"' EXIT HUP INT TERM

# V2.2 material belongs only to the admitted live viewport. The original
# C001-C005 startup sequence must still reach the presented surface with its
# four source palette phases before the Prison handoff.
# ReDMCSB's Enter route is the same C200 entrance command as the visible
# C407 mouse box.  Use it here because this headless probe deliberately runs
# with the user's persisted scaling policy; a fixed window coordinate would
# test that policy rather than the V2.2 runtime handoff.  The C407 pointer
# geometry remains covered by the direct entrance-pointer regression.
HOME="$test_home" \
FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR="$capture_dir" \
SDL_VIDEODRIVER=dummy "$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v22 --duration 7000 \
    >"$capture_dir/firestaff.log" 2>&1

capture_count="$(find "$capture_dir" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
if [ "$capture_count" -ne 4 ] ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=4' "$capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=5' "$capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=6' "$capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=7' "$capture_dir/firestaff.log"; then
    cat "$capture_dir/firestaff.log" >&2
    echo "FAIL: CSB V2.2 artpack changed the original startup palette chain" >&2
    exit 1
fi

# V2.2 art belongs to an admitted F0128 runtime command only. Its complete
# source-owned intro chain must therefore retain the V1 terminal pages. TITLE
# palettes 5/6 advance on VBlanks and cannot be compared by a wall-clock
# capture boundary; their source cadence has dedicated TITLE tests below.
HOME="$test_home" \
FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR="$v1_capture_dir" \
SDL_VIDEODRIVER=dummy "$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v1 --duration 7000 \
    >"$v1_capture_dir/firestaff.log" 2>&1

v1_capture_count="$(find "$v1_capture_dir" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
if [ "$v1_capture_count" -ne 4 ] ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=4' "$v1_capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=5' "$v1_capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=6' "$v1_capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=7' "$v1_capture_dir/firestaff.log"; then
    cat "$v1_capture_dir/firestaff.log" >&2
    echo "FAIL: CSB V1 baseline did not capture the complete source startup chain" >&2
    exit 1
fi

python3 - "$v1_capture_dir/firestaff.log" "$capture_dir/firestaff.log" <<'PY'
from pathlib import Path
import re
import struct
import sys

def canonical_bmp(path):
    data = path.read_bytes()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"invalid BMP: {path}")
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    bpp = struct.unpack_from("<H", data, 28)[0]
    if width <= 0 or abs(height) <= 0 or bpp != 24:
        raise ValueError(f"unexpected startup BMP geometry: {path}: {width}x{height}x{bpp}")
    stride = ((width * 3 + 3) // 4) * 4
    if offset + stride * abs(height) > len(data):
        raise ValueError(f"truncated BMP: {path}")
    rows = []
    for y in range(abs(height)):
        source_y = y if height < 0 else abs(height) - 1 - y
        begin = offset + source_y * stride
        rows.append(data[begin:begin + width * 3])
    return width, abs(height), b"".join(rows)

def capture_paths(log_path):
    captures = {}
    for line in Path(log_path).read_text().splitlines():
        match = re.search(r"CSB PRESENTED SOURCE CAPTURE: palette=(\d+) (.+\.bmp)$", line)
        if match:
            captures[int(match.group(1))] = Path(match.group(2))
    if sorted(captures) != [4, 5, 6, 7] or not all(path.is_file() for path in captures.values()):
        raise SystemExit(f"FAIL: CSB startup log lacks a complete palette capture chain: {log_path}")
    return captures

v1_files = capture_paths(sys.argv[1])
v22_files = capture_paths(sys.argv[2])
# Palette 4 (PRESENTS) and palette 7 (the completed Entrance page) are stable
# terminal source pages. Palette 5/6 are the advancing CHAOS/STRIKES sequence
# and are verified by phase/cadence receipts rather than capture timestamp.
for palette in (4, 7):
    v1, v22 = v1_files[palette], v22_files[palette]
    if canonical_bmp(v1) != canonical_bmp(v22):
        raise SystemExit(f"FAIL: CSB V2.2 changed source startup palette {palette}")
PY

HOME="$test_home" \
SDL_VIDEODRIVER=dummy \
FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$runtime_capture_dir" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$runtime_capture_dir/presented" \
"$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v22 \
    --boot-probe --width 960 --height 600 --scale-mode 4 \
    --script 'wait120,key:enter,wait200' >"$output" 2>&1

# V2.2 may replace only a receipt-admitted F0128 viewport command.  The
# terminal C017/C040 HUD is source-owned and must remain byte-identical to
# the V1 source page outside F0128's 224x136 aperture at (48,33).
v1_output="$v1_runtime_capture_dir/firestaff.log"
HOME="$test_home" \
SDL_VIDEODRIVER=dummy \
FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$v1_runtime_capture_dir" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$v1_runtime_capture_dir/presented" \
"$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v1 \
    --boot-probe --width 960 --height 600 --scale-mode 4 \
    --script 'wait120,key:enter,wait200' >"$v1_output" 2>&1

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=csb' "$output" ||
   ! grep -Fq 'presentationMode=3 presentation=640x400' "$output" ||
   ! grep -Fq 'phase=inactive startupActive=0' "$output" ||
   ! grep -Fq 'levelLoaded=1 map=0 party=9,0,2' "$output"; then
    cat "$output" >&2
    echo "FAIL: CSB V2.2 source artpack did not reach the original runtime handoff" >&2
    exit 1
fi

# The checked PC3.4 Prison ingress has no closed F0111 door in its first
# viewport, so it normally reports zero V2.2 replacements.  Do not turn that
# fixture detail into a global prohibition: a later route with an admitted
# closed door must be allowed to report one or more exact F0128 replacements.
# The dedicated in-place command regression owns the clip/provenance proof;
# this real-data probe owns the startup-to-runtime handoff and merely requires
# a well-formed runtime counter.
painted="$(sed -n 's/.*csbV22CellsPainted=\([0-9][0-9]*\).*/\1/p' "$output" | tail -1)"
case "$painted" in
    '')
        cat "$output" >&2
        echo "FAIL: CSB V2.2 runtime did not publish its F0128 replacement count" >&2
        exit 1
        ;;
    0)
        echo "INFO: CSB V2.2 Prison ingress has no admitted closed F0128 door"
        ;;
    *)
        echo "INFO: CSB V2.2 consumed $painted admitted F0128 door replacement(s)"
        ;;
esac

runtime_capture_count="$(find "$runtime_capture_dir" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
runtime_presented_capture_count="$(find "$runtime_capture_dir/presented" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
if [ "$runtime_capture_count" -ne 1 ] ||
   [ "$runtime_presented_capture_count" -ne 1 ]; then
    find "$runtime_capture_dir" -maxdepth 2 -type f -print >&2 || true
    echo "FAIL: CSB V2.2 boot probe did not capture the terminal runtime frame" >&2
    exit 1
fi

v1_runtime_capture_count="$(find "$v1_runtime_capture_dir" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
v1_runtime_presented_capture_count="$(find "$v1_runtime_capture_dir/presented" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
if [ "$v1_runtime_capture_count" -ne 1 ] ||
   [ "$v1_runtime_presented_capture_count" -ne 1 ]; then
    find "$v1_runtime_capture_dir" -maxdepth 2 -type f -print >&2 || true
    echo "FAIL: CSB V1 baseline did not capture the terminal runtime frame" >&2
    exit 1
fi

v22_runtime_bmp="$(find "$runtime_capture_dir" -maxdepth 1 -type f -name '*.bmp' -print -quit)"
v1_runtime_bmp="$(find "$v1_runtime_capture_dir" -maxdepth 1 -type f -name '*.bmp' -print -quit)"
python3 - "$v1_runtime_bmp" "$v22_runtime_bmp" "$painted" <<'PY'
import struct
import sys

def read_bmp(path):
    data = open(path, "rb").read()
    if len(data) < 54 or data[:2] != b"BM":
        raise ValueError(f"invalid BMP: {path}")
    offset = struct.unpack_from("<I", data, 10)[0]
    width, height = struct.unpack_from("<ii", data, 18)
    bpp = struct.unpack_from("<H", data, 28)[0]
    if width != 320 or abs(height) != 200 or bpp != 24:
        raise ValueError(f"unexpected runtime BMP geometry: {path}: {width}x{height}x{bpp}")
    stride = ((width * 3 + 3) // 4) * 4
    if offset + stride * abs(height) > len(data):
        raise ValueError(f"truncated BMP: {path}")
    rows = []
    for y in range(abs(height)):
        source_y = y if height < 0 else abs(height) - 1 - y
        start = offset + source_y * stride
        rows.append(data[start:start + width * 3])
    return rows

v1, v22 = read_bmp(sys.argv[1]), read_bmp(sys.argv[2])
painted = int(sys.argv[3])
for y in range(200):
    for x in range(320):
        # A zero counter means the V2.2 compositor admitted no F0128
        # replacement at all. In that case even the aperture must stay
        # byte-identical to V1. When one or more source-checked door clips
        # are admitted, only F0128's original aperture may differ.
        if painted > 0 and 48 <= x < 272 and 33 <= y < 169:
            continue
        start = x * 3
        if v1[y][start:start + 3] != v22[y][start:start + 3]:
            raise SystemExit(f"FAIL: CSB V2.2 changed source-owned HUD pixel at {x},{y}")
PY

echo "PASS: CSB V2.2 preserves startup captures and the source-owned F0128 runtime frame"
