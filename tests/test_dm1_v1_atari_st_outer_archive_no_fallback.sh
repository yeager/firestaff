#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_atari_st_outer_archive_no_fallback.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_ATARI_ST_OUTER_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_EN.zip"}

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 Atari ST preservation archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       ! grep -Eq 'assetMd5=(b3cfd84e44cdf07ce2eeba47e87f772b|9ce2eaf7a9e78620e3f17594437caffa)' <<<"$output" ||
       ! grep -Fq "dataDir=$archive" <<<"$output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$output" ||
       grep -Fq 'Dungeon-Master_Amiga_EN_Version-20.zip' <<<"$output"; then
        printf '%s\n' "$output" >&2
        return 1
    fi
}

probe --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --menu --game dm1 --platform atari-st --data-dir "$archive" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform atari-st --data-dir "$archive" \
    --script enter,enter,enter --duration 1000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive" <<<"$menu_output" ||
   ! grep -Fq 'handoff=atari-st-dmcsb1' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    exit 1
fi

gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script enter,enter,enter,up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic outer DM1 Atari archive did not reach native movement' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic outer DM1 Atari ZIP -> ZIP -> STX reaches CLI, menu, and native movement in memory'
