#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
media=${FIRESTAFF_THERON_CONVERTED_ISO:-"$HOME/.firestaff/data/theron/TQUS02End.iso"}
expected_md5=ceb02343868f80cec899e9b239aff2da

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$media" ]]; then
    printf 'SKIP: authentic converted Theron USA Track 02 ISO is not staged\n'
    exit 77
fi

output=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-converted-iso.XXXXXX")
trap 'rm -f "$output"' EXIT

SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron \
    --data-dir "$media" \
    --boot-probe \
    --boot-probe-frames 128 \
    --duration 0 >"$output" 2>&1

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' "$output" ||
   ! grep -Fq 'sourceKind=4 sourceId=theron ' "$output" ||
   ! grep -Fq "assetMd5=$expected_md5" "$output" ||
   ! grep -Fq 'phase=theron-startup-0' "$output" ||
   grep -Fq 'boot-probe expected selected-entry source' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: converted Theron ISO did not preserve the selected launcher route' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic split Theron USA ISO preserves selected launcher identity'
