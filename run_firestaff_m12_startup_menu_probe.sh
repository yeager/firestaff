#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m12/startup-menu}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_m12_startup_menu_probe_bin"

cc -std=c99 -Wall -Wextra -O2 -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/firestaff_m12_startup_menu_probe.c" \
    "$HERE/config_m12.c" \
    "$HERE/asset_status_m12.c" \
    "$HERE/branding_logo_m12.c" \
    "$HERE/card_art_m12.c" \
    "$HERE/menu_startup_m12.c"

"$PROBE_BIN" | tee "$OUT_DIR/startup_menu_probe.log"
