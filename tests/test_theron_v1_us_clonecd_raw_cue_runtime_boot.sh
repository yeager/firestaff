#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
cue=${FIRESTAFF_THERON_US_CLONECD_RAW_CUE:-"$HOME/.firestaff/data/theron/raw-us-clonecd/Dungeon Master - Theron's Quest (USA).cue"}
expected_md5=168bd6a63784e91885df8c47be62ab5a

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$cue" ]]; then
    printf 'SKIP: authentic Theron USA CloneCD-derived raw CUE/BIN is not staged\n'
    exit 77
fi

output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron --platform pce --data-dir "$cue" \
    --boot-probe --boot-probe-frames 2 --duration 0 2>&1) || {
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: authentic CloneCD-derived raw CUE failed to launch' >&2
    exit 1
}

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' <<<"$output" ||
   ! grep -Fq "assetMd5=$expected_md5" <<<"$output" ||
   ! grep -Fq 'phase=theron-startup-0' <<<"$output" ||
   ! grep -Fq 'startupActive=1' <<<"$output" ||
   ! grep -Fq '::slice@7606368:7928592' <<<"$output" ||
   grep -Fq 'deterministic fallback assets' <<<"$output"; then
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: raw CUE/BIN did not retain its bounded Track 02 source route' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic Theron USA CloneCD-derived raw CUE/BIN reaches native Track 02 startup'
