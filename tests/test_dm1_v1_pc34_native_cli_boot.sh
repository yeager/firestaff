#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_DM1_PC34_ARCHIVE:-
"$HOME/.firestaff/data/dm1/Dungeon-Master_DOS_EN_Version-34.zip"}

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM1 PC-34 archive is not staged'
    exit 77
fi

output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --data-dir "$archive" --boot-probe --boot-probe-frames 120 \
    --duration 0 2>&1)

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
   ! grep -Fq 'assetMd5=fa6b1aa29e191418713bf2cda93d962e' <<<"$output" ||
   ! grep -Fq 'phase=dm1-runtime' <<<"$output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$output"; then
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: authentic DM1 PC-34 archive did not reach native runtime' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic DM1 PC-34 archive reaches native runtime'
