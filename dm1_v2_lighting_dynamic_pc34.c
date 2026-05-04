#include "dm1_v2_lighting_dynamic_pc34.h"
#include <math.h>
#include <string.h>
#include <stdlib.h>

static M11_V2_LightSource g_sources[M11_V2_LIGHT_MAX_SOURCES];
static int g_source_count = 0;
static M11_V2_LightMap g_light_map;

void v2_light_init(void) {
    memset(g_light_map.r, 0, sizeof(g_light_map.r));
    memset(g_light_map.g, 0, sizeof(g_light_map.g));
    memset(g_light_map.b, 0, sizeof(g_light_map.b));
    g_source_count = 0;
}

int v2_light_add_source(float x, float y, float radius, uint8_t intensity,
                         uint8_t r, uint8_t g, uint8_t b) {
    int idx;
    if (g_source_count >= M11_V2_LIGHT_MAX_SOURCES) return -1;
    idx = g_source_count++;
    g_sources[idx].x = x;
    g_sources[idx].y = y;
    g_sources[idx].radius = radius;
    g_sources[idx].intensity = intensity;
    g_sources[idx].color_r = r;
    g_sources[idx].color_g = g;
    g_sources[idx].color_b = b;
    g_sources[idx].flicker_phase = (float)(rand() % 1000) / 1000.0f;
    return idx;
}

void v2_light_remove_source(int idx) {
    if (idx < 0 || idx >= g_source_count) return;
    g_sources[idx] = g_sources[g_source_count - 1];
    g_source_count--;
}

void v2_light_compute_map(void) {
    int x, y, i;
    for (y = 0; y < M11_V2_LIGHT_MAP_SIZE; y++) {
        for (x = 0; x < M11_V2_LIGHT_MAP_SIZE; x++) {
            float total_r = 0.0f, total_g = 0.0f, total_b = 0.0f;
            for (i = 0; i < g_source_count; i++) {
                float dx = (float)x - g_sources[i].x;
                float dy = (float)y - g_sources[i].y;
                float dist = sqrtf(dx * dx + dy * dy);
                if (dist < g_sources[i].radius) {
                    float falloff = 1.0f - (dist / g_sources[i].radius);
                    float intensity_f;
                    falloff = falloff * falloff;
                    intensity_f = (float)g_sources[i].intensity / 255.0f;
                    total_r += (float)g_sources[i].color_r * intensity_f * falloff;
                    total_g += (float)g_sources[i].color_g * intensity_f * falloff;
                    total_b += (float)g_sources[i].color_b * intensity_f * falloff;
                }
            }
            g_light_map.r[y][x] = (uint8_t)fminf(255.0f, total_r);
            g_light_map.g[y][x] = (uint8_t)fminf(255.0f, total_g);
            g_light_map.b[y][x] = (uint8_t)fminf(255.0f, total_b);
        }
    }
}

void v2_light_get_tile(int x, int y, uint8_t *r, uint8_t *g, uint8_t *b) {
    if (x < 0 || x >= M11_V2_LIGHT_MAP_SIZE || y < 0 || y >= M11_V2_LIGHT_MAP_SIZE) {
        *r = *g = *b = 0;
        return;
    }
    *r = g_light_map.r[y][x];
    *g = g_light_map.g[y][x];
    *b = g_light_map.b[y][x];
}

void v2_light_update_flicker(float dt) {
    int i;
    for (i = 0; i < g_source_count; i++) {
        g_sources[i].flicker_phase += dt * 2.0f;
        g_sources[i].intensity = (uint8_t)((float)g_sources[i].intensity *
            (0.8f + 0.2f * sinf(g_sources[i].flicker_phase)));
    }
}
