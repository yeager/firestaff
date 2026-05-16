#include "dm1_v2_anim_timing.h"
#include "dm1_v2_level_transition_pc34.h"

static M11_V2_LevelTransition g_transition;

void v2_level_trans_init(void) {
    memset(&g_transition, 0, sizeof(g_transition));
}

void v2_level_trans_start(M11_V2_TransType type, int from, int to, int dx, int dy, int ddir, float speed) {
    g_transition.type = type;
    g_transition.from_level = from;
    g_transition.to_level = to;
    g_transition.dest_x = dx;
    g_transition.dest_y = dy;
    g_transition.dest_dir = ddir;
    g_transition.speed = speed;
    g_transition.progress = 0.0f;
    g_transition.active = true;
}

bool v2_level_trans_update(float dt) {
    if (!g_transition.active) {
        return false;
    }
    g_transition.progress += g_transition.speed * dt;
    if (g_transition.progress >= 1.0f) {
        g_transition.progress = 1.0f;
        g_transition.active = false;
        return true;
    }
    return false;
}

bool v2_level_trans_is_active(void) {
    return g_transition.active;
}

void v2_level_trans_cancel(void) {
    g_transition.active = false;
    g_transition.progress = 0.0f;
}

void v2_level_trans_render_overlay(uint8_t* fb, int w, int h) {
    if (!fb || !g_transition.active) {
        return;
    }

    float p = g_transition.progress;
    uint8_t overlay_color = 0;

    switch (g_transition.type) {
        case TRANS_STAIRS_DOWN: {
            int line = (int)(p * (float)h);
            for (int y = 0; y < line; y++) {
                for (int x = 0; x < w; x++) {
                    fb[y * w + x] = overlay_color;
                }
            }
            break;
        }
        case TRANS_STAIRS_UP: {
            int line = (int)(p * (float)h);
            for (int y = h - line; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    fb[y * w + x] = overlay_color;
                }
            }
            break;
        }
        case TRANS_PIT_FALL: {
            uint8_t fade = (uint8_t)(p * 255.0f);
            for (int i = 0; i < w * h; i++) {
                fb[i] = (uint8_t)((fb[i] * (255 - fade)) / 255);
            }
            break;
        }
        case TRANS_TELEPORT: {
            if (p < 0.2f || p > 0.8f) {
                float intensity;
                if (p < 0.2f) {
                    intensity = p * 5.0f;
                } else {
                    intensity = (1.0f - p) * 5.0f;
                }
                uint8_t flash = (uint8_t)(intensity * 255.0f);
                for (int i = 0; i < w * h; i++) {
                    fb[i] = flash;
                }
            }
            break;
        }
        case TRANS_PORTAL: {
            float max_dist = sqrtf((float)(w * w + h * h)) * 0.5f;
            float radius = p * max_dist;
            int cx = w / 2;
            int cy = h / 2;
            for (int y = 0; y < h; y++) {
                for (int x = 0; x < w; x++) {
                    float dist = sqrtf((float)((x - cx) * (x - cx) + (y - cy) * (y - cy)));
                    if (dist < radius) {
                        fb[y * w + x] = overlay_color;
                    }
                }
            }
            break;
        }
        default:
            break;
    }
}

/* V2 Level Transition — animated descent/ascent effect */

typedef enum {
    V2_TRANS_NONE,
    V2_TRANS_DESCEND,
    V2_TRANS_ASCEND,
    V2_TRANS_PIT_FALL,
    V2_TRANS_TELEPORT,
} V2_TransitionType;

static struct {
    V2_TransitionType type;
    float progress; /* 0..1 */
    float speed;
    int from_level, to_level;
    int active;
} g_level_trans = {0};

void v2_level_transition_start(V2_TransitionType type, int from, int to, float speed) {
    g_level_trans.type = type;
    g_level_trans.from_level = from;
    g_level_trans.to_level = to;
    g_level_trans.progress = 0;
    g_level_trans.speed = speed > 0 ? speed : 1.0f;
    g_level_trans.active = 1;
}

int v2_level_transition_tick(float dt) {
    if (!g_level_trans.active) return 0;
    g_level_trans.progress += g_level_trans.speed * dt;
    if (g_level_trans.progress >= 1.0f) {
        g_level_trans.progress = 1.0f;
        g_level_trans.active = 0;
    }
    return g_level_trans.active;
}

float v2_level_transition_get_progress(void) { return g_level_trans.progress; }
V2_TransitionType v2_level_transition_get_type(void) { return g_level_trans.type; }
int v2_level_transition_is_active(void) { return g_level_trans.active; }

/* V2.2 level transitions: duration is measured in V1 ticks.
 * Stairs = 3 V1 ticks (165ms) — same as V1 movement cooldown.
 * Pit fall = 6 V1 ticks (330ms).
 * Teleport = 2 V1 ticks (110ms) — flash + appear.
 *
 * v22_transition_v1_sync marker */

#define V22_TRANS_STAIRS_TICKS   3
#define V22_TRANS_PIT_TICKS      6
#define V22_TRANS_TELEPORT_TICKS 2

float v22_transition_duration_for_type(int type) {
    switch (type) {
        case 1: /* descend */ return V22_TRANS_STAIRS_TICKS * V1_TICK_MS;
        case 2: /* ascend */  return V22_TRANS_STAIRS_TICKS * V1_TICK_MS;
        case 3: /* pit */     return V22_TRANS_PIT_TICKS * V1_TICK_MS;
        case 4: /* teleport */return V22_TRANS_TELEPORT_TICKS * V1_TICK_MS;
        default: return V1_TICK_MS;
    }
}

