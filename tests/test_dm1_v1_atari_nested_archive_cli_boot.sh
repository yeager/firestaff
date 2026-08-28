#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
archive=${FIRESTAFF_DM1_ATARI_NESTED_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_EN_Version-12.zip"}
# Both authenticated English Atari ST v1.2 media revisions are supported.
# The nested image distributed as "Dungeon Master V1.2 (1987)(FTL)(en)[!]"
# has the latter GRAPHICS.DAT identity; do not reject it merely because a
# different preservation dump was used when this probe was first written.
expected_md5s=(
    b3cfd84e44cdf07ce2eeba47e87f772b
    9ce2eaf7a9e78620e3f17594437caffa
)

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic nested DM1 Atari ST archive is not staged'
    exit 77
fi

probe() {
    local output
    local matched=0
    local expected_md5
    output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" "$@" 2>&1) || {
        printf '%s\n' "$output" >&2
        return 1
    }
    for expected_md5 in "${expected_md5s[@]}"; do
        if grep -Fq "assetMd5=$expected_md5" <<<"$output"; then
            matched=1
            break
        fi
    done
    if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
       [[ $matched -ne 1 ]] ||
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
menu_output="$(FIRESTAFF_FAIL_IF_NO_LAUNCH=1 FIRESTAFF_EXIT_AFTER_LAUNCH=1 \
    SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" --menu --game dm1 \
    --platform atari-st --data-dir "$archive" --script enter --duration 1000 2>&1)" || {
    printf '%s\n' "$menu_output" >&2; exit 1;
}
grep -Fq 'DM1 READY: gameId=dm1' <<<"$menu_output" &&
grep -Fq "dataDir=$archive" <<<"$menu_output" &&
grep -Fq 'handoff=atari-st-dmcsb1' <<<"$menu_output"

gameplay_output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --menu --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 500 --script enter,up --duration 0 2>&1) || {
    printf '%s\n' "$gameplay_output" >&2
    exit 1
}
if ! grep -Fq 'phase=dm1-runtime' <<<"$gameplay_output" ||
   ! grep -Fq 'levelLoaded=1' <<<"$gameplay_output" ||
   ! grep -Fq 'map=0 party=1,4,2' <<<"$gameplay_output"; then
    printf '%s\n' "$gameplay_output" >&2
    printf '%s\n' 'FAIL: authentic DM1 Atari ST up input did not reach native movement' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic DM1 nested Atari ZIP -> ZIP -> STX reaches CLI, menu, and native movement runtime'
