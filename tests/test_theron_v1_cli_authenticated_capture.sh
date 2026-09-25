#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 ]]; then
    printf 'usage: %s <firestaff-binary>\n' "$0" >&2
    exit 2
fi

app=$1
capture_prefix=${FIRESTAFF_THERON_CAPTURE_PREFIX:-/Volumes/Extern-disk/theron-auth-capture-active-dungeon-20260809.trace}
vram="$capture_prefix.vram"
vce="$capture_prefix.vce"
vdc="$capture_prefix.vdc-state"
sat="$capture_prefix.sat"
vdc_io="$capture_prefix.vdc-io"
track02=${FIRESTAFF_THERON_US_TRACK02_BIN:-"$HOME/.firestaff/data/theron/TQUS02.bin"}

if [[ ! -x "$app" ]]; then
    printf 'FAIL: Firestaff executable is unavailable: %s\n' "$app" >&2
    exit 1
fi
if [[ ! -f "$vram" || ! -f "$vce" || ! -f "$vdc" || ! -f "$sat" ||
      ! -f "$vdc_io" || ! -f "$track02" ]]; then
    printf 'SKIP: authentic Theron Track 02 and atomic VDC bundle are not staged\n'
    exit 77
fi
if [[ $(wc -c <"$vram") -ne 65536 || $(wc -c <"$vce") -ne 1024 ||
      $(wc -c <"$sat") -ne 512 ]]; then
    printf 'FAIL: authenticated Theron capture has unexpected raw size\n' >&2
    exit 1
fi

output=$(mktemp "${TMPDIR:-/tmp}/firestaff-theron-capture-cli.XXXXXX")
trap 'rm -f "$output"' EXIT

if SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron --data-dir "$track02" \
    --theron-vram-snapshot "$vram" --theron-vce-snapshot "$vce" \
    --theron-vdc-state "$vdc" --theron-vdc-sat "$sat" \
    --duration 0 >"$output" 2>&1; then
    printf '%s\n' 'FAIL: legacy four-file Theron bundle was accepted without VDC I/O' >&2
    exit 1
fi
if ! grep -Fq 'VRAM, VCE, VDC-state, SAT and VDC-I/O must be supplied together' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: incomplete atomic-bundle rejection was not explicit' >&2
    exit 1
fi

SDL_VIDEODRIVER=dummy SDL_AUDIODRIVER=dummy "$app" \
    --game theron \
    --data-dir "$track02" \
    --theron-vram-snapshot "$vram" \
    --theron-vce-snapshot "$vce" \
    --theron-vdc-state "$vdc" \
    --theron-vdc-sat "$sat" \
    --theron-vdc-io "$vdc_io" \
    --boot-probe \
    --boot-probe-frames 1 \
    --duration 0 >"$output" 2>&1

if ! grep -Fq 'THERON AUTHENTICATED ATOMIC VDC CAPTURE:' "$output" ||
   ! grep -Fq 'FIRESTAFF BOOT PROBE READY: gameId=theron' "$output"; then
    cat "$output" >&2
    printf '%s\n' 'FAIL: CLI did not bind the authenticated Theron VDC/VCE capture' >&2
    exit 1
fi

printf '%s\n' 'PASS: CLI binds the atomically authenticated Theron VDC bundle'
