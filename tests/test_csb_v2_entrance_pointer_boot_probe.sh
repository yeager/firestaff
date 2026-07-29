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

    trap 'rm -f "$output"' EXIT HUP INT TERM
    SDL_VIDEODRIVER=dummy "$firestaff_bin" \
        --game csb --data-dir "$data_dir" --presentation-mode "$mode" \
        --boot-probe --width "$width" --height "$height" \
        --script "wait120,click:${click_x}:${click_y},wait200" >"$output" 2>&1

    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=csb' "$output" ||
       ! grep -Fq 'phase=inactive startupActive=0' "$output" ||
       ! grep -Fq 'runtimeTick=148' "$output" ||
       ! grep -Fq "presentationMode=${expected_presentation_mode}" "$output"; then
        cat "$output" >&2
        echo "FAIL: CSB $mode Prison pointer did not reach runtime with presentation ${expected_presentation_mode} at ${width}x${height}" >&2
        exit 1
    fi
    rm -f "$output"
    trap - EXIT HUP INT TERM
}

# ReDMCSB COMMAND.C G0445 / ENTRANCE.C F0806: the Prison hit zone is
# source (244..298,45..58). These window points stay inside it at 1x and 3x.
# V2.2 must fail closed to V2.1 when no reviewed CSB artpack is present; it
# must nevertheless complete the same source-owned runtime handoff.
run_case v1  0 320 200 250 50
run_case v1  0 960 600 750 150
run_case v20 1 320 200 250 50
run_case v20 1 960 600 750 150
run_case v21 2 320 200 250 50
run_case v21 2 960 600 750 150
run_case v22 2 320 200 250 50
run_case v22 2 960 600 750 150

echo "PASS: CSB Prison pointer reaches runtime in V1/V2.0/V2.1 and gated V2.2 at 1x and 3x"
