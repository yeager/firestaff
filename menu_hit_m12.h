/*
 * menu_hit_m12.h — bounded M12 slice: mouse hit-testing for the
 * modern high-resolution startup menu.
 *
 * The modern renderer draws the startup menu at a fixed 1280x720
 * native resolution (see menu_startup_render_modern_m12.h). This
 * module maps a pointer in that same canvas space to a structured
 * hit target, and applies clicks back onto the menu state.
 *
 * Scope boundary (locked):
 *   - Startup menu only. V1 in-game rendering is NOT touched by this
 *     module.
 *   - No SDL dependency; works on pure C99 input.
 */

#ifndef FIRESTAFF_MENU_HIT_M12_H
#define FIRESTAFF_MENU_HIT_M12_H

#include "menu_startup_m12.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    M12_HIT_NONE = 0,
    /* Main view: card 0..4 (0,1,2 = games, 3 = museum, 4 = settings) */
    M12_HIT_MAIN_CARD,
    /* Museum view: category 0..4 */
    M12_HIT_MUSEUM_CATEGORY,
    /* Museum view: page cycle button/region */
    M12_HIT_MUSEUM_PAGE,
    /* Settings view: row 0..M12_SETTINGS_ROW_COUNT-1 */
    M12_HIT_SETTINGS_ROW,
    /* Settings view: row cycle button (left/right arrow area, delta +1 or -1) */
    M12_HIT_SETTINGS_CYCLE,
    /* Game-options view: row 0..M12_GAME_OPT_ROW_COUNT (last = launch row) */
    M12_HIT_GAMEOPT_ROW,
    /* Game-options view: row cycle (delta +1 or -1) */
    M12_HIT_GAMEOPT_CYCLE,
    /* Game-options view: launch row */
    M12_HIT_GAMEOPT_LAUNCH,
    /* Message view: anywhere inside the dialog dismisses */
    M12_HIT_MESSAGE_DISMISS,
    /* Back button (visible in all views except main) */
    M12_HIT_BACK
} M12_HitKind;

typedef struct {
    M12_HitKind kind;
    int index;     /* card index, row index, etc. (kind-specific) */
    int delta;     /* for CYCLE hits: +1 or -1 */
} M12_MouseHit;

/* Pure hit-test. Does not mutate state. x/y are canvas coordinates
 * in [0..1280) x [0..720). Returns {M12_HIT_NONE,0,0} when the
 * pointer does not land on any interactive region. */
M12_MouseHit M12_ModernMenu_HitTest(const M12_StartupMenuState* state,
                                    int x, int y);

/* Apply a hit result produced by M12_ModernMenu_HitTest. Returns 1
 * when the state changed (caller should redraw), 0 otherwise.
 * The function also updates state->hoverX/hoverY. */
int M12_ModernMenu_ApplyHit(M12_StartupMenuState* state,
                            M12_MouseHit hit);

/* Combined helper used by the SDL runtime: update hover, and on
 * activation (left-click down) hit-test and apply. Returns 1 if
 * the state changed. If `shouldExit` is non-null, it is set to 1
 * when the hit caused a top-level back that should exit. */
int M12_ModernMenu_HandlePointer(M12_StartupMenuState* state,
                                 int x, int y,
                                 int clicked,
                                 int* shouldExit);

#ifdef __cplusplus
}
#endif

#endif /* FIRESTAFF_MENU_HIT_M12_H */
