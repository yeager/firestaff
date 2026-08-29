#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_amiga_hd_archive_cli_boot.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_AMIGA_HD_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Amiga_EN.zip"}
expected_md5=6a2f135b53c2220f0251fa103e2a6e7e

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 Amiga HD archive is not staged'
    exit 77
fi

probe() {
    local output
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2; return 1;
    }
    grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" &&
    grep -Fq "assetMd5=$expected_md5" <<<"$output" &&
    grep -Fq 'phase=dm1-runtime' <<<"$output" &&
    grep -Fq 'levelLoaded=1' <<<"$output"
}

probe --game dm1 --platform amiga --data-dir "$archive" --boot-probe --boot-probe-frames 2 --duration 0
probe --menu --game dm1 --platform amiga --data-dir "$archive" --script enter,enter,enter --boot-probe --boot-probe-frames 2 --duration 0
menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform amiga --data-dir "$archive" --script enter,enter,enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2; exit 1;
}
grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" &&
grep -Fq "dataDir=$archive" <<<"$menu_output" &&
grep -Fq 'handoff=amiga-img2' <<<"$menu_output"
gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform amiga --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script enter,enter,enter,up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2; exit 1;
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 Amiga up input did not reach native movement' >&2
    exit 1
fi
printf '%s\n' 'PASS: authentic DM1 Amiga ZIP -> ZIP -> ADF reaches CLI, menu, and native movement runtime in memory'
