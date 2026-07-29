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

capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v21-presented-XXXXXX")"
trap 'rm -rf "$capture_dir"' EXIT HUP INT TERM

FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR="$capture_dir" \
SDL_VIDEODRIVER=dummy \
"$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v21 --duration 7000 \
    >"$capture_dir/firestaff.log" 2>&1

capture_count="$(find "$capture_dir" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
if [ "$capture_count" -ne 4 ] ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=4' "$capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=5' "$capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=6' "$capture_dir/firestaff.log" ||
   ! grep -Fq 'CSB PRESENTED SOURCE CAPTURE: palette=7' "$capture_dir/firestaff.log"; then
    cat "$capture_dir/firestaff.log" >&2
    echo "FAIL: CSB V2.1 EPX startup did not retain all source palette captures" >&2
    exit 1
fi

for capture in "$capture_dir"/*.bmp; do
    if ! verify_csb_presented_capture_surface "$capture"; then
        exit 1
    fi
done

echo "PASS: CSB V2.1 EPX startup retains PRESENTS/CHAOS/STRIKES/Entrance captures"
