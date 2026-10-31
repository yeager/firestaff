#!/usr/bin/env bash
set -euo pipefail

archive=${1:-}
if [[ -z "$archive" || ! -f "$archive" ]]; then
    printf 'FAIL: firestaff_theron archive path is unavailable\n' >&2
    exit 1
fi

objects=$(ar t "$archive")
repo_root=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cmake_file="$repo_root/CMakeLists.txt"
if [[ ! -f "$cmake_file" ]]; then
    printf 'FAIL: CMake source graph is unavailable\n' >&2
    exit 1
fi

# These translation units contain inferred/procedural or compatibility-only
# routes. Most must stay outside the production archive until an original
# Track 02 consumer binds the corresponding records. The V2.2 modern asset
# manifest reader is an explicit exception: startup uses it to validate the
# installed pack, while its pixel fallback remains fail-closed. Keep these
# lists synchronized with the THERON_SOURCES source graph in CMakeLists.txt.
for source in \
    theron_v1_compat.c \
    theron_v1_shop.c \
    theron_v1_save_load.c \
    theron_v1_srm_runtime.c \
    theron_v22_shape_cache_pc34.c \
    theron_v22_shapes.c \
    theron_v2_hud_widget_assets_pc34.c \
    theron_v1_ui_chrome.c \
    theron_v1_viewport.c \
    theron_v22_inplace_draw_pc34.c \
    theron_v1_startup_receipt.c \
    theron_v2_hud_overlay_pc34.c; do
    object="${source}.o"
    source_stem="${source%.c}"
    if ! awk -v stem="$source_stem" '
        /list\(FILTER THERON_SOURCES EXCLUDE REGEX/ && index($0, stem) { found = 1 }
        END { exit found ? 0 : 1 }
    ' "$cmake_file"; then
        printf 'FAIL: excluded Theron source has no CMake source-graph guard: %s\n' "$source" >&2
        exit 1
    fi
    if grep -Fxq "$object" <<<"$objects"; then
        printf 'FAIL: excluded Theron source is linked into production: %s\n' "$object" >&2
        exit 1
    fi
done

# The manifest reader is needed by the production V2.2 presentation-mode
# resolver. It validates provenance and availability only; it does not
# synthesize or provide replacement artwork.
if ! grep -Fq 'src/theron/theron_v22_modern_assets_pc34.c' "$cmake_file"; then
    printf 'FAIL: required Theron V2.2 manifest reader is absent from CMake source graph\n' >&2
    exit 1
fi
if ! grep -Fq 'src/theron/theron_v2_presentation_mode_pc34.c' "$cmake_file"; then
    printf 'FAIL: Theron V2.2 presentation resolver is absent from CMake source graph\n' >&2
    exit 1
fi

# Fixture constructors/resetters are compile-time test helpers even when
# their implementation shares a production translation unit. They must not
# be exported by the shipped archive.
if command -v nm >/dev/null 2>&1; then
    symbols=$(nm -gU "$archive" 2>/dev/null || nm -g "$archive")
    for symbol in theron_v1_party_clear_fixture_defaults \
                  theron_v1_first_room_buffer_size \
                  theron_v1_first_room_synthesize \
                  theron_v1_startup_fallback_room_synthesize \
                  theron_v1_save_to_slot \
                  theron_v1_save_to_slot_with_gold \
                  theron_v1_save_load_from_slot \
                  theron_v1_save_load_from_path \
                  theron_v1_save_import_slot \
                  theron_v1_save_export_slot \
                  theron_v1_srm_runtime_export_path \
                  theron_v1_srm_runtime_continue_path; do
        if awk -v wanted="$symbol" '
            { name = $NF; sub(/^_/, "", name); if (name == wanted) found = 1 }
            END { exit found ? 0 : 1 }
        ' <<<"$symbols"; then
            printf 'FAIL: fixture-only Theron symbol is exported: %s\n' "$symbol" >&2
            exit 1
        fi
    done
fi

# The classifier remains in production for authentic 2 KiB HUBM Backup RAM,
# but Firestaff's retired fixture-envelope signatures must not survive in the
# shipped archive, even as diagnostic text.
if command -v strings >/dev/null 2>&1; then
    if strings "$archive" | grep -Eq 'FSTQPRG1|FSTQPTY1'; then
        printf 'FAIL: synthetic Theron save-envelope signature is linked into production\n' >&2
        exit 1
    fi
fi

printf 'PASS: inferred/procedural Theron sources are absent from firestaff_theron\n'
