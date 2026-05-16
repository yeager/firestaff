#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/pass40-viewport-lock}
mkdir -p "$OUT_DIR"

PROBE_BIN="$HERE/firestaff_m11_pass40_viewport_lock_probe_bin"
LOG="$OUT_DIR/pass40_viewport_lock_probe.log"

cc -std=c99 -Wall -Wextra -O1 \
    -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/firestaff_m11_pass40_viewport_lock_probe.c"

( cd "$HERE" && "$PROBE_BIN" ) | tee "$LOG"

SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1)
PASSED=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f1)
TOTAL=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f2)
echo "Pass 40 viewport lock probe: $SUMMARY"
if [ -z "$PASSED" ] || [ "$PASSED" != "$TOTAL" ]; then
    exit 1
fi
