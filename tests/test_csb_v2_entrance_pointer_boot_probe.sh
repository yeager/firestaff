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

run_case() {
    mode="$1"
    expected_presentation_mode="$2"
    width="$3"
    height="$4"
    click_x="$5"
    click_y="$6"
    output="${TMPDIR:-/tmp}/firestaff_csb_pointer_${mode}_${width}x${height}_$$.log"
    capture_dir="$(mktemp -d "${TMPDIR:-/tmp}/firestaff-csb-pointer-${mode}-${width}x${height}-XXXXXX")"

    trap 'rm -f "$output"; rm -rf "$capture_dir"' EXIT HUP INT TERM
    FIRESTAFF_AUTOTEST_SCREENSHOT_DIR="$capture_dir" \
    FIRESTAFF_AUTOTEST_PRESENTED_SCREENSHOT_DIR="$capture_dir/presented" \
    SDL_VIDEODRIVER=dummy "$firestaff_bin" \
        --game csb --data-dir "$data_dir" --presentation-mode "$mode" \
        --boot-probe --width "$width" --height "$height" --scale-mode 4 \
        --script 'wait120,key:enter,wait200' >"$output" 2>&1

    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=csb' "$output" ||
       ! grep -Fq 'phase=inactive startupActive=0' "$output" ||
       ! grep -Fq 'runtimeTick=148' "$output" ||
       ! grep -Eq 'csbViewportHash=[1-9][0-9]*' "$output"; then
        cat "$output" >&2
        echo "FAIL: CSB $mode Prison pointer did not reach runtime with presentation ${expected_presentation_mode} at ${width}x${height}" >&2
        exit 1
    fi
    if [ "$expected_presentation_mode" = '2-or-3' ]; then
        if ! grep -Eq 'presentationMode=(2|3)( |$)' "$output"; then
            cat "$output" >&2
            echo "FAIL: CSB $mode did not resolve to V2.1 fallback or admitted V2.2 at ${width}x${height}" >&2
            exit 1
        fi
    elif ! grep -Fq "presentationMode=${expected_presentation_mode}" "$output"; then
        cat "$output" >&2
        echo "FAIL: CSB $mode Prison pointer did not reach presentation ${expected_presentation_mode} at ${width}x${height}" >&2
        exit 1
    fi
    indexed_count="$(find "$capture_dir" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
    presented_count="$(find "$capture_dir/presented" -maxdepth 1 -type f -name '*.bmp' | wc -l | tr -d ' ')"
    if [ "$indexed_count" -ne 1 ] || [ "$presented_count" -ne 1 ]; then
        find "$capture_dir" -maxdepth 2 -type f -print >&2 || true
        echo "FAIL: CSB $mode did not capture its terminal Prison runtime frame at ${width}x${height}" >&2
        exit 1
    fi
    rm -f "$output"
    rm -rf "$capture_dir"
    trap - EXIT HUP INT TERM
}

# ReDMCSB ENTRANCE.C F0806 accepts C200 from the original Enter route.
# This verifies that every presentation reaches the same source-owned runtime
# handoff without tying a startup test to dummy SDL's window-coordinate model.
# The C407 pointer geometry is covered by the direct source-coordinate
# entrance-pointer regression; an actual host-window pointer roundtrip remains
# a separate M11 input integration concern.
# V2.2 resolves to V2.1 when no reviewed CSB artpack is present and stays V2.2
# when a completed pack is installed; both must complete the same source-owned
# runtime handoff. The separate V2.2 source-artpack test requires mode 3.
run_case v1  0 320 200 250 50
run_case v1  0 960 600 750 150
run_case v20 1 320 200 250 50
run_case v20 1 960 600 750 150
run_case v21 2 320 200 250 50
run_case v21 2 960 600 750 150
run_case v22 2-or-3 320 200 250 50
run_case v22 2-or-3 960 600 750 150

echo "PASS: CSB Prison pointer captures runtime in V1/V2.0/V2.1 and gated V2.2 at 1x and 3x"
