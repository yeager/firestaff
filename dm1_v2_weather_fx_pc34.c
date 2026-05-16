#include "dm1_v2_weather_fx_pc34.h"

static uint32_t v2_weather_rng_state = 12345;
static uint32_t v2_weather_next_rand(void) {
    v2_weather_rng_state = v2_weather_rng_state * 1103515245 + 12345;
    return (v2_weather_rng_state >> 16) & 0x7FFF;
}

void v2_weather_init(M11_V2_WeatherState* state) {
    memset(state, 0, sizeof(M11_V2_WeatherState));
    state->type = M11_V2_WEATHER_NONE;
    state->intensity = 0.0f;
    state->wind_x = 0.0f;
}

void v2_weather_set(M11_V2_WeatherState* state, M11_V2_WeatherType type, float intensity) {
    state->type = type;
    state->intensity = intensity;
    if (state->intensity < 0.0f) state->intensity = 0.0f;
    if (state->intensity > 1.0f) state->intensity = 1.0f;
    state->drop_count = 0;
    state->spawn_timer = 0.0f;
}

void v2_weather_update(M11_V2_WeatherState* state, float dt) {
    if (state->type == M11_V2_WEATHER_NONE || state->intensity <= 0.0f) return;

    state->spawn_timer += dt;
    float spawn_rate = 1.0f / (state->intensity * 200.0f);
    if (spawn_rate < 0.01f) spawn_rate = 0.01f;

    while (state->spawn_timer >= spawn_rate && state->drop_count < 512) {
        state->spawn_timer -= spawn_rate;
        M11_V2_WeatherDrop* d = &state->drops[state->drop_count];
        d->x = (float)v2_weather_next_rand() * 0.01f;
        d->y = (float)v2_weather_next_rand() * 0.01f;
        d->speed = 2.0f + (float)v2_weather_next_rand() * 0.02f;
        d->length = 0.02f + (float)v2_weather_next_rand() * 0.03f;
        d->alpha = state->intensity;
        state->drop_count++;
    }

    for (int i = 0; i < state->drop_count; i++) {
        M11_V2_WeatherDrop* d = &state->drops[i];
        d->y += d->speed * dt;
        d->x += state->wind_x * dt;
        if (d->y > 1.0f || d->x < 0.0f || d->x > 1.0f) {
            state->drop_count--;
            state->drops[i] = state->drops[state->drop_count];
            i--;
        }
    }
}

void v2_weather_render(const M11_V2_WeatherState* state, uint8_t* framebuffer, int w, int h) {
    if (!framebuffer || state->type == M11_V2_WEATHER_NONE) return;

    if (state->type == M11_V2_WEATHER_FOG) {
        uint8_t fog_color = (uint8_t)(state->intensity * 128.0f);
        for (int i = 0; i < w * h; i++) {
            uint8_t base = framebuffer[i];
            framebuffer[i] = (uint8_t)(base * (1.0f - state->intensity) + fog_color * state->intensity);
        }
        return;
    }

    uint8_t drop_color = 0xCC;
    if (state->type == M11_V2_WEATHER_DUST) drop_color = 0xAA;
    if (state->type == M11_V2_WEATHER_DRIP) drop_color = 0x88;

    for (int i = 0; i < state->drop_count; i++) {
        const M11_V2_WeatherDrop* d = &state->drops[i];
        int sx = (int)(d->x * w);
        int sy = (int)(d->y * h);
        int sl = (int)(d->length * h);
        if (sl < 1) sl = 1;

        for (int j = 0; j < sl; j++) {
            int py = sy + j;
            if (py >= 0 && py < h && sx >= 0 && sx < w) {
                uint8_t base = framebuffer[py * w + sx];
                framebuffer[py * w + sx] = (uint8_t)(base * (1.0f - d->alpha) + drop_color * d->alpha);
            }
        }
    }
}

void v2_weather_set_wind(M11_V2_WeatherState* state, float x) {
    state->wind_x = x;
}

/* ══════════════════════════════════════════════════════════════════════
 * V2.2 Weather FX — Atmospheric effects per dungeon zone
 *
 * Dungeon zones can have weather overlays:
 *   - Rain (outdoor/cave entrance areas)
 *   - Fog (deep dungeon)
 *   - Embers (fire areas)
 *   - Dripping water (wet caves)
 * ══════════════════════════════════════════════════════════════════════ */

typedef enum {
    V22_WEATHER_NONE,
    V22_WEATHER_RAIN,
    V22_WEATHER_FOG,
    V22_WEATHER_EMBERS,
    V22_WEATHER_DRIP,
} V22_WeatherType;

static V22_WeatherType g_current_weather = V22_WEATHER_NONE;
static float g_weather_intensity = 0.5f;

void v22_weather_set(V22_WeatherType type, float intensity) {
    g_current_weather = type;
    g_weather_intensity = intensity > 1.0f ? 1.0f : (intensity < 0.0f ? 0.0f : intensity);
}

V22_WeatherType v22_weather_current(void) { return g_current_weather; }
float v22_weather_intensity(void) { return g_weather_intensity; }

