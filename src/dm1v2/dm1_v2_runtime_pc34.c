#include "dm1_v2_runtime_pc34.h"
#include "dm1_v2_lighting_dynamic_pc34.h"
#include "dm1_v2_particle_system_pc34.h"

#include <string.h>

/* DM1 V2 runtime shell.
 *
 * Source-lock anchors:
 * - ReDMCSB GAMELOOP.C:164 sets G0321_B_StopWaitingForPlayerInput before the
 *   input command loop.
 * - ReDMCSB GAMELOOP.C:215 dispatches F0380_COMMAND_ProcessQueue_CPSC each
 *   loop, then GAMELOOP.C:219 waits until a command stops input waiting and
 *   game time is ticking.
 * - ReDMCSB DUNGEON.C:1371-1391 updates map coordinates from facing direction,
 *   forward steps and right/strafe steps using direction-to-step tables.
 *
 * V2 keeps those semantics split into a logical runtime tick and a modern
 * viewport/camera presentation layer.  This file intentionally does not make
 * gameplay decisions; it owns the V2 shell state used by later lanes. */

#define DM1_V2_DEFAULT_MOVE_SPEED 256
#define DM1_V2_DEFAULT_TURN_SPEED 1
#define DM1_V2_DEFAULT_STEP_THRESHOLD 256
#define DM1_V2_DEFAULT_SUBPIXEL_ACCEL 0
#define DM1_V2_DEFAULT_COLLISION_PREDICT_FRAMES 0
#define DM1_V2_COMMAND_MOVE_FORWARD 1
#define DM1_V2_COMMAND_MOVE_BACKWARD 2
#define DM1_V2_COMMAND_TURN_LEFT 3
#define DM1_V2_COMMAND_TURN_RIGHT 4
#define DM1_V2_COMMAND_MOVE_RIGHT 5
#define DM1_V2_COMMAND_MOVE_LEFT 6

void dm1_v2_runtime_init(DM1_V2_RuntimeState* runtime) {
    if (!runtime) return;
    memset(runtime, 0, sizeof(*runtime));
    runtime->mode = DM1_V2_RUNTIME_STOPPED;
    dm1_v2_pos_init(&runtime->player, 0, 0, 0);
    runtime->movement.moveSpeed = DM1_V2_DEFAULT_MOVE_SPEED;
    runtime->movement.turnSpeed = DM1_V2_DEFAULT_TURN_SPEED;
    runtime->movement.stepThreshold = DM1_V2_DEFAULT_STEP_THRESHOLD;
    runtime->movement.subPixelAccel = DM1_V2_DEFAULT_SUBPIXEL_ACCEL;
    runtime->movement.collisionPredictFrames = DM1_V2_DEFAULT_COLLISION_PREDICT_FRAMES;
    dm1_v2_vp_init(&runtime->viewport);
    runtime->lastCommand = 0;
}

void dm1_v2_runtime_start(DM1_V2_RuntimeState* runtime, uint32_t nowMs) {
    if (!runtime) return;
    runtime->mode = DM1_V2_RUNTIME_RUNNING;
    runtime->tickCount = 0;
    runtime->lastTickMs = nowMs;
    runtime->lastCommand = 0;
    dm1_v2_vp_mark_dirty(&runtime->viewport);
}

void dm1_v2_runtime_stop(DM1_V2_RuntimeState* runtime) {
    if (!runtime) return;
    runtime->mode = DM1_V2_RUNTIME_STOPPED;
}

int dm1_v2_runtime_is_running(const DM1_V2_RuntimeState* runtime) {
    return runtime && runtime->mode == DM1_V2_RUNTIME_RUNNING;
}

void dm1_v2_runtime_tick(DM1_V2_RuntimeState* runtime, uint32_t nowMs) {
    uint32_t dt;
    if (!dm1_v2_runtime_is_running(runtime)) return;
    dt = nowMs >= runtime->lastTickMs ? nowMs - runtime->lastTickMs : 0U;
    runtime->tickCount++;
    runtime->lastTickMs = nowMs;
    dm1_v2_vp_tick_scroll(&runtime->viewport, (int)dt);
    if (!dm1_v2_vp_is_scrolling(&runtime->viewport)) {
        dm1_v2_vp_present(&runtime->viewport, (int32_t)nowMs);
    }
}

int dm1_v2_runtime_apply_command(DM1_V2_RuntimeState* runtime, int command, uint32_t nowMs) {
    if (!dm1_v2_runtime_is_running(runtime)) return 0;
    runtime->lastCommand = command;
    switch (command) {
        case DM1_V2_COMMAND_MOVE_FORWARD:
            dm1_v2_move_step(&runtime->player, &runtime->movement, runtime->player.facingDir, (int32_t)nowMs);
            dm1_v2_vp_begin_scroll(&runtime->viewport, 0, -8, 16);
            break;
        case DM1_V2_COMMAND_MOVE_BACKWARD:
            dm1_v2_move_step(&runtime->player, &runtime->movement, (runtime->player.facingDir + 4) & 7, (int32_t)nowMs);
            dm1_v2_vp_begin_scroll(&runtime->viewport, 0, 8, 16);
            break;
        case DM1_V2_COMMAND_TURN_LEFT:
            dm1_v2_turn(&runtime->player, -1);
            dm1_v2_vp_begin_scroll(&runtime->viewport, -8, 0, 16);
            break;
        case DM1_V2_COMMAND_TURN_RIGHT:
            dm1_v2_turn(&runtime->player, 1);
            dm1_v2_vp_begin_scroll(&runtime->viewport, 8, 0, 16);
            break;
        case DM1_V2_COMMAND_MOVE_RIGHT:
            dm1_v2_move_step(&runtime->player, &runtime->movement, (runtime->player.facingDir + 2) & 7, (int32_t)nowMs);
            dm1_v2_vp_begin_scroll(&runtime->viewport, 8, 0, 16);
            break;
        case DM1_V2_COMMAND_MOVE_LEFT:
            dm1_v2_move_step(&runtime->player, &runtime->movement, (runtime->player.facingDir + 6) & 7, (int32_t)nowMs);
            dm1_v2_vp_begin_scroll(&runtime->viewport, -8, 0, 16);
            break;
        default:
            return 0;
    }
    dm1_v2_vp_mark_dirty(&runtime->viewport);
    return 1;
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.1 Runtime — Frame loop integration
 *
 * V2.1 frame loop:
 *   1. V1 game loop tick (dm1_v1_game_loop_pc34_compat)
 *   2. V1 viewport render to indexed framebuffer
 *   3. V2.1 EPX upscale
 *   4. Palette → RGBA
 *   5. SDL present
 *
 * This preserves V1 timing exactly (VBlank-locked at original rate)
 * while rendering at modern resolution.
 * ══════════════════════════════════════════════════════════════════════ */

typedef enum {
    V21_MODE_V1_FAITHFUL,     /* Exact V1 behavior, upscaled */
    V21_MODE_V1_ENHANCED,     /* V2.2 features layered on V1 */
} V21_RuntimeMode;

static V21_RuntimeMode g_v21_mode = V21_MODE_V1_FAITHFUL;

void v21_runtime_set_mode(V21_RuntimeMode mode) {
    g_v21_mode = mode;
}

int v21_runtime_is_faithful(void) {
    return g_v21_mode == V21_MODE_V1_FAITHFUL;
}

const char *v21_runtime_source_evidence(void) {
    return
        "GAMELOOP.C F0002_MAIN_GameLoop_CPSDF: V1 game loop drives V2.1\n"
        "VBLANK.C F0526: VBlank timing preserved\n"
        "V2.1: V1 tick -> V1 render -> EPX upscale -> palette -> present\n";
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.2 Runtime Integration — enhanced feature tick
 *
 * Called once per frame when V2.2 mode is active.
 * Coordinates all enhanced subsystems.
 * ══════════════════════════════════════════════════════════════════════ */


/* Forward declarations for V2.2 subsystems (linked when available) */
/* v2_light_tick: weak fallback here; real impl in dm1_v2_lighting_dynamic_pc34.c
 * (linked in game binary, absent in shell tests) */
__attribute__((weak)) void v2_light_tick(float dt) { (void)dt; }
__attribute__((weak)) void v22_light_rebuild_map(void) {}
__attribute__((weak)) int v22_smooth_tick(float*,float*,float*) { return 0; }
__attribute__((weak)) void v22_shake_tick(float dt,float*dx,float*dy) { (void)dt;if(dx)*dx=0;if(dy)*dy=0; }
__attribute__((weak)) void v2_particle_tick(float dt) { (void)dt; }
__attribute__((weak)) void v22_damage_tick(float dt) { (void)dt; }
__attribute__((weak)) void v2_msglog_tick(float dt) { (void)dt; }
__attribute__((weak)) void v22_stats_tick_playtime(void) {}
__attribute__((weak)) int v2_level_transition_tick(float dt) { (void)dt; return 0; }

void v22_runtime_enhanced_tick(float dt) {
    /* 1. V2 dynamic lighting — update flicker + rebuild additive light map */
    v2_light_tick(dt);
    /* 1b. V22 dynamic lighting — rebuild per-tile propagation light map */
    v22_light_rebuild_map();

    /* 2. Smooth movement interpolation */
    v22_smooth_tick(NULL, NULL, NULL);

    /* 3. Camera shake */
    {
        float dx, dy;
        v22_shake_tick(dt, &dx, &dy);
        (void)dx; (void)dy; /* applied by renderer */
    }

    /* 4. Particle system update */
    v2_particle_tick(dt);

    /* 5. Damage numbers fade */
    v22_damage_tick(dt);

    /* 6. Weather effects */
    /* Weather is per-zone, updated on zone change */

    /* 7. Message log fade */
    v2_msglog_tick(dt);

    /* 8. Stat tracker — tick playtime every second */
    {
        static float playtime_acc = 0;
        playtime_acc += dt;
        if (playtime_acc >= 1.0f) {
            playtime_acc -= 1.0f;
            v22_stats_tick_playtime();
        }
    }

    /* 9. Level transition animation */
    v2_level_transition_tick(dt);

    /* 10. Screenshot capture (if pending) */
    /* Done by caller after render, not during tick */
}

int v22_runtime_enhanced_is_active(void) {
    return !v21_runtime_is_faithful();
}

const char *v22_runtime_version_string(void) {
    return "DM1 V2.2 Enhanced";
}

