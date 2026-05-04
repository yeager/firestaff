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
