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
capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v22-source-startup-XXXXXX")"
runtime_capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-v22-source-runtime-capture-XXXXXX")"
trap 'rm -f "$output"; rm -rf "$capture_dir" "$runtime_capture_dir"' EXIT HUP INT TERM

# V2.2 material belongs only to the admitted live viewport. The original
# C001-C005 startup sequence must still reach the presented surface with its
# four source palette phases before the Prison handoff.
# ReDMCSB's Enter route is the same C200 entrance command as the visible
# C407 mouse box.  Use it here because this headless probe deliberately runs
# with the user's persisted scaling policy; a fixed window coordinate would
# test that policy rather than the V2.2 runtime handoff.  The C407 pointer
# geometry remains covered by the direct entrance-pointer regression.
FIRESTAFF_CSB_PRESENTED_CAPTURE_DIR="$capture_dir" \
FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$runtime_capture_dir" \
FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$runtime_capture_dir/presented" \
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

SDL_VIDEODRIVER=dummy "$firestaff_bin" \
    --game csb --data-dir "$data_dir" --presentation-mode v22 \
    --boot-probe --width 960 --height 600 --scale-mode 4 \
    --script 'wait120,key:enter,wait200' >"$output" 2>&1

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

echo "PASS: CSB V2.2 preserves startup captures and the source-owned F0128 runtime frame"
