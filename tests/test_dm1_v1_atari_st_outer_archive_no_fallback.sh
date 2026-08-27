#!/usr/bin/env bash
set -euo pipefail

app=${1:?usage: test_dm1_v1_atari_st_outer_archive_no_fallback.sh <firestaff-binary>}
archive=${FIRESTAFF_DM1_ATARI_ST_OUTER_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_EN.zip"}

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 Atari ST preservation archive is not staged'
    exit 77
fi

set +e
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0 2>&1)
status=$?
set -e

if [[ $status -eq 0 ]] ||
   ! grep -Fq 'game unavailable for --game: dm1' <<<"$output" ||
   grep -Fq 'Dungeon-Master_Amiga_EN_Version-20.zip' <<<"$output" ||
   grep -Fq 'assetMd5=6a2f135b53c2220f0251fa103e2a6e7e' <<<"$output"; then
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: explicit Atari ST archive borrowed a sibling edition or launched without a verified dungeon source' >&2
    exit 1
fi

printf '%s\n' 'PASS: explicit DM1 Atari ST archive fails closed instead of borrowing sibling Amiga data'
