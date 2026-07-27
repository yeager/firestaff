#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
media_root=${FIRESTAFF_THERON_RAW_CUE_ROOT:-"$HOME/.firestaff/data/theron/raw-us"}
cue="$media_root/Dungeon Master - Theron's Quest (USA).cue"
track02="$media_root/Dungeon Master - Theron's Quest (USA) (Track 02).bin"
expected_md5=f23601102138f87c33025877767ebf76

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$cue" || ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron USA raw CUE/BIN is not staged\n'
    exit 77
fi

output=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-raw-cue.XXXXXX")
trap 'rm -f "$output"' EXIT

SDL_VIDEODRIVER=dummy "$app" \
    --game theron \
    --data-dir "$media_root" \
    --boot-probe \
    --boot-probe-frames 32 \
    --script 'enter,wait4,enter,wait4,enter,wait4' \
    --duration 0 >"$output" 2>&1

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' "$output" ||
   ! grep -Fq "assetMd5=$expected_md5" "$output" ||
   ! grep -Fq 'phase=theron-startup-2' "$output" ||
   ! grep -Fq 'startupAnimation=theron-startup' "$output"; then
    cat "$output" >&2
    printf 'FAIL: authentic raw CUE/BIN did not reach the Theron startup route\n' >&2
    exit 1
fi

printf 'PASS: authentic Theron USA raw CUE/BIN reaches the startup route\n'
