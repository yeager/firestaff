#!/bin/sh
set -eu
HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/pass214-viewport-collision-integration}
BUILD_DIR=${BUILD_DIR:-$HERE/build-pass214-viewport-collision}
mkdir -p "$OUT_DIR" "$BUILD_DIR"
cmake -S "$HERE" -B "$BUILD_DIR" -DBUILD_TESTING=ON >/dev/null
cmake --build "$BUILD_DIR" --target firestaff_dm1_v1_viewport_collision_integration_probe >/dev/null
"$BUILD_DIR/firestaff_dm1_v1_viewport_collision_integration_probe" "$OUT_DIR" | tee "$OUT_DIR/probe.log"
test -s "$OUT_DIR/viewport_collision_route.tsv"
test -s "$OUT_DIR/viewport_collision_route.json"
