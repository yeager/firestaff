#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
data_root=${FIRESTAFF_THERON_RAW_BIN_ROOT:-"$HOME/.firestaff/data/theron"}
track02="$data_root/TQUS02.bin"
expected_md5=f23601102138f87c33025877767ebf76

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron USA raw Track 02 BIN is not staged\n'
    exit 77
fi

output=$(SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron \
    --data-dir "$data_root" \
    --boot-probe \
    --boot-probe-frames 0 \
    --script 'enter,enter,down,down,down,down,down,down,enter,down,enter' \
    --boot-probe-expect-phase theron-startup-2 \
    --boot-probe-expect-level-loaded 0 \
    --boot-probe-expect-asset-md5 "$expected_md5" \
    --boot-probe-expect-startup-active 1 \
    --duration 0 2>&1) || {
    printf '%s\n' "$output" >&2
    printf '%s\n' 'FAIL: authentic Theron USA raw BIN did not reach the source-backed Soul Room route' >&2
    exit 1
}

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' "$output" ||
   ! grep -Fq "assetMd5=$expected_md5" "$output" ||
   ! grep -Fq 'phase=theron-startup-2' "$output" ||
   ! grep -Fq 'levelLoaded=0' "$output" ||
   ! grep -Fq 'startupActive=1' "$output" ||
   grep -Fq 'deterministic fallback assets' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: authentic Theron USA raw BIN did not reach the source-backed Soul Room route' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic Theron USA raw BIN reaches the source-backed Soul Room route'
