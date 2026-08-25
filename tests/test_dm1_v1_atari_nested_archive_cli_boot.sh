#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_DM1_ATARI_NESTED_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_EN.zip"}
expected_md5=b3cfd84e44cdf07ce2eeba47e87f772b

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic nested DM1 Atari ST archive is not staged'
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
       ! grep -Fq 'levelLoaded=1' <<<"$output"; then
        printf '%s\n' "$output" >&2
        return 1
    fi
}

probe --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0
probe --menu --game dm1 --platform atari-st --data-dir "$archive" \
    --script enter --boot-probe --boot-probe-frames 2 --duration 0

printf '%s\n' 'PASS: authentic DM1 nested Atari ZIP -> ZIP -> STX reaches CLI and menu runtime'
