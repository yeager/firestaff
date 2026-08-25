#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_THERON_US_CLONECD_ZIP:-"$HOME/.firestaff/data/theron/Dungeon-Master-Therons-Quest_TurboGrafx-CD_EN.zip"}
expected_md5=168bd6a63784e91885df8c47be62ab5a

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$archive" ]]; then
    printf 'SKIP: authentic Theron USA CloneCD ZIP is not staged\n'
    exit 77
fi

assert_route() {
    local label=$1
    local expected_phase=$2
    shift 2
    local output

    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        printf 'FAIL: %s failed to launch\n' "$label" >&2
        exit 1
    }
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' <<<"$output" ||
       ! grep -Fq "assetMd5=$expected_md5" <<<"$output" ||
       ! grep -Fq "phase=$expected_phase" <<<"$output" ||
       ! grep -Fq 'titleReady=1' <<<"$output" ||
       grep -Fq 'deterministic fallback assets' <<<"$output"; then
        printf '%s\n' "$output" >&2
        printf 'FAIL: %s did not reach the source-backed Theron route\n' "$label" >&2
        exit 1
    fi
}

assert_route 'direct authentic CloneCD ZIP' theron-startup-0 \
    "$app" --game theron --platform pce --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0

assert_route 'start-menu authentic CloneCD ZIP' theron-startup-2 \
    env FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 "$app" \
    --menu --game theron --platform pce --data-dir "$archive" \
    --script 'down,down,down,down,enter,enter,enter,down,down,down,down,down,down,enter,down,enter' \
    --boot-probe --boot-probe-frames 2 --duration 0

printf '%s\n' 'PASS: authentic Theron USA CloneCD ZIP reaches native direct and start-menu routes in memory'
