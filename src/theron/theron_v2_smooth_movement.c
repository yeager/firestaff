/*
 * theron_v2_smooth_movement.c
 *
 * Theron's Quest V2.2 Smooth Movement — implementation.
 *
 * Module-static V2_Anim state.  No global accessor for the struct
 * itself (the read-side is per-anim via theron_v2_smooth_get_*),
 * which keeps the symbol surface small and lets the runtime binding
 * seam (theron_v2_smooth_movement_runtime.c, separate file) manage
 * the binding lifecycle.
 *
 * Easing matches the cross-game V2 pattern:
 *   - Walk  : ease-out cubic   (CSB V2 / DM1 V2 / DM2 V2)
 *   - Turn  : ease-out quad    (CSB V2 / DM1 V2 / DM2 V2)
 *   - Fade  : ease-in-out cubic (replaces DM1 V2 stairs easing,
 *             since Theron has no "stairs" — only teleporter chains)
 *
 * Source: THQUEST.ASM T520, T560, T600, T700; ReDMCSB CLIKMENU.C
 *         F0365/F0366/F0364; ReDMCSB COMMAND.C F0380; ReDMCSB
 *         GAMELOOP.C:47-50 (V1 tick cadence 55ms).
 *         HuC6260/HuC6270 VDC/VCE; HuC6280 CPU + ADPCM.
 * Reference: csb_v2_smooth_movement.c (the cross-game pattern this
 *            module mirrors, adapted to Theron's 4-direction compass
 *            and tile-based walk).
 */

#include "theron_v2_smooth_movement.h"

#include <math.h>
#include <string.h>

/* ── Easing choices (cross-game V2 standard) ──────────────────────── */
#define THERON_V2_WALK_EASE  V2_EASE_OUT_CUBIC
#define THERON_V2_TURN_EASE  V2_EASE_OUT_QUAD
#define THERON_V2_FADE_EASE  V2_EASE_IN_OUT_CUBIC

/* ── Module-static animation state ────────────────────────────────── */
static V2_Anim g_walk_x;
static V2_Anim g_walk_y;
static V2_Anim g_turn;
static V2_Anim g_fade;

float theron_v2_dir_to_angle(int dir) {
    /* Theron V1 uses leader_dir & 3: 0=N, 1=E, 2=S, 3=W.
     * Map to V2 compass degrees (0=N, 90=E, 180=S, 270=W). */
    switch (dir & 3) {
        case THERON_V2_DIR_N: return 0.0f;
        case THERON_V2_DIR_E: return 90.0f;
        case THERON_V2_DIR_S: return 180.0f;
        case THERON_V2_DIR_W: return 270.0f;
        default:              return 0.0f;
    }
}

void theron_v2_smooth_init(void) {
    /* Zero out the entire struct (not just the active flag) so that
     * the read-side returns 0.0f after init, not the last value. */
    memset(&g_walk_x, 0, sizeof(g_walk_x));
    memset(&g_walk_y, 0, sizeof(g_walk_y));
    memset(&g_turn,   0, sizeof(g_turn));
    memset(&g_fade,   0, sizeof(g_fade));
}

void theron_v2_smooth_start_walk(float fx, float tx, float fy, float ty) {
    /* Theron walk is single-axis-at-a-time (no diagonal).  Callers
     * pass (from, to) on either X or Y and zero out the other axis. */
    v2_anim_start_v1_tick(&g_walk_x, fx, tx, THERON_V2_WALK_EASE);
    v2_anim_start_v1_tick(&g_walk_y, fy, ty, THERON_V2_WALK_EASE);
}

void theron_v2_smooth_start_turn(float fa, float ta) {
    /* Normalise inputs to [0, 360).  Shortest-path wrap: if the delta
     * is more than 180°, take the other way by animating past 360
     * (or past 0).  The anim stores the wrapped from/to so that
     * v2_anim_value() returns a value in the wrapped range during
     * the animation, which V2 presentation can modulo 360 if it
     * wants a [0, 360) reading. */
    fa = fmodf(fa, 360.0f);
    if (fa < 0.0f) fa += 360.0f;
    ta = fmodf(ta, 360.0f);
    if (ta < 0.0f) ta += 360.0f;
    float delta = ta - fa;
    if (delta > 180.0f) {
        /* Long way +delta — short way goes -360+delta, e.g. 0→270
         * becomes 0→-90 (going backwards past 0/360). */
        ta -= 360.0f;
    } else if (delta < -180.0f) {
        /* Long way -delta — short way goes +360+delta, e.g. 270→0
         * becomes 270→360 (going forwards past 360/0). */
        ta += 360.0f;
    }
    v2_anim_start_v1_tick(&g_turn, fa, ta, THERON_V2_TURN_EASE);
}

void theron_v2_smooth_start_fade(float from, float to) {
    /* Fade animation is independent of walk/turn.  Duration: 1 V1 tick. */
    v2_anim_start_v1_tick(&g_fade, from, to, THERON_V2_FADE_EASE);
}

void theron_v2_smooth_update_from_clock(const V2_AnimClock *clock) {
    if (!clock) return;
    const float dt = clock->dt_ms;
    v2_anim_update(&g_walk_x, dt);
    v2_anim_update(&g_walk_y, dt);
    v2_anim_update(&g_turn,   dt);
    v2_anim_update(&g_fade,   dt);
}

float theron_v2_smooth_get_x(void)      { return v2_anim_value(&g_walk_x); }
float theron_v2_smooth_get_y(void)      { return v2_anim_value(&g_walk_y); }
float theron_v2_smooth_get_angle(void)  { return v2_anim_value(&g_turn); }
float theron_v2_smooth_get_fade(void)   { return v2_anim_value(&g_fade); }

int theron_v2_smooth_is_moving(void) {
    return g_walk_x.active || g_walk_y.active || g_turn.active || g_fade.active;
}

int theron_v2_smooth_fade_active(void) {
    return g_fade.active;
}

const char *theron_v2_smooth_source_evidence(void) {
    return "Theron V2.2: smooth movement + 4-direction compass turn\n"
           "V1 tick rate preserved (55ms), visual interpolation only\n"
           "Walk: ease-out cubic, Turn: ease-out quad, Fade: ease-in-out cubic\n"
           "Source: THQUEST.ASM T520 (party/movement tick)\n"
           "        THQUEST.ASM T560 (dungeon viewport)\n"
           "        THQUEST.ASM T600 (UI overlay zones)\n"
           "        THQUEST.ASM T700 (timers / world tick)\n"
           "        ReDMCSB CLIKMENU.C F0365 (turn), F0366 (move),\n"
           "                         F0364 (stairs — easing reference)\n"
           "        ReDMCSB COMMAND.C F0380 (queue dispatch)\n"
           "        ReDMCSB GAMELOOP.C:47-50 (V1 tick cadence 55ms)\n"
           "        HuC6260/HuC6270 VDC/VCE; HuC6280 CPU; ADPCM\n"
           "Reference: csb_v2_smooth_movement.c (cross-game V2 pattern)\n"
           "           dm1_v2_smooth_movement_pc34.c, dm2_v2_smooth_movement.c\n"
           "Theron-specific: 4-direction compass (N/E/S/W), single-axis walk,\n"
           "                 teleporter fade replaces DM1/CSB/DM2 stairs.\n";
}
