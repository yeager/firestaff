#!/usr/bin/env bash
set -euo pipefail

# Production ingestion is native and in-memory.  Do not let a developer's
# diagnostic external-tool opt-in turn this real-media test into a wrapper test.
unset FIRESTAFF_ENABLE_EXTERNAL_ARCHIVE_TOOLS

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_DM1_ATARI_ST_FR_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_FR_Version-13.zip"}
expected_md5=0d7af44dd14f383464288abdcec76afc

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic French DM1 Atari ST 1.3 archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       ! grep -Fq "assetMd5=$expected_md5" <<<"$output" ||
       ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
       ! grep -Fq 'levelLoaded=1' <<<"$output" ||
       ! grep -Fq 'map=0 party=1,3,2' <<<"$output"; then
        printf '%s\n' "$output" >&2
        return 1
    fi
}

probe --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --menu --game dm1 --platform atari-st --data-dir "$archive" \
    --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0

menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$archive" --script enter,enter,enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive" <<<"$menu_output" ||
   ! grep -Fq 'handoff=atari-st-dmcsb1' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic French DM1 Atari ST start menu did not bind DMCSB1 source media' >&2
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
    printf '%s\n' 'FAIL: authentic French DM1 Atari ST start menu did not reach native movement' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic French DM1 Atari ST 1.3 nested ZIP -> ST reaches CLI, menu, and native movement runtime'
