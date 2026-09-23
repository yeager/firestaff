#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
data_root=${FIRESTAFF_GAME_DATA_ROOT:-"$HOME/.firestaff/data"}
theron_root="$data_root/theron"
jp_track02="$theron_root/TQJP02.bin"
us_track02="$theron_root/TQUS02.bin"

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$jp_track02" || ! -f "$us_track02" ]]; then
    printf 'SKIP: authentic Theron JP and US raw Track 02 BINs are not staged\n'
    exit 77
fi

output=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-scan.XXXXXX")
trap 'rm -f "$output"' EXIT

"$app" --data-dir "$data_root" --scan-data >"$output" 2>&1

if ! grep -Fq "Theron's Quest         READY" "$output" ||
   ! grep -Fq 'Verified Theron editions:' "$output" ||
   ! grep -Fq "TurboGrafx-16 US (Track 02) FOUND  $us_track02" "$output" ||
   ! grep -Fq "PC Engine JP (Track 02)    FOUND  $jp_track02" "$output" ||
   ! grep -Fq "Track 02 data image (JP/US BIN/ISO) FOUND  $us_track02" "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: scan did not report both authentic Theron editions while retaining the selected US launch file' >&2
    exit 1
fi

printf '%s\n' 'PASS: scan reports authentic Theron JP and US editions and retains the selected US launch file'
