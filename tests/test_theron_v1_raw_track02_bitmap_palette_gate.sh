#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
probe=${FIRESTAFF_THERON_RAW_BITMAP_PROBE:-"$repo/build/firestaff_theron_v1_raw_media_cd_read_bitmap_chain_probe"}

if [[ ! -x "$probe" ]]; then
    printf 'SKIP: raw Track 02 bitmap probe is not built\n'
    exit 0
fi
if [[ -z ${THERON_RAW_TRACK02:-} || -z ${THERON_SYSTEM_CARD:-} ]]; then
    printf 'SKIP: THERON_RAW_TRACK02 and THERON_SYSTEM_CARD are required\n'
    exit 0
fi

set +e
output=$(THERON_BITMAP_ROUTE=title "$probe" 2>&1)
status=$?
set -e

if [[ $status -ne 1 ]]; then
    printf 'FAIL: corpus probe should remain blocked without a palette receipt (exit=%s)\n' "$status" >&2
    printf '%s\n' "$output" >&2
    exit 1
fi
for fact in \
    'status=blocked route=title raw_bitmap_consumed=1' \
    'palette_descriptor=unproven' \
    'rgba_output=blocked' \
    'fallback=not_run'; do
    if ! grep -Fq "$fact" <<<"$output"; then
        printf 'FAIL: corpus probe omitted required gate fact: %s\n' "$fact" >&2
        printf '%s\n' "$output" >&2
        exit 1
    fi
done

printf 'PASS: authentic Track 02 bytes reach the raw bitmap consumer; unbound palette blocks RGB and fallback output\n'
