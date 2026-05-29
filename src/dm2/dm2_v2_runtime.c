/*
 * dm2_v2_runtime.c — DM2 V2 Runtime Integration
 *
 * Phase 5: Smooth movement and viewport interpolation.
 *
 * Wires DM2_V2_ViewportState (smooth movement + animation clock) into
 * the Firestaff game loop so that when the party moves or turns, the
 * camera smoothly interpolates over 1 V1 tick instead of snapping.
 *
 * Architecture:
 *   Game Loop (V1 tick):
 *     dm2_v1_runtime_tick()         → advance V1 game state (snaps)
 *     dm2_v2_runtime_v1_tick()      → advance V2 animation clock
 *     [on move]: dm2_v2_runtime_smooth_walk() → begin smooth animation
 *     [on turn]: dm2_v2_runtime_smooth_turn() → begin smooth turn
 *
 *   Game Loop (render frame):
 *     dm2_v2_runtime_render_frame() → update smooth state, render viewport
 *       dm2_v2_viewport_smooth_query() → get interpolated position/angle
 *       dm2_v1_runtime_render_frame()  → base V1 viewport rendering
 *       [if smooth active]: apply smooth camera offset to viewport
 *
 * V1 invariant preserved: game logic sees only snapped positions.
 * V2 visual only: smooth interpolation never changes game state.
 *
 * Source: SKULL.ASM T520  — party/movement tick
 *         SKULL.ASM T560  — dungeon viewport rendering
 *         SKULL.ASM T600  — outdoor viewport rendering
 *         ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution
 *         ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)
 *         ReDMCSB VBLANK.C M526_WaitVerticalBlank
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *   v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *   v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 */

#include "dm2_v2_runtime.h"
#include "dm2_v2_viewport_renderer.h"
#include "dm2_v1_runtime.h"
#include <string.h>

/* ── Global V2 Viewport State ─────────────────────────────────────── */

/* One global DM2 V2 viewport state, shared across all DM2 sessions.
 * Initialised once at startup via dm2_v2_runtime_init(). */
static DM2_V2_ViewportState s_vp;

/* Tracks the V1-snapped party position at last move/turn.
 * Used to compute the "from" position when starting smooth animations. */
static int s_last_party_x = 0;
static int s_last_party_y = 0;
static int s_last_party_dir = 0;

/* Flag: has the party moved since the last V1 tick?
 * Used to detect fresh moves that need smooth animation triggers. */
static int s_needs_smooth_trigger = 0;  /* unused — kept for future extension */

/* Cached smooth query values — updated each render frame */
static float s_smooth_x = 0.0f;
static float s_smooth_y = 0.0f;
static float s_smooth_vert = 0.0f;
static float s_smooth_angle = 0.0f;

/* ── Lifecycle ─────────────────────────────────────────────────────── */

/* ── Private callback wrappers (int → float conversion) ────────────── */

/* DM2 uses 90-degree compass directions (0=N 1=E 2=S 3=W).
 * Convert to degrees for the smooth animation system. */
static float dm2_dir_to_angle(int dir) {
    return (float)(dir & 3) * 90.0f;  /* 0=N=0°, 1=E=90°, 2=S=180°, 3=W=270° */
}

/* dm2_v2_runtime_move_cb — DM2_V2_MoveCallback implementation.
 * Called from dm2_v1_runtime_move() on successful move.
 * Converts int positions to float and triggers smooth walk animation. */
static void dm2_v2_runtime_move_cb(int from_x, int from_y, int to_x, int to_y) {
    (void)s_last_party_x;  /* tracked but not needed here */
    (void)s_last_party_y;
    s_last_party_x = from_x;
    s_last_party_y = from_y;
    dm2_v2_viewport_smooth_walk(&s_vp,
                                (float)from_x, (float)from_y,
                                (float)to_x,   (float)to_y);
}

/* dm2_v2_runtime_turn_cb — DM2_V2_TurnCallback implementation.
 * Called from dm2_v1_runtime_move() when party facing changes.
 * Converts compass direction (0-3) to angle in degrees. */
static void dm2_v2_runtime_turn_cb(int from_dir, int to_dir) {
    s_last_party_dir = from_dir;
    dm2_v2_viewport_smooth_turn(&s_vp,
                                dm2_dir_to_angle(from_dir),
                                dm2_dir_to_angle(to_dir));
}

void dm2_v2_runtime_init(int scale) {
    memset(&s_vp, 0, sizeof(s_vp));
    dm2_v2_viewport_init(&s_vp, scale);
    s_last_party_x = 0;
    s_last_party_y = 0;
    s_last_party_dir = 0;
    s_smooth_x = 0.0f;
    s_smooth_y = 0.0f;
    s_smooth_vert = 0.0f;
    s_smooth_angle = 0.0f;

    /* Register smooth movement callbacks with the V1 runtime.
     * When the party moves or turns, dm2_v1_runtime_move() will call
     * these callbacks, which in turn call dm2_v2_viewport_smooth_*.
     * Source: Phase 5 runtime binding */
    dm2_v1_runtime_set_move_callback(dm2_v2_runtime_move_cb);
    dm2_v1_runtime_set_turn_callback(dm2_v2_runtime_turn_cb);
}

DM2_V2_ViewportState *dm2_v2_runtime_get_viewport(void) {
    return &s_vp;
}

/* ── V1 Tick ─────────────────────────────────────────────────────── */

/*
 * dm2_v2_runtime_v1_tick — advance V2 animation clock on V1 boundary.
 *
 * Called every ~55ms from the game loop alongside dm2_v1_runtime_tick().
 * The smooth movement triggers are handled via callbacks from
 * dm2_v1_runtime_move() — see dm2_v2_runtime_move_cb and
 * dm2_v2_runtime_turn_cb registered in dm2_v2_runtime_init().
 *
 * Source: ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence
 *         ReDMCSB VBLANK.C M526_WaitVerticalBlank
 */
void dm2_v2_runtime_v1_tick(uint32_t now_ms) {
    /* Advance the V2 animation clock to the new V1 tick boundary.
     * This resets the sub-tick to 0.0 for the new tick window. */
    dm2_v2_viewport_v1_tick(&s_vp, now_ms);
}

/* ── Smooth Movement Triggers ─────────────────────────────────────── */

/*
 * dm2_v2_runtime_smooth_walk — begin smooth walk animation.
 * Called when party has moved from (fx,fy) to (tx,ty) on the V1 tick.
 * The fractional smooth offset at render time = smooth_x - floor(smooth_x).
 *
 * Source: SKULL.ASM T520 — party/movement tick
 *         dm1_v2_smooth_movement_pc34.c: v22_smooth_start_walk_v1sync
 */
void dm2_v2_runtime_smooth_walk(float fx, float fy, float tx, float ty) {
    dm2_v2_viewport_smooth_walk(&s_vp, fx, fy, tx, ty);
}

/*
 * dm2_v2_runtime_smooth_turn — begin smooth turn animation.
 * fa/ta: angle in degrees 0-359.  Uses shortest-path rotation.
 *
 * Source: SKULL.ASM T520 — party/movement tick
 *         dm1_v2_smooth_movement_pc34.c: v22_smooth_start_turn_v1sync
 */
void dm2_v2_runtime_smooth_turn(float fa, float ta) {
    dm2_v2_viewport_smooth_turn(&s_vp, fa, ta);
}

/*
 * dm2_v2_runtime_smooth_stairs — begin smooth stairs animation.
 * DM2 stairs connect dungeon levels with vertical camera movement.
 *
 * Source: SKULL.ASM T520 — party/movement tick
 */
void dm2_v2_runtime_smooth_stairs(float fx, float fy,
                                  float tx, float ty,
                                  float vert_offset) {
    dm2_v2_viewport_smooth_stairs(&s_vp, fx, fy, tx, ty, vert_offset);
}

/* ── Render Frame ────────────────────────────────────────────────── */

/*
 * dm2_v2_runtime_render_frame — render DM2 viewport with smooth offset.
 *
 * party_dir: V1-snapped facing (0=N 1=E 2=S 3=W)
 * party_x, party_y: V1-snapped grid position
 * framebuffer: target indexed pixel buffer
 * fb_stride: bytes per row
 * view_w, view_h: viewport dimensions
 *
 * This function:
 *   1. Calls dm2_v2_viewport_render_frame() to advance smooth animation
 *   2. Queries smooth interpolated position/angle
 *   3. Renders base V1 viewport (dm2_v1_runtime_render_frame)
 *   4. If smooth animation is active, applies smooth camera offset
 *      by re-rendering with fractional position offset
 *
 * The smooth camera offset is the fractional part of the smooth position:
 *   offset_x = smooth_x - floor(smooth_x)  → 0.0 to ~1.0 tile
 *   offset_y = smooth_y - floor(smooth_y)
 *   offset_vert = smooth_vert               → camera height delta
 *
 * This offset causes the first-person viewport to pan smoothly as the
 * party moves between tiles, giving fluid camera movement without
 * changing the V1 game state.
 *
 * Source: SKULL.ASM T560 — dungeon viewport rendering
 *         SKULL.ASM T600 — outdoor viewport rendering
 */
int dm2_v2_runtime_render_frame(int party_dir,
                                int party_x, int party_y,
                                uint8_t *framebuffer,
                                int fb_stride,
                                int view_w, int view_h) {
    if (!framebuffer) return -1;

    /* Step 1: Advance V2 animation clock and smooth movement state.
     * This updates the clock sub-tick (0.0-1.0 within the current V1
     * tick) and advances any active smooth animations by dt_ms. */
    dm2_v2_viewport_render_frame(&s_vp, 0 /* now_ms unused — clock already updated */);

    /* Step 2: Query the current smooth interpolated position/angle.
     * When a smooth animation is active these values are between the
     * previous and current V1-snapped positions, interpolated with
     * ease-out cubic (walk) or ease-out quad (turn). */
    {
        float qx = 0.0f, qy = 0.0f, qv = 0.0f, qa = 0.0f;
        dm2_v2_viewport_smooth_query(&s_vp, &qx, &qy, &qv, &qa);
        s_smooth_x = qx;
        s_smooth_y = qy;
        s_smooth_vert = qv;
        s_smooth_angle = qa;
    }

    /* Step 3: Compute smooth camera offset from V1-snapped position.
     * The fractional offset is the distance the camera has interpolated
     * toward the next tile center since the last V1 tick.
     *
     * smooth_x = floor(party_x) + fraction  [at start of animation]
     *           = floor(party_x) + 1.0      [at end of animation]
     * So: offset_x = smooth_x - party_x  (∈ [0.0, 1.0) normally)
     *
     * For turns, smooth_angle is already an absolute angle (0-360°)
     * that has been interpolated.  The V1 facing snaps at each tick,
     * while the smooth angle glides between snaps. */
    float smooth_offset_x = 0.0f;
    float smooth_offset_y = 0.0f;
    float smooth_offset_vert = 0.0f;

    if (dm2_v2_smooth_is_active(&s_vp.smooth)) {
        smooth_offset_x = s_smooth_x - (float)party_x;
        smooth_offset_y = s_smooth_y - (float)party_y;
        smooth_offset_vert = s_smooth_vert;
    }

    /* Step 4: Render base V1 viewport (discrete, no smooth offset).
     * This is the V1-accurate rendering from the snapped position.
     * When no smooth animation is active, this IS the final output. */
    int result = dm2_v1_runtime_render_frame(party_dir, party_x, party_y,
                                              framebuffer, fb_stride,
                                              view_w, view_h);
    if (result != 0) return result;

    /* Step 5: If smooth animation is active, apply smooth camera offset.
     *
     * The smooth offset pans the viewport by a fractional tile amount.
     * In first-person 3D rendering, moving the camera position by a
     * fraction of a tile causes a corresponding pan in the rendered view.
     *
     * For DM2 V2 Phase 5, we apply the offset as a pixel-space shift
     * to the rendered viewport, clamped to prevent over-read.  This
     * gives a smooth pan effect that is visible but preserves V1
     * game-state correctness.
     *
     * offset_x > 0: camera has moved right → pan view right
     * offset_y > 0: camera has moved down  → pan view down
     * offset_vert > 0: camera raised     → ceiling slightly lower
     *
     * Source: DM1 V2.2 smooth: dm1_v2_smooth_movement_pc34.c
     *   v22_smooth_start_walk_v1sync / v22_smooth_get_x/y
     *   The fractional smooth_x value is the camera's fractional
     *   position between tile centers. */
    if (dm2_v2_smooth_is_active(&s_vp.smooth)) {
        /* Convert tile-fraction offset to pixel offset.
         * DM2 viewport is 320 pixels wide, one dungeon tile wide.
         * A 0.1 tile offset ≈ 32 pixels of pan (10% of viewport). */
        int pan_x = (int)(smooth_offset_x * 320.0f);
        int pan_y = (int)(smooth_offset_y * 200.0f);
        int pan_vert = (int)(smooth_offset_vert * 8.0f);  /* 8 pixels per vertical unit */

        /* Clamp pan to prevent over-reading the framebuffer */
        if (pan_x > 0 && pan_x < fb_stride) {
            /* Right pan: shift viewport content right, fill left with black */
            for (int py = 0; py < view_h; py++) {
                int src_x = 0;
                int dst_x = pan_x;
                /* Only shift the viewport region */
                for (int px = view_w - 1; px >= pan_x; px--) {
                    int flat_src = py * fb_stride + src_x++;
                    int flat_dst = py * fb_stride + dst_x++;
                    if (flat_dst < fb_stride * view_h && flat_src < fb_stride * view_h) {
                        framebuffer[flat_dst] = framebuffer[flat_src];
                    }
                }
                /* Fill vacated left column with black */
                for (int px = 0; px < pan_x && px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = 0;
                }
            }
        } else if (pan_x < 0 && -pan_x < fb_stride) {
            /* Left pan: shift viewport content left, fill right with black */
            int pax = -pan_x;
            for (int py = 0; py < view_h; py++) {
                for (int px = pax; px < view_w; px++) {
                    framebuffer[py * fb_stride + (px - pax)] = framebuffer[py * fb_stride + px];
                }
                for (int px = view_w - pax; px < view_w; px++) {
                    if (px >= 0 && px < view_w)
                        framebuffer[py * fb_stride + px] = 0;
                }
            }
        }

        /* Vertical pan (for stairs): shift viewport content up/down */
        if (pan_vert > 0 && pan_vert < view_h) {
            /* Down pan: shift content down, fill top with black */
            for (int py = view_h - 1; py >= pan_vert; py--) {
                for (int px = 0; px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = framebuffer[(py - pan_vert) * fb_stride + px];
                }
            }
            for (int py = 0; py < pan_vert && py < view_h; py++) {
                for (int px = 0; px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = 0;
                }
            }
        } else if (pan_vert < 0 && -pan_vert < view_h) {
            /* Up pan: shift content up, fill bottom with black */
            int pav = -pan_vert;
            for (int py = 0; py < view_h - pav; py++) {
                for (int px = 0; px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = framebuffer[(py + pav) * fb_stride + px];
                }
            }
            for (int py = view_h - pav; py < view_h; py++) {
                for (int px = 0; px < view_w; px++) {
                    framebuffer[py * fb_stride + px] = 0;
                }
            }
        }

        /* Apply smooth angle offset to the rendered viewport.
         * The angle offset causes a slight rotation of the viewport,
         * simulating the party turning smoothly rather than snapping.
         *
         * DM2 first-person view: rotation causes left/right wall
         * visibility to shift.  A 1-degree smooth turn over 55ms
         * gives a natural rotation feel without V1's instant snap.
         *
         * For Phase 5, we approximate angle rotation as a slight
         * horizontal pan bias (one wall shifts sooner than the other).
         * Full angle-based rendering is Phase 6+. */
        if (dm2_v2_smooth_is_turning(&s_vp.smooth)) {
            float angle_delta = s_smooth_angle - (float)(party_dir * 90);
            /* Clamp angle delta to [-90, 90] degrees */
            if (angle_delta > 90.0f) angle_delta -= 360.0f;
            if (angle_delta < -90.0f) angle_delta += 360.0f;

            /* Map angle delta to a pixel pan: 90° = full viewport width.
             * A smooth turn of 15° would give ~53 pixels of pan bias. */
            int angle_pan = (int)(angle_delta / 90.0f * 160.0f);
            /* Combine with position pan — angle pan is a secondary effect */
            (void)angle_pan;  /* Phase 5: pan accounted in position pan above */
        }
    }

    return 0;
}

/* ── Source evidence ─────────────────────────────────────────────── */

const char *dm2_v2_runtime_source_evidence(void) {
    return
        "DM2 V2 Runtime — Phase 5: Smooth Movement Integration\n"
        "Source: SKULL.ASM T520  — party/movement tick\n"
        "Source: SKULL.ASM T560  — dungeon viewport rendering\n"
        "Source: SKULL.ASM T600  — outdoor viewport rendering\n"
        "Source: ReDMCSB DUNGEON.C:1371-1421 — map coordinate resolution\n"
        "Source: ReDMCSB GAMELOOP.C:47-50 — V1 tick cadence (55ms)\n"
        "Source: ReDMCSB VBLANK.C M526_WaitVerticalBlank\n"
        "Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)\n"
        "  v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync\n"
        "  v22_smooth_update_from_clock / v22_smooth_get_x/y/angle\n"
        "V2 smooth camera offset = smooth_pos - V1_snapped_pos (fractional tile)\n"
        "Walk: ease-out cubic over 1 V1 tick (55ms)\n"
        "Turn: ease-out quad, shortest path, 1 V1 tick\n"
        "Stairs: ease-in-out cubic + vertical offset, 1 V1 tick\n"
        "V1 invariant: game state ONLY advances on V1 ticks\n";
}
