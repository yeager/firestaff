#ifndef FIRESTAFF_M11_GAME_VIEW_A11Y_H
#define FIRESTAFF_M11_GAME_VIEW_A11Y_H

/*
 * m11_game_view_a11y.h
 *
 * M11 gameplay-side screen-reader / accessibility state manifest.
 *
 * Public surface for tools that need to know what the gameplay
 * surface is currently showing - Peekaboo, the macOS Accessibility
 * bridge, a speech dispatcher, or any future game-state-aware
 * automation.
 *
 * The M11 gameplay view (DM1 V1 viewport + HUD) has a wide,
 * custom-rendered UI: dungeon viewport, movement arrows, spell
 * area, HUD panel, and a series of full-screen overlays (inventory
 * panel, full-screen map, dialog overlay, candidate mirror / hall
 * of champions mirror, endgame portraits). A screen reader cannot
 * parse this directly from the framebuffer; instead,
 * m11_screen_reader_update_ex() inspects the existing
 * M11_GameViewState and emits a deterministic per-frame manifest
 * using the same fs_ax_* writer the launcher-side M12 harness
 * already uses (src/ui/menu_startup_m12.c tail).
 *
 * This is the gameplay-side analogue of the launcher's manifest
 * block. Both are gated on fs_ax_is_enabled() (set via
 * FS_ACCESSIBILITY=1), so a normal user running Firestaff without
 * the env var pays zero cost. The manifest output is the same
 * ~/.firestaff/accessibility.json file.
 *
 * Determinism / privacy:
 *   - All element labels are stable, hand-curated strings
 *     ("Forward", "Turn Left", "Spell Area", "Champion Portrait",
 *     "Hand Slot Left", ...). The only runtime-derived strings
 *     are champion names / locale codes; those come from the
 *     M11_GameViewState rather than the file system, so the
 *     manifest is stable across runs with the same locale and
 *     game data.
 *   - No player save data, hash bytes, or runtime counters are
 *     emitted.
 *
 * Source-lock: this layer does not consult ReDMCSB at runtime.
 * The M11_GameViewState fields (inventoryPanelActive, mapOverlayActive,
 * dialogOverlayActive, candidateMirrorPanelActive, gameWon,
 * actingChampionOrdinal, ...) are Firestaff's own; the screen
 * reader only reads them and routes through fs_ax_*.
 *
 * The implementation lives in src/engine/m11_game_view_a11y.c.
 * This header is the public contract only.
 */

#include "firestaff_accessibility.h"
#include "m11_game_view.h"  /* M11_GameViewState struct definition */

#ifdef __cplusplus
extern "C" {
#endif

/* The M11_GameViewState struct is brought in by m11_game_view.h
 * above.  No forward declaration needed. */

/* Screen-reader state names emitted into the JSON manifest's
 * "gameState" field. Mirrors the M12 side (which uses "main",
 * "settings", ...) but with gameplay-specific strings so a
 * screen reader can route announcers by phase without parsing
 * the rest of the manifest. The enum is stable and exposed so
 * unit tests can pin the gameplay-state -> state-name mapping. */
typedef enum {
    M11_AX_STATE_GAMEPLAY = 0,       /* Normal dungeon view (default) */
    M11_AX_STATE_INVENTORY,          /* Full-screen inventory panel */
    M11_AX_STATE_MAP,                /* Full-screen automap */
    M11_AX_STATE_DIALOG,             /* Plaque / dialog overlay */
    M11_AX_STATE_ENTRANCE_MIRROR,    /* Hall of Champions / candidate mirror */
    M11_AX_STATE_ENDGAME,            /* Endgame / champion portraits */
    M11_AX_STATE_OTHER,              /* Future / unused enum bucket */
    M11_AX_STATE_COUNT
} M11_AX_State;

/* Decide which gameplay phase the state represents. Reads from
 * state->inventoryPanelActive / mapOverlayActive / dialogOverlayActive /
 * candidateMirrorPanelActive / gameWon and returns one of the
 * M11_AX_STATE_* enum values. Exposed so unit tests can pin the
 * gameplay-state -> state-name mapping without parsing the
 * manifest output. */
M11_AX_State m11_screen_reader_state_for(const M11_GameViewState* state);

/* Render a screen-reader state as a stable, Peekaboo-compatible
 * string (alphanumeric + underscore, no spaces, no NUL bytes).
 * Never NULL; out-of-range inputs collapse to "other".
 *
 * Canonical strings:
 *   "gameplay" | "inventory" | "map" | "dialog"
 *   | "entrance_mirror" | "endgame" | "other" */
const char* m11_screen_reader_state_name(M11_AX_State state);

/* Convenience over m11_screen_reader_state_name(
 * m11_screen_reader_state_for(state)). NULL-safe - returns
 * "other" when state is NULL. */
const char* m11_screen_reader_view_name(const M11_GameViewState* state);

/* Emit one frame's accessibility manifest for the gameplay state.
 *
 *   state                 - current M11 gameplay state. NULL is a no-op.
 *   framebufferWidth,
 *   framebufferHeight     - framebuffer pixel size. The caller
 *                           (M11_GameView_Draw) supplies the exact size
 *                           it is rendering at so the bounding rectangles
 *                           match the visible UI.
 *
 * Always emits the always-on zones (viewport, movement arrows,
 * spell area, HUD panel, control strip). When the corresponding
 * state flag is set, also emits:
 *   - inventory panel: portrait, name, hand slots, body column,
 *     backpack grid
 *   - map overlay: full-screen map region
 *   - dialog overlay: dialog text + choice buttons
 *   - candidate mirror: panel + reincarnate / cancel zones
 *   - endgame: champion portrait grid (4 zones)
 *
 * Routes through fs_ax_begin_frame / fs_ax_add_element /
 * fs_ax_flush so the JSON is rewritten atomically each frame.
 * No-op when fs_ax_is_enabled() returns 0 - a normal run with
 * no FS_ACCESSIBILITY env var pays exactly one
 * fs_ax_is_enabled() call per Draw.
 *
 * Returns 1 if the manifest was flushed, 0 if disabled / skipped. */
int m11_screen_reader_update_ex(const M11_GameViewState* state,
                                int framebufferWidth,
                                int framebufferHeight);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_M11_GAME_VIEW_A11Y_H */
