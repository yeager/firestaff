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

test_home="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v20-home-XXXXXX")"
capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v20-presented-XXXXXX")"
trap 'rm -rf "$test_home" "$capture_dir"' EXIT HUP INT TERM

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

echo "PASS: CSB V2.0 filtered startup retains PRESENTS/CHAOS/STRIKES/Entrance captures"
