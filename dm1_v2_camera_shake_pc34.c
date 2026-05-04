#include "dm1_v2_camera_shake_pc34.h"

static M11_V2_CameraShake g_shake = {0.0f, 0.0f, false, 0.0f, 0.0f, 10.0f, 0.0f};

void v2_shake_init(void) {
    g_shake.intensity = 0.0f;
    g_shake.decay = 0.0f;
    g_shake.active = false;
    g_shake.offset_x = 0.0f;
    g_shake.offset_y = 0.0f;
    g_shake.frequency = 10.0f;
    g_shake.phase = 0.0f;
}

void v2_shake_trigger(float intensity, float decay_rate) {
    g_shake.intensity = intensity;
    g_shake.decay = decay_rate;
    g_shake.active = true;
    g_shake.phase = 0.0f;
}

void v2_shake_update(float dt) {
    if (!g_shake.active) {
        g_shake.offset_x = 0.0f;
        g_shake.offset_y = 0.0f;
        return;
    }

    g_shake.intensity -= g_shake.decay * dt;
    if (g_shake.intensity < 0.01f) {
        g_shake.intensity = 0.0f;
        g_shake.active = false;
        g_shake.offset_x = 0.0f;
        g_shake.offset_y = 0.0f;
        return;
    }

    g_shake.phase += g_shake.frequency * dt;
    g_shake.offset_x = sinf(g_shake.phase) * g_shake.intensity;
    g_shake.offset_y = cosf(g_shake.phase * 1.3f) * g_shake.intensity * 0.8f;
}

void v2_shake_get_offset(float *x, float *y) {
    if (x) *x = g_shake.offset_x;
    if (y) *y = g_shake.offset_y;
}

bool v2_shake_is_active(void) {
    return g_shake.active;
}

void v2_shake_stop(void) {
    g_shake.active = false;
    g_shake.intensity = 0.0f;
    g_shake.offset_x = 0.0f;
    g_shake.offset_y = 0.0f;
}
