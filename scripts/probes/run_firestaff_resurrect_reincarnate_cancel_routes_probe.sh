#!/bin/sh
set -eu
HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
ROOT="$(cd -- "$HERE/../.." >/dev/null 2>&1 && pwd)"
BUILD_DIR="${FIRESTAFF_PROBE_BUILD_DIR:-/dev/shm/firestaff-probes}"
mkdir -p "$BUILD_DIR"
APP="$BUILD_DIR/test_resurrect_reincarnate_cancel_routes_pc34_compat_integration"
cc -std=c99 -Wall -Wextra -pedantic -I"$ROOT/include" \
  "$ROOT/tests/test_resurrect_reincarnate_cancel_routes_pc34_compat_integration.c" \
  "$ROOT/src/shared/resurrect_reincarnate_cancel_routes_pc34_compat.c" -o "$APP"
"$APP"
