#!/bin/sh
set -eu

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR="$HERE/verification-screens"
DATA_DIR=${FIRESTAFF_DATA:-${1:-$HOME/.firestaff/data}}
BUILD_DIR="$OUT_DIR"
mkdir -p "$OUT_DIR"

CAPTURE_BIN="$OUT_DIR/capture_bin"
CAPTURE_SRC="$OUT_DIR/capture_firestaff_ingame_series.c"

# SDL3 flags
SDL3_CFLAGS=""
SDL3_LIBS=""
if pkg-config --exists sdl3 2>/dev/null; then
    SDL3_CFLAGS=$(pkg-config --cflags sdl3)
    SDL3_LIBS="$(pkg-config --libs sdl3) -lm"
else
    SDL3_CFLAGS="-DFIRESTAFF_NO_SDL_AUDIO"
    SDL3_LIBS="-lm"
fi

CFLAGS_COMMON="-std=c99 -Wall -Wextra -O2 -I $HERE $SDL3_CFLAGS"
CFLAGS_M10="$CFLAGS_COMMON -DCOMPILE_H -DSTATICFUNCTION=static -DSEPARATOR=, -DFINAL_SEPARATOR=)"

# Compile M10 GRAPHICS.DAT pipeline objects
GFX_OBJS=""
for src in \
    memory_graphics_dat_pc34_compat \
    memory_graphics_dat_state_pc34_compat \
    memory_graphics_dat_header_pc34_compat \
    memory_graphics_dat_select_pc34_compat \
    memory_graphics_dat_metadata_pc34_compat \
    memory_frontend_pc34_compat \
    memory_cache_frontend_pc34_compat \
    expand_frontend_pc34_compat \
    bitmap_call_pc34_compat \
    image_expand_pc34_compat \
    image_backend_pc34_compat \
    graphics_dat_entry_classify_pc34_compat \
    late_special_dispatch_pc34_compat \
    memory_graphics_dat_transaction_pc34_compat \
    memory_load_expand_pc34_compat \
    bitmap_copy_pc34_compat; do
    OBJ="$BUILD_DIR/${src}.o"
    cc $CFLAGS_M10 -c "$HERE/${src}.c" -o "$OBJ"
    GFX_OBJS="$GFX_OBJS $OBJ"
done

cc $CFLAGS_COMMON \
    -o "$CAPTURE_BIN" \
    "$CAPTURE_SRC" \
    "$HERE/m11_game_view.c" \
    "$HERE/audio_sdl_m11.c" \
    "$HERE/graphics_dat_snd3_loader_v1.c" \
    "$HERE/song_dat_loader_v1.c" \
    "$HERE/sound_event_snd3_map_v1.c" \
    "$HERE/asset_loader_m11.c" \
    "$HERE/font_m11.c" \
    "$HERE/fs_portable_compat.c" \
    "$HERE/config_m12.c" \
    "$HERE/asset_status_m12.c" \
    "$HERE/branding_logo_m12.c" \
    "$HERE/card_art_m12.c" \
    "$HERE/creature_art_m12.c" \
    "$HERE/menu_startup_m12.c" \
    "$HERE/memory_tick_orchestrator_pc34_compat.c" \
    "$HERE/memory_champion_lifecycle_pc34_compat.c" \
    "$HERE/memory_runtime_dynamics_pc34_compat.c" \
    "$HERE/memory_projectile_pc34_compat.c" \
    "$HERE/memory_creature_ai_pc34_compat.c" \
    "$HERE/memory_savegame_pc34_compat.c" \
    "$HERE/memory_magic_pc34_compat.c" \
    "$HERE/memory_combat_pc34_compat.c" \
    "$HERE/memory_timeline_pc34_compat.c" \
    "$HERE/memory_sensor_execution_pc34_compat.c" \
    "$HERE/memory_movement_pc34_compat.c" \
    "$HERE/memory_door_action_pc34_compat.c" \
    "$HERE/memory_champion_state_pc34_compat.c" \
    "$HERE/memory_dungeon_dat_pc34_compat.c" \
    $GFX_OBJS \
    $SDL3_LIBS

FIRESTAFF_DATA="$DATA_DIR" "$CAPTURE_BIN" "$OUT_DIR" "$DATA_DIR"

# Convert PPM to PNG for reviewable evidence.  Prefer ImageMagick when it is
# present, but fall back to Pillow so clean macOS worktrees still get the
# normalized PNG screenshots used by the source-zone overlay probes.
for ppm in "$OUT_DIR"/*_latest.ppm "$OUT_DIR"/*_viewport_224x136.ppm; do
    [ -f "$ppm" ] || continue
    png="${ppm%.ppm}.png"
    if command -v magick >/dev/null 2>&1; then
        magick "$ppm" "$png" 2>/dev/null || true
    elif command -v convert >/dev/null 2>&1; then
        convert "$ppm" "$png" 2>/dev/null || true
    else
        python3 - "$ppm" "$png" <<'PY' || true
import sys
from PIL import Image
Image.open(sys.argv[1]).convert("RGB").save(sys.argv[2], optimize=True)
PY
    fi
done

# Write a deterministic evidence manifest for the exact bytes that were
# captured.  The viewport crops are the DM1 source aperture
# (0,33,224,136); hashing them here gives parity work a stable visual
# fingerprint without claiming an emulator overlay we do not have yet.
MANIFEST="$OUT_DIR/capture_manifest_sha256.tsv"
TMP_HASHES="$OUT_DIR/.capture_viewport_hashes.tmp"
: > "$TMP_HASHES"
{
    echo "# Firestaff in-game capture manifest"
    echo "# columns: kind<TAB>filename<TAB>width<TAB>height<TAB>bytes<TAB>sha256"
    for ppm in "$OUT_DIR"/*_latest.ppm "$OUT_DIR"/*_viewport_224x136.ppm; do
        [ -f "$ppm" ] || continue
        header_dims=$(awk 'NR==2 { print $1 " " $2; exit }' "$ppm")
        width=${header_dims% *}
        height=${header_dims#* }
        bytes=$(wc -c < "$ppm" | tr -d ' ')
        if command -v shasum >/dev/null 2>&1; then
            hash=$(shasum -a 256 "$ppm" | awk '{print $1}')
        elif command -v sha256sum >/dev/null 2>&1; then
            hash=$(sha256sum "$ppm" | awk '{print $1}')
        else
            hash="NO_SHA256_TOOL"
        fi
        case "$ppm" in
            *_viewport_224x136.ppm)
                kind="viewport_224x136"
                echo "$hash" >> "$TMP_HASHES"
                ;;
            *)
                kind="fullframe_320x200"
                ;;
        esac
        printf '%s\t%s\t%s\t%s\t%s\t%s\n' \
            "$kind" "$(basename "$ppm")" "$width" "$height" "$bytes" "$hash"
    done
} > "$MANIFEST"

viewport_count=$(wc -l < "$TMP_HASHES" | tr -d ' ')
viewport_unique=$(sort "$TMP_HASHES" | uniq | wc -l | tr -d ' ')
rm -f "$TMP_HASHES"

if [ "$viewport_count" != "6" ]; then
    echo "expected 6 viewport crops in manifest, found $viewport_count" >&2
    exit 1
fi
if [ "$viewport_unique" = "0" ]; then
    echo "viewport crop hashing produced no hashes" >&2
    exit 1
fi

echo "Capture manifest: $MANIFEST ($viewport_count viewport crops, $viewport_unique unique hashes)"

echo "Screenshots captured in $OUT_DIR"
