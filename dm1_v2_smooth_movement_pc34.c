#include "dm1_v2_smooth_movement_pc34.h"
#include <math.h>

static M11_V2_SmoothMove g_move;
static M11_V2_SmoothRotation g_rot;

void v2_smooth_init(void) {
    g_move.active = false;
    g_move.progress = 0.0f;
    g_rot.active = false;
    g_rot.progress = 0.0f;
}

void v2_smooth_start_move(float from_x, float from_y, float to_x, float to_y, float speed) {
    g_move.from_x = from_x;
    g_move.from_y = from_y;
    g_move.to_x = to_x;
    g_move.to_y = to_y;
    g_move.speed = speed;
    g_move.progress = 0.0f;
    g_move.active = true;
}

void v2_smooth_update(float dt) {
    if (g_move.active) {
        g_move.progress += g_move.speed * dt;
        if (g_move.progress >= 1.0f) {
            g_move.progress = 1.0f;
            g_move.active = false;
        }
    }
    if (g_rot.active) {
        g_rot.progress += g_rot.speed * dt;
        if (g_rot.progress >= 1.0f) {
            g_rot.progress = 1.0f;
            g_rot.active = false;
        }
    }
}

void v2_smooth_get_position(float *x, float *y) {
    if (g_move.active) {
        *x = g_move.from_x + (g_move.to_x - g_move.from_x) * g_move.progress;
        *y = g_move.from_y + (g_move.to_y - g_move.from_y) * g_move.progress;
    } else {
        *x = g_move.to_x;
        *y = g_move.to_y;
    }
}

bool v2_smooth_is_moving(void) {
    return g_move.active;
}

void v2_smooth_start_rotation(float from_angle, float to_angle, float speed) {
    g_rot.from_angle = from_angle;
    g_rot.to_angle = to_angle;
    g_rot.speed = speed;
    g_rot.progress = 0.0f;
    g_rot.active = true;
}

float v2_smooth_get_rotation(void) {
    if (g_rot.active) {
        return g_rot.from_angle + (g_rot.to_angle - g_rot.from_angle) * g_rot.progress;
    }
    return g_rot.to_angle;
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.2 Smooth Movement — Interpolated party transitions
 *
 * V1 movement is instant (one frame = new position).
 * V2.2 interpolates between positions over N frames:
 *   - Walk: 8 frames interpolation
 *   - Turn: 6 frames rotation
 *   - Stairs: 12 frames with vertical shift
 *
 * The interpolation is purely visual — game state updates instantly
 * as in V1, but the camera smoothly transitions.
 * ══════════════════════════════════════════════════════════════════════ */

typedef struct {
    float from_x, from_y, from_angle;
    float to_x, to_y, to_angle;
    float t; /* 0.0 = start, 1.0 = complete */
    float speed; /* frames to complete */
    int active;
} V22_SmoothTransition;

static V22_SmoothTransition g_smooth = {0};

void v22_smooth_start_walk(float fx, float fy, float tx, float ty, int frames) {
    g_smooth.from_x = fx; g_smooth.from_y = fy;
    g_smooth.to_x = tx; g_smooth.to_y = ty;
    g_smooth.from_angle = g_smooth.to_angle = 0;
    g_smooth.t = 0.0f;
    g_smooth.speed = frames > 0 ? (1.0f / frames) : 1.0f;
    g_smooth.active = 1;
}

void v22_smooth_start_turn(float from_angle, float to_angle, int frames) {
    g_smooth.from_angle = from_angle;
    g_smooth.to_angle = to_angle;
    g_smooth.from_x = g_smooth.to_x = 0;
    g_smooth.from_y = g_smooth.to_y = 0;
    g_smooth.t = 0.0f;
    g_smooth.speed = frames > 0 ? (1.0f / frames) : 1.0f;
    g_smooth.active = 1;
}

int v22_smooth_tick(float *out_x, float *out_y, float *out_angle) {
    if (!g_smooth.active) return 0;
    g_smooth.t += g_smooth.speed;
    if (g_smooth.t >= 1.0f) {
        g_smooth.t = 1.0f;
        g_smooth.active = 0;
    }
    /* Ease-out cubic: t' = 1 - (1-t)^3 */
    float et = 1.0f - (1.0f - g_smooth.t) * (1.0f - g_smooth.t) * (1.0f - g_smooth.t);
    if (out_x) *out_x = g_smooth.from_x + (g_smooth.to_x - g_smooth.from_x) * et;
    if (out_y) *out_y = g_smooth.from_y + (g_smooth.to_y - g_smooth.from_y) * et;
    if (out_angle) *out_angle = g_smooth.from_angle + (g_smooth.to_angle - g_smooth.from_angle) * et;
    return g_smooth.active;
}

