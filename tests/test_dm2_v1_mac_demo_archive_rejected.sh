#!/usr/bin/env bash
# The Macintosh First Chapter demo is not the retail DM2 release. An explicit
# demo archive must fail closed and must not select a retail sibling instead.
set -euo pipefail

if [[ $# -ne 2 ]]; then
    printf 'usage: %s <firestaff-binary> <build-directory>\n' "$0" >&2
    exit 2
fi

app=$1
work_root=$2
archive=${FIRESTAFF_DM2_MAC_DEMO_ARCHIVE:-"$HOME/.firestaff/data/dm2/Dungeon-Master-II-Skullkeep_Mac_EN (1).zip"}

if [[ ! -x "$app" || ! -f "$archive" ]]; then
    printf '%s\n' 'SKIP: authentic DM2 Macintosh First Chapter demo is not staged'
    exit 77
fi

# Do not allow the user's normal game root into this process. The selected
# demo is the only candidate, so READY would prove an unsafe demo admission
# or a sibling fallback rather than an authentic retail launch.
stage=$(mktemp -d "$work_root/firestaff-dm2-mac-demo.XXXXXX")
cleanup() { rm -rf "$stage"; }
trap cleanup EXIT
mkdir -p "$stage/home"

set +e
output=$(HOME="$stage/home" SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game dm2 --platform mac --data-dir "$archive" \
    --boot-probe --boot-probe-frames 2 --duration 0 2>&1)
status=$?
set -e

if [[ $status -eq 0 ]] ||
   grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=dm2' <<<"$output" ||
   grep -Fq 'phase=dm2-runtime' <<<"$output" ||
   grep -Fq 'DM2 READY: gameId=dm2' <<<"$output"; then
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: DM2 Macintosh First Chapter demo was admitted as retail media' >&2
    exit 1
fi

printf '%s\n' 'PASS: DM2 Macintosh First Chapter demo is rejected without sibling fallback'
