#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}
# Card startup flow: game card -> verified PC card -> Original card.
menu_original=enter,enter,enter

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 PC-34 archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       ! grep -Fq 'assetMd5=fa6b1aa29e191418713bf2cda93d962e' <<<"$output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$output"; then
        printf '%s\n' "$output" >&2
        return 1
    fi
}

probe --game dm1 --platform pc --data-dir "$archive" --boot-probe --boot-probe-frames 120 \
    --duration 0
probe --menu --game dm1 --platform pc --data-dir "$archive" --script "$menu_original" \
    --boot-probe --boot-probe-frames 120 --duration 0

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 --platform pc \
    --data-dir "$archive" --script "$menu_original" --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive" <<<"$menu_output" ||
   ! grep -Fq 'handoff=pc-img3' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 PC-34 start menu did not bind IMG3 source media' >&2
    exit 1
fi

# The authentic PC-3.4 dungeon starts at (map=0,x=1,y=3,dir=2).  A native
# `up` input advances to y=4; this proves the selected archive has reached
# the actual M11 movement route rather than only a title/startup receipt.
gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform pc --data-dir "$archive" --boot-probe --boot-probe-frames 500 \
    --script "$menu_original",up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 PC-34 up input did not reach native movement' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic DM1 PC-34 archive reaches CLI, menu, and native movement runtime'
