/*
 * csb_v2_runtime.c — CSB V2 Runtime Integration (Phase 5)
 *
 * Wires CSB_V2_ViewportState (smooth movement + V2_AnimClock) into the
 * CSB V1 game tick path.  Mirrors the DM2 V2 runtime pattern:
 *
 *   - Global CSB V2 viewport state (smooth + clock)
 *   - V1 tick: advance V2 animation clock
 *   - Render frame: drive smooth animation by wall-clock dt
 *   - Runtime binding: observe V1 party state changes, trigger
 *     smooth walk/turn/stairs animations
 *
 * CSB shares the DM1 movement engine (ReDMCSB COMMAND.C, DUNGEON.C,
 * CLIKMENU.C, GAMELOOP.C).  The V2.2 smooth-movement is presentation-
 * only; V1 game state is never mutated by V2 code.
 *
 * Easing per axis:
 *   - Walk: ease-out cubic  — snappy, natural deceleration
 *   - Turn: ease-out quad   — quick rotation snap
 *   - Stairs: ease-in-out cubic — deliberate vertical feel
 *
 * Source: ReDMCSB COMMAND.C F0380 (queue dispatch)
 *         ReDMCSB CLIKMENU.C F0365 (turn), F0366 (move)
 *         ReDMCSB CLIKMENU.C F0364 (stairs)
 *         ReDMCSB GAMELOOP.C (tick cadence, VBLANK-locked 55ms)
 *         ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *   v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *   v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 * Reference: dm2_v2_runtime.c (DM2 V2 runtime — same shape)
 *   dm2_v2_runtime_smooth_walk / dm2_v2_runtime_smooth_turn
 *   dm2_v2_viewport_smooth_walk / dm2_v2_viewport_smooth_turn
 */

#include "csb_v2_runtime.h"
#include "csb_v2_smooth_movement.h"
#include "csb_v2_phase_gate_pc34.h"
#include <stddef.h>
#include <string.h>

/* ── Global state ─────────────────────────────────────────────────── */

static CSB_V2_ViewportState s_vp;
static int s_vp_inited = 0;

/* V1 binding state.  s_v1_profile is observed (read-only) at every
 * v1_tick.  s_last_x/y/dir cache the previous V1-snapped position so
 * deltas are detected across ticks.  s_last_vert tracks party_z deltas
 * (stairs-level changes are treated as stairs transitions). */
static CSB_V1_RuntimeProfile *s_v1_profile = NULL;
static int s_last_party_x = 0;
static int s_last_party_y = 0;
static int s_last_party_z = 0;
static int s_last_party_dir = 0;
static int s_last_bound_state = 0;  /* 1=bound, 0=unbound, for change detection */

/* CSB V1 stores party position as integers (party_x, party_y, party_z,
 * party_dir).  A delta of at least 1 tile on x/y, 1 unit on z, or
 * 1 compass step (90°) starts a smooth animation.  This matches the
 * V1 source semantics: every accepted F0366 step and every accepted
 * F0365 turn is exactly 1 unit on its axis.  We compare integers
 * directly — no epsilon needed. */

/* CSB uses 90-degree compass directions (0=N 1=E 2=S 3=W).
 * Convert to degrees for the smooth animation system. */
static float csb_dir_to_angle(int dir) {
    /* Mask to 0..3 to defend against corrupt V1 state. */
    return (float)(dir & 3) * 90.0f;
}

/* ── V2 viewport accessors ───────────────────────────────────────── */

CSB_V2_ViewportState *csb_v2_runtime_get_viewport(void) {
    return s_vp_inited ? &s_vp : NULL;
}

/* ── Lifecycle ───────────────────────────────────────────────────── */

void csb_v2_runtime_init(int scale) {
    memset(&s_vp, 0, sizeof(s_vp));
    csb_v2_viewport_init(&s_vp, scale);
    s_vp_inited = 1;

    /* Unbind any prior V1 profile.  Re-binding is the caller's
     * responsibility via csb_v2_runtime_bind_to_v1(). */
    s_v1_profile = NULL;
    s_last_bound_state = 0;
    s_last_party_x = 0;
    s_last_party_y = 0;
    s_last_party_z = 0;
    s_last_party_dir = 0;
}

void csb_v2_runtime_cleanup(void) {
    if (!s_vp_inited) return;
    s_v1_profile = NULL;
    s_last_bound_state = 0;
    /* Note: csb_v2_viewport_init was called via csb_v2_runtime_init.
     * The viewport's global clock/light/chaos state is reset only on
     * next init — that is intentional and matches DM2's lifecycle. */
    s_vp_inited = 0;
}

/* ── V1 tick ─────────────────────────────────────────────────────── */

void csb_v2_runtime_v1_tick(uint32_t now_ms) {
    if (!s_vp_inited) return;

    /* Advance the V2 animation clock to the new V1 tick boundary.
     * Source: ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms). */
    csb_v2_viewport_v1_tick(&s_vp, now_ms);

    /* Phase 5 binding seam: if a V1 profile is bound, observe the
     * current party state and trigger smooth animations on deltas.
     *
     * The "from" position is the previous V1-snapped position; the
     * "to" position is the new V1-snapped position.  Both are passed
     * as float coordinates; V1's integer grid becomes the animation
     * endpoint, and the renderer interpolates visually between them.
     *
     * Source: ReDMCSB COMMAND.C F0380 (queue dispatch) — lock + dequeue
     *         ReDMCSB CLIKMENU.C F0365 (turn), F0366 (move)
     *         ReDMCSB CLIKMENU.C F0364 (stairs — party_z delta). */
    if (s_v1_profile && s_last_bound_state) {
        const int cur_x = s_v1_profile->party_x;
        const int cur_y = s_v1_profile->party_y;
        const int cur_z = s_v1_profile->party_z;
        const int cur_dir = s_v1_profile->party_dir;

        /* Vertical delta: party_z change is a stairs event
         * (CLIKMENU.C F0364).  Stairs takes priority over walk. */
        if (cur_z != s_last_party_z) {
            const float dx = (float)cur_x;
            const float dy = (float)cur_y;
            const float vx = (float)s_last_party_x;
            const float vy = (float)s_last_party_y;
            /* Vert offset is the change in floor/height level, scaled
             * to a visual offset (1 unit per level, signed). */
            const float vert_offset = (float)(cur_z - s_last_party_z);
            csb_v2_smooth_start_stairs(vx, vy, dx, dy, vert_offset);
        }
        /* Walk: x/y change without z change (F0366 forward/right step) */
        else if (cur_x != s_last_party_x || cur_y != s_last_party_y) {
            const float fx = (float)s_last_party_x;
            const float fy = (float)s_last_party_y;
            const float tx = (float)cur_x;
            const float ty = (float)cur_y;
            csb_v2_smooth_start_walk(fx, fy, tx, ty);
        }
        /* Turn: direction change only (F0365 turn), no x/y move */
        else if (cur_dir != s_last_party_dir) {
            const float fa = csb_dir_to_angle(s_last_party_dir);
            const float ta = csb_dir_to_angle(cur_dir);
            csb_v2_smooth_start_turn(fa, ta);
        }

        /* Cache the new V1-snapped state for the next tick. */
        s_last_party_x = cur_x;
        s_last_party_y = cur_y;
        s_last_party_z = cur_z;
        s_last_party_dir = cur_dir;
    }
}

/* ── Render frame ────────────────────────────────────────────────── */

void csb_v2_runtime_render_frame(uint32_t now_ms) {
    if (!s_vp_inited) return;
    /* csb_v2_viewport_render_frame advances the smooth animations by
     * the wall-clock delta since the previous render frame, then runs
     * light + chaos ticks.  Source: csb_v2_viewport_renderer.c:25 */
    csb_v2_viewport_render_frame(&s_vp, now_ms);
}

/* ── Smooth movement triggers (manual API) ──────────────────────── */

void csb_v2_runtime_smooth_walk(float fx, float fy, float tx, float ty) {
    if (!s_vp_inited) return;
    csb_v2_smooth_start_walk(fx, fy, tx, ty);
    /* Keep the binding cache in sync with the manual trigger so the
     * automatic binding seam does not double-trigger an animation on
     * the next v1_tick. */
    if (s_v1_profile) {
        s_last_party_x = s_v1_profile->party_x;
        s_last_party_y = s_v1_profile->party_y;
        s_last_party_z = s_v1_profile->party_z;
    }
}

void csb_v2_runtime_smooth_turn(float fa, float ta) {
    if (!s_vp_inited) return;
    csb_v2_smooth_start_turn(fa, ta);
    if (s_v1_profile) {
        s_last_party_dir = s_v1_profile->party_dir;
    }
}

void csb_v2_runtime_smooth_stairs(float fx, float fy,
                                  float tx, float ty,
                                  float vert_offset) {
    if (!s_vp_inited) return;
    csb_v2_smooth_start_stairs(fx, fy, tx, ty, vert_offset);
    if (s_v1_profile) {
        s_last_party_x = s_v1_profile->party_x;
        s_last_party_y = s_v1_profile->party_y;
        s_last_party_z = s_v1_profile->party_z;
    }
}

/* ── Runtime binding ────────────────────────────────────────────── */

void csb_v2_runtime_bind_to_v1(CSB_V1_RuntimeProfile *profile) {
    s_v1_profile = profile;
    s_last_bound_state = (profile != NULL) ? 1 : 0;

    if (profile) {
        /* Anchor "from" to current V1 state.  No animation starts
         * (binding is silent).  Subsequent v1_tick() calls will detect
         * deltas vs this anchor and trigger animations. */
        s_last_party_x = profile->party_x;
        s_last_party_y = profile->party_y;
        s_last_party_z = profile->party_z;
        s_last_party_dir = profile->party_dir;
    } else {
        s_last_party_x = 0;
        s_last_party_y = 0;
        s_last_party_z = 0;
        s_last_party_dir = 0;
    }
}

int csb_v2_runtime_is_bound(void) {
    return s_last_bound_state && s_v1_profile != NULL;
}

void csb_v2_runtime_force_sync(void) {
    if (!s_v1_profile) return;
    s_last_party_x = s_v1_profile->party_x;
    s_last_party_y = s_v1_profile->party_y;
    s_last_party_z = s_v1_profile->party_z;
    s_last_party_dir = s_v1_profile->party_dir;
    /* Cancel any in-flight smooth animation that was triggered before
     * the sync — a saved-game load jumps party state; we don't want
     * the renderer to interpolate from a stale "from" position. */
    csb_v2_smooth_init();
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char *csb_v2_runtime_source_evidence(void) {
    return
        "CSB V2 Runtime Integration (Phase 5: smooth movement binding)\n"
        "\n"
        "Source locks (ReDMCSB Common Toolchains):\n"
        "  COMMAND.C:2045-2156 F0380_COMMAND_ProcessQueue_CPSC\n"
        "    lock, empty/movement-disabled gate, dequeue, dispatch\n"
        "  CLIKMENU.C:142-179  F0365_COMMAND_ProcessTypes1To2_TurnParty\n"
        "  CLIKMENU.C:180-347  F0366_COMMAND_ProcessTypes3To6_MoveParty\n"
        "  CLIKMENU.C:135-141  F0364_COMMAND_TakeStairs (party_z delta)\n"
        "  GAMELOOP.C:47-50     V1 tick cadence (VBLANK-locked 55ms)\n"
        "  GAMELOOP.C:164-219   V1 input wait / command queue loop\n"
        "  GAMELOOP.C:215-219   F0380 until input waiting stops\n"
        "\n"
        "Phase 5 binding seam:\n"
        "  csb_v2_runtime_bind_to_v1(profile) — observe V1 party state\n"
        "  csb_v2_runtime_v1_tick()           — detect deltas, trigger\n"
        "  csb_v2_runtime_render_frame()      — drive smooth animation\n"
        "\n"
        "V1 state writes (profile->party_x/y/z/dir) are NEVER mutated by\n"
        "V2 code.  CSB V2 only reads them once per V1 tick and starts a\n"
        "visual interpolation.  Cooldowns, collision, sensors, and redraw\n"
        "cadence are unchanged.\n"
        "\n"
        "Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth)\n"
        "  v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync\n"
        "  v22_smooth_update_from_clock / v22_smooth_get_x/y/angle\n"
        "Reference: dm2_v2_runtime.c (DM2 V2 runtime — same shape)\n"
        "  dm2_v2_runtime_smooth_walk / dm2_v2_runtime_smooth_turn\n"
        "  dm2_v2_viewport_smooth_walk / dm2_v2_viewport_smooth_turn\n";
}
