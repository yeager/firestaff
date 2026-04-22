#!/bin/sh
set -eu

# run_firestaff_m11_phase_a_probe.sh
#
# Builds and runs the M11 Phase A probe (render_sdl_m11). Headless by
# default via SDL_VIDEODRIVER=dummy; callers may override.
#
# Also builds the proof-of-life `firestaff` main binary so it can be
# launched manually by the developer (or smoke-tested in CI with
# SDL_VIDEODRIVER=dummy ./firestaff --duration 200).
#
# Exits 0 when every Phase A invariant passes, 1 otherwise.

HERE="$(cd -- "$(dirname -- "$0")" >/dev/null 2>&1 && pwd)"
OUT_DIR=${1:-$HERE/verification-m11/phase-a}
mkdir -p "$OUT_DIR"

PROBE_BIN="$HERE/firestaff_m11_phase_a_probe_bin"
MAIN_BIN="$HERE/firestaff"
PROBE_SRC_LOCAL="$HERE/firestaff_m11_phase_a_probe.c"
PROBE_SRC_NESTED="$HERE/probes/m11/firestaff_m11_phase_a_probe.c"
if [ -f "$PROBE_SRC_LOCAL" ]; then
    PROBE_SRC="$PROBE_SRC_LOCAL"
else
    PROBE_SRC="$PROBE_SRC_NESTED"
fi

# Select SDL via pkg-config. Prefer SDL3, fall back to SDL2.
SDL_FLAG=""
SDL_CFLAGS=""
SDL_LIBS=""
if pkg-config --exists sdl3 2>/dev/null; then
    SDL_FLAG="-DFS_USE_SDL3=1"
    SDL_CFLAGS=$(pkg-config --cflags sdl3)
    SDL_LIBS=$(pkg-config --libs sdl3)
    SDL_PICKED=sdl3
elif pkg-config --exists sdl2 2>/dev/null; then
    SDL_FLAG="-DFS_USE_SDL2=1"
    SDL_CFLAGS=$(pkg-config --cflags sdl2)
    SDL_LIBS=$(pkg-config --libs sdl2)
    SDL_PICKED=sdl2
else
    echo "ERROR: neither sdl3 nor sdl2 found via pkg-config" >&2
    echo "       install SDL3: brew install sdl3  (macOS)" >&2
    echo "                     apt install libsdl3-dev (Linux)" >&2
    exit 2
fi

echo "# M11 Phase A probe: SDL=$SDL_PICKED"

# Compile shared M11 translation units + the probe.
# -Wall -Wextra: zero tolerance for warnings on M11 code.
# -O2: optimised release build; the Phase A probe is trivial, this is
# mostly to catch strict-aliasing / unused-variable issues surfaced at
# higher optimisation levels.
# M11 code compiles under strict warnings.
CFLAGS_M11="-std=c99 -Wall -Wextra -O2 -I $HERE $SDL_FLAG $SDL_CFLAGS"

# The M10 vga_palette translation unit is frozen Turbo-C compat and must
# be compiled with the same macro shims the M10 probes use. We compile it
# as a separate object so our -Wall -Wextra requirement only applies to
# M11 source.
CFLAGS_M10="-std=c99 -O2 -DCOMPILE_H -DSTATICFUNCTION=static -DSEPARATOR=, -DFINAL_SEPARATOR=) -I $HERE"

VGA_OBJ="$OUT_DIR/vga_palette_pc34_compat.o"
cc $CFLAGS_M10 -c "$HERE/vga_palette_pc34_compat.c" -o "$VGA_OBJ"

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
    OBJ="$OUT_DIR/${src}.o"
    cc $CFLAGS_M10 -c "$HERE/${src}.c" -o "$OBJ"
    GFX_OBJS="$GFX_OBJS $OBJ"
done

cc $CFLAGS_M11 \
    -o "$PROBE_BIN" \
    "$PROBE_SRC" \
    "$HERE/render_sdl_m11.c" \
    "$VGA_OBJ" \
    $SDL_LIBS

cc $CFLAGS_M11 \
    -o "$MAIN_BIN" \
    "$HERE/firestaff_main_m11.c" \
    "$HERE/main_loop_m11.c" \
    "$HERE/m11_game_view.c" \
    "$HERE/audio_sdl_m11.c" \
    "$HERE/asset_loader_m11.c" \
    "$HERE/font_m11.c" \
    "$HERE/fs_portable_compat.c" \
    "$HERE/config_m12.c" \
    "$HERE/asset_status_m12.c" \
    "$HERE/branding_logo_m12.c" \
    "$HERE/card_art_m12.c" \
    "$HERE/creature_art_m12.c" \
    "$HERE/menu_startup_m12.c" \
    "$HERE/render_sdl_m11.c" \
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
    "$HERE/memory_champion_state_pc34_compat.c" \
    "$HERE/memory_dungeon_dat_pc34_compat.c" \
    "$VGA_OBJ" \
    $GFX_OBJS \
    $SDL_LIBS -lm

# Headless by default. If the caller already set SDL_VIDEODRIVER we
# respect that.
: "${SDL_VIDEODRIVER:=dummy}"
export SDL_VIDEODRIVER

PROBE_LOG="$OUT_DIR/phase_a_probe.log"
"$PROBE_BIN" | tee "$PROBE_LOG"
PROBE_RC=${PIPESTATUS:-0}
if [ -n "${PIPESTATUS:-}" ]; then :; fi

# POSIX sh lacks PIPESTATUS; re-run capture via a temp file if needed.
# The simple solution: invoke once with tee and check the status via the
# exit of tee + the presence of "summary:" in the log.
if ! grep -q '^# summary: ' "$PROBE_LOG"; then
    echo "M11 Phase A probe: FAIL (no summary line)" | tee -a "$PROBE_LOG"
    exit 1
fi

# Parse "# summary: X/Y invariants passed" and verify X == Y.
SUMMARY=$(grep '^# summary: ' "$PROBE_LOG" | tail -n 1)
PASSED=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f1)
TOTAL=$(printf '%s\n' "$SUMMARY" | awk '{print $3}' | cut -d/ -f2)
echo "M11 Phase A probe: $SUMMARY"

if [ "$PASSED" != "$TOTAL" ]; then
    echo "M11 Phase A probe: FAIL ($PASSED/$TOTAL)" >&2
    exit 1
fi

echo "M11 Phase A probe: PASS ($PASSED/$TOTAL)"

GAME_VIEW_DIR="$OUT_DIR/game-view"
"$HERE/run_firestaff_m11_game_view_probe.sh" "$GAME_VIEW_DIR"

echo "Artifact: $PROBE_LOG"
echo "Binary:   $MAIN_BIN"
