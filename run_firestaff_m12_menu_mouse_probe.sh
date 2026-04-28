#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m12/menu-mouse}
mkdir -p "$OUT_DIR"

PROBE_BIN="$OUT_DIR/firestaff_m12_menu_mouse_probe_bin"

cc -std=c99 -Wall -Wextra -O2 -I "$HERE" \
    -o "$PROBE_BIN" \
    "$HERE/probes/m12/firestaff_m12_menu_mouse_probe.c" \
    "$HERE/fs_portable_compat.c" \
    "$HERE/config_m12.c" \
    "$HERE/asset_status_m12.c" \
    "$HERE/branding_logo_m12.c" \
    "$HERE/branding_logo_readme_m12.c" \
    "$HERE/card_art_m12.c" \
    "$HERE/card_art_generated_m12.c" \
    "$HERE/creature_art_m12.c" \
    "$HERE/menu_startup_m12.c" \
    "$HERE/menu_startup_render_modern_m12.c" \
    "$HERE/menu_hit_m12.c" \
    -lm

cd "$HERE"
"$PROBE_BIN" | tee "$OUT_DIR/menu_mouse_probe.log"
