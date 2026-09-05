#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
REPO_ROOT="$(cd -- "$HERE/../.." >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$REPO_ROOT/build-dm1-csb-native/verification-m11/pass42-chrome-reduction}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_m11_pass42_chrome_reduction_probe_bin"
LOG="$OUT_DIR/pass42_chrome_reduction_probe.log"

cc -std=c99 -Wall -Wextra -O1 \
    -o "$PROBE_BIN" \
    "$REPO_ROOT/probes/firestaff_m11_pass42_chrome_reduction_probe.c"

( cd "$REPO_ROOT" && "$PROBE_BIN" ) | tee "$LOG"

SUMMARY=$(grep '^# summary: ' "$LOG" | tail -n 1)
PASSED=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f1)
TOTAL=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f2)
echo "Pass 42 chrome reduction probe: $SUMMARY"
if [ -z "$PASSED" ] || [ "$PASSED" != "$TOTAL" ]; then
    exit 1
fi
