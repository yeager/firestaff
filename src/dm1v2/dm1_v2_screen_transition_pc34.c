#include "dm1_v2_screen_transition_pc34.h"

static M11_V2_Transition g_trans;
static uint32_t g_rand_state = 12345;

static uint32_t v2_rand(void) {
    g_rand_state = g_rand_state * 1103515245 + 12345;
    return (g_rand_state >> 16) & 0x7FFF;
}

void v2_transition_init(void) {
    memset(&g_trans, 0, sizeof(g_trans));
}

void v2_transition_start(M11_V2_TransitionType type, float speed, int w, int h) {
    g_trans.type = type;
    g_trans.progress = 0.0f;
    g_trans.speed = speed;
    g_trans.active = true;
    g_trans.w = w;
    g_trans.h = h;
}

void v2_transition_update(float dt) {
    if (!g_trans.active) return;
    g_trans.progress += g_trans.speed * dt;
    if (g_trans.progress >= 1.0f) {
        g_trans.progress = 1.0f;
        g_trans.active = false;
    }
}

void v2_transition_apply(uint8_t* src, uint8_t* dst, int w, int h) {
    if (!src || !dst) return;
    memcpy(dst, src, w * h);

    if (!g_trans.active && g_trans.progress < 1.0f) return;

    float p = g_trans.progress;
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int idx = y * w + x;
            switch (g_trans.type) {
                case FADE_BLACK:
                    dst[idx] = (uint8_t)(dst[idx] * (1.0f - p));
                    break;
                case FADE_WHITE:
                    dst[idx] = (uint8_t)(dst[idx] * (1.0f - p) + 255.0f * p);
                    break;
                case DISSOLVE:
                    if ((v2_rand() % 100) < (int)(p * 100)) {
                        dst[idx] = 0;
                    }
                    break;
                case WIPE_LEFT:
                    if (x >= (int)(w * p)) {
                        dst[idx] = 0;
                    }
                    break;
                case WIPE_DOWN:
                    if (y >= (int)(h * p)) {
                        dst[idx] = 0;
                    }
                    break;
                case PIXELATE:
                    int block_size = 8;
                    int bx = (x / block_size) * block_size;
                    int by = (y / block_size) * block_size;
                    if ((bx + by) / (w + h) > p) {
                        dst[idx] = 0;
                    }
                    break;
            }
        }
    }
}

bool v2_transition_is_active(void) {
    return g_trans.active;
}

void v2_transition_skip(void) {
    g_trans.progress = 1.0f;
    g_trans.active = false;
}
