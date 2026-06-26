#ifndef FIRESTAFF_MENU_STARTUP_A11Y_M12_H
#define FIRESTAFF_MENU_STARTUP_A11Y_M12_H

/*
 * menu_startup_a11y_m12.h — Launcher screen-reader / state manifest emitter
 *
 * Closes the gap between Firestaff's per-frame accessibility manifest
 * (firestaff_accessibility.c, which already covers M11 game-view
 * viewport + HUD) and the M12 launcher. Today the launcher is invisible
 * to external automation and screen readers because M12_StartupMenu_Draw
 * never calls into fs_ax_*. The launcher is the front door for every
 * presentation mode, so a blind user has no way to detect the game
 * cards, settings tabs, or the "no game data found" popup that fires
 * when required hashes are missing.
 *
 * This module turns the public M12_StartupMenuState into a stable,
 * deterministic set of accessibility elements using the same fs_ax_*
 * writer that M11 already populates. Element IDs are pinned (e.g.
 * "GAME_CARD_DM1", "POPUP_OK", "TAB_DISPLAY") so screen readers and
 * regression probes can target them across releases.
 *
 * Privacy/safety contract:
 *   - The state manifest is derived from public M12_StartupMenuState
 *     fields only. No file paths leak: the popup "data dir" line is
 *     suppressed unless the caller passes `includePaths` non-zero.
 *   - The output is JSON, deterministic (stable key order, fixed
 *     element order), and writes atomically via the existing
 *     fs_ax_flush() path.
 *   - Re-deriving on every frame is O(elements); MAX 128 elements
 *     fits in the existing fs_ax_add_element budget.
 *
 * Source-locked parts of this file: NONE — this is pure UI automation
 * glue on top of public launcher state. ReDMCSB does not cover the
 * launcher, so there is no source citation required here. (M11
 * viewport emission lives in m11_game_view.c around line 28214 and
 * already cites the same fs_ax_* API.)
 */

#include "menu_startup_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Emit one frame of launcher accessibility elements to the global
 * fs_ax_* writer. Call AFTER M12_StartupMenu_Draw returns so the
 * snapshot reflects the same pixel buffer the user just saw.
 *
 * Pass `framebufferWidth` / `framebufferHeight` so element bounds stay
 * in framebuffer coords (480x270 legacy or 1920x1080 modern — the
 * runtime already knows the difference).
 *
 * Pass `includePaths` non-zero if the popup's data-dir line should
 * be emitted as element value text. The default (0) suppresses the
 * absolute path so the manifest is safe to publish. */
void m12_launcher_a11y_emit(const M12_StartupMenuState* state,
                             int framebufferWidth,
                             int framebufferHeight,
                             int includePaths);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MENU_STARTUP_A11Y_M12_H */
