#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
script=$repo/scripts/capture_theron_mednafen_live_trace.sh

if [[ ! -x "$script" ]]; then
    printf 'FAIL: live Mednafen capture script is not executable\n' >&2
    exit 1
fi
bash -n "$script"

output=$(env -u MEDNAFEN_BIN -u THERON_US_CUE -u THERON_SYSTEM_CARD \
    -u THERON_LIVE_TRACE_OUTPUT "$script")
if [[ "$output" != 'SKIP: MEDNAFEN_BIN, THERON_US_CUE, THERON_SYSTEM_CARD, and THERON_LIVE_TRACE_OUTPUT are required' ]]; then
    printf 'FAIL: capture script did not reject unstaged live inputs\n' >&2
    printf '%s\n' "$output" >&2
    exit 1
fi

printf 'PASS: live capture script requires explicit authentic inputs\n'
