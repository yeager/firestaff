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
probe --menu --game dm1 --platform amiga --data-dir "$archive" --script enter --boot-probe --boot-probe-frames 2 --duration 0
printf '%s\n' 'PASS: authentic DM1 Amiga ZIP -> ZIP -> ADF reaches CLI and menu runtime in memory'
