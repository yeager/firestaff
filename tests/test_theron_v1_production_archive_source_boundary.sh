#!/usr/bin/env bash
set -euo pipefail

archive=${1:-}
if [[ -z "$archive" || ! -f "$archive" ]]; then
    printf 'FAIL: firestaff_theron archive path is unavailable\n' >&2
    exit 1
fi

objects=$(ar t "$archive")

# These translation units contain inferred/procedural or compatibility-only
# routes. CMake must keep them outside the production archive until an
# original Track 02 consumer binds the corresponding records. Keep this list
# synchronized with the THERON_SOURCES exclusion block in CMakeLists.txt.
for source in \
    theron_v1_compat.c \
    theron_v1_combat_runtime_source.c \
    theron_v1_track02_creature.c \
    theron_v1_shop.c \
    theron_v22_shape_cache_pc34.c \
    theron_v22_shapes.c \
    theron_v2_hud_widget_assets_pc34.c \
    theron_v1_ui_chrome.c \
    theron_v1_viewport.c \
    theron_v22_inplace_draw_pc34.c \
    theron_v22_modern_assets_pc34.c \
    theron_v1_startup_receipt.c \
    theron_v2_hud_overlay_pc34.c; do
    object="${source}.o"
    if grep -Fxq "$object" <<<"$objects"; then
        printf 'FAIL: excluded Theron source is linked into production: %s\n' "$object" >&2
        exit 1
    fi
done

printf 'PASS: inferred/procedural Theron sources are absent from firestaff_theron\n'
