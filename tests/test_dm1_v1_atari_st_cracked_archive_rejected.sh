#!/usr/bin/env bash
# An explicit DM1 Atari ST archive containing only cracked disk images must
# never become a native runtime owner or fall back to a sibling edition.
set -euo pipefail

if [[ $# -ne 2 ]]; then
    printf 'usage: %s <firestaff-binary> <build-directory>\n' "$0" >&2
    exit 2
fi

app=$1
work_root=$2
archive=${FIRESTAFF_DM1_ATARI_ST_DE_CRACKED_ARCHIVE:-"$HOME/.firestaff/data/dm1/Dungeon-Master_Atari-ST_DE_Version-12-alt.zip"}

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: cracked-only DM1 Atari ST preservation archive is not staged'
    exit 77
fi

# Keep the user's normal data root outside the process.  The explicitly
# selected archive is the only available candidate, so a successful launch
# would prove an unsafe cracked-media admission rather than a valid fallback.
stage=$(mktemp -d "$work_root/firestaff-dm1-atari-cracked.XXXXXX")
cleanup() { rm -rf "$stage"; }
trap cleanup EXIT
mkdir -p "$stage/home"

set +e
output=$(HOME="$stage/home" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm1 --platform atari-st --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0 2>&1)
status=$?
set -e

if [[ $status -eq 0 ]] ||
   grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm1' <<<"$output" ||
   grep -Fq 'phase=dm1-runtime' <<<"$output" ||
   grep -Fq 'DM1 READY: gameId=dm1' <<<"$output"; then
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: cracked-only Atari ST archive was admitted as DM1 media' >&2
    exit 1
fi

printf '%s\n' 'PASS: cracked-only DM1 Atari ST archive is rejected without sibling fallback'
