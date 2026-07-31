#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
cue=${FIRESTAFF_THERON_JP_CUE:-"$HOME/.firestaff/data/theron/TQJP.cue"}
expected_md5_raw=b7afb338ad31be1025b53f9aff12d73a
expected_md5_iso=397039af02d50d15c70b74088eb8a1cb

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$cue" ]]; then
    printf 'SKIP: authentic Theron JP CUE is not staged\n'
    exit 77
fi

output=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-jp-cue.XXXXXX")
trap 'rm -f "$output"' EXIT

SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron \
    --data-dir "$cue" \
    --boot-probe \
    --boot-probe-frames 128 \
    --duration 0 >"$output" 2>&1

if ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' "$output" ||
   ! grep -Fq 'sourceKind=4 sourceId=theron ' "$output" ||
   ! grep -Eq "assetMd5=($expected_md5_raw|$expected_md5_iso)" "$output" ||
   ! grep -Fq 'phase=theron-startup-0' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: authentic Theron JP CUE did not reach title startup' >&2
    exit 1
fi

printf '%s\n' 'PASS: authentic Theron JP CUE reaches title startup'
