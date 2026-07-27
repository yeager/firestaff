#!/usr/bin/env bash
set -euo pipefail

repo=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
launcher="$repo/src/engine/main_loop_m11.c"
loader="$repo/src/theron/theron_v1_asset_loader.c"

grep -Fq 'menuState->messageLine2 = "VERIFY CUE/BIN AND STARTUP DETAILS";' "$launcher"
if grep -Fq 'CHECK TRACK 02 AND GRAPHICS DATA' "$launcher"; then
    printf '%s\n' 'FAIL: Theron startup diagnostic still blames graphics data' >&2
    exit 1
fi
grep -Fq 'Verified Track 02 accepted:' "$loader"
grep -Fq 'fallback visuals stay disabled' "$loader"
if grep -Fq 'No Track 03/04 markers in %s; retaining raw Track 02' "$loader"; then
    printf '%s\n' 'FAIL: raw Track 02 diagnostic is still misleading' >&2
    exit 1
fi

printf '%s\n' 'PASS: Theron CUE/BIN startup diagnostics distinguish verified media from uncaptured graphics'
