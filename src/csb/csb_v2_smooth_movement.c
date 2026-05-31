#include "csb_v2_smooth_movement.h"

/* ══════════════════════════════════════════════════════════════════════
 * CSB V2.2 Smooth Movement — visual interpolation of party transitions
 *
 * CSB shares the DM1 movement engine (ReDMCSB COMMAND.C, DUNGEON.C,
 * CLIKMENU.C, GAMELOOP.C).  V2.2 interpolation is presentation-only:
 *
 *   - V1 game state updates instantly (cooldowns, collision, sensors
 *     unchanged — source-lock preserved via ReDMCSB GAMELOOP.C:164-219
 *     and COMMAND.C:2045-2156).
 *   - V2 renderer interpolates from previous tile to new tile over
 *     exactly one V1 tick (55ms, V1_TICK_MS).
 *
 * Easing:
 *   - Walk: ease-out cubic  — snappy but not jarring
 *   - Turn: ease-out quad   — quick rotation snap
 *   - Stairs: ease-in-out cubic — deliberate vertical feel
 *
 * ReDMCSB source: COMMAND.C F0380 (queue dispatch), CLIKMENU.C F0365
 * (turn), F0366 (move), GAMELOOP.C (tick cadence, VBLANK-locked 55ms).
 * Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth movement)
 *   v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync
 *   v22_smooth_update_from_clock / v22_smooth_get_x/y/angle
 * ══════════════════════════════════════════════════════════════════════ */

static V2_Anim g_csb_walk_x, g_csb_walk_y;
static V2_Anim g_csb_turn;
static V2_Anim g_csb_stairs_x, g_csb_stairs_y, g_csb_stairs_vert;

void csb_v2_smooth_init(void) {
    g_csb_walk_x.active  = 0;
    g_csb_walk_y.active  = 0;
    g_csb_turn.active     = 0;
    g_csb_stairs_x.active  = 0;
    g_csb_stairs_y.active  = 0;
    g_csb_stairs_vert.active = 0;
}

void csb_v2_smooth_start_walk(float fx, float fy, float tx, float ty) {
    /* Each axis animates independently so diagonal movement is smooth */
    v2_anim_start_v1_tick(&g_csb_walk_x, fx, tx, CSB_V2_WALK_EASE);
    v2_anim_start_v1_tick(&g_csb_walk_y, fy, ty, CSB_V2_WALK_EASE);
}

void csb_v2_smooth_start_turn(float from, float to) {
    v2_anim_start_v1_tick(&g_csb_turn, from, to, CSB_V2_TURN_EASE);
}

void csb_v2_smooth_start_stairs(float fx, float fy,
                                float tx, float ty,
                                float vert_offset) {
    /* Ease-in-out cubic for deliberate vertical feel */
    v2_anim_start_v1_tick(&g_csb_stairs_x,    fx,         tx,         CSB_V2_STAIRS_EASE);
    v2_anim_start_v1_tick(&g_csb_stairs_y,    fy,         ty,         CSB_V2_STAIRS_EASE);
    v2_anim_start_v1_tick(&g_csb_stairs_vert,  0.0f, vert_offset, CSB_V2_STAIRS_EASE);
}

void csb_v2_smooth_update_from_clock(const V2_AnimClock *clock) {
    if (!clock) return;
    /* dt_ms carries the frame delta since last render — advance all anims */
    const float dt = clock->dt_ms;

    v2_anim_update(&g_csb_walk_x, dt);
    v2_anim_update(&g_csb_walk_y, dt);
    v2_anim_update(&g_csb_turn,   dt);
    v2_anim_update(&g_csb_stairs_x,    dt);
    v2_anim_update(&g_csb_stairs_y,    dt);
    v2_anim_update(&g_csb_stairs_vert, dt);
}

float csb_v2_smooth_get_x(void) {
    /* Stairs takes priority over walk when active */
    if (g_csb_stairs_x.active)
        return v2_anim_value(&g_csb_stairs_x);
    return v2_anim_value(&g_csb_walk_x);
}

float csb_v2_smooth_get_y(void) {
    if (g_csb_stairs_y.active)
        return v2_anim_value(&g_csb_stairs_y);
    return v2_anim_value(&g_csb_walk_y);
}

float csb_v2_smooth_get_vertical(void) {
    return v2_anim_value(&g_csb_stairs_vert);
}

float csb_v2_smooth_get_angle(void) { return v2_anim_value(&g_csb_turn); }

int csb_v2_smooth_is_moving(void) {
    return g_csb_walk_x.active || g_csb_walk_y.active ||
           g_csb_stairs_x.active || g_csb_stairs_y.active ||
           g_csb_turn.active;
}

const char *csb_v2_smooth_source_evidence(void) {
    return "CSB V2.2: shared DM1 V2.2 smooth movement, CSB-specific easing\n"
           "V1 tick rate preserved (55ms), visual interpolation only\n"
           "ReDMCSB COMMAND.C F0380, CLIKMENU.C F0365/F0366, GAMELOOP.C\n"
           "Reference: dm1_v2_smooth_movement_pc34.c (DM1 V2.2 smooth)\n"
           "  v22_smooth_start_walk_v1sync / v22_smooth_start_turn_v1sync\n"
           "  v22_smooth_update_from_clock / v22_smooth_get_x/y/angle\n";
}