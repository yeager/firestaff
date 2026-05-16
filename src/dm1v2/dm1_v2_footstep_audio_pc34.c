#include "dm1_v2_footstep_audio_pc34.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>

static M11_V2_FootstepConfig g_footstep_cfg;
static float g_env_val = 0.0f;
static bool g_triggered = false;
static bool g_last_foot = false;
static int16_t g_delay_buf[512];
static int g_delay_idx = 0;

void v2_footstep_init(void) {
    memset(&g_footstep_cfg, 0, sizeof(g_footstep_cfg));
    g_footstep_cfg.surface = STONE;
    g_footstep_cfg.volume = 128;
    g_footstep_cfg.pitch_variance = 0.1f;
    g_footstep_cfg.echo_enabled = false;
    g_env_val = 0.0f;
    g_triggered = false;
    memset(g_delay_buf, 0, sizeof(g_delay_buf));
    g_delay_idx = 0;
}

void v2_footstep_set_surface(M11_V2_SurfaceType type) {
    g_footstep_cfg.surface = type;
}

void v2_footstep_trigger(bool left_right) {
    g_triggered = true;
    g_last_foot = left_right;
}

void v2_footstep_set_echo(bool enabled) {
    g_footstep_cfg.echo_enabled = enabled;
}

int v2_footstep_get_sample(int16_t* buf, int* len) {
    if (!buf || !len) return -1;
    int sample_count = (*len > 0) ? *len : 256;
    *len = sample_count;

    float vol_scale = (float)g_footstep_cfg.volume / 255.0f;
    float decay_rate = 0.02f;
    switch (g_footstep_cfg.surface) {
        case STONE: decay_rate = 0.015f; break;
        case WATER: decay_rate = 0.005f; break;
        case METAL: decay_rate = 0.03f; break;
        case WOOD:  decay_rate = 0.02f; break;
        case DIRT:  decay_rate = 0.01f; break;
    }

    float pitch_mod = 1.0f + (g_footstep_cfg.pitch_variance * ((float)(rand() % 100) / 100.0f - 0.5f));

    for (int i = 0; i < sample_count; i++) {
        if (g_triggered) {
            g_env_val = 1.0f;
            g_triggered = false;
        }

        g_env_val *= (1.0f - decay_rate * pitch_mod);
        if (g_env_val < 0.001f) g_env_val = 0.0f;

        int noise = (rand() % 256) - 128;
        int16_t sample = (int16_t)((float)noise * g_env_val * vol_scale);

        if (g_footstep_cfg.echo_enabled) {
            int16_t delayed = g_delay_buf[g_delay_idx];
            sample = (int16_t)((sample * 0.7f) + (delayed * 0.3f));
            g_delay_buf[g_delay_idx] = sample;
            g_delay_idx = (g_delay_idx + 1) % 512;
        }

        buf[i] = sample;
    }
    return 0;
}

/* V2.2 Footstep Audio — surface-aware directional step sounds
 *
 * Different surfaces produce different footstep sounds:
 *   - Stone corridor: hard echo
 *   - Wooden floor: creak
 *   - Wet cave: splash
 *   - Gravel: crunch
 *   - Stairs: clang
 */

typedef enum {
    V22_SURFACE_STONE = 0,
    V22_SURFACE_WOOD,
    V22_SURFACE_WET,
    V22_SURFACE_GRAVEL,
    V22_SURFACE_STAIRS,
    V22_SURFACE_SILENT,
    V22_SURFACE_COUNT
} V22_SurfaceType;

static const char *g_footstep_samples[V22_SURFACE_COUNT] = {
    "sfx/step_stone.wav",
    "sfx/step_wood.wav",
    "sfx/step_wet.wav",
    "sfx/step_gravel.wav",
    "sfx/step_stairs.wav",
    NULL
};

static int g_footstep_enabled = 1;
static int g_step_counter = 0;
static V22_SurfaceType g_current_surface = V22_SURFACE_STONE;

V22_SurfaceType v22_footstep_surface_for_square(int square_type) {
    switch (square_type) {
        case 1: return V22_SURFACE_STONE;  /* corridor */
        case 2: case 3: return V22_SURFACE_STAIRS;
        case 4: return V22_SURFACE_SILENT; /* pit — falling, no step */
        case 5: return V22_SURFACE_STONE;  /* teleporter */
        default: return V22_SURFACE_STONE;
    }
}

const char *v22_footstep_get_sample(V22_SurfaceType surface, int step_index) {
    if (surface >= V22_SURFACE_COUNT || !g_footstep_enabled) return NULL;
    (void)step_index; /* could alternate L/R foot */
    return g_footstep_samples[surface];
}

void v22_footstep_on_step(int square_type) {
    g_current_surface = v22_footstep_surface_for_square(square_type);
    g_step_counter++;
    /* Caller plays: v22_footstep_get_sample(g_current_surface, g_step_counter) */
}

void v22_footstep_enable(int enabled) { g_footstep_enabled = enabled; }
int v22_footstep_is_enabled(void) { return g_footstep_enabled; }

