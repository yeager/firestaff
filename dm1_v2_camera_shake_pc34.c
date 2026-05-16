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

/* ══════════════════════════════════════════════════════════════════════
 * V2.2 Camera Shake — Trauma-based screen shake
 *
 * Based on Squirrel Eiserloh's GDC talk: trauma = 0..1,
 * shake_amount = trauma^2, random offset per frame.
 * Trauma decays over time.
 * ══════════════════════════════════════════════════════════════════════ */

static float g_trauma = 0.0f;
static float g_decay_rate = 2.0f; /* trauma units per second */
static float g_max_offset = 8.0f; /* pixels at trauma=1 */
static uint32_t g_shake_seed = 42;

static float shake_random(void) {
    g_shake_seed = g_shake_seed * 1103515245 + 12345;
    return ((float)((g_shake_seed >> 16) & 0x7FFF) / 32767.0f) * 2.0f - 1.0f;
}

void v22_shake_add_trauma(float amount) {
    g_trauma += amount;
    if (g_trauma > 1.0f) g_trauma = 1.0f;
}

void v22_shake_tick(float dt, float *out_dx, float *out_dy) {
    float shake;
    g_trauma -= g_decay_rate * dt;
    if (g_trauma < 0.0f) g_trauma = 0.0f;
    shake = g_trauma * g_trauma; /* quadratic falloff */
    if (out_dx) *out_dx = shake * g_max_offset * shake_random();
    if (out_dy) *out_dy = shake * g_max_offset * shake_random();
}

void v22_shake_reset(void) { g_trauma = 0.0f; }
float v22_shake_get_trauma(void) { return g_trauma; }

