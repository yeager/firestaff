#include "dm1_v2_anim_timing.h"
#include "dm1_v2_screen_transition_pc34.h"
#include <string.h>

static V2_Anim g_trans;
static M11_V2_TransitionType g_trans_kind;

void v2_screen_transition_start(int kind, float duration_ms) {
    g_trans_kind = (M11_V2_TransitionType)kind;
    v2_anim_start(&g_trans, 0.0f, 1.0f, duration_ms, V2_EASE_IN_OUT_QUAD);
}

void v2_screen_transition_update(float dt_ms) {
    v2_anim_update(&g_trans, dt_ms);
}

float v2_screen_transition_progress(void) {
    return v2_anim_value(&g_trans);
}

int v2_screen_transition_is_done(void) {
    return v2_anim_is_done(&g_trans);
}

void v2_screen_transition_apply(const uint8_t *src, uint8_t *dst, int w, int h) {
    float p = v2_anim_value(&g_trans);
    int x, y;
    if (!src || !dst) return;
    memcpy(dst, src, w * h);
    for (y = 0; y < h; y++) {
        for (x = 0; x < w; x++) {
            int idx = y * w + x;
            switch (g_trans_kind) {
                case FADE_BLACK: if (p > 0.5f) dst[idx] = 0; break;
                case FADE_WHITE: if (p < 0.5f) dst[idx] = 15; break;
                case WIPE_LEFT:  if (x < (int)(w * p)) dst[idx] = 0; break;
                case WIPE_DOWN:  if (y >= (int)(h * p)) dst[idx] = 0; break;
                case DISSOLVE:   break;
                case PIXELATE:   break;
            }
        }
    }
}

static V2_Anim g_screen_fade;
void v22_screen_fade_start(int fade_in) {
    float from = fade_in ? 0.0f : 1.0f;
    float to = fade_in ? 1.0f : 0.0f;
    v2_anim_start(&g_screen_fade, from, to, 4 * V1_TICK_MS, V2_EASE_IN_OUT_QUAD);
}
void v22_screen_fade_update(float dt_ms) { v2_anim_update(&g_screen_fade, dt_ms); }
float v22_screen_fade_alpha(void) { return v2_anim_value(&g_screen_fade); }
int v22_screen_fade_is_done(void) { return v2_anim_is_done(&g_screen_fade); }
