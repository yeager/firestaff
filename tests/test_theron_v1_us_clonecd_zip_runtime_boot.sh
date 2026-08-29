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

assert_card_launch() {
    local label=$1
    local script=$2
    local output

    output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
        SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
        --menu --game theron --platform pce --data-dir "$archive" \
        --script "$script" --duration 1000 2>&1) || {
        printf '%s\n' "$output" >&2
        printf 'FAIL: %s did not launch from its menu cards\n' "$label" >&2
        exit 1
    }
    if ! grep -Fq 'Verified Track 02 accepted:' <<<"$output" ||
       ! grep -Fq '::@suffix=.img::slice@' <<<"$output" ||
       grep -Fq 'deterministic fallback assets' <<<"$output"; then
        printf '%s\n' "$output" >&2
        printf 'FAIL: %s did not retain the verified in-memory Track 02 route\n' "$label" >&2
        exit 1
    fi
}

# Game card -> PC Engine card -> presentation card.  Both launch routes must
# preserve the real IMG Track 02 slice, never the CloneCD .ccd descriptor.
assert_card_launch 'Original card authentic CloneCD ZIP' \
    'key:enter,key:enter,key:enter'
assert_card_launch 'Modern card authentic CloneCD ZIP' \
    'key:enter,key:enter,key:down,key:enter'

# Theron's card is in the second main-menu row.  Select it, its single PC
# Engine platform card, and Original by mouse only.  Explicit launcher
# dimensions make the physical click positions independent of CI defaults.
mouse_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --width 1920 --height 1080 --menu --game theron --platform pce \
    --data-dir "$archive" \
    --script 'wait20,click:1173:728,wait20,click:410:405,wait20,click:450:405,wait20' \
    --duration 3000 2>&1) || {
        printf '%s\n' "$mouse_output" >&2
        printf 'FAIL: mouse card flow did not launch the authentic CloneCD ZIP\n' >&2
        exit 1
    }
if ! grep -Fq 'Verified Track 02 accepted:' <<<"$mouse_output" ||
   ! grep -Fq '::@suffix=.img::slice@' <<<"$mouse_output" ||
   grep -Fq 'deterministic fallback assets' <<<"$mouse_output"; then
    printf '%s\n' "$mouse_output" >&2
    printf 'FAIL: mouse card flow did not retain the verified Track 02 slice\n' >&2
    exit 1
fi

assert_route 'boot-probe launcher selection authentic CloneCD ZIP' theron-startup-2 \
    env FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 "$app" \
    --menu --game theron --platform pce --data-dir "$archive" \
    --script 'down,down,down,down,enter,enter,enter,down,down,down,down,down,down,enter,down,enter' \
    --boot-probe --boot-probe-frames 2 --duration 0

printf '%s\n' 'PASS: authentic Theron USA CloneCD ZIP reaches native direct and start-menu routes in memory'
