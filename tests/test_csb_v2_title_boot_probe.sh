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
    expected_mode="$2"
    output="$(mktemp "${TMPDIR:-/tmp}/firestaff-csb-title-${mode}-XXXXXX.log")"
    trap 'rm -f "$output"' EXIT HUP INT TERM

    SDL_VIDEODRIVER=dummy "$firestaff_bin" \
        --game csb --data-dir "$data_dir" --presentation-mode "$mode" \
        --boot-probe --boot-probe-frames 50 >"$output" 2>&1

    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=csb' "$output" ||
       ! grep -Fq 'phase=csb-title-1' "$output" ||
       ! grep -Fq 'startupActive=1' "$output" ||
       ! grep -Fq 'startupAnimation=csb-title' "$output" ||
       ! grep -Fq 'startupAnimationActive=1' "$output" ||
       ! grep -Fq 'titleFrame=50' "$output" ||
       ! grep -Fq 'titleFrameMax=102' "$output" ||
       ! grep -Fq 'titleReady=0' "$output"; then
        cat "$output" >&2
        echo "FAIL: CSB $mode did not retain the real C001 title at VBlank 50" >&2
        exit 1
    fi
    if [ "$expected_mode" = '2-or-3' ]; then
        if ! grep -Eq 'presentationMode=(2|3)( |$)' "$output"; then
            cat "$output" >&2
            echo "FAIL: CSB $mode did not resolve to V2.1 fallback or admitted V2.2" >&2
            exit 1
        fi
    elif ! grep -Fq "presentationMode=${expected_mode}" "$output"; then
        cat "$output" >&2
        echo "FAIL: CSB $mode resolved to the wrong presentation mode" >&2
        exit 1
    fi

    rm -f "$output"
    trap - EXIT HUP INT TERM
}

# TITLE.C F0437 keeps C424 PRESENTS for 60 VBlanks. Frame 50 must still be
# that source-owned phase in every presentation mode; V2 filters/upscaling
# may only alter final host presentation, never shortcut C001 into Entrance.
run_case v1 0
run_case v20 1
run_case v21 2
run_case v22 2-or-3

echo "PASS: CSB V1/V2.x retain C001 PRESENTS through source VBlank 50"
