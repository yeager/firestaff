#!/usr/bin/env sh
set -eu

app=${1:?usage: test_dm2_v1_mac_demo_rejected_no_fallback.sh <firestaff>}
archive=${FIRESTAFF_DM2_MAC_DEMO_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip"}

if [ ! -x "$app" ] || [ ! -f "$archive" ]; then
    echo 'SKIP: authentic DM2 Macintosh demo archive is not staged'
    exit 77
fi

set +e
output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform mac --data-dir "$archive" --boot-probe \
    --boot-probe-frames 2 --duration 0 2>&1)
status=$?
set -e

if [ "$status" -eq 0 ] ||
   ! printf '%s\n' "$output" | grep -Fq 'game unavailable for --game: dm2' ||
   printf '%s\n' "$output" | grep -Fq 'assetMd5=5cab25f6b975957eae4a203174e7f2a6' ||
   printf '%s\n' "$output" | grep -Fq 'Dungeon-Master-II-Skullkeep_Mac_EN.zip'; then
    printf '%s\n' "$output" >&2
    echo 'FAIL: removed DM2 Macintosh demo support fell back to retail media' >&2
    exit 1
fi

echo 'PASS: DM2 Macintosh demo is rejected without retail-media fallback'
