#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_CSB_AMIGA_ADF_ARCHIVE:-"$HOME/.firestaff/data/csb/Chaos Strikes Back (FTL).zip"}
expected_md5=21197b1d4994fd835c403d5a33dcac2b

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic CSB Amiga ADF archive is not staged'
    exit 77
fi

run_probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    printf '%s\n' "$output"
}

title_output=$(run_probe --game csb --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0)
if ! grep -Fq "assetMd5=$expected_md5" <<<"$title_output" ||
   ! grep -Fq 'sourceKind=5 sourceId=csb' <<<"$title_output" ||
   ! grep -Fq 'phase=csb-entrance-0' <<<"$title_output" ||
   ! grep -Fq 'startupActive=1' <<<"$title_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$title_output"; then
    printf '%s\n' "$title_output" >&2
    printf '%s\n' 'FAIL: authentic CSB Amiga ADF archive did not retain its source entrance phase' >&2
    exit 1
fi

movement_output=$(run_probe --menu --game csb --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 800 --script enter,up --duration 0)
if ! grep -Fq 'phase=inactive' <<<"$movement_output" ||
   ! grep -Fq 'startupActive=0' <<<"$movement_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$movement_output" ||
   ! grep -Fq 'party=9,1,2' <<<"$movement_output" ||
   ! grep -Fq 'runtimeTick=' <<<"$movement_output"; then
    printf '%s\n' "$movement_output" >&2
    printf '%s\n' 'FAIL: authentic CSB Amiga ADF archive start menu did not reach native movement' >&2
    exit 1
fi

menu_output=$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game csb --platform amiga --data-dir "$archive" \
    --script enter --duration 1000 2>&1) || {
    printf '%s\n' "$menu_output" >&2
    exit 1
}
if ! grep -Fq 'CSB READY: gameId=csb' <<<"$menu_output" ||
   ! grep -Fq "dataDir=$archive" <<<"$menu_output" ||
   ! grep -Fq 'route=startup' <<<"$menu_output"; then
    printf '%s\n' "$menu_output" >&2
    printf '%s\n' 'FAIL: authentic CSB Amiga ADF start-menu launch did not retain its archive source' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic CSB Amiga ZIP -> ADF route reaches source entrance, menu launch, and native movement'
