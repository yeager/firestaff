#!/bin/sh
set -eu

firestaff_bin="${1:?firestaff executable path is required}"
data_dir="${FIRESTAFF_CSB_V22_PC_DATA:-${FIRESTAFF_CSB_PC_DATA:-$HOME/.firestaff/data/csb}}"
manifest="$data_dir/../../assets/csb/modern/modern_asset_manifest.json"

if [ ! -x "$firestaff_bin" ]; then
    echo "SKIP: firestaff executable is unavailable: $firestaff_bin"
    exit 0
fi
if [ ! -f "$data_dir/GRAPHICS.DAT" ] || [ ! -f "$data_dir/DUNGEON.DAT" ]; then
    echo "SKIP: verified PC CSB data is unavailable: $data_dir"
    exit 0
fi
if [ ! -f "$manifest" ] ||
   ! grep -Fq '"packId": "firestaff-csb-v22-pc34-source"' "$manifest"; then
    echo "SKIP: complete source-derived CSB V2.2 artpack is unavailable"
    exit 0
fi

output="$(mktemp "${TMPDIR:-/tmp}/firestaff-csb-v22-source-runtime-XXXXXX.log")"
trap 'rm -f "$output"' EXIT HUP INT TERM

SDL_VIDEODRIVER=dummy "$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v22 \
    --boot-probe --width 960 --height 600 \
    --script 'wait120,click:750:150,wait200' >"$output" 2>&1

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=csb' "$output" ||
   ! grep -Fq 'presentationMode=3 presentation=640x400' "$output" ||
   ! grep -Fq 'phase=inactive startupActive=0' "$output" ||
   ! grep -Fq 'levelLoaded=1 map=0 party=9,0,2' "$output"; then
    cat "$output" >&2
    echo "FAIL: CSB V2.2 source artpack did not reach the original runtime handoff" >&2
    exit 1
fi

echo "PASS: CSB V2.2 source artpack reaches runtime with the V2.2 route"
